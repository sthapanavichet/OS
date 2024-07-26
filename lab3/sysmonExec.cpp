//sysmonExec.cpp - A system monitor using fork and exec
//
// 13-Jul-20  M. Watler         Created.

#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
const int NUM=2;

int systemMonitor(pid_t childPid[]);
bool isRunning=true;
bool isParent = true;//Distinguishes between the parent
                     //process and the child process(es)
pid_t childPid[NUM];

char *intf[]={"lo", "ens33"};

int main()
{
    cout << endl << "parent:main: pid:"<<getpid()<<endl;
    for(int i=0; i<NUM & isParent; ++i) {
        childPid[i] = fork();
        if(childPid[i]==0) {//the child
            cout << "child:main: pid:"<<getpid()<<endl;
            isParent=false;
            execlp("./intfMonitor", "./intfMonitor", intf[i], NULL);
            cout << "child:main: pid:"<<getpid()<<" I should not get here!"<<endl;
	        cout<<strerror(errno)<<endl;
        }
    }
    if(isParent) {
        sleep(10);
        systemMonitor(childPid);
    }

    cout << "parent:main("<<getpid()<<"): Finished!" << endl;

    return 0;
}

int systemMonitor(pid_t childPid[])//run by the parent process
{
    int status=-1;
    pid_t pid=0;
    
    //TODO: Send start signals to the children (SIGUSR1)
    //TODO: sleep for 30 seconds
    //TODO: Send stop signals to the children (SIGUSR2)

    for(int i = 0; i < NUM; i++) {
        kill(childPid[i], SIGUSR1);
    }

    sleep(30);

    for(int i = 0; i < NUM; i++) {
        kill(childPid[i], SIGUSR2);
    }

    //Wait for children to terminate
    while(pid>=0) {
        pid=wait(&status);//blocking. waitpid() is non-blocking and
                          //waits for a specific pid to terminate
        if(pid != -1) cout << "parent:systemMonitor: status:"<<status<<". The child pid:"<<pid<<" has finished"<< endl;
    }

    return 0;
}
