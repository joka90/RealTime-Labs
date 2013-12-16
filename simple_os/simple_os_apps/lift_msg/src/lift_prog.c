#include "lift.h"
#include "draw.h"
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

/* stack for move_lift_task */ 
stack_item Move_Lift_Stack[STACK_SIZE]; 

/* Stacks for passenger tasks */
stack_item Passenger_Stack[MAX_N_PERSONS][STACK_SIZE];


/* task identities, to be set when 
   the tasks are created and then not modified */ 
int User_Task_Id; 
int Lift_Task_Id; 
int Move_Lift_Task_Id; 
int Lowest_Passenger_Task_Id; 

/* the lift to be initialised */
lift_data_type mainliftStatic;
lift_data_type *mainlift=&mainliftStatic;

void create_passenger(int id, int priority);

/* random_level: computes a randomly chosen level */
int random_level(int id)
{
    /* return random number between 0 and N_FLOORS-1 */
    return ((mainlift->floor)*id) % N_FLOORS;
}


/* ======== tasks ======== */


/* The shall be one task for each person. All person tasks shall be implemented by the same C function, called passenger_task. */
void passenger_task(void)//TODO
{
	//printf("blaba\n");

	int id;
	int length;
	int send_task_id;
	
	int current;
	int from;
	int to;
	
	int arrived;
    int n_travels;
    n_travels=0;

    message_data_type msg;
	

	
	

    /* receive id */ 
    si_message_receive((char *) &id, &length, &send_task_id);//TODO Ska vi skicka via usertask först? Sedan ändra till att ta emot msg

	//printf("Person %d spawned as task %d \n", id, id_to_task_id(id));

	/* message för att be om hisstur */
	message_data_type travel_msg;
	travel_msg.type = TRAVEL_MESSAGE;
	travel_msg.id = id;
	
	current = random_level(id);	

	while (n_travels < 2)
    {
	
	
		arrived = 0;
		from = current;
        to = random_level(id);
		
	    travel_msg.from_floor = from;
    	travel_msg.to_floor = to;
		
		si_message_send((char *) &travel_msg, sizeof(travel_msg), Lift_Task_Id);
        si_ui_show_error("pasengertask wants to travel");
		while(!arrived)
		{
			si_message_receive((char *) &msg, &length, &send_task_id);
			
			if(msg.type == TRAVEL_DONE_MESSAGE)
			{
				arrived = 1;
			}
		}
		
		
		current = to;

		n_travels++;
		si_wait_n_ms(TIME_TO_NEW_JOURNEY);
    }
}


/* There shall be one task, called lift_task, for the lift.  */
void lift_task(void)//TODO
{

	int next;
	int dirchange;

    int send_task_id;
    int length;//DUMMY	
    message_data_type msg;

	draw_lift(mainlift);


    while (1)
    {
        si_message_receive((char *) &msg, &length, &send_task_id);

        if(msg.type == MOVE_MESSAGE)
        {
			lift_next_floor(mainlift, &next, &dirchange);
			lift_move(mainlift, next, dirchange);
			lift_has_arrived(mainlift);
        }
        else if(msg.type == TRAVEL_MESSAGE)
        {
            /* a travel message is sent to the lift task when a 
               person would like to make a lift travel */ 
		    enter_floor(mainlift, msg.id, msg.from_floor, msg.to_floor);
		    si_ui_show_error("Passenger began travel");
        }
		
		draw_lift(mainlift);
    }

}

/*
There shall be one task, called user_task, which receives commands from the graphical user interface. This task shall create new person tasks. A new person task shall be created when the text string new is received. The task user_task shall also contain functionality for termination of the program. The program shall be terminated when the text string exit is received. 
*/
void user_task(void)//TODO
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
		/*printf("Spawning...\n");*/
			if (n_persons == MAX_N_PERSONS)
			{
				si_ui_show_error("Failure to comply: Overpopulation!");
			} else {
			
				int id;
				id=n_persons++;
                create_passenger(id, 17);
			}
        }
        /* check if it is an exit message */ 
        else if (si_string_compare(message, "exit") == 0)
        {
            return; 
        }
        /* not a legal message */ 
        else 
        {
            si_ui_show_error("unexpected message type"); 
        }
    }
}

/* create_passenger: create a person task */ 
void create_passenger(int id, int priority) 
{
    int task_id; 

	/*printf("Created task...\n");*/

    /* create task */ 
    task_id = si_task_create_task_id(
        passenger_task, &Passenger_Stack[id][STACK_SIZE - 1], priority);

    /* send id message to created task */ 
    si_message_send((char *) &id, sizeof(int), task_id);

    //printf("Sent ID %d to task with id %d...\n",id_to_task_id(id),task_id);
}

/* move_lift_task: controls the motion of the lift */ 
void move_lift_task(void)
{
    /* message to be sent to lift task */ 
    message_data_type message; 

    /* it is a move message */ 
    message.type = MOVE_MESSAGE; 

    while (1)
    {
        /* send move message to lift task */ 
        si_message_send((char *) &message, sizeof(message), Lift_Task_Id); 

        /* it takes two seconds to move to the next floor */ 
        si_wait_n_ms(TIME_BETWEEN_FLOORS); 
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
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

    /* initialise variables */         
    lift_init(mainlift);

    draw_init();

    si_task_create(
        lift_task, &Lift_Stack[STACK_SIZE - 1], LIFT_PRIORITY);

    Lift_Task_Id = 1; 

    si_task_create(
        move_lift_task, &Move_Lift_Stack[STACK_SIZE - 1], MOVE_LIFT_PRIORITY);

    Move_Lift_Task_Id = 2; 

    si_task_create(
        user_task, &User_Stack[STACK_SIZE - 1], USER_PRIORITY);

    User_Task_Id = 3; 

    Lowest_Passenger_Task_Id = TASK_ID_FIRST_PERSON;
 
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
