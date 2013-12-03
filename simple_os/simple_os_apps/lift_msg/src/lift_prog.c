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
lift_type mainlift;



/* random_level: computes a randomly chosen level */
int random_level(void)
{
    /* return random number between 0 and N_FLOORS-1 */
    return rand() % N_FLOORS;
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

    message_data_type msg;
	
	/* message för att be om hisstur */
	message_data_type travel_msg;
	travel_msg.type = TRAVEL_MESSAGE;
	travel_msg.id = id;
	
	

    /* receive id */ 
    si_message_receive((char *) &id, &length, &send_task_id);//TODO Ska vi skicka via usertask först? Sedan ändra till att ta emot msg


	
	current = random_level();	

	while (1)
    {
		arrived = 0;
		from = current;
        to = random_level();
		
	    travel_msg.from_floor = from;
    	travel_msg.to_floor = to;
		
		si_message_send((char *) &travel_msg, sizeof(travel_msg), id);

		while(arrived == 0)
		{
			si_message_receive((char *) &msg, &length, &send_task_id);
			
			if(msg.type == TRAVEL_DONE_MESSAGE)
			{
				arrived = 1;
			}
		}
		
		current = to;
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
			/*lift_has_arrived(mainlift);*/
        }
        else if(msg.type == TRAVEL_MESSAGE)
        {
/* a travel message is sent to the lift task when a 
   person would like to make a lift travel */ 
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
				si_task_create(passenger_task, &Passenger_Stack[id][STACK_SIZE-1], 17);//SAMMA som innan
			
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
	srand(12345);
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* set size of GUI window */ 
    si_ui_set_size(670, 700); 

    /* initialise variables */         
    mainlift=lift_create();

    /* create tasks */ 

    /* create tasks */ 

    si_task_create(
        lift_task, &Lift_Stack[STACK_SIZE - 1], LIFT_PRIORITY);

    Lift_Task_Id = 1; 

    si_task_create(
        move_lift_task, &Move_Lift_Stack[STACK_SIZE - 1], MOVE_LIFT_PRIORITY);

    Move_Lift_Task_Id = 2; 

    si_task_create(
        user_task, &User_Stack[STACK_SIZE - 1], USER_PRIORITY);

    User_Task_Id = 3; 

    Lowest_Passenger_Task_Id = 4;
 
    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}
