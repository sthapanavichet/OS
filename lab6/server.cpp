// client1.cpp - C Program for Message Queue (Read/Write) 
//
// 27-Mar-20  M. Watler         Created.
//
#include <errno.h> 
#include <iostream> 
#include <queue> 
#include <signal.h> 
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <unistd.h>
#include "client.h"

using namespace std;

key_t key;
int msgid;

bool is_running;
queue<Message> msgQueue;

void *recv_func(void *arg);
pthread_mutex_t lock_x;

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}
  
int main() 
{ 
    int ret;
    pthread_t tid;
  
    // handle ctrl+c
    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    // ftok to generate unique key 
    key = ftok("serverclient", 65); 

    // get message queue
    msgid = msgget(key, 0666 | IPC_CREAT);

    pthread_mutex_init(&lock_x, NULL);
    is_running=true;
    ret = pthread_create(&tid, NULL, recv_func, NULL);
    if(ret!=0) {
        is_running = false;
        cout<< strerror(errno) <<endl;
        return -1;
    }

    while(is_running) {
        while(msgQueue.size()>0) {
            pthread_mutex_lock(&lock_x);
            Message recvMsg= msgQueue.front();
            msgQueue.pop();
            pthread_mutex_unlock(&lock_x);
            recvMsg.mtype = recvMsg.msgBuf.dest;
            msgsnd(msgid, &recvMsg, sizeof(recvMsg), 0);
        }
        sleep(1);
    }

    Message quitMsg;
    sprintf(quitMsg.msgBuf.buf, "Quit");

    for(int i = 0; i < 3; i++) {
        quitMsg.mtype = i + 1;
        msgsnd(msgid, &quitMsg, sizeof(quitMsg), 0);
    }
    cout<<"Server: quitting..."<<endl;
    
    pthread_join(tid, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
} 

void *recv_func(void *arg)
{
    while(is_running) {
        // extract messages of mtype id from client id
        Message msg;
        msgrcv(msgid, &msg, sizeof(msg), 4, 0);
        pthread_mutex_lock(&lock_x);
        msgQueue.push(msg);
        pthread_mutex_unlock(&lock_x);
    }
    pthread_exit(NULL);
}

/*
1. My favorite is message queue because everything is in one place and it is asynchronous, which makes it easy to implement.
Socket is also good because it can be used for many cases like IPC or communicates with process on other machine. Fifos and pipes
on the other hand seems redundant because everything they do, message queues and sockets can do better.

2. Fifos is my least favorite because it needs to be synchronus and many things can wrong when implementing it. Same thing with pipes,
I would avoid using them. 

3. For this lab, a server isn't needed because every client is reading and writing on a different mtype. So instead of sending it all to 
the server, the client can just set mtype to the number that the destination client is using.
*/
