#include "lift.h"

/* Simple_OS include */ 
#include <simple_os.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 

/* stack for user_task */ 
stack_item user_Stack[STACK_SIZE];

/* stack for lift_task */ 
stack_item lift_Stack[STACK_SIZE]; 

//TODO fix passenger_task

/* Stacks for passenger tasks */
stack_item Passenger_Stack[MAX_N_PERSONS][STACK_SIZE];

/* the lift to be initialised */
lift_type mainlift;


/* The shall be one task for each person. All person tasks shall be implemented by the same C function, called passenger_task. */
void passenger_task(void)
{

}

/* There shall be one task, called lift_task, for the lift.  */
void lift_task(void)
{

}

/*
There shall be one task, called user_task, which receives commands from the graphical user interface. This task shall create new person tasks. A new person task shall be created when the text string new is received. The task user_task shall also contain functionality for termination of the program. The program shall be terminated when the text string exit is received. 
*/
void user_task(void)
{

}

/* main */ 
int main(void)
{
    /* initialise kernel */ 
    si_kernel_init(); 
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* initialise display */
    display_init();

    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

    /* initialise variables */         
    mainlift=lift_create();

    /* create tasks */ 

    si_task_create(lift_task, &Clock_Stack[STACK_SIZE-1], 20); 

    si_task_create(user_task, &Set_Stack[STACK_SIZE-1], 15);
 
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
