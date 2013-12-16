#ifndef DRAW_H 
#define DRAW_H

#include "lift.h"

/* fig_begin draw_lift_h */ 
/* draw_lift: draws the lift, including the lift building, 
   waiting persons, and travelling passengers. It also prints 
   the person id for each waiting person, and prints id and 
   destination floor for each passenger */ 
void draw_lift(lift_data_type* lift);

/* fig_end draw_lift_h */ 
void draw_init(void);


void insert_log(lift_data_type *lift, char *str);
void insert_log_with_val(lift_data_type *lift, char *msg, int val)

#endif

/*
int log_i;
char log_msgs[LOG_SIZE][LOG_MSG_LEN];


log_i = 0;

for (i = 0; i < LOG_SIZE; i++) 
{
    lift->log_msgs[i][0] = '\0'; 
}

*/
