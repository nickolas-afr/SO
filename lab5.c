#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/shared_memory_example"
#define SEM_NAME "/semaphore_example"
#define SHM_SIZE sizeof(int)

int toss_coin(){
    return rand() % 2;
}

int main(){
    int fd, *shared_num;
    sem_t *sem;

    srand(time(NULL) ^ getpid());

    fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(fd < 0){
        perror("shm_open");
        exit(1);
    }

    if(ftruncate(fd, SHM_SIZE) == -1){
        perror("ftruncate");
        exit(1);
    }

    shared_num = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_num == MAP_FAILED){
        perror("mmap");
        exit(1);
    }

    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED){
        perror("sem_open");
        exit(1);
    }

    while(1){
        sem_wait(sem);

        int current = *shared_num;

        if(current >= 1000){
            sem_post(sem);
            break;
        }

        printf("Process %d read: %d\n", getpid(), current);

        while(toss_coin() == 1){
            (*shared_num)++;
            printf("Process %d writes: %d\n", getpid(), *shared_num);
        }

            sem_post(sem);

            usleep(100000);
    }

        munmap(shared_num, SHM_SIZE);
        close(fd);

        sem_close(sem);

        if(getpid() % 2 == 0){
            shm_unlink(SHM_NAME);
            shm_unlink(SEM_NAME);
        }

        return 0;
}
    
