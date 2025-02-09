#include "main.h"

// Global variables
Item* items = NULL;  // Linked list head
int current_id = 1;  // ID counter

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d...\n", PORT);

    // Main server loop
    while (1) {
        if ((client_sock = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        handle_connection(client_sock);
        close(client_sock);
    }

    return 0;
}

void handle_connection(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    char method[16], path[256], body[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE * 2];

    // Read request
    read(sock, buffer, BUFFER_SIZE);
    parse_request(buffer, method, path, body);

    // Simple router
    if (strstr(path, "/items")) {
        if (strcmp(method, "GET") == 0) {
            char* items_list = list_items();
            snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n\r\n"
                "%s", items_list);
            free(items_list);
        }
        else if (strcmp(method, "POST") == 0) {
            // Simple "JSON" parsing
            char name[50];
            if (sscanf(body, "name=%49s", name) == 1) {
                create_item(name);
                snprintf(response, sizeof(response),
                    "HTTP/1.1 201 Created\r\n"
                    "Content-Type: application/json\r\n\r\n"
                    "{\"status\":\"created\"}");
            }
        }
    } else {
        snprintf(response, sizeof(response),
            "HTTP/1.1 404 Not Found\r\n\r\n");
    }

    write(sock, response, strlen(response));
}

void add_item(Item** head, Item* new_item) {
    new_item->next = *head;
    *head = new_item;
}

Item* create_item(char* name) {
    Item* new_item = malloc(sizeof(Item));
    if (!new_item) {
        perror("malloc failed");
        return NULL;
    }

    new_item->id = current_id++;
    strncpy(new_item->name, name, sizeof(new_item->name) - 1);
    new_item->next = NULL;

    add_item(&items, new_item);
    return new_item;
}

char* list_items() {
    char* json = malloc(BUFFER_SIZE);
    if (!json) return NULL;

    strcpy(json, "[");
    Item* current = items;
    while (current) {
        char temp[100];
        snprintf(temp, sizeof(temp),
            "{\"id\":%d,\"name\":\"%s\"},",
            current->id, current->name);
        strncat(json, temp, BUFFER_SIZE - strlen(json) - 1);
        current = current->next;
    }

    // Remove trailing comma
    if (json[strlen(json)-1] == ',') {
        json[strlen(json)-1] = '\0';
    }

    strcat(json, "]");
    return json;
}

void parse_request(char* req, char* method, char* path, char* body) {
    sscanf(req, "%s %s", method, path);

    char* body_start = strstr(req, "\r\n\r\n");
    if (body_start) {
        strncpy(body, body_start + 4, BUFFER_SIZE - 1);
    }
}