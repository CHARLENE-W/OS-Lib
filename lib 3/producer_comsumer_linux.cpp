#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include<semaphore.h>
#define myKey 6666
struct BUFFER //缓冲区
{
    int s[4];
    int head;
    int trail;
    bool IsEmpty;
};
void show(struct BUFFER *tmp)
{

    printf("当前缓冲区：");
    for (int i = tmp->head; i != tmp->trail;)
    {
        printf("%d ", tmp->s[i]);
        i++;
        if (i == 4)
            i = i % 4;
    }
    printf("\n");
}
void P(int semID, int n)
{ //第n个信号量，第一个为0

    struct sembuf tmp;

    tmp.sem_num = n;
    tmp.sem_op = -1;
    tmp.sem_flg = 0; //访问标志

    semop(semID, &tmp, 1);
}
void V(int semID, int n)
{ //第n个信号量，第一个为0
    struct sembuf tmp;
    tmp.sem_num = n;
    tmp.sem_op = 1;
    tmp.sem_flg = 0; //访问标志   SEM_UNDO，程序结束，信号量为semop调用前的值。
    semop(semID, &tmp, 1);
}
void consumer(int shmID, int semID)
{
    for (int i = 0; i < 3; i++)
    {
        pid_t pid = fork();
        if (pid < 0) //error
        {
            printf("consumer fork error!\n");
            exit(0);
        }
        else if (pid == 0)
        {

            void *shm = shmat(shmID, 0, 0);
            if (shm == (void *)-1)
            {
                printf("producer shmat error!\n");
                exit(0);
            }
            struct BUFFER *tmp = (struct BUFFER *)shm;
            for (int j = 0; j < 4; j++)
            {

                int random_time = rand() % 5;
                sleep(random_time);
           
                P(semID, 1);
                P(semID, 2);

                time_t t;
                time(&t);
              //   printf("进行中： empty=%d,full=%d,mutex=%d\n",semctl(semID,0,GETVAL),semctl(semID,1,GETVAL),semctl(semID,2,GETVAL));
                printf("%s ", ctime(&t));
                printf("消费者 %d 从缓存区取出  %d号 货物\n", i, tmp->s[tmp->head]);

                tmp->head = (tmp->head + 1) % 4;
                if (tmp->head == tmp->trail)
                    tmp->IsEmpty = true;
                else
                    tmp->IsEmpty = false;

                if (tmp->IsEmpty)
                    printf("缓存区已空\n");
                else
                    show(tmp);
                printf("\n");

                V(semID, 0);
                V(semID, 2);

                
            }
            shmdt(tmp);
            exit(0); // 类似于 UnmapViewOfFile(lpMap);
        }
    }
}
void producer(int shmID, int semID)
{
    for (int i = 0; i < 2; i++)
    {
        pid_t pid = fork();
        if (pid < 0) //error
        {
            printf("producer fork error!\n");
            exit(0);
        }
        else if (pid == 0)
        {
            void *shm = shmat(shmID, 0, 0);
            if (shm == (void *)-1)
            {
                printf("producer shmat error!\n");
                exit(0);
            }

            struct BUFFER *tmp = (struct BUFFER *)shm;
            for (int j = 0; j < 6; j++)
            {
                int random_time = rand() % 5;
                sleep(random_time);

                P(semID, 0);
                P(semID, 2);

                int num_pro = rand() % 100;
                tmp->s[tmp->trail] = num_pro;
                tmp->trail = (tmp->trail + 1) % 4;
                tmp->IsEmpty = false;
                time_t t;
                time(&t);
              //  printf("empty=%d,full=%d,mutex=%d\n",semctl(semID,0,GETVAL),semctl(semID,1,GETVAL),semctl(semID,2,GETVAL));
                printf("%s ", ctime(&t));
                printf("生产者 %d 放入缓存区  %d号 货物\n", i, num_pro);

                if ((tmp->trail + 1) % 4 == tmp->head)
                    printf("缓存区已满\n");
                show(tmp);
                printf("\n");

              
                V(semID, 1);
                  V(semID, 2);
            }
            shmdt(tmp); 
            exit(0);// 类似于 UnmapViewOfFile(lpMap);
        }
    }
}

int main(int argc, char *argv[])
{
    int semID = semget(myKey, 3, IPC_CREAT|0666); //创建信号量
    if (semID < 0)
    {
        printf("create semaphore error\n");
        exit(0);
    }
 
    semctl(semID, 0, SETVAL, 3);                                //empty
    semctl(semID, 1, SETVAL, 0);                                //full
    semctl(semID, 2, SETVAL, 1);                                //mutex
    int shmID = shmget(8888, sizeof(struct BUFFER), IPC_CREAT); //申请共享内存区
    if (shmID < 0)
    {
        printf("shmget error!\n");
        exit(0);
    }
    void *shm = shmat(shmID, 0, 0); //将共享段附加到申请通信的进程空间
    if (shm == (void *)-1)
    {
        printf("shmat error!\n");
        perror("shmat");
        exit(0);
    }
    struct BUFFER *tmp = (struct BUFFER *)shm;
    tmp->head = 0;
    tmp->trail = 0;
    tmp->IsEmpty = true;

    producer(shmID, semID);
    consumer(shmID, semID);

    while (wait(0)!=-1){
         
    };
   // printf("+++++++++++++");
        
    shmdt(tmp);
    semctl(semID, IPC_RMID, 0); //删除信号量
    shmctl(shmID, IPC_RMID, 0); //删除共享段
    return 0;
}
