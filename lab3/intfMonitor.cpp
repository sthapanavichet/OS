//intfMonitor_solution.cpp - An interface monitor
//
// 13-Jul-20  M. Watler         Created.

#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

using namespace std;

const int MAXBUF=128;
bool isRunning=false;
const int SIGNALNUM = 4;

static void signalHandler(int);

//TODO: Declare your signal handler function prototype

int main(int argc, char *argv[])
{
    //TODO: Declare a variable of type struct sigaction
    //For sigaction, see http://man7.org/linux/man-pages/man2/sigaction.2.html
	struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    const int signals[SIGNALNUM] = {SIGINT, SIGUSR1, SIGUSR2, SIGTSTP};

    char interface[MAXBUF];
    char statPath[MAXBUF];
    const char logfile[]="Network.log";//store network data in Network.log
    int retVal=0;

    //TODO: Register signal handlers for SIGUSR1, SIGUSR2, ctrl-C and ctrl-Z
    //TODO: Ensure there are no errors in registering the handlers
    for(int i = 0; i < SIGNALNUM; i++) {
        if(sigaction(signals[i], &action, NULL) == -1) {
            cout<< "Cannot create the signal handler " << signals[i] <<endl;
            cout<< strerror(errno) <<endl;
            return -1;
        }
    }

    strncpy(interface, argv[1], MAXBUF);//The interface has been passed as an argument to intfMonitor
    int fd=open(logfile, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    cout<<"intfMonitor:main: interface:"<<interface<<":  pid:"<<getpid()<<endl;

    //TODO: Wait for SIGUSR1 - the start signal from the parent
    while(!isRunning);

    while(isRunning) {
        //gather some stats
        int tx_bytes=0;
        int rx_bytes=0;
        int tx_packets=0;
        int rx_packets=0;
	    ifstream infile;
            sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_bytes;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_bytes;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_packets;
	        infile.close();
	    }
            sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_packets;
	        infile.close();
	    }
	    char data[MAXBUF];
	    //write the stats into Network.log
	    int len=sprintf(data, "%s: tx_bytes:%d rx_bytes:%d tx_packets:%d rx_packets: %d\n", interface, tx_bytes, rx_bytes, tx_packets, rx_packets);
	    write(fd, data, len);
	    sleep(1);
    }
    close(fd);

    return 0;
}

//TODO: Create a signal handler that starts your program on SIGUSR1 (sets isRunning to true),
//      stops your program on SIGUSR2 (sets isRunning to false),
//      and discards any ctrl-C or ctrl-Z.
//
//      If the signal handler receives a SIGUSR1, the following message should appear on the screen:
//      intfMonitor: starting up
//
//      If the signal handler receives a ctrl-C, the following message should appear on the screen:
//      intfMonitor: ctrl-C discarded
//
//      If the signal handler receives a ctrl-Z, the following message should appear on the screen:
//      intfMonitor: ctrl-Z discarded
//
//      If the signal handler receives a SIGUSR2, the following message should appear on the screen:
//      intfMonitor: shutting down
//
//      If the signal handler receives any other signal, the following message should appear on the screen:
//      intfMonitor: undefined signal

static void signalHandler(int signum) {
	switch(signum) {
        case SIGINT:
            cout<<"signalHandler("<<signum<<"): SIGINT (Ctrl+C)"<<endl;
            cout<<"Program is continuing!"<<endl;
	    break;
        case SIGUSR1:
            cout<<"signalHandler("<<signum<<"): SIGUSR1"<<endl;
            cout << "program is starting" << endl;
            isRunning = true;
	    break;
        case SIGUSR2:
            cout<<"signalHandler("<<signum<<"): SIGUSR2"<<endl;
            cout << "Program is closing" << endl;
            isRunning=false;
	    break;
        case SIGTSTP:
            cout<<"signalHandler("<<signum<<"): SIGTSTP (Ctrl+Z)"<<endl;
            cout<<"Program is continuing!"<<endl;
	    break;
        default:
            cout<<"signalHandler("<<signum<<"): Undefined signal"<<endl;
    }
}
