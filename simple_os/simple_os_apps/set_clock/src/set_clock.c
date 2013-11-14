/* A clock program 

Two tasks are used, clock_task for updating a clock, and 
set_task for setting the clock 

*/ 

/* Simple_OS include */ 
#include <simple_os.h>

/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 

/* stack for clock_task */ 
stack_item Clock_Stack[STACK_SIZE]; 

/* stack for set_task */ 
stack_item Set_Stack[STACK_SIZE]; 

/* clock variables */ 

/* semaphore for protecting the current time */ 
si_semaphore Mutex; 

/* the current time */ 
int Hours, Minutes, Seconds; 

/* functions operating on the clock */ 

/* init_clock: initialises the clock variables */ 
void init_clock(void)
{
    /* set starting time */ 
    Hours = 12; 
    Minutes = 59; 
    Seconds = 50; 

    /* initialise semaphore */ 
    si_sem_init(&Mutex, 1); 
}

/* increment_time: increments the current time with 
   one second */ 
void increment_time(void)
{
    /* reserve clock variables */ 
    si_sem_wait(&Mutex); 

    /* increment time */ 
    Seconds++; 
    if (Seconds > 59)
    {
        Seconds = 0; 
        Minutes++; 
        if (Minutes > 59)
        {
            Minutes = 0; 
            Hours++; 
            if (Hours > 12)
            {
                Hours = 1; 
            }
        }
    }

    /* release clock variables */ 
    si_sem_signal(&Mutex); 
}

/* set_time: set time to hours, minutes and seconds */ 
void set_time(int hours, int minutes, int seconds)
{
    /* reserve clock variables */ 
    si_sem_wait(&Mutex); 

    /* assign values */
    Hours = hours; 
    Minutes = minutes;
    Seconds = seconds; 
    
    /* release clock variables */ 
    si_sem_signal(&Mutex); 
}

/* get_time: read time from common clock variables */ 
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */ 
    si_sem_wait(&Mutex); 

    /* read values */ 
    *hours = Hours; 
    *minutes = Minutes; 
    *seconds = Seconds;
        
    /* release clock variables */ 
    si_sem_signal(&Mutex); 
}

/* make_display_message: create string for printout, given a time expressed 
   in hours, minutes and seconds */ 
void make_display_message(char display_message[], int hours, int minutes, int seconds) 
{
    display_message[0] = hours / 10 + '0'; 
    display_message[1] = hours % 10 + '0'; 
    display_message[2] = ':'; 
    display_message[3] = minutes / 10 + '0'; 
    display_message[4] = minutes % 10 + '0'; 
    display_message[5] = ':'; 
    display_message[6] = seconds / 10 + '0'; 
    display_message[7] = seconds % 10 + '0'; 
    display_message[8] = '\0'; 
}

/* time_from_set_message: extract time from set message from user interface */ 
void time_from_set_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set:%d:%d:%d",hours, minutes, seconds); 
}


/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */ 
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 && 
        seconds >= 0 && seconds <= 59; 
}

/* display_time: displays hours, minutes and seconds by 
   sending them to the user interface */ 
void display_time(int hours, int minutes, int seconds)
{
    char display_message[20]; 

    char draw_message[100]; 

    /* convert hours, minutes and seconds to display format */ 
    make_display_message(display_message, hours, minutes, seconds); 

    sprintf(draw_message, "Time: %s", display_message); 

    /* write to user interface */ 
    si_ui_draw_begin(); 
    si_ui_draw_string("send \"set:hh:mm:ss\" to set time", 90, 20); 
    si_ui_draw_string(draw_message, 145, 55);
    si_ui_draw_end(); 
}

/* tasks */ 

/* clock_task: clock task */ 
void clock_task(void) 
{
    /* local copies of the current time */ 
    int hours, minutes, seconds; 

    /* infinite loop */ 
    while (1)
    {
        /* read and display current time */ 
        get_time(&hours, &minutes, &seconds); 
        display_time(hours, minutes, seconds); 

        /* increment time */ 
        increment_time(); 

        /* wait one/2 second */ 
        si_wait_n_ms(500); 
    }
}

/* set_task: reads messages from the user interface, and 
   sets the clock, or exits the program */ 
void set_task(void)
{
    /* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    /* time read from user interface */ 
    int hours, minutes, seconds; 

    /* set GUI size */ 
    si_ui_set_size(400, 200); 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is a set message */ 
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_message(message, &hours, &minutes, &seconds); 
            if (time_ok(hours, minutes, seconds))
            {
                set_time(hours, minutes, seconds); 
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds"); 
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

/* main */ 
int main(void)
{
    /* initialise kernel */ 
    si_kernel_init(); 
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* initialise variables */         
    init_clock(); 

    /* create tasks */ 

    si_task_create(clock_task, &Clock_Stack[STACK_SIZE-1], 20); 

    si_task_create(set_task, &Set_Stack[STACK_SIZE-1], 15); 

    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}

