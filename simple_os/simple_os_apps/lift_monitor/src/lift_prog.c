#include "lift.h"

/* Simple_OS include */ 
#include <simple_os.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

/* unistd is needed for usleep and sleep */ 
#include <unistd.h>

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 

/* stack for user_task */ 
stack_item User_Stack[STACK_SIZE];

/* stack for lift_task */ 
stack_item Lift_Stack[STACK_SIZE]; 

/* Stacks for passenger tasks */
stack_item Passenger_Stack[MAX_N_PERSONS][STACK_SIZE];

/* the lift to be initialised */
lift_type mainlift;



/* random_level: computes a randomly chosen level */
int random_level(void)
{
    /* return random number between 0 and N_FLOORS-1 */
    return rand() % N_FLOORS;
}


/* ======== tasks ======== */


/* The shall be one task for each person. All person tasks shall be implemented by the same C function, called passenger_task. */
void passenger_task(void)
{
	//printf("blaba\n");

	int id;
	int length;
	int send_task_id;
	
	int current;
	int from;
	int to;

    /* receive id */ 
    si_message_receive((char *) &id, &length, &send_task_id);
	
	current = random_level();	

	while (1)
    {
    	from = current;
    	to = random_level();
    	
    	
    	pthread_mutex_lock(&mainlift->mutex);
        enter_floor(mainlift,id, current);//spawna
    	pthread_mutex_unlock(&mainlift->mutex);

    	printf("Passenger %d starting journey from %d to %d.\n", id, from, to);
    	
    	lift_travel(mainlift, id, from, to);//res
    	current = to;

    	printf("Passenger %d arrived at %d.\n", id, current);
    	
    	pthread_mutex_lock(&mainlift->mutex);

        leave_floor(mainlift, id, current);//ta bort från våning

    	pthread_mutex_unlock(&mainlift->mutex);
    	
    	usleep(1000*TIME_TO_NEW_JOURNEY);
    
    }
}

/* There shall be one task, called lift_task, for the lift.  */
void lift_task(void)
{

	int next;
	int dirchange;
	
	draw_lift(mainlift);

    while (1)
    {
	
	lift_next_floor(mainlift, &next, &dirchange);
	lift_move(mainlift, next, dirchange);
	lift_has_arrived(mainlift);
	
	
    }
}

/*
There shall be one task, called user_task, which receives commands from the graphical user interface. This task shall create new person tasks. A new person task shall be created when the text string new is received. The task user_task shall also contain functionality for termination of the program. The program shall be terminated when the text string exit is received. 
*/
void user_task(void)
{
    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

	int n_persons = 0;
	
	/* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is a set time message */ 
        if (strncmp(message, "new", 3) == 0)
        {
		printf("blaba!!!!\n");
			if (n_persons == MAX_N_PERSONS)
			{
				si_ui_show_error("Failure to comply: Overpopulation!");
			} else {
			
				int id = n_persons++;
				si_task_create(passenger_task, &Passenger_Stack[id][STACK_SIZE-1], 17);
			
				/* send id message to created task */ 
				si_message_send((char *) &id, sizeof(int), id_to_task_id(id)); 
			}
        }
        /* check if it is an exit message */ 
        else if (strcmp(message, "exit") == 0)
        {
            exit(0); 
        }
        /* not a legal message */ 
        else 
        {
            si_ui_show_error("unexpected message type"); 
        }
    }
}



/* ======== main ======== */
 
int main(void)
{
    /* initialise kernel */ 
    si_kernel_init(); 
	
	/* initialise message handling */ 
    si_message_init(); 
	
	/* set up random number generator */
	srand(12345);
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

    /* initialise variables */         
    mainlift=lift_create();

    /* create tasks */ 

    si_task_create(lift_task, &Lift_Stack[STACK_SIZE-1], 20); 

    si_task_create(user_task, &User_Stack[STACK_SIZE-1], 15);
 
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
