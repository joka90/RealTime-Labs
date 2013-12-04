/* includes */

#include <simple_os.h>

#include <stdio.h>

/* size of stacks */
#define STACK_SIZE 5000

/* stack for task creator  task */ 
stack_item Task_Creator_Stack[STACK_SIZE]; 

/* max number of time-sharing tasks */ 
#define MAX_N_TIME_SHARED 10

/* stacks for time-sharing tasks */ 
stack_item Time_Shared_Stack[MAX_N_TIME_SHARED][STACK_SIZE]; 

/* stack för real-time task */ 
stack_item Real_Time_Stack[STACK_SIZE]; 

/* functionality for task stop */ 

si_semaphore Task_Stop_Sem; 

void init_task_stop(void)
{
    si_sem_init(&Task_Stop_Sem, 0); 
}

void stop_task(void)
{
    si_sem_wait(&Task_Stop_Sem); 
}

/* function that performs work */     
void do_work(void)
{
    long max_count = 400000000; 
    long i; int x, y; 
    for (i = 0; i < max_count; i++)
    {
       x = 2; 
       y = 3*x - 12; 
       x = 4*y; 
    }
}

/* tasks */

void time_shared_task(void)
{
    int task_number; 
    int length; 
    int send_task_id; 
  
    /* receive task number */ 
    si_message_receive((char *) &task_number, &length, &send_task_id); 

    while (1) 
    {
        printf("---- TS task %d is alive ----\n", task_number); 
        do_work(); 
    }
}

void task_creator_task(void)
{
    int n_tasks = 4; 
    int task_number; 
    int first_task_prio = TIME_SHARED_PRIORITY_BASE; 
    int task_id; 
    
    for (task_number = 0; task_number < n_tasks; task_number++)
    {
        /* fig_begin si_task_create_task_id */ 
        task_id = si_task_create_task_id(
            time_shared_task, 
            &Time_Shared_Stack[task_number][STACK_SIZE-1], 
            first_task_prio + task_number); 
        /* fig_end si_task_create_task_id */ 

        si_message_send((char *) &task_number, sizeof(int), task_id); 
    }
    /* make this task stop */ 
    stop_task(); 
}


void real_time_task(void)
{
    while(1)
    {
        console_put_string("This is a real-time task\n"); 
        si_wait_n_ms(5000); 
    }
}

/* main function */ 
int main(void)
{
    /* initialise kernel */ 
    si_kernel_init(); 

    /* initialise message handling */ 
    si_message_init(); 

    init_task_stop(); 

    /* create tasks */
    si_task_create(task_creator_task,
       &Task_Creator_Stack[STACK_SIZE-1], TIME_SHARED_PRIORITY_BASE-2);
    si_task_create(real_time_task,
       &Real_Time_Stack[STACK_SIZE-1], TIME_SHARED_PRIORITY_BASE-1);
   
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
