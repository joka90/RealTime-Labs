/* This file is part of Simple_OS, a real-time operating system  */
/* designed for research and education */
/* Copyright (c) 2003-2013 Ola Dahl */

/* The software accompanies the book Into Realtime, available at  */
/* http://theintobooks.com */

/* Simple_OS is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "schedule.h"

#include "ready_list.h"
#include "task.h"

#include "tcb.h"

#include "console.h"

/* fig_begin schedule */ 
/* schedule: perform priority based scheduling */ 
void schedule(void)//TODO
{
    /* task id for the running task */ 
    int task_id_running; 

    /* task id for the task in ready list with 
       highest priority */ 
    int task_id_highest_prio; 

    /* get task id for the running task */ 
    task_id_running = task_get_task_id_running(); 

    /* get task id for task in ready list with 
       highest priority */ 
    task_id_highest_prio = 
        ready_list_get_task_id_highest_prio();

    //printf("---- task %d is on the top ----\n", task_id_highest_prio);
    //IF CURRENT TASK is not realtime task: INCREMENT TICKS
    if(task_get_prio(task_id_running) >= TIME_SHARED_PRIORITY_BASE)
    {
        task_increment_ticks(task_id_running);
    }

    /* check if a task switch shall be performed */ 
    if (task_id_highest_prio != task_id_running && task_get_prio(task_id_highest_prio) < TIME_SHARED_PRIORITY_BASE)
    {
        /* perform task switch */ 
        task_switch(task_id_running, task_id_highest_prio);//task_switch(int task_id_old, int task_id_new); 
        return; 
    }


    if (MAX_RUN_TICKS < task_get_ticks(task_id_running))
    {
        int idleast, ticksleast; 
        //get leas ticks and prio id
        ready_list_get_task_id_and_least_ticks(&idleast, &ticksleast);
        if(ticksleast>=100)
        {
           printf("nolla\n");
            ready_list_zero_ticks();
            /* perform task switch */ 
            task_switch(task_id_running, task_id_highest_prio);//task_switch(int task_id_old, int task_id_new); 
           return;
        }
        else if(task_id_running == idleast)//byt inte om vi redan är på samma
        {
            printf("borde inte vara har\n");
            return;
        }
        else//annars byt till dem med least.
        {
            /* perform task switch */ 
            task_switch(task_id_running, idleast);
        }
        
    }



}
/* fig_end schedule */ 
