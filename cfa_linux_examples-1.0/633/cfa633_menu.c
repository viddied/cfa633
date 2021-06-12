// cfa633_menu.c
//      LCD menu control program

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "menustructure.h"
#include "typedefs.h"
#include "serial.h"
#include "cf_packet.h"
#include "show_packet.h"

//============================================================================
#define PREFIX_DEBUG        "<7>"
#define PREFIX_INFORM       "<6>"
#define PREFIX_NOTICE       "<5>"
#define PREFIX_WARNING      "<4>"
#define PREFIX_ERROR        "<3>"
#define PREFIX_CRITICAL     "<2>"
#define PREFIX_ALERT        "<1>"
#define PREFIX_EMERGENCY    "<0>"

//============================================================================
int main(int argc, char* argv[]) {
    // If only 0 or 1 parameter is entered, prompt for the missing parameter(s)
    if (argc < 3) {
        printf("\nMISSING A PARAMETER. Enter both PORT and BAUD.\n\n");
        
        return(0);
    }

    // Check for optional "clear" parameter and set flag if found
    int cleardisplay = 0;
    if ((argc > 3) && (!strcmp(argv[3], "clear"))) {
        cleardisplay = 1;
    }
    cleardisplay++;
    cleardisplay--;

    int baud;
    
    // Default the baud to 19200
    if (strcmp(argv[2], "115200")) {
        baud = 19200;
    }
    else {
        baud = 115200;
    }

    if (Serial_Init(argv[1], baud)) {
        printf(PREFIX_ERROR "Could not open port \"%s\" at \"%d\" baud.\n", argv[1], baud);
        
        return(1);
    }
    else {
        printf("\"%s\" opened at \"%d\" baud.\n\n",argv[1],baud);
    }

    // Create menu structure
    menu_init();

    // For some reason, Linux seems to buffer up data from the LCD, and they are sometimes
    // dumped at the start of the program. Clear the serial buffer.
    while (BytesAvail()) {
        GetByte();
    }

    int keypressed = 0;
    while (!cleardisplay) {
        usleep(100000);  // 1/10 second
        
        if (check_for_packet()) {
            // If return value > 0, key has been pressed
            keypressed = ShowReceivedPacket();

            switch (keypressed) {
                case 3  :   move_entry(menu_head->prevPtr);
                            break; /*
                case 9  :   printf("Left2\n");
                            break; */
                case 4  :   move_entry(menu_head->nextPtr);
                            break; /*
                case 10 :   printf("Right2\n");
                            break; */
                case 1  :   move_entry(menu_head->upPtr);
                            break; /*
                case 7  :   printf("Up2\n");
                            break; */
                case 2  :   move_entry(menu_head->downPtr);
                            break; /*
                case 8  :   printf("Down2\n");
                            break;*/
                case 5  :   submit();
                            break; /*
                case 11 :   printf("Ok2\n");
                            break; */
            }
        }
    }
    
    if (cleardisplay) {
        printf("Display Cleared.\n");
        printf("Done.\n\n");
    }
    
    Uninit_Serial();
    
    return 0;
}
//============================================================================
