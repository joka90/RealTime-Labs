#include "lift.h"

/* Simple_OS include */ 
#include <simple_os.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* panic function, to be called when fatal errors occur */ 
static void lift_panic(const char message[])
{
    printf("LIFT_PANIC!!! "); 
    printf("%s", message); 
    printf("\n"); 
    exit(0); 
}

/* --- monitor data type for lift and operations for create and delete START --- */

/* lift_create: creates and initialises a variable of type lift_type */
lift_type lift_create(void) 
{
    /* the lift to be initialised */
    lift_type lift;

    /* floor counter */ 
    int floor; 

    /* loop counter */
    int i;

    /* allocate memory */
    lift = (lift_type) malloc(sizeof(lift_data_type));

    /* initialise variables */

    /* initialise floor */
    lift->floor = 0; 
    
    /* set direction of lift travel to up */
    lift->up = 1;

    /* initialise person information */
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            lift->persons_to_enter[floor][i].id = NO_ID; 
            lift->persons_to_enter[floor][i].to_floor = NO_FLOOR; 
        }
    }

    /* initialise passenger information */
    for (i = 0; i < MAX_N_PASSENGERS; i++) 
    {
        lift->passengers_in_lift[i].id = NO_ID; 
        lift->passengers_in_lift[i].to_floor = NO_FLOOR; 
    }


    return lift;
}

/* lift_delete: deallocates memory for lift */
void lift_delete(lift_type lift)
{
    free(lift);
}


/* --- monitor data type for lift and operations for create and delete END --- */


/* --- functions related to lift task START --- */

/* MONITOR function lift_next_floor: computes the floor to which the lift 
   shall travel. The parameter *change_direction indicates if the direction 
   shall be changed */
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)//TODO
{
   	
	if (lift->up)
	{
		if (lift->floor == N_FLOORS-1)
		{
			*change_direction = lift->up;
			*next_floor = lift->floor - 1;
		} else {
			*change_direction = 0;
			*next_floor = lift->floor + 1;
		}
			
	} else {
		if (lift->floor == 0)
		{
			*change_direction = !lift->up;
			*next_floor = lift->floor + 1;
		} else {
			*change_direction = 0;
			*next_floor = lift->floor - 1;
		}
	}
	
}

void lift_move(lift_type lift, int next_floor, int change_direction)//TODO
{

        
    /* it takes two seconds to move to the next floor */ 
    si_wait_n_ms(TIME_BETWEEN_FLOORS); 
        
      

    /* the lift has arrived at next_floor */ 
    lift->floor = next_floor; 

    /* check if direction shall be changed */ 
    if (change_direction)
    {
        lift->up = !lift->up; 
    }

    /* draw, since a change has occurred */ 
    draw_lift(lift); 


}

/* this function is used also by the person tasks */ 
static int n_passengers_in_lift(lift_type lift)
{
    int n_passengers = 0; 
    int i; 
        
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            n_passengers++; 
        }
    }
    return n_passengers; 
}

/* Ger hur många i hissen som ska av på en viss våning */
static int n_passengers_to_floor(lift_type lift, int floor)
{
	int n = 0;
	int i;
	
	for (i=0; i < MAX_N_PASSENGERS; i++)
		{
			if(lift->passengers_in_lift[i].to_floor == floor)
			{
				n++;
			}
		}
		
	return n;
}
		

/* Ger hur många i på en viss våning som vill in */
static int n_passengers_on_floor(lift_type lift, int floor)
{
	int n = 0;
	int i;
	
	for (i=0; i < MAX_N_PASSENGERS; i++)
		{
			if(lift->persons_to_enter[floor][i].id != NO_ID)
			{
				n++;
			}
		}
		
	return n;
}		

/* --- functions related to lift task END --- */


/* --- functions related to person task START --- */

/* passenger_wait_for_lift: returns non-zero if the passenger shall
   wait for the lift, otherwise returns zero */
static int passenger_wait_for_lift(lift_type lift, int wait_floor)
{
    int waiting_ready =
        /* and the lift is at wait_floor */ 
        lift->floor == wait_floor && 
        /* and the lift is not full */ 
        n_passengers_in_lift(lift) < MAX_N_PASSENGERS; 

    return !waiting_ready;
}


