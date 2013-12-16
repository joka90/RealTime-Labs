#include "lift.h"

/* Simple_OS include */ 
#include <simple_os.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 

/* stack for user_task */ 
stack_item User_Stack[STACK_SIZE];

/* stack for lift_task */ 
stack_item Lift_Stack[STACK_SIZE]; 

//TODO fix passenger_task

/* Stacks for passenger tasks */
stack_item Passenger_Stack[MAX_N_PERSONS][STACK_SIZE];


si_semaphore mutex_passenger_alive_list; 
int passenger_alive_list[MAX_N_PERSONS];

/* the lift to be initialised */
lift_data_type mainliftStatic;
lift_data_type *mainlift;

void create_passenger(int id, int priority);

int randVari;

/* random_level: computes a randomly chosen level */
int random_level(int id)
{
    /* return random number between 0 and N_FLOORS-1 */
    return ((mainlift->floor)*id+randVari+56) % N_FLOORS;
}



/* ======== tasks ======== */


/* The shall be one task for each person. All person tasks shall be implemented by the same C function, called passenger_task. */
void passenger_task(void)
{
	////printf("blaba\n");

	int id;
	int length;
	int send_task_id;
	
	int current;
	int from;
	int to;

    int n_travels;
    n_travels=0;

    /* receive id */ 
    si_message_receive((char *) &id, &length, &send_task_id);
	
	current = random_level(id);	

	while (n_travels < 2)
    {
	
    	from = current;
    	to = random_level(id);
    	
    	
    	si_sem_wait(&mainlift->mutex);
        enter_floor(mainlift,id, current);//spawna
    	si_sem_signal(&mainlift->mutex);

    	//printf("Passenger %d starting journey from %d to %d.\n", id, from, to);
    	
    	lift_travel(mainlift, id, from, to);//res
    	current = to;

    	//printf("Passenger %d arrived at %d.\n", id, current);
    	
    	si_sem_wait(&mainlift->mutex);

        leave_floor(mainlift, id, current);//ta bort från våning

    	si_sem_signal(&mainlift->mutex);
    	
		n_travels++;
    	si_wait_n_ms(TIME_TO_NEW_JOURNEY);
    
    }
    si_sem_wait(&mutex_passenger_alive_list);
    
    passenger_alive_list[id]=0;
    
    si_sem_signal(&mutex_passenger_alive_list);
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
	    randVari++;
    }
}

/* create_passenger: create a person task */ 
void create_passenger(int id, int priority) 
{
    int task_id; 

	/*printf("Created task...\n");*/
    int i;
    int id_index;
    int found;
    found =0;
    si_sem_wait(&mutex_passenger_alive_list);
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if(passenger_alive_list[i]==0)
        {
            found=1;
            id_index=i;
            passenger_alive_list[i]=1;
        }
    }
    si_sem_signal(&mutex_passenger_alive_list);
    /* create task */ 
    task_id = si_task_create_task_id(
        passenger_task, &Passenger_Stack[id_index][STACK_SIZE - 1], priority);

    /* send id message to created task */ 
    si_message_send((char *) &id_index, sizeof(int), task_id);

    //printf("Sent ID %d to task with id %d...\n",id_to_task_id(id),task_id);
}

/*
There shall be one task, called user_task, which receives commands from the graphical user interface. This task shall create new person tasks. A new person task shall be created when the text string new is received. The task user_task shall also contain functionality for termination of the program. The program shall be terminated when the text string exit is received. 
*/
void user_task(void)
{
    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

	int n_persons;
	n_persons = 0;
	
	/* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is a set time message */ 
        if (si_string_compare(message, "new") == 0)
        {
		//printf("blaba!!!!\n");
		    int i;
		    n_persons = 0;
            for (i = 0; i < MAX_N_PERSONS; i++)
            {
                if(passenger_alive_list[i]!=0)
                {
                    n_persons++;
                }
            }
			if (n_persons == MAX_N_PERSONS)
			{
				si_ui_show_error("Failure to comply: Overpopulation!");
			} else {
			
				int id = n_persons++;
				create_passenger(id, 17);
			}
        }
        /* check if it is an exit message */ 
        else if (si_string_compare(message, "exit") == 0)
        {
            return;//exit(0); 
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
	//srand(12345);
    randVari=0;
    
    /* initialise UI channel */ 
    si_ui_init(); 
    draw_init();

    int i;
    for (i = 0; i < MAX_N_PERSONS; i++)
    {
        passenger_alive_list[i]=0;
    }
    
    
    si_sem_init(&mutex_passenger_alive_list, 1); 
    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

    mainlift=&mainliftStatic;
    /* initialise variables */         
    lift_create(mainlift);

    /* create tasks */ 

    si_task_create(lift_task, &Lift_Stack[STACK_SIZE-1], 20); 

    si_task_create(user_task, &User_Stack[STACK_SIZE-1], 15);
 
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
