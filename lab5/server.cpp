#include <queue>
#include <iostream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>

using namespace std;

int const NUM_CLIENTS = 3;
pthread_mutex_t lock;
queue<string> msgs;

bool isRunning = true;

void *recv_thread(void *);
static void signalHandler(int);

int main(int argc, char* argv[]) {
    if(argc != 2) {
        cout << "Invalid command-line argument" << endl;
        return -1;
    }
    // signal handling
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    // socket
    int fd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));  // empty socket struct
    addr.sin_family = AF_INET;  // set up sockaddr_in struct
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    addr.sin_port = htons(atoi(argv[1]));

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        cout << "Server:" <<strerror(errno) <<endl;
        exit(-1);
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);  // non blocking socket

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cout << "Server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    if(listen(fd, 5) == -1) {
        cout << "Server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }
    // threading
    int cl[NUM_CLIENTS];
    pthread_t tid[NUM_CLIENTS];
    int con = 0;
    pthread_mutex_init(&lock, NULL);
    
    while(isRunning) {
        if(con < NUM_CLIENTS) {
            cout << "Waiting for connection from client..." << endl;
            cl[con] = accept(fd, NULL, NULL);
            if(cl[con] != -1) {
                if(pthread_create(&tid[con], NULL, recv_thread, &cl[con]) == -1) {
                    cout << "Server: cannot create recieve thread" << endl;
                    cout << strerror(errno) << endl;
                    close(fd);
                    exit(-1);
                }
                con++;
            }
        }

        while(!msgs.empty() && isRunning) {
            pthread_mutex_lock(&lock);
            string msg = msgs.front();
	        msgs.pop();
            pthread_mutex_unlock(&lock);
            cout << msg <<endl;
        }

        sleep(1);
    }

    for(int i = 0; i < NUM_CLIENTS; i++) {
        write(cl[i], "Quit", 5);
        close(cl[i]);
        pthread_join(tid[i], NULL);
    }
    close(fd);

}

void *recv_thread(void *arg) {
    int cl = *(int *)arg;
    char buf[4096];

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    // read timeout 5s
    if (setsockopt(cl, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        cout << "Server:" <<strerror(errno) <<endl;
        close(cl);
        exit(-1);
    }
    while(isRunning) {
        int rc = read(cl,buf,sizeof(buf));
        if(rc > 0) {
            pthread_mutex_lock(&lock);
            msgs.push(buf);
            pthread_mutex_unlock(&lock);
        }
    }
    pthread_exit(NULL);
}

static void signalHandler(int signal) {
    cout << "Server is closing" << endl;
    isRunning = false;
}


/* 
Q1: Asynchronous communication is when the server and client commincates at the same time, unlike
synchornous communication where it is done one after another, not at the same time.

Q2: socket read in a thread gives the server much more control on the read operation. For example in this lab
we can have timeout on the read operation alone rather than the whole socket. With socket read in a thread, 
each thread handle a reading operation from a client so it is much easier to recieve information and when one 
of the connection with the client is down we can just close that thread (not implemented in this lab).
*/