/* enter_floor: makes a person with id id stand at floor floor */ 
void enter_floor(
    lift_type lift, int id, int floor)
{
    int i; 
    int floor_index; 
    int found; 

    /* stand at floor */ 
    found = 0; 
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[floor][i].id == NO_ID)
        {
            found = 1; 
            floor_index = i; 
        }
    }
        
    if (!found)
    {
        lift_panic("cannot enter floor"); 
    }

    /* enter floor at index floor_index */ 
    lift->persons_to_enter[floor][floor_index].id = id; 
    lift->persons_to_enter[floor][floor_index].to_floor = NO_FLOOR; 
}

/* leave_floor: makes a person with id id at enter_floor leave 
   enter_floor */ 
void leave_floor(lift_type lift, int id, int enter_floor)
{
    int i;
    int floor_index;
    int found;

    /* leave the floor */
    found = 0;
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->persons_to_enter[enter_floor][i].id == id)
        {
            found = 1;
            floor_index = i;
        }
    }
        
    if (!found)
    {
        lift_panic("cannot leave floor");
    }

    /* leave floor at index floor_index */
    lift->persons_to_enter[enter_floor][floor_index].id = NO_ID;
    lift->persons_to_enter[enter_floor][floor_index].to_floor = NO_FLOOR;
}


void leave_lift(lift_type lift, int id)
{
    int i; 
    int lift_index; 
    int found; 

    /* leave the floor */
    found = 0; 
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->passengers_in_lift[i].id == id)
        {
            found = 1; 
            lift_index = i; 
        }
    }
        
    if (!found)
    {
        lift_panic("cannot leave lift"); 
    }

    lift->passengers_in_lift[lift_index ].id = NO_ID; 
    lift->passengers_in_lift[lift_index ].to_floor = NO_FLOOR; 
}


void enter_lift(lift_type lift, int floor, int max_enterers)
{
    int i; 
    int j;

    int has_entered;
    
    int found_enterer; 


    has_entered = 0;
    
    for (i = 0; i < MAX_N_PASSENGERS && has_entered < max_enterers; i++)
    {
        if (lift->passengers_in_lift[i].id == NO_ID)
        {
        
            found_enterer = 0;
            
            for (j = 0; j < MAX_N_PERSONS && !found_enterer; j++)
            {
                if(lift->persons_to_enter[floor][j].id != NO_ID 
                   && lift->persons_to_enter[floor][j].to_floor != NO_FLOOR)
                {
                    found_enterer = 1; 
                  
                    lift->passengers_in_lift[j].id = lift->persons_to_enter[floor][j].id; 
                    lift->passengers_in_lift[j].to_floor = lift->persons_to_enter[floor][j].to_floor;
                    
                    lift->persons_to_enter[floor][j].id = NO_ID; 
                    lift->persons_to_enter[floor][j].to_floor = NO_FLOOR;
                    
                    has_entered++;
                }
            }           
   
        }
    }
}


void lift_has_arrived(lift_type lift)
{
/*

typedef struct
{
    int floor; 

    int up;

   -> person_data_type persons_to_enter[N_FLOORS][MAX_N_PERSONS];

   -> person_data_type passengers_in_lift[MAX_N_PASSENGERS];

} lift_data_type;

*/
}

/* TASK_ID_FIRST_PERSON is the task_id of the first person. It assumes creation of 

1. idle task (created by si_kernel_init), task_id 0
2. lift task, task_id 1
3. user task, task_id 2

*/ 

/* id_to_task_id: convert id to task_id */ 
int id_to_task_id(int id)
{
    return id + TASK_ID_FIRST_PERSON; 
}

/* n_passengers_to_leave: returns the number of passengers in the 
   lift having the destination floor equal to floor */
int n_passengers_to_leave(lift_type lift, int floor)
{
//TODO
} 

/* n_persons_to_enter: returns the number of persons standing on 
   floor floor */ 
int n_persons_to_enter(lift_type lift, int floor)
{
//TODO
} 

/* lift_is_full: returns nonzero if the lift is full, returns zero 
   otherwise */ 
int lift_is_full(lift_type lift)
{
//TODO
}  

/* get_current_floor: returns the floor on which the lift is positioned */ 
int get_current_floor(lift_type lift)
{
//TODO
} 

/* --- functions related to person task END --- */

