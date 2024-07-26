// client1.cpp - A client that communicates with a second client using triple RSA encrpytion/decryption
#include <arpa/inet.h>
#include <iostream>
#include <math.h>
#include <net/if.h>
#include <netinet/in.h>
#include <queue>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

const char IP_ADDR[]="127.0.0.1";
const int BUF_LEN=256;
bool is_running = true;
int srcPort=1153;
int destPort=1155;
// Encrpytion/Decryption variables
double n;
double e;
double d;
double phi;

queue<unsigned char*> messageQueue;

pthread_mutex_t lock_x;
struct sockaddr_in cl1addr, cl2addr;

void *recv_func(void *arg);
void *send_func(void *arg);

static void shutdownHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            is_running=false;
            break;
    }
}

// Returns a^b mod c
unsigned char PowerMod(int a, int b, int c)
{
    int res = 1;
    for(int i=0; i<b; ++i) {
        res=fmod(res*a, c);
    }
    return (unsigned char)res;
}
  
// Returns gcd of a and b
int gcd(int a, int h)
{
    int temp;
    while (1)
    {
        temp = a%h;
        if (temp == 0)
          return h;
        a = h;
        h = temp;
    }
}
  
// Code to demonstrate RSA algorithm
int main()
{
    // Two random prime numbers
    double p = 11;
    double q = 23;
  
    // First part of public key:
    n = p*q;
  
    // Finding other part of public key.
    // e stands for encrypt
    e = 2;
    phi = (p-1)*(q-1);
    while (e < phi)
    {
        // e must be co-prime to phi and
        // smaller than phi.
        if (gcd((int)e, (int)phi)==1)
            break;
        else
            e++;
  
    }
    // Private key (d stands for decrypt)
    // choosing d such that it satisfies
    // d*e = 1 + k * totient
    int k = 2;  // A constant value
    d = (1 + (k*phi))/e;
    cout<<"p:"<<p<<" q:"<<q<<" n:"<<n<<" phi:"<<phi<<" e:"<<e<<" d:"<<d<<endl;

    signal(SIGINT, shutdownHandler);

    //TODO: Complete the rest
    int sockfd, ret;
    socklen_t socklen = sizeof(cl1addr);
    cout << "Client 1 is starting up" << endl;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    // Non-blocking socket
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        perror("Error setting socket to non-blocking mode");
        close(sockfd);
        return 1;
    }
    
    // setting up cl1addr
    memset(&cl1addr, 0, socklen);
    cl1addr.sin_family = AF_INET; // IPv4
    cl1addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    cl1addr.sin_port = htons(srcPort);

    // setting up cl2addr
    memset(&cl2addr, 0, socklen);
    cl2addr.sin_family = AF_INET; // IPv4
    cl2addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    cl2addr.sin_port = htons(destPort);

    cout << "Binding socket with server address" << endl;
    // Binding socket with server address
    if (bind(sockfd, (struct sockaddr *)&cl1addr, socklen) < 0) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    pthread_t recv_tid, send_tid;
    if (pthread_create(&recv_tid, NULL, recv_func, (void*) &sockfd) < 0) {
        is_running = false;
        close(sockfd);
        perror("Thread creation failed");
        return -1;
    }
    cout << "Receive thread is created" << endl;
    
    sleep(5);
    
    if (pthread_create(&send_tid, NULL, send_func, (void*) &sockfd) < 0) {
        is_running = false;
        close(sockfd);
        perror("Thread creation failed");
        return -1;
    }
    cout << "Send thread is created" << endl;

    unsigned char* decrypted;

    while(is_running) {
        pthread_mutex_lock(&lock_x);
        if (messageQueue.empty() == 0) {
            decrypted = messageQueue.front();
            messageQueue.pop();
            cout << "Client 1 recieved: " << decrypted << endl;
            if(strncmp((const char*) decrypted, "Quit", 4) == 0) {
                cout << "Client 1 is quitting" << endl;
                is_running = false;
            }
        }
        pthread_mutex_unlock(&lock_x);
        sleep(1);
    }
    
    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);
}

void* recv_func(void* arg) {
    int sockfd = *(int *) arg;
    struct sockaddr_in cl2addr;
    socklen_t socklen;
    double encrypted[BUF_LEN];
    unsigned char decrypted[BUF_LEN];
    while(is_running) {
        int ret = recvfrom(sockfd, encrypted, sizeof(encrypted) * BUF_LEN, 0, (struct sockaddr *)&cl2addr, &socklen);
        if(ret < 0) {
            sleep(1);
        }
        else {
            for(int i = 0; i < BUF_LEN; i++) {
                if(encrypted[i] != 0) {
                decrypted[i] = PowerMod(encrypted[i], d, n);
                }
                else {
                    decrypted[i] = '\0';
                    break;
                }
            }
            cout << "Client 1 received: " << decrypted << endl;
            if(strncmp((const char*) decrypted, "Quit", 4) == 0) {
                cout << "Client 1 is quitting" << endl;
                is_running = false;
            }
        }
        
    }
    pthread_exit(NULL);
}

void *send_func(void *arg) {
    const int numMessages=5;
    const unsigned char messages[numMessages][BUF_LEN]={
	    "House? You were lucky to have a house!",
	    "We used to live in one room, all hundred and twenty-six of us, no furniture.",
	    "Half the floor was missing;",
	    "we were all huddled together in one corner for fear of falling.",
        "Quit"};
    int sockfd = *(int *)arg;
    for(int i = 0; i < numMessages; i++) {
        const int LEN = strlen((const char*)messages[i]);
        double encrypted[LEN+1];
        for(int j = 0; j < LEN; j++) {
            encrypted[j] = PowerMod((double)messages[i][j], e, n);
        }
        encrypted[LEN] = 0;
        int len = sizeof(double) * (LEN+1);
        pthread_mutex_lock(&lock_x);
        int ret = sendto(sockfd, &encrypted, len, 0, (struct sockaddr *)&cl2addr, sizeof(cl1addr));
        pthread_mutex_unlock(&lock_x);
        cout << "Client 1 sending: " << messages[i] << endl;
        if(ret < 0) {
            perror("sendto failed");
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

//TODO: Complete the receive thread

/*
Question 1: When n is too small, computers can try to brute force guess p and q
until they get it right. The way we encrpyt text in this lab is very flawed, because
the attacker can try to associate the encrypted ascii value to its real value and 
try to guess what each character's encrypted number is. A better way off encrpyting 
would be encrypting a word or a certain amount of letters at a time. This way it is 
harder to guess what each value represent.

Question 2: The strerngth of RSA encryption relies on the ability of a computer 
to not be able to guess what the factors of n is; by making n really big, it would 
make it impossible for any kind of computer to be able to find the factors because 
it would take too long to guess all the combinations. A quantum computer can perform 
many calculations at the same time, which makes the guessing significantly faster 
than normal computers.

Question 3: For very large values of p and q and the large data spaces involved, it
would be more efficient to perform the encryption using hardware since it is possible.
Operations like exponentiation and modulus can be performed by FPGAs much faster than 
software can. But operations like choosing ps and qs and their coprimes is impossible
to achieve without using software, so only some part of the encryption can be implemented
in hardware directly.
*/