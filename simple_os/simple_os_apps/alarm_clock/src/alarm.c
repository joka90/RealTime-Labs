#include "alarm.h"

/* Simple_OS include */ 
#include <simple_os.h>

/* standard library includes */ 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "display.h"

/* stack size */ 
#define STACK_SIZE 5000 

/* stack declarations */ 

/* stack for clock_task */ 
stack_item Clock_Stack[STACK_SIZE]; 

/* stack for set_task */ 
stack_item Set_Stack[STACK_SIZE]; 

/* time data type */ 
typedef struct 
{
    int hours; 
    int minutes; 
    int seconds; 
} time_data_type; 

/* clock data type */ 
typedef struct
{
    /* the current time */ 
    time_data_type time; 
    /* alarm time */ 
    time_data_type alarm_time; 
    /* alarm enabled flag */ 
    int alarm_enabled; 

    /* semaphore for mutual exclusion */ 
    si_semaphore mutex; 

    /* semaphore for alarm activation */ 
    si_semaphore start_alarm; 

} clock_data_type; 

/* the actual clock */ 
static clock_data_type Clock; 

/* clock_init: initialise clock */ 
void clock_init(void)
{
    /* initialise time and alarm time */ 

    Clock.time.hours = 0; 
    Clock.time.minutes = 0; 
    Clock.time.seconds = 0; 

    Clock.alarm_time.hours = 0; 
    Clock.alarm_time.minutes = 0; 
    Clock.alarm_time.seconds = 0; 
    
    /* alarm is not enabled */ 
    Clock.alarm_enabled = 0; 

    /* initialise semaphores */ 
    si_sem_init(&Clock.mutex, 1); 
    si_sem_init(&Clock.start_alarm, 0); 
}

/* clock_set_time: set current time to hours, minutes and seconds */ 
void clock_set_time(int hours, int minutes, int seconds)
{
    si_sem_wait(&Clock.mutex); 

    Clock.time.hours = hours; 
    Clock.time.minutes = minutes; 
    Clock.time.seconds = seconds; 

    si_sem_signal(&Clock.mutex); 
}

/* clock_set_alarm: set current time to hours, minutes and seconds */ 
void clock_set_alarm(int hours, int minutes, int seconds)
{
    si_sem_wait(&Clock.mutex); 

    Clock.alarm_time.hours = hours; 
    Clock.alarm_time.minutes = minutes; 
    Clock.alarm_time.seconds = seconds; 

    Clock.alarm_enabled = 1;

    display_alarm_time(hours, minutes, seconds);

    si_sem_signal(&Clock.mutex); 
}

/* clock_set_alarm: set current time to hours, minutes and seconds */ 
void clock_reset_alarm(void)
{
    si_sem_wait(&Clock.mutex); 

    Clock.alarm_enabled = 0;

    erase_alarm_time();
    erase_alarm_text();

    si_sem_signal(&Clock.mutex); 
}

/* increment_time: increments the current time with 
   one second */ 
void increment_time(void)
{
    /* reserve clock variables */ 
    si_sem_wait(&Clock.mutex); 

    /* increment time */ 
    Clock.time.seconds++; 
    if (Clock.time.seconds > 59)
    {
        Clock.time.seconds = 0; 
        Clock.time.minutes++; 
        if (Clock.time.minutes > 59)
        {
            Clock.time.minutes = 0; 
            Clock.time.hours++; 
            if (Clock.time.hours > 23)
            {
                Clock.time.hours = 0; 
            }
        }
    }

    /* release clock variables */ 
    si_sem_signal(&Clock.mutex); 
}

/* get_time: read time from common clock variables */ 
void get_time(int *hours, int *minutes, int *seconds)
{
    /* reserve clock variables */ 
    si_sem_wait(&Clock.mutex); 

    /* read values */ 
    *hours = Clock.time.hours; 
    *minutes = Clock.time.minutes; 
    *seconds = Clock.time.seconds;
        
    /* release clock variables */ 
    si_sem_signal(&Clock.mutex); 
}

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

        /* wait one second */ 
        si_wait_n_ms(1000); 
    }
}

/* time_ok: returns nonzero if hours, minutes and seconds represents a valid time */ 
int time_ok(int hours, int minutes, int seconds)
{
    return hours >= 0 && hours <= 23 && minutes >= 0 && minutes <= 59 && 
        seconds >= 0 && seconds <= 59; 
}

/* time_from_set_time_message: extract time from set time 
   message from user interface */ 
void time_from_set_time_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"set %d %d %d", hours, minutes, seconds); 
}

/* time_from_set_alarm_message: extract time from set time 
   message from user interface */ 
void time_from_set_alarm_message(char message[], int *hours, int *minutes, int *seconds)
{
    sscanf(message,"alarm %d %d %d", hours, minutes, seconds); 
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
        /* check if it is a set time message */ 
        if (strncmp(message, "set", 3) == 0)
        {
            time_from_set_time_message(message, &hours, &minutes, &seconds); 
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_time(hours, minutes, seconds); 
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds"); 
            }
        }
        else if (strncmp(message, "alarm", 5) == 0)
        {
            time_from_set_alarm_message(message, &hours, &minutes, &seconds); 
            if (time_ok(hours, minutes, seconds))
            {
                clock_set_alarm(hours, minutes, seconds); 
            }
            else
            {
                si_ui_show_error("Illegal value for hours, minutes or seconds"); 
            }
        }
        else if (strncmp(message, "reset", 5) == 0)
        {
            clock_reset_alarm();
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

    /* initialise display */
    display_init();

    /* initialise variables */         
    clock_init(); 

    /* create tasks */ 

    si_task_create(clock_task, &Clock_Stack[STACK_SIZE-1], 20); 

    si_task_create(set_task, &Set_Stack[STACK_SIZE-1], 15); 

    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}

