// client.h - Header file for shared memory
//
// 04-Aug-21  M. Watler         Created.
//
#ifndef CLIENT_H
#define CLIENT_H

const char SEM_NAME[] = "lab7_sem";
const char MEM_NAME[] = "lab7_mem";
const mode_t PERMS=(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

//TODO: A memory name and a semaphore name along with semaphore permissions
const int BUF_LEN=1024;
const int NUM_MESSAGES=30;

struct Memory {
    int            packet_no;
    unsigned short srcClientNo;
    unsigned short destClientNo;
    char           message[BUF_LEN];
};

void *recv_func(void *arg);
#endif//CLIENT_H
