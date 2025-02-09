#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Constants
#define PORT 8080
#define BUFFER_SIZE 2048

// Data structure for items
typedef struct Item {
    int id;
    char name[50];
    struct Item* next;
} Item;

// Function prototypes
void handle_connection(int sock);
void parse_request(char* req, char* method, char* path, char* body);
Item* create_item(char* name);
void add_item(Item** head, Item* new_item);
char* list_items();

#endif // SERVER_H