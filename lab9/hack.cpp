// hack.cpp - A program that attempts to break the RSA encryption while varying the
//            priority of the current process
//
//
#include <fstream>
#include <iostream>
#include <math.h>
#include <sched.h>
#include <sys/resource.h>

using namespace std;

bool DeterminePrimes();

// Code to hack the RSA algorithm
int main()
{
    int ret=0;
    int retVal=0;
    const long nanosecsPerSecond=1000000000;
    timespec startTime, endTime;
    double totalTime;

    //TODO: Open a file
    ofstream outputFile("hackTime.dat");

    if (!outputFile.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    //TODO: Call DeterminePrimes() 20 times while varying the
    //      priority from -20 to +19
    //TODO: For each call to DeterminePrimes() measure the time
    //      taken to crack the modulus n of the RSA scheme.
    //TODO: Use the high resolution timing function clock_gettime()
    //      which gives time to the nanosecond.
    //      Store the time before the call to DeterminePrimes() and
    //      after the call to DeterminePrimes(). The difference is the
    //      time it took to hack them modulus n of the RSA scheme.
    //TODO: Store the hack time of each call in a file
    //TODO: You will make a plot of hack-time vs nice value

    char buf[256];
    for (int niceVal = -20; niceVal <= 19; ++niceVal) {
        setpriority(PRIO_PROCESS, 0, niceVal);
        clock_gettime(CLOCK_REALTIME, &startTime); // Start timing
        DeterminePrimes();
        clock_gettime(CLOCK_REALTIME, &endTime); // End timing
        totalTime = (double)(endTime.tv_sec-startTime.tv_sec)*nanosecsPerSecond + (endTime.tv_nsec-startTime.tv_nsec);
        outputFile << niceVal << "," << totalTime << endl;
    }
    //TODO: Be sure to close your file when done
    outputFile.close();

    return ret;
}

bool DeterminePrimes() {
    unsigned long long n=5335952767283123;//49182449ull * 108493027ull
    unsigned long long p=2;
    unsigned long long q=0;
    long double temp;
    bool found=false;
    while(!found && p<n) {
        temp = (long double)n/p;
        q = n/p;
	    if(temp==(long double)q) found=true;//no numbers after the decimal place
                                            //means we have found a factor
	    else ++p;//Usually we would call a function to get the next prime number
	             //instead of simply incrementing by one. But such a function does
                 //basically the same thing as we are doing here.
    }
    if(found) {
        cout<<"p:"<<p<<" q:"<<q<<" n:"<<n<<endl;
    } else {
        cout<<"factors not found for n:"<<n<<endl;
    }
    return found;
}


/*
Question 1: It would take a very long time to crack a 1024 bit key since it already takes 0.5s to crack an 8 bytes (32 bit). 
And not to mention that the time it takes would increase exponentially instead of linearly so it would take many lifetimes of
computation to crack a 1024 bit key.
Question 2: It took 3.97952e+08 nanoseconds (0.4s)
Question 3: It took 4.08636e+09 nanoseconds (4s)
Quesstion 4: It took 2.79673e+11 nanoseconds which is 270s (3mins)
*/