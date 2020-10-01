
#include<iostream>
#include<pthread.h>
#include<queue>
#include<semaphore.h>
#include<stdio.h>
#include<string>
#include<unistd.h>

#define chocolateCake 1
#define vanillaCake 2

using namespace std;

//chefX and chefY makes cake and push them in cakeQ, chefZ pops it
//chefZ pushes cakes in vanillaQ and chocolate, waiter 1 & 2 pops from them
queue<int> vanillaQ, chocolateQ;
queue<int> cakeQ;

sem_t cakeFull, cakeEmpty;
sem_t vanillaFull, chocolateFull, vanillaEmpty, chocolateEmpty;

pthread_mutex_t lock, consoleLock;

//initialize the semaphore for three queues and two locks
void init_semaphore()
{
    sem_init(&cakeFull, 0, 0);
    sem_init(&cakeEmpty, 0, 5);

    sem_init(&vanillaFull, 0, 0);
    sem_init(&vanillaEmpty, 0, 5);

    sem_init(&chocolateFull, 0, 0);
    sem_init(&chocolateEmpty, 0, 5);

    pthread_mutex_init(&lock, 0);
    pthread_mutex_init(&consoleLock, 0);
}

void pfs(string str)
{
    pthread_mutex_lock(&consoleLock);
    printf("%s", str.c_str());
    pthread_mutex_unlock(&consoleLock);
}

void *chef_x(void *arg)
{
    while(1)
    {
        //emptiness decreases by one -> if not zero that means we can push into it
        pfs("\nchef x is in a tea-break\n");
        sem_wait(&cakeEmpty);   

        //queue locked
        pfs("chef x is in a tea-break\n");
        pthread_mutex_lock(&lock);

        pfs("chef x in critical region\n");
        
        cakeQ.push(chocolateCake);
        
        pthread_mutex_lock(&consoleLock);
        printf("chef x produced a chocolate-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
        pthread_mutex_unlock(&consoleLock);

        //unlock queue 
        pthread_mutex_unlock(&lock);

        //amount of cake increases by one
        sem_post(&cakeFull);

        sleep((rand() % 5) + 1);
    }
}

void *chef_y(void *arg)
{
    while(1)
    {
        //emptiness decreases by one
        //if not zero that means we can push into it
        pfs("\nchef y is in a tea-break\n");
        sem_wait(&cakeEmpty);   

         //queue locked
        pfs("chef y is in a tea-break\n");
        pthread_mutex_lock(&lock); 

        pfs("chef y in critical region\n");
        
        cakeQ.push(vanillaCake);
        
        pthread_mutex_lock(&consoleLock);
        printf("chef y produced a vanilla-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
        pthread_mutex_unlock(&consoleLock);

        //unlock queue 
        pthread_mutex_unlock(&lock);

        //amount of cake increases by one
        sem_post(&cakeFull);

        sleep((rand() % 5) + 1);
    }
}

void *chef_z(void *arg)
{
    while(1)
    {
        //wait for the queue to be full
        pfs("\nchef z is in a cofee-break\n");
        sem_wait(&cakeFull);

        pfs("chef z in critical range\n");

        if(cakeQ.front() == chocolateCake)
        {
            //wait until you can push into the chocolate-cake queue
            sem_wait(&chocolateEmpty);
        
            cakeQ.pop();
            chocolateQ.push(chocolateCake);

            pthread_mutex_lock(&consoleLock);
            printf("chef z decorated a chocolate-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
            pthread_mutex_unlock(&consoleLock);

            //chocolate queue can be popped now
            sem_post(&chocolateFull);
        }

        else
        {
            //wait until you can push into the vanilla-cake queue
            sem_wait(&vanillaEmpty);
            
            cakeQ.pop();
            vanillaQ.push(vanillaCake);

            pthread_mutex_lock(&consoleLock);
            printf("chef z decorated a vanilla-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
            pthread_mutex_unlock(&consoleLock);

            //vanilla queue can be popped now
            sem_post(&vanillaFull);
        }

        //another slot in the queue is free
        sem_post(&cakeEmpty);
        sleep((rand() % 5) + 1);
    }
}

//delivers chocolate cake
void *waiter_1(void *arg)
{
   while(1)
   {
        //wait till the queue has at-least one
        pfs("waiter-1 is in a cofee-break\n");
        sem_wait(&chocolateFull);

        pfs("waiter-1 is in critical region\n");

        chocolateQ.pop();
       
        pthread_mutex_lock(&consoleLock);
        printf("waiter-1 delivered a chocolate-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
        pthread_mutex_unlock(&consoleLock);

        //a slot in the queue is free 
        sem_post(&chocolateEmpty);

        sleep(1);
   }
}

void *waiter_2(void *arg)
{
    while(1)
    {
        //wait till the queue has at-least one
        pfs("waiter-2 is in a cofee-break");
        sem_wait(&vanillaFull);

        pfs("waiter-2 is in critical region\n");

        vanillaQ.pop();
        
        pthread_mutex_lock(&consoleLock);
        printf("waiter-2 delivered a vanilla-cake. cake: %d. chocolate-cake: %d. vanilla-cake: %d\n\n", (int)cakeQ.size(), (int)chocolateQ.size(), (int)vanillaQ.size());
        pthread_mutex_unlock(&consoleLock);

        //a slot in the queue is free 
        sem_post(&vanillaEmpty);

        sleep(1);
    }
}

int main()
{
    srand(time(NULL));
    init_semaphore();

    pthread_t chefX, chefY, chefZ;
    pthread_t waiter1, waiter2;

    pthread_create(&chefX, NULL, chef_x, NULL);
    pthread_create(&chefY, NULL, chef_y, NULL);
    pthread_create(&chefZ, NULL, chef_z, NULL);
    pthread_create(&waiter1, NULL, waiter_1, NULL);
    pthread_create(&waiter2, NULL, waiter_2, NULL);

    while(true);

    exit(EXIT_SUCCESS);
}