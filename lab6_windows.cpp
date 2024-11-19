#include <iostream>
#include <vector>
#include <windows.h>

bool isPrime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return true;
}

DWORD WINAPI findPrimes(LPVOID param) {
    int* range = (int*)param;
    int start = range[0];
    int end = range[1];
    HANDLE hPipe = (HANDLE)range[2];
    std::vector<int> primes;

    for (int i = start; i <= end; ++i) {
        if (isPrime(i)) {
            primes.push_back(i);
        }
    }

    DWORD written;
    WriteFile(hPipe, primes.data(), primes.size() * sizeof(int), &written, NULL);
    CloseHandle(hPipe);
    return 0;
}

int main() {
    const int numProcesses = 10;
    const int rangeSize = 1000;
    HANDLE hPipes[numProcesses][2];
    HANDLE hThreads[numProcesses];
    int ranges[numProcesses][3];

    for (int i = 0; i < numProcesses; ++i) {
        int start = i * rangeSize + 1;
        int end = (i + 1) * rangeSize;
        CreatePipe(&hPipes[i][0], &hPipes[i][1], NULL, 0);
        ranges[i][0] = start;
        ranges[i][1] = end;
        ranges[i][2] = (int)hPipes[i][1];
        hThreads[i] = CreateThread(NULL, 0, findPrimes, ranges[i], 0, NULL);
    }

    for (int i = 0; i < numProcesses; ++i) {
        WaitForSingleObject(hThreads[i], INFINITE);
        CloseHandle(hThreads[i]);
    }

    for (int i = 0; i < numProcesses; ++i) {
        std::vector<int> primes(rangeSize);
        DWORD read;
        ReadFile(hPipes[i][0], primes.data(), rangeSize * sizeof(int), &read, NULL);
        for (int prime : primes) {
            if (prime != 0) {
                std::cout << prime << " ";
            }
        }
        CloseHandle(hPipes[i][0]);
    }

    return 0;
}
