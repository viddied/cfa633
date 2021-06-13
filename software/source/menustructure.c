#include <stdio.h>
#include <ifaddrs.h>                // getifaddrs, freeifaddrs
#include <net/if.h>                 // ifreq
#include <netdb.h>                  // getnameinfo, NI_NUMERICHOST
#include <stdlib.h>                 // exit
#include <string.h>
#include <sys/ioctl.h>              // ioctl
#include <sys/socket.h>             //
#include "typedefs.h"
#include "cf_packet.h"
#include "menustructure.h"          // MENUENTRY, NI_MAXHOST
#include "show_packet.h"

int fill_nic(char *centered, struct MENUENTRY *nic) {
    // Obtain network interface information
    int total_nics = 4;
    char *nic_name;
    struct ifaddrs *ifaddr, *ifaddr_copy;
    int family, namefailure, n;
    char ipv4[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    int32_t sd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        // Free memory allocated by getifaddrs
        freeifaddrs(ifaddr);
        return 0;
    }

    // Walk through linked list, maintaining head pointer (released later)
    for (ifaddr_copy = ifaddr, n = 0; 
        (ifaddr_copy != NULL) || (n < total_nics); 
        ifaddr_copy = ifaddr_copy->ifa_next, n++) {

        // Avoid loopback interface
        nic_name = ifaddr_copy->ifa_name;
        if (strncmp(nic_name, "lo", 2) == 0) {
            continue;
        }

        // Interface name menu entry
        center(centered, "Interface:");
        memcpy(nic->line1, centered, 16);
        center(centered, nic_name);
        memcpy(nic->line2, centered, 16);

        // Get MAC address
        // NOTE: We get MAC addresses first since AF_PACKET family
        //       structures come before AF_INET it seems.
        if (ifaddr_copy->ifa_data != 0) {
            struct ifreq req;

            strcpy(req.ifr_name, nic_name);

            if (ioctl(sd, SIOCGIFHWADDR, &req) != -1) {
                printf("[*] Name:\t%s\n", nic_name);
            }
        }

        if (ifaddr_copy->ifa_addr == NULL) {
            printf("[*] No IP address detected\n");
            continue;
        }

        family = ifaddr_copy->ifa_addr->sa_family;

        // Get IPv4 address
        //  family:
        //      2 = AF_INET
        //     10 = AF_INET6
        //     17 = AF_PACKET
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
            
            // Interface IPv4 menu entry
            center(centered, "IPv4 Address:");
            memcpy(nic->downPtr->line1, centered, 16);
            center(centered, ipv4);
            memcpy(nic->downPtr->line2, centered, 16);

            // Interface MAC menu entry
            center(centered, "MAC Address:");
            memcpy(nic->downPtr->downPtr->line1, centered, 16);
            center(centered, "0123456789AB");
            memcpy(nic->downPtr->downPtr->line2, centered, 16);

            printf("[*] Got IP Address\n");

            nic = nic->nextPtr;
        }
        
        if (family == AF_PACKET) {
            /*/ Interface IPv4 menu entry
            center(centered, "IPv4 Address:");
            memcpy(nic->downPtr->line1, centered, 16);
            center(centered, "< empty >");
            memcpy(nic->downPtr->line2, centered, 16);

            // Interface MAC menu entry
            center(centered, "MAC Address:");
            memcpy(nic->downPtr->downPtr->line1, centered, 16);
            center(centered, "0123456789AB");
            memcpy(nic->downPtr->downPtr->line2, centered, 16);
            */
            printf("[*] Got MAC Address\n");
            
            //nic = nic->nextPtr;
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
    menu_network.downPtr = &menu_network_int1_name;
   
    center(centered, "NETWORK");
    memcpy(menu_network.line1, centered, 16);
    center(centered, "SETTINGS");
    memcpy(menu_network.line2, centered, 16);
    
    //printf("\nNetwork Menu Struct Address:\t%p\n", &menu_network);
    //printf("Network Menu Line 1 Address:\t%p\n", &menu_network.line1);
    //printf("Network Menu Line 2 Address:\t%p\n", &menu_network.line2);

    // Initialize interface 1 menu
    menu_network_int1_name.id = 21;

    menu_network_int1_name.prevPtr = NULL;
    menu_network_int1_name.nextPtr = &menu_network_int2_name;
    menu_network_int1_name.upPtr = &menu_network;
    menu_network_int1_name.downPtr = &menu_network_int1_ipv4;
   
    // Initialize interface 1 IPv4 menu
    menu_network_int1_ipv4.id = 211;

    menu_network_int1_ipv4.prevPtr = NULL;
    menu_network_int1_ipv4.nextPtr = &menu_network_int2_ipv4;
    menu_network_int1_ipv4.upPtr = &menu_network_int1_name;
    menu_network_int1_ipv4.downPtr = &menu_network_int1_mac;
   
    // Initialize interface 1 MAC menu
    menu_network_int1_mac.id = 212;

    menu_network_int1_mac.prevPtr = NULL;
    menu_network_int1_mac.nextPtr = &menu_network_int2_mac;
    menu_network_int1_mac.upPtr = &menu_network_int1_ipv4;
    menu_network_int1_mac.downPtr = NULL;
   
    // Initialize interface 2 menu
    menu_network_int2_name.id = 22;

    menu_network_int2_name.prevPtr = &menu_network_int1_name;
    menu_network_int2_name.nextPtr = &menu_network_int3_name;
    menu_network_int2_name.upPtr = &menu_network;
    menu_network_int2_name.downPtr = &menu_network_int2_ipv4;

    // Initialize interface 2 IPv4 menu
    menu_network_int2_ipv4.id = 221;

    menu_network_int2_ipv4.prevPtr = &menu_network_int1_ipv4;
    menu_network_int2_ipv4.nextPtr = &menu_network_int3_ipv4;
    menu_network_int2_ipv4.upPtr = &menu_network_int2_name;
    menu_network_int2_ipv4.downPtr = &menu_network_int2_mac;
   
    // Initialize interface 2 MAC menu
    menu_network_int2_mac.id = 222;

    menu_network_int2_mac.prevPtr = &menu_network_int1_mac;
    menu_network_int2_mac.nextPtr = &menu_network_int3_mac;
    menu_network_int2_mac.upPtr = &menu_network_int2_ipv4;
    menu_network_int2_mac.downPtr = NULL;

    // Initialize interface 3 menu
    menu_network_int3_name.id = 23;

    menu_network_int3_name.prevPtr = &menu_network_int2_name;
    menu_network_int3_name.nextPtr = NULL;
    menu_network_int3_name.upPtr = &menu_network;
    menu_network_int3_name.downPtr = &menu_network_int2_ipv4;

    // Initialize interface 3 IPv4 menu
    menu_network_int3_ipv4.id = 231;

    menu_network_int3_ipv4.prevPtr = &menu_network_int2_ipv4;
    menu_network_int3_ipv4.nextPtr = NULL;
    menu_network_int3_ipv4.upPtr = &menu_network_int3_name;
    menu_network_int3_ipv4.downPtr = &menu_network_int2_mac;
   
    // Initialize interface 3 MAC menu
    menu_network_int3_mac.id = 232;

    menu_network_int3_mac.prevPtr = &menu_network_int2_mac;
    menu_network_int3_mac.nextPtr = NULL;
    menu_network_int3_mac.upPtr = &menu_network_int3_ipv4;
    menu_network_int3_mac.downPtr = NULL;


    fill_nic(centered, &menu_network_int1_name);

    return 0;
}
