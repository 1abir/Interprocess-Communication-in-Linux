#include <iostream>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <random>

using namespace std;

#define NUMBER_OF_CELL 10
#define NUMBER_OF_CUSTOMER 20
// typedef int pthread_mutex_t;
// typedef int sem_t;

void *cell_worker(void *no);
void *payment_booth(void *booth_no);
int mutexs_init();
int mutexs_destroy();

// Random Number
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);
#define RANDOM_TIME dis(gen)

class Customer
{
public:
    double cycle_repairing_time[NUMBER_OF_CELL];
    double leave_time;
    int customer_no;

    Customer(int no) : customer_no(no)
    {
        for (int i = 0; i < NUMBER_OF_CELL; i++)
        {
            cycle_repairing_time[i] = RANDOM_TIME;
        }
        leave_time = RANDOM_TIME;
    }
};

//Enter the factory
queue<Customer> enter_the_factory_queue;
sem_t enter_sem;
pthread_mutex_t factory_lock;

//Repairing Cells
int customer_served[NUMBER_OF_CUSTOMER];
Customer *customer_in_cell = (Customer *)malloc(sizeof(Customer) * (NUMBER_OF_CUSTOMER - 1));
pthread_mutex_t cell_lock[NUMBER_OF_CELL - 1];
sem_t cell_share_sem_empty[NUMBER_OF_CELL - 1];
sem_t cell_share_sem_full[NUMBER_OF_CELL - 1];
sem_t cell_share_sem_acknowledgement[NUMBER_OF_CELL - 1];

//Go to payment after repairing
queue<Customer> payment_queue;
sem_t payment_queue_sem;
pthread_mutex_t payment_queue_lock;

//Exit the factory
queue<Customer> exit_factory_queue;
sem_t exit_sem;
pthread_mutex_t exit_queue_lock;
// int factory_cells[NUMBER_OF_CELL];

pthread_mutex_t console_lock;

// sem_t factory_full,factory_empty;

int main()
{
    cout << "initializing" << endl;
    mutexs_init();
    int ret = 0;

    // Customer *customers = new Customer[NUMBER_OF_CUSTOMER];
    for (int i = 0; i < NUMBER_OF_CUSTOMER; i++)
    {
        // customers[i] = Customer(i);
        // enter_the_factory_queue.push(customers[i]);
        enter_the_factory_queue.push(Customer(i));
        sem_post(&enter_sem);
    }
    cout << "Pushing complete" << endl;
    pthread_t cells_thread[NUMBER_OF_CELL];
    int temp[NUMBER_OF_CELL];

    for (int i = 0; i < NUMBER_OF_CELL; i++)
    {
        temp[i] = i;
        ret |= pthread_create(&cells_thread[i], NULL, cell_worker, (void *)&temp[i]);
    }
    pthread_mutex_lock(&console_lock);
    cout << "Thread Creation complete" << endl;
    pthread_mutex_unlock(&console_lock);
    if (ret != 0)
    {
        pthread_mutex_lock(&console_lock);
        cout << "Error Creating Thread" << endl;
        pthread_mutex_unlock(&console_lock);
    }

    ret = 0;
    for (int i = 0; i < NUMBER_OF_CELL; i++)
    {
        ret |= pthread_join(cells_thread[i], NULL);
    }
    if (ret != 0)
    {
        pthread_mutex_lock(&console_lock);
        cout << "Error Joining Thread" << endl;
        pthread_mutex_unlock(&console_lock);
    }

    // delete[] customers;

    mutexs_destroy();
    return 0;
}

