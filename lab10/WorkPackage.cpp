#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "WorkPackage.h"

using namespace std;

int main() {
    int selection;
    double amount;
    const int MEM_SIZE = sizeof(WorkPackage);
    const char WorkData[] = "WorkData.bin";
    struct stat sb;
    bool fileExisted = true;
    WorkPackage *addr;

    if(stat(WorkData, &sb) == -1) {
        fileExisted = false;
    }

    int openFlags = O_RDWR | O_CREAT;
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;/* rw-rw-rw- */
    int fd=open(WorkData, openFlags, filePerms);
    if(fd == -1) {
	    cout<< strerror(errno) <<endl;
        return -1;
    }

    if(!fileExisted) {  // initialize memory after file creation.
        char buf[MEM_SIZE];
        memset(buf,0,MEM_SIZE);
        write(fd, buf, MEM_SIZE);
    }
    
    addr = (WorkPackage *)mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(addr == MAP_FAILED) {
        cout << strerror(errno) << endl;
        return -3;
    }
    close(fd);  // file descriptor is not needed after mapping
                

    do {
        system("clear");
        cout << "WORK PACKAGE" << endl;
        cout << "Entries: " << addr->entries<< endl << "Hours: " << addr->hours << endl << "Balance: $" << addr->balance << endl << endl;
        cout << "Make a selection:" << endl;
        cout << "1.Log hours" << endl;
        cout << "2.Add Expense" << endl;
        cout << "3.Add Deposit" << endl;
        cout << "4.Refresh" << endl;
        cout << "5.Clear Data" << endl;
        cout << "0.Quit" << endl << endl;
        cout <<  "Selection: ";
        cin>>selection;
        
        switch(selection) {
        case 1:
            cout << "Enter hour for the new entry: ";
            cin >> amount;
            addr->hours += amount;
            addr->entries += 1;
            msync(addr, sb.st_size, MS_SYNC);
            break;
        case 2:
            cout << "Enter the amount of expense: $";
            cin >> amount;
            addr->balance -= amount;
            msync(addr, sb.st_size, MS_SYNC);
            break;
        case 3:
            cout << "Enter the deposit amount: $";
            cin >> amount;
            addr->balance += amount;
            msync(addr, sb.st_size, MS_SYNC);
            break;
        case 4:
            cout << "Refreshed";
            msync(addr, sb.st_size, MS_SYNC);
            break;
        case 5:
            memset(addr, 0, MEM_SIZE);
            msync(addr, sb.st_size, MS_SYNC);
            break;
        case 0:
            cout<< "Exiting"<< endl;
            break;
        default:
            cout<< "Invalid Selection" << endl;
            sleep(1);
        }

    } while (selection != 0);

    munmap(addr, sb.st_size);

    return 0;
}

/* 
Question 1: 
Similarities:
- Both memory mapping and shared memory is used for inter-process communication.
- Both require synchronization mechanisms, such as semaphores or mutexes.

Differences:
- In memory mapping, a file is mapped into the address space of a process, 
allowing the process to read from or write to the mapped memory region. While
shared memory creates a region of memory that is shared between 
multiple processes without needing a file. 
- Access control in memory mapping is managed through file permissions. While 
access to the shared memory segment is controlled using permissions and keys.
- Memory mapping is commonly used for accessing large files efficiently and 
shared memory is commonly used for fast communication and data sharing between 
processes.

Question 2:
I would use memory mappings when i want to store that data for long term and 
when read/write needs to be fast.

Question 3:
I would use memory sharing when i want to share data between multiple processes
and it's only short term because memory sharing doesn't last after the processes
is done.
*/