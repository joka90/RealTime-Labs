/* A program with three tasks
   One task is modifying common data, and 
   one task is reading and displaying data 
   A third task is used for closing the program 
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

/* stack for display task */
stack_item Display_Stack[STACK_SIZE];

/* stack for change task */
stack_item Change_Stack[STACK_SIZE];

/* stack for closing task */ 
stack_item Exit_Stack[STACK_SIZE]; 

/* common variables */

/* integer number to be displayed */
int Number;

/* variable indicating if Number is even */
int Even_Number;

/* semaphore for protecting the odd/even calc */ 
si_semaphore Mutex; 

/* long_calculation: function simulating a general floating point
   calculation with significant processing time */
void long_calculation(void) 
{
    int i;
    double x = 0.0;

    for (i = 0; i < 1000; i++) 
    {
        x+= 0.0001;
    }
}

/* ---- functions with access to common variables Number and Even_Number ---- */ 

/* increment_number: increments Number by adding 
   inc_value to Number, and updates Even_Number */ 
void increment_number(int inc_value) 
{
    /* reserve variables */ 
    si_sem_wait(&Mutex);

    /* fig_begin number_inc_value */ 
    Number += inc_value;
    long_calculation();
    /* fig_end number_inc_value */ 

    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }

    /* release variables */ 
    si_sem_signal(&Mutex); 
}

/* decrement_number: decrements Number with dec_value, 
   and updates Even_Number */
void decrement_number(int dec_value) 
{
    /* reserve variables */ 
    si_sem_wait(&Mutex); 

    Number -= dec_value;
    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }

    /* release variables */ 
    si_sem_signal(&Mutex); 
}

/* set_number: sets Number and updates Even_Number */
void set_number(int new_number) 
{
    /* reserve variables */ 
    si_sem_wait(&Mutex);

    Number = new_number;
    if (Number % 2 == 0) 
    {
        Even_Number = 1;
    } 
    else 
    {
        Even_Number = 0;
    }

    /* release variables */ 
    si_sem_signal(&Mutex); 
}

/* get_number: gets the value of Number, together with the 
   value of Even_Number, which indicates if Number is even or odd */ 
void get_number(
    int *number_value, int *even_number_value) 
{
    /* reserve variables */ 
    si_sem_wait(&Mutex); 

    *number_value = Number;
    *even_number_value = Even_Number;

    /* release variables */ 
    si_sem_signal(&Mutex); 
}


/* ---- auxiliary functions ---- */ 

/* print_number: prints an integer number, together with information
   whether the number is odd or even */
void print_number(int number, int even_number) 
{
    /* value which should have the same value as the common variable
       Even_Number if everything is Ok */
    int check_even_number;

    /* string for the text to be printed */
    char write_str[40];

    /* conversion of number from integer to string */
    sprintf(write_str,"The number is: %d ",number);

    /* display information on odd or even number */
    if (even_number) 
    {
        strcat(write_str,"The number is EVEN");
    } 
    else 
    {
        strcat(write_str,"The number is ODD");
    }

    /* start drawing, i.e. prepare for writing to user interface */ 
    si_ui_draw_begin(); 

    /* display number information */ 
    si_ui_draw_string(write_str, 70, 50);

    /* check if the number really is odd or even */
    if (number % 2 == 0) 
    {
        check_even_number = 1;
    } 
    else 
    {
        check_even_number = 0;
    }

    /* display error message if necessary */
    if (even_number != check_even_number) 
    {
        if (check_even_number) 
        {
            sprintf(write_str,"ERROR: The number is really EVEN");
        } 
        else 
        {
            sprintf(write_str,"ERROR: The number is really ODD");
        } 
    }
    else
    {
        sprintf(write_str, "odd/even check without errors"); 
    }

    /*  display odd/even information */ 
    si_ui_draw_string(write_str, 70, 70);

    /* finish drawing */ 
    si_ui_draw_end(); 

}


/* ---- tasks ---- */ 

/* display_task: display task */ 
void display_task(void) 
{
    /* current value of the integer number */
    int number;
    /* current noted information whether the integer number is
       odd or even (a copy of the common variable Even_Number) */
    int even_number;

    /* set GUI size */ 
    si_ui_set_size(400, 200); 

    /* set initial value for the integer number */
    set_number(100); 

    while (1) 
    {
        /* get the number and the information regarding odd or even */
        get_number(&number, &even_number);
        /* print the number */
        print_number(number, even_number);
        /* wait for a while */ 
        si_wait_n_ms(1000 + rand() % 1000); 
    }
}

/* change_task: modifies the  integer number, and notes if the modified number
   is odd or even by setting the value of the common variable Even_Number */
void change_task(void) 
{
    /* increment value */
    const int inc_value = 1;

    int i; 

    while(1) 
    {
        /* change the number */
        for (i = 0; i < 1000; i++) 
        {
            increment_number(inc_value);
        }
        for (i = 0; i < 1000; i++) 
        {
            decrement_number(inc_value);
        }
    }
}

/* exit_task: used for closing the program */ 
void exit_task(void)
{
    /* message array */ 
    char message[SI_UI_MAX_MESSAGE_SIZE]; 

    while(1)
    {
        /* read a message */ 
        si_ui_receive(message); 
        /* check if it is an exit message */ 
        if (strcmp(message, "exit") == 0)
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

/* ---- main ---- */ 

int main(void)
{
    /* initialise kernel */ 
    si_kernel_init(); 
    
    /* initialise UI channel */ 
    si_ui_init(); 

    /* initialise semaphore */ 
    si_sem_init(&Mutex, 1); 

    /* create tasks */ 

    si_task_create(display_task, &Display_Stack[STACK_SIZE - 1], 10);

    si_task_create(change_task, &Change_Stack[STACK_SIZE - 1], 20);

    si_task_create(exit_task, &Exit_Stack[STACK_SIZE - 1], 15);

    /* start the kernel */ 
    si_kernel_start(); 

    /* will never be here! */ 
    return 0; 
}


