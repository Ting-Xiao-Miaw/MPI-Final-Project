// UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "socket.h"


void Send(const int* encoded_data, const char* server_address, int bufflen) {
    int sockfd;
    struct sockaddr_in servaddr;
   
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, server_address, &servaddr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }
       
    int n;
    socklen_t len;
       
    sendto(sockfd, (int *)encoded_data, sizeof(int) * bufflen, MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    /*
    for (int i = 0; i < bufflen; i++)
        printf("%d ", encoded_data[i]);
    printf("\n");
    */
    close(sockfd);
}


int bindSocket() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}


void Receive(int* buffer, int sockfd, int bufflen) {
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    socklen_t len = sizeof(cliaddr);

    recvfrom(sockfd, (int *)buffer, sizeof(int) * bufflen, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
}


