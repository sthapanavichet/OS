#ifndef SEISMIC_DATA_H
#define SEISMIC_DATA_H

#include "SeismicData.h"
#include <semaphore.h>
#include <queue>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const int PORT = 1153;
const char IP_ADDR[] = "127.0.0.1";
const int MAX_CLIENT = 10;
const char PASSWORD[] = "Leaf";

/*
struct DataCenter
Structure representing a subscribed data center.
*/
struct DataCenter {
    char username[64];
    struct sockaddr_in addr;
};

/*
DataPacket
Structure representing a data packet.
*/
struct DataPacket {
    uint16_t packetNo;
    uint8_t packetLen;
    char data[BUF_LEN];
};

/*
Function to handle the read thread for receiving data packets.

This function is responsible for receiving data packets from data centers
and processing them accordingly.

param: Pointer to any arguments needed by the function.
return: Pointer to any return value of the function.
*/
void *read_thread(void *);

/*
Function to handle the write thread for sending data packets.

This function is responsible for sending data packets to subscribed data centers
at regular intervals.

param: Pointer to any arguments needed by the function.
return: Pointer to any return value of the function.
*/
void *write_thread(void *);

/*
Function to convert a sockaddr_in structure to a string representation.

This function converts a sockaddr_in structure representing a socket address
to a string representation in the format "IP:Port".

param: addr The sockaddr_in structure to convert.
return: String representation of the socket address.
*/
std::string sockaddr_toip(struct sockaddr_in);

/*
Function to implement security protocols for data communication.

This function implements security protocols to detect rogue clients,
preventing DDoS attack and password bruteforcing.
*/
void security_protocol();

/*
Function to handle commands received from data centers.

This function parses and processes commands received from data centers,
such as subscription requests and cancellation requests.

param: fd File descriptor of the socket,
buf Buffer containing the received command,
claddr Pointer to the sockaddr_in structure of the client.
*/
void command(int, char*, struct sockaddr_in*);

#endif