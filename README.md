# Interprocess-Communication-in-Linux

```
Imagine there exists a cycle repairing shop. This shop has some weird rules for the cycles coming
here to take service. All the cycles that want servicing from this shop must be present at the time
of opening of the shop. There are S number of servicemen in the shop. They work sequentially one
after another; i.e., serviceman 1 first inspects a cycle, then the cycle is passed to serviceman 2 and
so on. After serviceman 1 finishes inspecting a cycle and this cycle is passed to serviceman 2;
another cycle from the remaining cycles starts taking service from serviceman 1 and so on.
After a cycle’s servicing is finished (i.e., it took service from all S servicemen), owner of the cycle
must go to a room for the payment. However, that room can only contain C number of customers.
After a cyclist is done with his payment, he will go out of the shop. However, he must return
through the path that is being used by the servicemen for servicing the cycles. But, this path
is so narrow that only one directional movement is possible; i.e., when any cycle is being serviced
by any one of the servicemen, no cyclist can return through this path. Similarly, while a cyclist is
returning through this path for departure, no new cycle can enter this path for starting its service.
Your task is to simulate the mentioned scenario.
```
Service rooms Payment room^
Cycles entering
through this
path
Only one way
movement
These two paths are
independent and can contain
any number of cycles.
No time needed for cycle to
pass through this path.


## Specifications

1. You have to use C or C++ as language and Linux as operating system.
2. It is clear that the processes here are not independent. Rather, they are cooperating one
    another. For example, a cycle can’t start taking service while another cycle is being
    serviced by serviceman 1. Therefore, some sort of Inter Process Communication is needed
    here.
3. It is not desired that a cyclist has completed his payment and is waiting for departure for
    too long. So, whenever a cyclist is waiting for departure, you need to temporarily block
    new cycles from entering into service to let that cyclist depart.
4. You need to use Shared Memory paradigm for accomplishing IPC in this offline; i.e.,
    different cyclists will take information (e.g., how many customers are already in the room
    for payment) from a shared memory which is accessible by all the cyclists. As this memory
    needs to be accessed from more than one process, you need to synchronize those using
    semaphores and locks.
5. Each serviceman takes random amount of time to finish servicing of each cycle.
6. Each cyclist takes random amount of time for finishing the payment.
7. Each cyclist takes random amount of time for departure after payment.



October 2, 11: 1 5PM.


