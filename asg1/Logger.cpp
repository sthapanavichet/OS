#include "Logger.h"
#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

const char IP_ADDR[]="127.0.0.1"; // addr and port needs to be decided this is just a placeholder for now.
const int PORT = 1154;
const int BUF_LEN = 2048;
int logLevel = 0;
bool is_running = false;
int fd;
struct sockaddr_in servaddr;
socklen_t socklen = sizeof(servaddr);

pthread_mutex_t lock_x;
pthread_t tid;

void *recv_func(void *arg);

int InitializeLog() {
    cout << "Starting Log" << endl;
    int ret;
    // Creating socket file descriptor
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    // setting up servaddr
    memset(&servaddr, 0, socklen);
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    servaddr.sin_port = htons(PORT);

    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        perror("Error setting socket to non-blocking mode");
        close(fd);
        return 1;
    }

    pthread_mutex_init(&lock_x, NULL);
    is_running=true;
    ret = pthread_create(&tid, NULL, recv_func, NULL);
    if(ret!=0) {
        is_running = false;
        perror("Thread creation failed");
        close(fd);
        return -1;
    }
    return 0;
}

void SetLogLevel(LOG_LEVEL level) {
    pthread_mutex_lock(&lock_x);
    logLevel = level;
    pthread_mutex_unlock(&lock_x);
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message) {
    pthread_mutex_lock(&lock_x);
    if(level >= logLevel) {
        pthread_mutex_unlock(&lock_x);
        
        char buf[BUF_LEN];
        memset(buf, 0, BUF_LEN);
        
        time_t now = time(0);
        char *dt = ctime(&now);
        
        char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
        // format string into a char array
        int len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
        buf[len-1] = '\0';
        printf("Sending: %s", buf);
        // send log to server
        len = sendto(fd, buf, len, 0, (struct sockaddr *)&servaddr, socklen);
        if(len == -1) perror("sendto operation failed");
    }
    else pthread_mutex_unlock(&lock_x);
}

void ExitLog() {
    cout << "Exiting Log" << endl;
    is_running = false;
    pthread_join(tid, NULL);
    close(fd);
}

void *recv_func(void *arg) {
    char buf[BUF_LEN];
    char setLog[] = "Set Log Level=";
    int setLogLen = strlen(setLog);

    while(is_running)
    {
        int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&servaddr, &socklen);
        if(len < 0) sleep(1); // sleep if nothing is being read
        else {
            if(strncmp(buf, setLog, setLogLen) == 0) {
                int logLevel = buf[setLogLen] - '0';
                SetLogLevel((LOG_LEVEL) logLevel);
            }
            sleep(1);
        }
    }
}