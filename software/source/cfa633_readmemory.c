// 633_COMMAND_LINE_TEST.cpp
// Ultra-simple 633 command-line communications example

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "typedefs.h"
#include "serial.h"
#include "cf_packet.h"
#include "show_packet.h"

//============================================================================
int main(int argc, char* argv[])
  {
  printf("\nREAD LCD MEMORY\n\n");
  printf("Usage:\n");
  printf("%s PORT BAUD\n",argv[0]);
  printf("PORT is something like \"/dev/ttyS0\" or \"/dev/ttyUSB0\"\n");
  printf("BAUD is 19200 or 115200\n");
  printf("To clear the display, enter \"clear\" as an optional third parameter\n\n");

  //If only 0 or 1 parameter is entered, prompt for the missing parameter(s)
  if(argc < 3)
    {
      printf("\nMISSING A PARAMETER. Enter both PORT and BAUD.\n\n");
      return(0);
    }

  //Check for optional "clear" parameter and set flag if found
  int cleardisplay=0;
  if((argc > 3)&&(!strcmp(argv[3],"clear"))) cleardisplay=1;

  int baud;
  //default the baud to 19200
  if (strcmp(argv[2], "115200")) {
      baud=19200;
  }
  else {
      baud=115200;
  }

  if (Serial_Init(argv[1],baud)) {
      printf("Could not open port \"%s\" at \"%d\" baud.\n",argv[1],baud);
      
      return(1);
  }
  else {
      printf("\"%s\" opened at \"%d\" baud.\n\n",argv[1],baud);
  }

  //For some reason, Linux seems to buffer up data from the LCD, and they are sometimes
  //dumped at the start of the program. Clear the serial buffer.
  while (BytesAvail())
    GetByte();

//Outgoing command packets. Either clear the screen
//or send our line information
//**********************************************
  if (cleardisplay) {
      outgoing_response.command = 6;
      outgoing_response.data_length = 0;
      send_packet();
  }
  else {
    //Send command one to the 633
    outgoing_response.command = 10;
    outgoing_response.data[0] = 0x40;
    outgoing_response.data_length = 1;
    send_packet();

  //CFA-631 / CFA-633 communications protocol only allows
  //one outstanding packet at a time. Wait for the response
  //packet from the CFA-631 / CFA-633 before sending another
  //packet.
  int k;
  int timed_out;

  timed_out = 1; //default timed_out is true
  
  for (k=0; k <= 10000; k++) {
    if (check_for_packet()) {
        ShowReceivedPacket();
        timed_out = 0; //set timed_out to false
        break;
    }
  }

  if(timed_out)
    printf("Timed out waiting for a response.\n");

    //Send command two to the 633
    outgoing_response.command = 10;
    outgoing_response.data[0] = 0x80;
    outgoing_response.data_length = 1;
    send_packet();

  //CFA-631 / CFA-633 communications protocol only allows
  //one outstanding packet at a time. Wait for the response
  //packet from the CFA-631 / CFA-633 before sending another
  //packet.

  printf("\n");

  timed_out = 1; //default timed_out is true

  for (k=0; k <= 10000; k++)
    if(check_for_packet())
      {
      ShowReceivedPacket();
      timed_out = 0; //set timed_out to false
      break;
      }
  if(timed_out)
    printf("Timed out waiting for a response.\n");

    //Send command three to the 633
    outgoing_response.command = 10;
    outgoing_response.data[0] = 0xC0;
    outgoing_response.data_length = 1;
    send_packet();

  //CFA-631 / CFA-633 communications protocol only allows
  //one outstanding packet at a time. Wait for the response
  //packet from the CFA-631 / CFA-633 before sending another
  //packet.

  printf("\n");

  timed_out = 1; //default timed_out is true
  for(k=0;k<=10000;k++)
    if(check_for_packet())
      {
      ShowReceivedPacket();
      timed_out = 0; //set timed_out to false
      break;
      }
  if(timed_out)
    printf("Timed out waiting for a response.\n");

//**********************************************

  printf("\n[*] Done.\n\n");
  Uninit_Serial();
  }
  
  return 0;
 }
//============================================================================
