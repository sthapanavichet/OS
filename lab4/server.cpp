#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>

using namespace std;

char socket_path[] = "/tmp/lab4";

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    char buf[100];
    int fd, client, rc;
    bool isRunning = true;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;

    //Create the socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    //Set the socket path to a local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
    cout << "server: addr.sun_path:" << addr.sun_path << endl;
    unlink(socket_path);

    //Bind the socket to this local socket file
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        cout << "server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    //Listen for a clientient to connect to this local socket file
    if (listen(fd, 1) == -1) {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }

    cout << "Waiting for the client..." << endl;
    //Accept the clientient's connection to this local socket file
    if((client = accept(fd, NULL, NULL)) == -1) {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }

    cout << "server: accept()" << endl;
    cout << "client connected to the server" << endl;

    cout << "Sending Pid Request" << endl;
    if ((rc = write(client, "pid", 4)) != 4) {
        if (rc > 0) fprintf(stderr, "partial write");
        else {
            cout << "client: " << strerror(errno) << endl;
            close(fd);
            exit(-1);
        }
    }

    // read the data from the pid request
    read(client, buf, sizeof(buf));
    cout << buf << endl;

    cout << "Sending Sleep Request" << endl;
    if ((rc = write(client, "sleep", 6)) != 6) {
        if (rc > 0) fprintf(stderr, "partial write");
        else {
            cout << "client: " << strerror(errno) << endl;
            close(fd);
            exit(-1);
        }
    }

    while(true) {
        read(client, buf, sizeof(buf));
        if(strncmp("done", buf, 4)==0) break;
    }

    cout << "Sending Quit Request" << endl;
    if ((rc = write(client, "quit", 5)) != 5) {
        if (rc > 0) fprintf(stderr, "partial write");
        else {
            cout << "client: " << strerror(errno) << endl;
            close(fd);
            exit(-1);
        }
    }
    cout << "server: close(fd), close(client)" << endl;
    unlink(socket_path);
    close(fd);
    close(client);
    return 0;
}
