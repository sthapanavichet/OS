#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define PORT 1154
#define BUF_LEN 2048

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUF_LEN];

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("Error setting socket to non-blocking mode");
        close(sockfd);
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Bind socket to server address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Receive and process log messages
    while (1) {
        sendto(sockfd, "Set Log Level=1", BUF_LEN, 0, (struct sockaddr *)&client_addr, client_addr_len);
        ssize_t len = recvfrom(sockfd, buffer, BUF_LEN, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            perror("Error receiving data");
            sleep(1);
            continue;
        }

        // Print received log message
        printf("Received message from %s:%d:\n%s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
    }

    // Close the socket
    close(sockfd);

    return 0;
}