#include "draw.h"

#include "lift.h"

/* include simple_os.h, to get access to Simple_OS GUI 
   interaction functions */ 
#include <simple_os.h>

#define LEVEL_OFFSET 75

static char message[SI_UI_MAX_MESSAGE_SIZE]; 

static int xBuilding = 50; 
static int yBuilding = 100; 

static int xLift = 355; 
static int yLift[N_FLOORS] = {180, 268, 356, 444, 532}; 

static int cxGubbe = 41; 
static int cyGubbe = 73; 
static int delta_y_Gubbe = 0; 

static int info_x = 160;
static int info_y = 20;

static int id_x_offset = 7; 
static int id_y_offset = -10; 

void draw_init(void)
{
    xBuilding = 50; 
    yBuilding = 100; 

    xLift = 355; 

    yLift[0] = 180; 
    yLift[1] = 268; 
    yLift[2] = 356; 
    yLift[3] = 444; 
    yLift[4] = 532; 

    cxGubbe = 41; 
    cyGubbe = 73; 
    delta_y_Gubbe = 0; 

    info_x = 160;
    info_y = 20;

    id_x_offset = 7; 
    id_y_offset = -10; 
}

void draw_lift(lift_type lift)
{
    int floor, i; 

    si_ui_draw_begin(); 

    si_ui_draw_string(
        "Lift Simulation (send \"new\" to create new passengers)",
        info_x, info_y); 

    si_ui_draw_image("building", xBuilding, yBuilding); 

    si_ui_draw_image("lift", xLift, yLift[lift->floor]); 

    /* draw lift passengers */ 
    for (i = 0; i < MAX_N_PASSENGERS; i++)
    {
        if (lift->passengers_in_lift[i].id != NO_ID)
        {
            si_ui_draw_image("gubbe", 
                           xLift + i*cxGubbe, yLift[lift->floor] - cyGubbe + delta_y_Gubbe); 

            si_string_copy(message, "%x   %x"); 
            si_insert_int_as_hex_no_leading_zeros(message, lift->passengers_in_lift[i].id); 
            si_insert_int_as_hex_no_leading_zeros(message, lift->passengers_in_lift[i].to_floor); 
            si_ui_draw_string(message, xLift + i*cxGubbe + id_x_offset, 
                            yLift[lift->floor] - cyGubbe + delta_y_Gubbe + id_y_offset);
        }
    }

    /* draw waiting persons */ 
    for (floor = 0; floor < N_FLOORS; floor++)
    {
        si_string_copy(message, " %x "); 
        si_insert_int_as_hex_no_leading_zeros(message, floor); 
        si_ui_draw_string(message, xLift - LEVEL_OFFSET/2 + 5, yLift[floor] - 3); 

        for (i = 0; i < MAX_N_PERSONS; i++)
        {
            if (lift->persons_to_enter[floor][i].id != NO_ID)
            {
                si_ui_draw_image("gubbe", xLift - LEVEL_OFFSET - i*(cxGubbe + 2), 
                               yLift[floor] - cyGubbe); 
                si_string_copy(message, "%x"); 
                si_insert_int_as_hex_no_leading_zeros(message, lift->persons_to_enter[floor][i].id); 
                si_ui_draw_string(message, xLift  - LEVEL_OFFSET - i*(cxGubbe + 2) - 3, 
                                yLift[floor] - cyGubbe + 5);

            }
        }
    }

    si_ui_draw_end(); 
}

