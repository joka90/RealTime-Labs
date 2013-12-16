#include "lift.h"

/* Simple_OS include */ 
//#include <simple_os.h>

/* drawing module */ 
#include "draw.h"

/* standard includes */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

/* unistd is needed for usleep and sleep */ 
#include <unistd.h>

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

    /* the lift is not moving */ 
    lift->moving = 0; 

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

    /* initialise semaphore and event variable */
    pthread_mutex_init(&lift->mutex, NULL); 
    pthread_cond_init(&lift->change, &lift->mutex); 

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
void lift_next_floor(lift_type lift, int *next_floor, int *change_direction)
{
   
    /* reserve lift */ 
    pthread_mutex_lock(&lift->mutex);
	
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
	
	/* release lift */ 
    pthread_mutex_unlock(&lift->mutex); 
}

void lift_move(lift_type lift, int next_floor, int change_direction)
{
    /* reserve lift */ 
    pthread_mutex_lock(&lift->mutex); 

    /* the lift is moving */ 
    lift->moving = 1; 
        
    /* release lift */ 
    pthread_mutex_unlock(&lift->mutex); 
        
    /* it takes two seconds to move to the next floor */ 
    usleep(1000*TIME_BETWEEN_FLOORS); 
        
    /* reserve lift */ 
    pthread_mutex_lock(&lift->mutex); 
        
    /* the lift is not moving */ 
    lift->moving = 0; 

    /* the lift has arrived at next_floor */ 
    lift->floor = next_floor; 

    /* check if direction shall be changed */ 
    if (change_direction)
    {
        lift->up = !lift->up; 
    }

    /* draw, since a change has occurred */ 
    draw_lift(lift); 

    /* release lift */ 
    pthread_mutex_unlock(&lift->mutex); 
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

/* MONITOR function lift_has_arrived: shall be called by the lift task
   when the lift has arrived at the next floor. This function indicates
   to other tasks that the lift has arrived, and then waits until the lift
   shall move again. */
void lift_has_arrived(lift_type lift)
{
	
	/* Kolla om nån ska på eller av */
	if (n_passengers_on_floor(lift, lift->floor) || (n_passengers_to_floor(lift, lift->floor)))
		{
			/* Berätta för alla att hissen är på ny våning */
			pthread_cond_broadcast(&lift->change);
	
			/* Vänta i så fall ett tag på våningen */
			usleep(1000*TIME_ON_FLOOR);
		}
}

/* --- functions related to lift task END --- */


/* --- functions related to person task START --- */

/* passenger_wait_for_lift: returns non-zero if the passenger shall
   wait for the lift, otherwise returns zero */
static int passenger_wait_for_lift(lift_type lift, int wait_floor)
{
    int waiting_ready =
        /* the lift is not moving */ 
        !lift->moving && 
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

/* leave_floor: makes a person with id id at enter_floor leave 
   enter_floor */ 
static void leave_lift(lift_type lift, int id)
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


/* enter_floor: makes a person with id id stand at floor floor */ 
static void enter_lift(lift_type lift, int id, int to_floor)
{
    int i; 
    int lift_index; 
    int found; 

    /* stand at floor */ 
    found = 0; 
    for (i = 0; i < MAX_N_PERSONS && !found; i++)
    {
        if (lift->passengers_in_lift[i].id == NO_ID)
        {
            found = 1; 
            lift_index = i; 
        }
    }
        
    if (!found)
    {
        lift_panic("cannot enter lift"); 
    }

    /* enter floor at index floor_index */ 
    lift->passengers_in_lift[lift_index].id = id; 
    lift->passengers_in_lift[lift_index].to_floor = to_floor; 
}

/* MONITOR function lift_travel: performs a journey with the lift
   starting at from_floor, and ending at to_floor */ 
void lift_travel(lift_type lift, int id, int from_floor, int to_floor)
{
    int boarded = 0;
    int arrived = 0;

    
    while (!boarded)
    {
        pthread_mutex_lock(&lift->mutex);
        pthread_cond_wait(&lift->change,&lift->mutex);
                    printf("Passenger %d  from %d to %d.\n", id, from_floor, to_floor);
        if (!passenger_wait_for_lift(lift, from_floor))
        {
            leave_floor(lift, id, from_floor);
            enter_lift(lift, id, to_floor);
            boarded = 1;
        }
        pthread_mutex_unlock(&lift->mutex);
    }
    
    while (!arrived)
    {
        pthread_mutex_lock(&lift->mutex);
        pthread_cond_wait(&lift->change,&lift->mutex);
        
                    printf("Passenger %d  from %d to %d.\n", id, from_floor, to_floor);
        if (lift->floor == to_floor)
        {
            leave_lift(lift, id);
            enter_floor(lift, id, to_floor);

            
            arrived = 1;
        }
        pthread_mutex_unlock(&lift->mutex);        
    }
}

/* TASK_ID_FIRST_PERSON is the task_id of the first person. It assumes creation of 

1. idle task (created by si_kernel_init), task_id 0
2. lift task, task_id 1
3. user task, task_id 2

*/ 
#define TASK_ID_FIRST_PERSON 3

/* id_to_task_id: convert id to task_id */ 
int id_to_task_id(int id)
{
    return id + TASK_ID_FIRST_PERSON; 
}

/* --- functions related to person task END --- */

