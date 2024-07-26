#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

char socket_path[] = "/tmp/lab4";

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    char buf[100];
    int fd,rc;
    bool isRunning = true;

    memset(&addr, 0, sizeof(addr));
    //Create the socket
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        cout << "client: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    cout << "client: addr.sun_path: " << addr.sun_path << endl;

    cout << "client: connect()" << endl;
    //Connect to the local socket
    cout << "Atempting connection to server" << endl;
    while(true) {
        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            cout << "client: " << strerror(errno) << endl;
            sleep(5);
        }
        else break;
    }

    cout << "connected to server" << endl;
    while(isRunning) {
		rc=read(fd, buf, sizeof(buf));
        cout << "client: read(" << buf << ")" << endl;
        
        if(strncmp("quit", buf, 4)==0) {
            isRunning = false;
        }   

        if(strncmp("pid", buf, 3)==0) {
            char message[40];
            int message_length = sprintf(message, "This client has pid %d", getpid());
            message_length++;
            if((rc =write(fd, message, message_length)) != message_length) {
                if (rc > 0) fprintf(stderr, "partial write");
                else {
                    cout << "client: " << strerror(errno) << endl;
                    close(fd);
                    exit(-1);
                }
            }
        }

        if(strncmp("sleep", buf, 5)==0) {
            sleep(5);
            cout << "Client is sleeping for 5s" << endl;
            if((rc = write(fd, "done", 5)) != 5) {
                if (rc > 0) fprintf(stderr, "partial write");
                else {
                    cout << "client: " << strerror(errno) << endl;
                    close(fd);
                    exit(-1);
                }
            }
        }
    }
   
    cout << "client: close(fd)" << endl;
    close(fd);
    return 0;
}
