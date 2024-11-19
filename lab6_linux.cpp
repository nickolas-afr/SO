#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

bool isPrime(int n){
    if(n<=1) return false;
    for(int i=2; i*i <=n; ++i){
        if(n%i == 0) return false;
    }
    return true;
}

void findPrimes(int start, int end, int writePipe){
    std::vector<int> primes;
    for(int i= start; i<=end; ++i){
        if(isPrime(i)){
            primes.push_back(i);
        }
    }
    write(writePipe, primes.data(), primes.size() * sizeof(int));
    close(writePipe);
}

int main(){
    const int numProcesses = 10;
    const int rangeSize = 1000;
    int pipes[numProcesses][2];
    pid_t pids[numProcesses];

    for(int i=0; i<numProcesses; ++i){
        int start = i * rangeSize + 1;
        int end = (i + 1) * rangeSize;
        pipe(pipes[i]);
        pids[i] = fork();
        if(pids[i] == 0){
            close(pipes[i][0]);
            findPrimes(start, end, pipes[i][1]);
            return 0;
        } else{
            close(pipes[i][1]);
        }
    }

    for(int i=0; i<numProcesses; ++i){
        waitpid(pids[i], NULL, 0);
    }

    for(int i=0; i<numProcesses; ++i){
        std::vector<int> primes(rangeSize);
        read(pipes[i][0], primes.data(), rangeSize * sizeof(int));
        for(int prime : primes){
            if(prime != 0){
                std::cout << prime << " ";
            }
        }
        close(pipes[i][0]);
    }

    return 0;
}