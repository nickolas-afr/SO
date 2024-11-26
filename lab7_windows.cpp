#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <process.h> // Pentru _beginthreadex

// Tipurile de fir
#define WHITE 0
#define BLACK 1

// Variabile globale pentru sincronizare
HANDLE resource_mutex;
HANDLE queue_semaphore;

int active_white = 0; // număr de fire albe active
int active_black = 0; // număr de fire negre active
int waiting_white = 0; // fire albe în așteptare
int waiting_black = 0; // fire negre în așteptare

unsigned __stdcall thread_function(void* arg) {
    int type = *(int*)arg;

    if (type == WHITE) {
        WaitForSingleObject(resource_mutex, INFINITE);
        waiting_white++;
        // Așteaptă până când nu mai sunt fire negre active
        while (active_black > 0) {
            ReleaseMutex(resource_mutex);
            WaitForSingleObject(queue_semaphore, INFINITE);
            WaitForSingleObject(resource_mutex, INFINITE);
        }
        waiting_white--;
        active_white++;
        ReleaseMutex(resource_mutex);

        // Utilizează resursa
        printf("White thread using resource.\n");
        Sleep(1000);

        WaitForSingleObject(resource_mutex, INFINITE);
        active_white--;
        // Eliberează resursa pentru altele
        if (active_white == 0 && waiting_black > 0) {
            for (int i = 0; i < waiting_black; i++) {
                ReleaseSemaphore(queue_semaphore, 1, NULL);
            }
        }
        ReleaseMutex(resource_mutex);

    }
    else if (type == BLACK) {
        WaitForSingleObject(resource_mutex, INFINITE);
        waiting_black++;
        // Așteaptă până când nu mai sunt fire albe active
        while (active_white > 0) {
            ReleaseMutex(resource_mutex);
            WaitForSingleObject(queue_semaphore, INFINITE);
            WaitForSingleObject(resource_mutex, INFINITE);
        }
        waiting_black--;
        active_black++;
        ReleaseMutex(resource_mutex);

        // Utilizează resursa
        printf("Black thread using resource.\n");
        Sleep(1000);

        WaitForSingleObject(resource_mutex, INFINITE);
        active_black--;
        // Eliberează resursa pentru altele
        if (active_black == 0 && waiting_white > 0) {
            for (int i = 0; i < waiting_white; i++) {
                ReleaseSemaphore(queue_semaphore, 1, NULL);
            }
        }
        ReleaseMutex(resource_mutex);
    }

    return 0;
}

int main() {
    HANDLE threads[10];
    int thread_types[10] = { WHITE, BLACK, WHITE, BLACK, WHITE, WHITE, BLACK, BLACK, WHITE, BLACK };

    // Inițializează mutex-ul și semaforul
    resource_mutex = CreateMutex(NULL, FALSE, NULL);
    queue_semaphore = CreateSemaphore(NULL, 0, 10, NULL);

    if (resource_mutex == NULL || queue_semaphore == NULL) {
        printf("Failed to create synchronization objects.\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, thread_function, &thread_types[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Failed to create thread %d.\n", i);
            return 1;
        }
    }

    for (int i = 0; i < 10; i++) {
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
    }

    // Distruge obiectele de sincronizare
    CloseHandle(resource_mutex);
    CloseHandle(queue_semaphore);

    return 0;
}
