#include <iostream>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

using namespace std;

#define NUMBER_OF_CELL 10
#define NUMBER_OF_CUSTOMER 100
typedef int pthread_mutex_t;
typedef int sem_t;

class Customer;

// srand(static_cast<unsigned_int>time(NULL))
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);
#define RANDOM_TIME dis(gen)
// #define RANDOM_TIME ((((double)rand())/RAND_MAX)

int customer_served[NUMBER_OF_CUSTOMER];

queue<Customer> enter_the_factory_queue;
queue<Customer> out_of_the_factory_queue;
queue<Customer> payment_queue;

// int factory_cells[NUMBER_OF_CELL];

pthread_mutex_t factory_lock, console_lock;
pthread_mutex_t cell_lock[NUMBER_OF_CELL-1];
sem_t cell_share_sem_empty[NUMBER_OF_CELL-1];
sem_t cell_share_sem_full[NUMBER_OF_CELL-1];
sem_t enter_sem;

Customer customer_in_cell[NUMBER_OF_CELL-1];

// sem_t factory_full,factory_empty;

class Customer
{
public:
    int cycle_repairing_time[NUMBER_OF_CELL];
    int leave_time;
    int cusomemr_no;

    Customer(int no) : customer_no(no)
    {
        for (int i = 0; i < NUMBER_OF_CELL; i++)
        {
            /* code */
            cycle_repairing_time[i] = RANDOM_TIME
        }
        leave_time = RANDOM_TIME
    }
}

int
main()
{
    mutexs_init();
    int ret = 0;

    Customer customers[NUMBER_OF_CUSTOMER];
    for (int i = 0; i < NUMBER_OF_CUSTOMER; i++)
    {
        customers[i] = Customer(i);
        enter_the_factory_queue.push(customers[i]);
        sem_post(&enter_sem);
    }

    pthread_t cells_thread[NUMBER_OF_CELL];
    int temp[NUMBER_OF_CELL];

    for (int i = 0; i < NUMBER_OF_CELL; i++)
    {
        temp[i] = i + 1;
        ret |= pthread_create(&cells_thread[i], NULL, cell_worker, (void *)&temp[i])
    }

    if (ret != 0)
    {
        cout << "Error Creating Thread" << endl;
    }

    mutexs_destroy();
    return 0;
}

void *cell_worker(void *no)
{
    int worker_no = *((int *)no);

    if (worker_no == 0)
    {
        if (customer_served[worker_no] == NUMBER_OF_CUSTOMER)
        {
            pthread_exit(NULL);
        }
        sem_wait(&enter_sem);

        pthread_mutex_lock(&factory_lock);

        Customer customer = enter_the_factory_queue.front();
        enter_the_factory_queue.pop();

        pthread_mutex_unlock(&factory_lock);

        pthread_mutex_lock(&console_lock);
        cout << "Customer " << customer.customer_no << " has entered Cell : " << worker_no << endl;
        pthread_mutex_unlock(&console_lock);

        sleep(customer.cycle_repairing_time[worker_no]);

        pthread_mutex_lock(&console_lock);
        cout << "Customer " << customer.customer_no << " has been served Cell " << worker_no << " in " << customer.cycle_repairing_time[worker_no] << " seconds" << endl;
        pthread_mutex_unlock(&console_lock);
        


        //acts as producer        
        sem_wait(&cell_share_sem_empty[worker_no]);

        pthread_mutex_lock(&cell_lock[worker_no]);
        customer_in_cell[worker_no] = customer;
        pthread_mutex_unlock(&cell_lock[worker_no])

        sem_post(&cell_share_sem_full[worker_no]);

        customer_served[worker_no] += 1;
    }

    else if(worker_no<NUMBER_OF_CELL-1){

    }

    else if (worker_no == NUMBER_OF_CELL - 1)
    {
        if (customer_served[worker_no] == NUMBER_OF_CUSTOMER)
        {
            pthread_exit(NULL);
        }

        //acts as consumer
        sem_wait(&cell_share_sem_full[worker_no - 1]);

        pthread_mutex_lock(&cell_lock[worker_no -1]);
        Customer customer = customer_in_cell[worker_no -1 ];
        pthread_mutex_unlock(&cell_lock[worker_no -1 ]);

        sem_post(&cell_share_sem_empty[worker_no - 1]);
    }
}

int mutexs_init()
{
    int ret = 0;
    ret = pthread_mutex_init(&factory_lock, NULL);
    ret |= pthread_mutex_init(&console_lock, NULL);

    for (int i = 0; i < NUMBER_OF_CELL-1; i++)
    {
        ret |= pthread_mutex_init(&cell_lock[i], NULL);
    }
    ret |= sem_init(&enter_sem, 0, 0);
    for (int i = 0; i < NUMBER_OF_CELL-1; i++)
    {
        ret |= sem_init(&cell_share_sem_empty[i], 0, 1);
    }

    if (ret != 0)
    {
        cout << "Error initializing semaphores" << endl;
    }
    return ret;
}

int mutexs_destroy()
{
    int ret = 0;
    ret = pthread_mutex_destroy(&factory_lock);
    for (int i = 0; i < NUMBER_OF_CELL-1; i++)
    {
        ret |= pthread_mutex_destroy(&cell_lock[i]);
    }

    ret |= sem_destroy(&eneter_sem);
    for (int i = 0; i < NUMBER_OF_CELL-1; i++)
    {
        ret |= sem_destroy(&cell_share_sem_empty[i]);
    }
    if (ret != 0)
    {
        cout << "Error destroying semaphores" << endl;
    }
    return ret;
}
