#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<queue>

#define VANILLA 1
#define CHOCOLATE 2


using namespace std;

class node
{
	public:
	int cake_type;
	int number;
	
	node(int a,int b)
	{
		cake_type=a;
		number=b;
	}
};

//semaphore to control sleep and wake up
sem_t empty1,empty2,empty3;
sem_t full1,full2,full3;


queue<node> q1;
queue<node> q2;
queue<node> q3;

pthread_mutex_t Console_lock;
pthread_mutex_t Vanilla_lock;
pthread_mutex_t Chocolate_lock;
pthread_mutex_t cake_lock;


void init_semaphore()
{
	sem_init(&empty1,0,5);
	sem_init(&empty2,0,5);
	sem_init(&empty3,0,5);
	sem_init(&full1,0,0);
	sem_init(&full2,0,0);
	sem_init(&full3,0,0);
	pthread_mutex_init(&Console_lock,0);
	pthread_mutex_init(&Vanilla_lock,0);
	pthread_mutex_init(&Chocolate_lock,0);
	pthread_mutex_init(&cake_lock,0);
}

void * BakeVanilla(void * arg)
{
	for(int i=0;;i++)
	{
		sem_wait(&empty1);
		
		pthread_mutex_lock(&Console_lock);
		printf("Chef Y producing %d number vanilla cake and putting it into queue 1.\n",i+1);
		pthread_mutex_unlock(&Console_lock);
		
		pthread_mutex_lock(&cake_lock);
		node n=node(VANILLA,i+1);
		q1.push(n);
		pthread_mutex_unlock(&cake_lock);
		
		sleep(1);
		
		sem_post(&full1);
			

		
	}
	
	
}
void * BakeChocolate(void * arg)
{
	for(int i=0;;i++)
	{
		sem_wait(&empty1);
	
		pthread_mutex_lock(&Console_lock);
		printf("Chef X producing %d number chocolate cake and putting it into queue 1.\n",i+1);
		pthread_mutex_unlock(&Console_lock);
		
		pthread_mutex_lock(&cake_lock);
		node n=node(CHOCOLATE,i+1);
		q1.push(n);
		pthread_mutex_unlock(&cake_lock);
		
		sleep(1);
		
		sem_post(&full1);
	}
}


void * Decorate(void * arg)
{
	for(int i=0;;i++)
	{
		sem_wait(&full1);
	
		pthread_mutex_lock(&Console_lock);
		printf("Chef Z picking up a cake from queue 1 for decorating.\n");
		pthread_mutex_unlock(&Console_lock);
		
		
		
		pthread_mutex_lock(&cake_lock);
		node n=q1.front();
		q1.pop();
		pthread_mutex_unlock(&cake_lock);
		
		sleep(1);
		
		sem_post(&empty1);
		if(n.cake_type==CHOCOLATE)
		{

			
			sem_wait(&empty3);
			
			pthread_mutex_lock(&Console_lock);
			printf("Chef Z decorating %d number chocolate cake and putting it into queue 3.\n",n.number);
			pthread_mutex_unlock(&Console_lock);
			
			pthread_mutex_lock(&Chocolate_lock);
			q3.push(n);
			pthread_mutex_unlock(&Chocolate_lock);
			
			sleep(1);
			
			sem_post(&full3);
		}
		else if(n.cake_type==VANILLA)
		{

			sem_wait(&empty2);
			
			pthread_mutex_lock(&Console_lock);
			printf("Chef Z decorating %d number vanilla cake and putting it into queue 2.\n",n.number);
			pthread_mutex_unlock(&Console_lock);
			
			pthread_mutex_lock(&Vanilla_lock);
			q2.push(n);
			pthread_mutex_unlock(&Vanilla_lock);
			
			sleep(1);
			
			sem_post(&full2);
		}
		
		
	}
	
}

void * ServeChocolateCake(void * arg)
{
	for(int i=0;;i++)
	{
		sem_wait(&full3);
		pthread_mutex_lock(&Console_lock);
		printf("Waiter 1 taking a chocolate cake from queue 3.\n");
		pthread_mutex_unlock(&Console_lock);
		
		pthread_mutex_lock(&Chocolate_lock);
		node n=q3.front();
		q3.pop();
		pthread_mutex_unlock(&Chocolate_lock);
		
		sleep(1);
					
		pthread_mutex_lock(&Console_lock);
		printf("Waiter 1 serving %d number chocolate cake.\n",n.number);
		pthread_mutex_unlock(&Console_lock);
		
		sem_post(&empty3);
	}
	
}

void * ServeVanillaCake(void * arg)
{
	for(int i=0;;i++)
	{
		sem_wait(&full2);
		pthread_mutex_lock(&Console_lock);
		printf("Waiter 2 taking a vanilla cake from queue 2.\n");
		pthread_mutex_unlock(&Console_lock);
		
		pthread_mutex_lock(&Vanilla_lock);
		node n=q2.front();
		q2.pop();
		pthread_mutex_unlock(&Vanilla_lock);
		
		sleep(1);
					
		pthread_mutex_lock(&Console_lock);
		printf("Waiter 2 serving %d number vanilla cake.\n",n.number);
		pthread_mutex_unlock(&Console_lock);
		
		sem_post(&empty2);
	}	
	
}

int main()
{
	pthread_t chefX;
	pthread_t chefY;
	pthread_t chefZ;
	pthread_t waiter1;
	pthread_t waiter2;
	
	init_semaphore();
	
	pthread_create(&chefX,NULL,BakeChocolate,NULL);
	pthread_create(&chefY,NULL,BakeVanilla,NULL);
	pthread_create(&chefZ,NULL,Decorate,NULL);
	pthread_create(&waiter1,NULL,ServeChocolateCake,NULL);
	pthread_create(&waiter2,NULL,ServeVanillaCake,NULL);
	pthread_exit(NULL);
	return 0;
	
	
}
