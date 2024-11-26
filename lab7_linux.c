#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Tipurile de fir
#define WHITE 0
#define BLACK 1

// Variabile globale pentru sincronizare
pthread_mutex_t resource_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t queue_sem;

int active_white = 0; // număr de fire albe active
int active_black = 0; // număr de fire negre active
int waiting_white = 0; // fire albe în așteptare
int waiting_black = 0; // fire negre în așteptare

void *thread_function(void *arg) {
    int type = *(int *)arg;

    if (type == WHITE) {
        pthread_mutex_lock(&resource_mutex);
        waiting_white++;
        // Așteaptă până când nu mai sunt fire negre active
        while (active_black > 0) {
            pthread_mutex_unlock(&resource_mutex);
            sem_wait(&queue_sem);
            pthread_mutex_lock(&resource_mutex);
        }
        waiting_white--;
        active_white++;
        pthread_mutex_unlock(&resource_mutex);

        // Utilizează resursa
        printf("White thread using resource.\n");
        sleep(1);

        pthread_mutex_lock(&resource_mutex);
        active_white--;
        // Eliberează resursa pentru altele
        if (active_white == 0 && waiting_black > 0) {
            for (int i = 0; i < waiting_black; i++) {
                sem_post(&queue_sem);
            }
        }
        pthread_mutex_unlock(&resource_mutex);

    } else if (type == BLACK) {
        pthread_mutex_lock(&resource_mutex);
        waiting_black++;
        // Așteaptă până când nu mai sunt fire albe active
        while (active_white > 0) {
            pthread_mutex_unlock(&resource_mutex);
            sem_wait(&queue_sem);
            pthread_mutex_lock(&resource_mutex);
        }
        waiting_black--;
        active_black++;
        pthread_mutex_unlock(&resource_mutex);

        // Utilizează resursa
        printf("Black thread using resource.\n");
        sleep(1);

        pthread_mutex_lock(&resource_mutex);
        active_black--;
        // Eliberează resursa pentru altele
        if (active_black == 0 && waiting_white > 0) {
            for (int i = 0; i < waiting_white; i++) {
                sem_post(&queue_sem);
            }
        }
        pthread_mutex_unlock(&resource_mutex);
    }

    return NULL;
}

int main() {
    pthread_t threads[10];
    int thread_types[10] = {WHITE, BLACK, WHITE, BLACK, WHITE, WHITE, BLACK, BLACK, WHITE, BLACK};

    // Inițializează semaforul
    sem_init(&queue_sem, 0, 0);

    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_types[i]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    // Distruge semaforul
    sem_destroy(&queue_sem);

    return 0;
}
