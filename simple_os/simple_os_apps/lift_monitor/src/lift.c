#include "lift.h"

/* definition of monitor data type for lift */

typedef struct
{
    /* the floor where the lift is positioned */ 
    int floor; 

    /* a flag to indicate if the lift is moving */ 
    int moving; 

    /* variable to indicate if the lift is travelling in the up 
       direction, which is defined as the direction where the 
       floor number is increasing */
    int up;

    /* persons on each floor waiting to enter */ 
    person_data_type persons_to_enter[N_FLOORS][MAX_N_PERSONS];

    /* passengers in the lift */
    person_data_type passengers_in_lift[MAX_N_PASSENGERS];

    /* semaphore for mutual exclusion */
    si_semaphore mutex; 

    /* condition variable, to indicate that something has happend */ 
    si_condvar change; 

} lift_data_type;

typedef lift_data_type* lift_type;

/* data structure for person information */ 
typedef struct
{
    /* identity */ 
    int id; 
    /* destination floor */ 
    int to_floor; 
} person_data_type; 

/* random_level: computes a randomly chosen level */
int random_level(void)
{
    /* return random number between 0 and N_FLOORS-1 */
    return rand() % N_FLOORS;
}

void init_lift(void)
{
    /* use this statement for different random sequences at 
    different program executions, e.g. when the final version 
    of the program is ready */
    srand(time(NULL));
}

