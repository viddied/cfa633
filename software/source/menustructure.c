#include <stdio.h>
#include <ifaddrs.h>                // getifaddrs, freeifaddrs
#include <netdb.h>                  // getnameinfo, NI_NUMERICHOST
#include <stdlib.h>                 // exit
#include <string.h>
#include "typedefs.h"
#include "cf_packet.h"
#include "menustructure.h"          // MENUENTRY, NI_MAXHOST
#include "show_packet.h"

int fill_ipv4(char *centered, struct MENUENTRY *nic) {
    // Obtain network interface information
    struct ifaddrs *ifaddr, *ifaddr_copy;
    int family, namefailure, n;
    char ipv4[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Walk through linked list, maintaining head pointer (released later)
    for (ifaddr_copy = ifaddr, n = 0; 
         (ifaddr_copy != NULL) || (n < 3); 
         ifaddr_copy = ifaddr_copy->ifa_next, n++) {

         if (ifaddr_copy->ifa_addr == NULL) {
            continue;
         }

         family = ifaddr_copy->ifa_addr->sa_family;

         if (family == AF_INET) {
            namefailure = getnameinfo(ifaddr_copy->ifa_addr,
                                      sizeof(struct sockaddr_in),
                                      ipv4,
                                      NI_MAXHOST,
                                      NULL,
                                      0,
                                      NI_NUMERICHOST);

            if (namefailure) {
                printf("getnameinfo() failed\n");
                exit(EXIT_FAILURE);
            }
            
            printf("[*] INTERFACE:\t%s\n", ifaddr_copy->ifa_name);
            printf("[*] IP ADDRESS:\t%s\n\n", ipv4);

            center(centered, ifaddr_copy->ifa_name);
            memcpy(nic->line1, centered, 16);
            center(centered, ipv4);
            memcpy(nic->line2, centered, 16);

            nic = nic->nextPtr;
        }
    }

    freeifaddrs(ifaddr);

    return 0;
}

void show_menu_entry(struct MENUENTRY *entry) {
    // Clear display
    outgoing_response.command = 6;
    outgoing_response.data_length = 0;
    send_packet();

    // Send line one to the 633 using command 31
    outgoing_response.command = 31;
    outgoing_response.data[0] = 0;      // Column
    outgoing_response.data[1] = 0;      // Row

    //printf("[*] Showing menu entry. Address = %p\n", entry);

    memcpy(&outgoing_response.data[2], entry->line1, 16);

    //printf("[*] Line 1 = %s\n", entry->line1);
    
    outgoing_response.data_length = 18;
    send_packet();

    // CFA-631/633 protocol only allows one outstanding packet at a time.
    // Wait for the response packet before sending another packet.
    // NOTE: prepended "unused" to timed_out to prevent gcc warnings
    int k;
    __attribute__((unused)) int timed_out = 1;
    
    for (k = 0; k <= 10000; k++) {
        if (check_for_packet()) {
            ShowReceivedPacket();
            timed_out = 0;
            break;
        }
    }

    // Send line two to the 633 using command 31
    outgoing_response.command = 31;
    outgoing_response.data[0] = 0;      // Column
    outgoing_response.data[1] = 1;      // Row
    memcpy(&outgoing_response.data[2], entry->line2, 16);

    //printf("[*] Line 2 = %s\n", entry->line2);
    
    outgoing_response.data_length = 18;
    send_packet();

    // CFA-631/633 protocol only allows one outstanding packet at a time.
    // Wait for the response packet before sending another packet.
    timed_out = 1;
    
    for (k = 0; k <= 10000; k++) {
        if (check_for_packet()) {
            ShowReceivedPacket();
            timed_out = 0;
            break;
        }
    }
}

void move_entry(struct MENUENTRY *entryPtr) {
    //printf("\n[*] Inside move_entry. entryPtr = %p\n", entryPtr);
    if (entryPtr != NULL) {
        //printf("[*] Changing entry from %p to %p\n", menu_head, entryPtr);
        menu_head = entryPtr;

        show_menu_entry(menu_head);
    }
}

void submit() {
}

void center(char *centered, char *text) {
    int textlen = strlen(text);
    int padlen = (16 - textlen) / 2;
   
    // Store 16+1 bytes so that we can later strip the NULL
    snprintf(centered, 
            17, 
            "%*s%s%*s", 
            padlen, 
            "", 
            text, 
            16 - (padlen + textlen), 
            "");
}

int menu_init() {
    // Create a temporary buffer to store centered text
    char centered[17];

    // MAIN menu
    menu_main.id = 1;

    menu_main.prevPtr = NULL;
    menu_main.nextPtr = &menu_network;
    menu_main.upPtr = NULL;
    menu_main.downPtr = NULL;
    
    center(centered, "SYSTEM");
    memcpy(menu_main.line1, centered, 16);
    center(centered, "INFO");
    memcpy(menu_main.line2, centered, 16);

    //printf("\nMain Menu Struct Address:\t%p\n", &menu_main);
    //printf("Main Menu Line 1 Address:\t%p\n", &menu_main.line1);
    //printf("Main Menu Line 2 Address:\t%p\n", &menu_main.line2);

    // Make menu_main our top-level menu entry
    menu_head = &menu_main;
    show_menu_entry(menu_head);
    
    // NETWORK menu
    menu_network.id = 2;

    menu_network.prevPtr = &menu_main;
    menu_network.nextPtr = NULL;
    menu_network.upPtr = NULL;
    menu_network.downPtr = &menu_network_int1;
   
    center(centered, "NETWORK");
    memcpy(menu_network.line1, centered, 16);
    center(centered, "SETTINGS");
    memcpy(menu_network.line2, centered, 16);
    
    //printf("\nNetwork Menu Struct Address:\t%p\n", &menu_network);
    //printf("Network Menu Line 1 Address:\t%p\n", &menu_network.line1);
    //printf("Network Menu Line 2 Address:\t%p\n", &menu_network.line2);

    // Initialize interface 1 menu
    menu_network_int1.id = 21;

    menu_network_int1.prevPtr = NULL;
    menu_network_int1.nextPtr = &menu_network_int2;
    menu_network_int1.upPtr = &menu_network;
    menu_network_int1.downPtr = NULL;
   
    // Initialize interface 2 menu
    menu_network_int2.id = 22;

    menu_network_int2.prevPtr = &menu_network_int1;
    menu_network_int2.nextPtr = &menu_network_int3;
    menu_network_int2.upPtr = &menu_network;
    menu_network_int2.downPtr = NULL;

    // Initialize interface 3 menu
    menu_network_int3.id = 23;

    menu_network_int3.prevPtr = &menu_network_int2;
    menu_network_int3.nextPtr = NULL;
    menu_network_int3.upPtr = &menu_network;
    menu_network_int3.downPtr = NULL;
    
    fill_ipv4(centered, &menu_network_int1);

    return 0;
}
