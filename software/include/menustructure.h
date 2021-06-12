#ifndef MENUSTRUCTURE_H
#define MENUSTRUCTURE_H

#define NI_MAXHOST 1025

struct MENUENTRY {
    int id;

    struct MENUENTRY *prevPtr;
    struct MENUENTRY *nextPtr;
    struct MENUENTRY *upPtr;
    struct MENUENTRY *downPtr;

    char line1[17];
    char line2[17];

};

// menu_head is a simple pointer to the active menu entry
struct MENUENTRY *menu_head;
struct MENUENTRY menu_main;
struct MENUENTRY menu_network;
struct MENUENTRY menu_network_int1;
struct MENUENTRY menu_network_int2;
struct MENUENTRY menu_network_int3;

void center(char *centered, char *text);
int fill_ipv4(char *centered, struct MENUENTRY *nic);
int menu_init();
void move_entry(struct MENUENTRY *entryPtr);
void show_menu_entry(struct MENUENTRY *entry);
void submit();

#endif
