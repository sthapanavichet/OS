#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

using namespace std;

const int PORT = 1154;
const char IP_ADDR[] = "127.0.0.1";
const int BUF_LEN = 2048;
bool is_running = true;
pthread_mutex_t lock_x;
int sockfd;
ofstream logFile("server_log.txt", ios::out | ios::app);
struct sockaddr_in myaddr, remaddr;
socklen_t addrlen = sizeof(remaddr);

void *ReceiveThread(void *arg) {
    char buf[BUF_LEN];
    while (is_running) {
        memset(buf, 0, BUF_LEN);
        int recvlen = recvfrom(sockfd, buf, BUF_LEN, 0, (struct sockaddr *)&remaddr, &addrlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            pthread_mutex_lock(&lock_x);
            logFile << buf << endl;
            logFile.flush(); 
            pthread_mutex_unlock(&lock_x);
        } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            usleep(100000); // 100 ms
        }
    }
    pthread_exit(NULL);
}

void signalHandler(int signum) {
    is_running = false;
}

void dumpLogFile() {
    ifstream logFileRead("server_log.txt", ios::in);
    if (!logFileRead.is_open()) {
        cout << "Failed to open log file for reading." << endl;
        return;
    }

    cout << "Log file contents:" << endl;
    stringstream buffer;
    buffer << logFileRead.rdbuf();
    cout << buffer.str();
    logFileRead.close();
    cout << "End of log file. The file will now be emptied." << endl;

    // Empty the log file after dumping
    ofstream clearLog("server_log.txt", ios::out | ios::trunc);
    clearLog.close();
}

void setLogLevel() {
    cout << "Enter new log level (0-DEBUG, 1-WARNING, 2-ERROR, 3-CRITICAL): ";
    int level;
    cin >> level;
    if (level < 0 || level > 3) {
        cout << "Invalid log level." << endl;
        return;
    }

    char buf[BUF_LEN];
    int len = snprintf(buf, BUF_LEN, "Set Log Level=%d", level);
    if (sendto(sockfd, buf, len, 0, (struct sockaddr *)&remaddr, addrlen) < 0) {
        perror("sendto failed");
    }
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Cannot create socket\n";
        return -1;
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK); // Set the socket to non-blocking

    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    myaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        cerr << "Bind failed\n";
        return -1;
    }

    pthread_t thread;
    pthread_mutex_init(&lock_x, NULL);

    if (pthread_create(&thread, NULL, &ReceiveThread, NULL) != 0) {
        cerr << "Failed to create receive thread\n";
        return -1;
    }

    signal(SIGINT, signalHandler); // Setup signal handler for graceful shutdown

    int choice;
    while (is_running) {
        cout << "\nServer Menu:\n1. Set log level (dummy option, not implemented)\n2. Dump log file\n0. Exit\nSelect option: ";
        cin >> choice;
        switch (choice) {
            case 1:
                setLogLevel();
                break;
            case 2:
                dumpLogFile();
                break;
            case 0:
                cout << "Server shutting down" << endl;
                is_running = false;
                break;
            default:
                cout << "Invalid option." << endl;
        }
    }

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&lock_x);
    close(sockfd);
}