void *cell_worker(void *no)
{
    int worker_no = *((int *)no);
    pthread_mutex_lock(&console_lock);
    cout << worker_no << " has started working" << endl;
    pthread_mutex_unlock(&console_lock);

    if (worker_no == 0)
    {
        while (1)
        {
            if (customer_served[worker_no] == NUMBER_OF_CUSTOMER)
            {
                pthread_mutex_lock(&console_lock);
                cout << worker_no << " has stopped working" << endl;
                pthread_mutex_unlock(&console_lock);
                pthread_exit(NULL);
            }
            sem_wait(&enter_sem);

            pthread_mutex_lock(&factory_lock);

            Customer customer = enter_the_factory_queue.front();
            enter_the_factory_queue.pop();

            pthread_mutex_unlock(&factory_lock);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << "  started taking service from serviceman : " << worker_no << endl;
            pthread_mutex_unlock(&console_lock);

            sleep(customer.cycle_repairing_time[worker_no]);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << " finished taking service from serviceman " << worker_no << " in " << customer.cycle_repairing_time[worker_no] << " seconds" << endl;
            pthread_mutex_unlock(&console_lock);

            //acts as producer
            sem_wait(&cell_share_sem_empty[worker_no]);

            pthread_mutex_lock(&cell_lock[worker_no]);
            customer_in_cell[worker_no] = customer;
            pthread_mutex_unlock(&cell_lock[worker_no]);

            sem_post(&cell_share_sem_full[worker_no]);

            sem_wait(&cell_share_sem_acknowledgement[worker_no]);

            customer_served[worker_no] += 1;
        }
    }

    else if (worker_no < NUMBER_OF_CELL - 1)
    {
        while (1)
        {
            if (customer_served[worker_no] == NUMBER_OF_CUSTOMER)
            {
                pthread_mutex_lock(&console_lock);
                cout << worker_no << " has stopped working" << endl;
                pthread_mutex_unlock(&console_lock);
                pthread_exit(NULL);
            }

            //acts as consumer
            sem_wait(&cell_share_sem_full[worker_no - 1]);

            pthread_mutex_lock(&cell_lock[worker_no - 1]);
            Customer customer = customer_in_cell[worker_no - 1];
            pthread_mutex_unlock(&cell_lock[worker_no - 1]);

            sem_post(&cell_share_sem_empty[worker_no - 1]);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << "  started taking service from serviceman : " << worker_no << endl;
            pthread_mutex_unlock(&console_lock);

            sem_post(&cell_share_sem_acknowledgement[worker_no - 1]);

            sleep(customer.cycle_repairing_time[worker_no]);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << " finished taking service from serviceman " << worker_no << " in " << customer.cycle_repairing_time[worker_no] << " seconds" << endl;
            pthread_mutex_unlock(&console_lock);

            //acts as producer
            sem_wait(&cell_share_sem_empty[worker_no]);

            pthread_mutex_lock(&cell_lock[worker_no]);
            customer_in_cell[worker_no] = customer;
            pthread_mutex_unlock(&cell_lock[worker_no]);

            sem_post(&cell_share_sem_full[worker_no]);

            sem_wait(&cell_share_sem_acknowledgement[worker_no]);

            customer_served[worker_no] += 1;
        }
    }

    else if (worker_no == NUMBER_OF_CELL - 1)
    {
        while (1)
        {
            if (customer_served[worker_no] == NUMBER_OF_CUSTOMER)
            {
                pthread_mutex_lock(&console_lock);
                cout << worker_no << " has stopped working" << endl;
                pthread_mutex_unlock(&console_lock);
                pthread_exit(NULL);
            }

            //acts as consumer
            sem_wait(&cell_share_sem_full[worker_no - 1]);

            pthread_mutex_lock(&cell_lock[worker_no - 1]);
            Customer customer = customer_in_cell[worker_no - 1];
            pthread_mutex_unlock(&cell_lock[worker_no - 1]);

            sem_post(&cell_share_sem_empty[worker_no - 1]);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << "  started taking  service from serviceman : " << worker_no << endl;
            pthread_mutex_unlock(&console_lock);

            sem_post(&cell_share_sem_acknowledgement[worker_no - 1]);

            sleep(customer.cycle_repairing_time[worker_no]);

            pthread_mutex_lock(&console_lock);
            cout << "Customer " << customer.customer_no << " finished taking service from serviceman " << worker_no << " in " << customer.cycle_repairing_time[worker_no] << " seconds" << endl;
            pthread_mutex_unlock(&console_lock);

            pthread_mutex_lock(&payment_queue_lock);
            payment_queue.push(customer);
            pthread_mutex_unlock(&payment_queue_lock);

            sem_post(&payment_queue_sem);
            customer_served[worker_no] += 1;
        }
    }
}

void *payment_booth(void *booth_no)
{
}

int mutexs_init()
{
    int ret = 0;
    ret = pthread_mutex_init(&factory_lock, NULL);
    ret |= pthread_mutex_init(&console_lock, NULL);
    ret |= pthread_mutex_init(&payment_queue_lock, NULL);
    ret |= pthread_mutex_init(&exit_queue_lock, NULL);

    for (int i = 0; i < NUMBER_OF_CELL - 1; i++)
    {
        ret |= pthread_mutex_init(&cell_lock[i], NULL);
    }
    ret |= sem_init(&enter_sem, 0, 0);
    for (int i = 0; i < NUMBER_OF_CELL - 1; i++)
    {
        ret |= sem_init(&cell_share_sem_empty[i], 0, 1);
        ret |= sem_init(&cell_share_sem_full[i], 0, 0);
        ret |= sem_init(&cell_share_sem_acknowledgement[i], 0, 0);
    }

    ret |= sem_init(&payment_queue_sem, 0, 0);
    ret |= sem_init(&exit_sem, 0, 0);

    if (ret != 0)
    {
        pthread_mutex_lock(&console_lock);
        cout << "Error initializing semaphores" << endl;
        pthread_mutex_lock(&console_lock);
    }
    return ret;
}

int mutexs_destroy()
{
    int ret = 0;
    ret = pthread_mutex_destroy(&factory_lock);
    ret |= pthread_mutex_destroy(&console_lock);
    ret |= pthread_mutex_destroy(&payment_queue_lock);
    ret |= pthread_mutex_destroy(&exit_queue_lock);

    for (int i = 0; i < NUMBER_OF_CELL - 1; i++)
    {
        ret |= pthread_mutex_destroy(&cell_lock[i]);
    }

    ret |= sem_destroy(&enter_sem);
    for (int i = 0; i < NUMBER_OF_CELL - 1; i++)
    {
        ret |= sem_destroy(&cell_share_sem_empty[i]);
        ret |= sem_destroy(&cell_share_sem_full[i]);
        ret |= sem_destroy(&cell_share_sem_acknowledgement[i]);
    }

    if (ret != 0)
    {
        pthread_mutex_lock(&console_lock);
        cout << "Error destroying semaphores" << endl;
        pthread_mutex_lock(&console_lock);
    }
    return ret;
}
