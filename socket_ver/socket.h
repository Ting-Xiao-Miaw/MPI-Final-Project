#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 19053
#define MAXLEN 28

void Send(const int* encoded_data, const char* server_address, int bufflen);

int bindSocket();

void Receive(int* buffer, int sockfd, int bufflen);

