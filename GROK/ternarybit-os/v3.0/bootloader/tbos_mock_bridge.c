#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

void* mock_server(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        return NULL;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        return NULL;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9001);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return NULL;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return NULL;
    }

    printf("🌐 TBOS Mock Hardware Bridge listening on port 9001\n");

    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            continue;
        }

        printf("🔗 Client connected to mock hardware bridge\n");

        char buffer[1024];
        while (recv(client_fd, buffer, sizeof(buffer), 0) > 0) {
            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"type\":\"mock_response\",\"status\":\"ok\"}";
            send(client_fd, response, strlen(response), 0);
        }

        close(client_fd);
    }

    return NULL;
}

int main() {
    printf("🎭 TBOS v3.0 Mock Hardware Bridge\n");
    printf("=================================\n");
    printf("🔧 This is a demonstration bridge for testing\n");
    printf("🌐 Listening on port 9001 for web connections\n\n");

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, mock_server, NULL);

    while (1) {
        sleep(1);
    }

    return 0;
}
