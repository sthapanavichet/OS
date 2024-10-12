#include "DataAcquisition.h"

using namespace std;

struct DataCenter dataCenters[MAX_CLIENT];
string blockedIP[MAX_CLIENT];
string recentClients[3];
int dataCenterNo = 0;
int blockDataCenterNo = 0;
pthread_mutex_t lock;
queue<DataPacket> seismicData;

bool is_running;

static void sigHandler(int sig)
{
    switch(sig) {
        case SIGINT:
            cout << "Exiting" << endl;
            is_running=false;
	    break;
    }
}

int main() {
    // signal handling
    struct sigaction action;
    action.sa_handler = sigHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    // setting up sockets
    int fd;
    struct sockaddr_in servaddr;
    socklen_t socklen = sizeof(servaddr);
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }
    memset(&servaddr, 0, socklen);
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    servaddr.sin_port = htons(PORT);

    if (bind(fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Error Binding socket");
        return -1;
    }

    // initializing list of subscribers
    for(int i = 0; i < MAX_CLIENT; i++) {
        dataCenters[i].username[0] = 0;
    }

    // mutex and thread initialization
    pthread_t tid_read, tid_write; 
    pthread_mutex_init(&lock, NULL);
    is_running=true;
    if(pthread_create(&tid_read, NULL, read_thread, (void*)&fd) != 0) {
        is_running = false;
        perror("Read thread creation failed");
        close(fd);
        return -1;
    }
    if(pthread_create(&tid_write, NULL, write_thread, (void*)&fd) != 0) {
        is_running = false;
        perror("Write thread creation failed");
        close(fd);
        return -1;
    }

    // memory reading and semaphore initialization
    key_t ShmKey = ftok(MEMNAME, 65);
    int ShmID = shmget(ShmKey, sizeof(struct SeismicMemory), IPC_CREAT | 0666);
    struct SeismicMemory *ShmPTR = (struct SeismicMemory*) shmat(ShmID, NULL, 0);
    sem_t *sem_id = sem_open(SEMNAME, O_CREAT, 0666, 0);
    while(is_running) {
        cout << "dataPacket.size():" << seismicData.size() << " client.size():" << dataCenterNo << endl;
        int packetNo = ShmPTR->packetNo;
        for(int i = 0; i < packetNo; i++) {
            sem_wait(sem_id);
            if(ShmPTR->seismicData[i].status == WRITTEN) {
                struct DataPacket packet;
                packet.packetLen = ShmPTR->seismicData[i].packetLen;
                packet.packetNo = ShmPTR->packetNo;
                strcpy(packet.data, ShmPTR->seismicData[i].data);
                pthread_mutex_lock(&lock);
                seismicData.push(packet);
                pthread_mutex_unlock(&lock);
                ShmPTR->seismicData[i].status = READ;
            }
            sem_post(sem_id);
        }
        sleep(1);
    }
    // exiting
    cout << "Data Acquisition Unit: exiting" << endl;
    // close file descriptors
    close(fd);
    // join threads
    pthread_join(tid_read, NULL);
    pthread_join(tid_write, NULL);
    // destroy mutex
    pthread_mutex_destroy(&lock);
    // detach shared memory
    shmdt((void*)ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    // unlink semaphores
    sem_close(sem_id);
    sem_unlink(SEMNAME);
}

string sockaddr_toip(struct sockaddr_in addr) {
    char* ip_addr = inet_ntoa(addr.sin_addr);
    int port = ntohs(addr.sin_port);
    char buf[64];
    int ret = sprintf(buf, "%s:%d", ip_addr, port);
    return string(buf, ret);
}

void* read_thread(void* arg) {
    int fd = *(int*)arg;
    char buf[BUF_LEN];
    int clientNo = 0;
    char *token;
    char *username;
    socklen_t socklen = sizeof(struct sockaddr_in);
    struct sockaddr_in claddr;
    while(is_running) {
        memset(&claddr, 0, sizeof(claddr));
        int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&claddr, &socklen);
        if(len > 0) {
            string client_ip = sockaddr_toip(claddr);
            bool blocked = false;
            for(int i = 0; i < blockDataCenterNo; i++) {
                if(client_ip == blockedIP[i]) {
                    blocked = true;
                    break;
                }
            }
            if(!blocked) {
                command(fd, buf, &claddr);
                recentClients[clientNo] = client_ip;
                security_protocol();
                // cycle through slots in recentClients
                clientNo = (clientNo + 1) % 3;
            }
            memset(buf, 0, BUF_LEN);
        }
    }
    pthread_exit(NULL);
}

void* write_thread(void* arg) {
    int fd = *(int*)arg;
    int len;
    char buf[BUF_LEN];
    socklen_t socklen = sizeof(struct sockaddr_in);
    while(is_running) {
        // Check if there is any data in the queue
        if (!seismicData.empty()) {
            pthread_mutex_lock(&lock);
            DataPacket packet = seismicData.front();
            seismicData.pop();
            pthread_mutex_unlock(&lock);
            memcpy(buf, &packet.packetNo, 2);
            memcpy(buf + 2, &packet.packetLen, 1);
            memcpy(buf + 3, &packet.data, packet.packetLen);

            // Send the packet to all subscribed data centers
            for (int i = 0; i < MAX_CLIENT; i++) {
                if (dataCenters[i].username[0] != 0) {
                    len = sendto(fd, buf, BUF_LEN, 0, (struct sockaddr *)&dataCenters[i].addr, socklen);
                    if(len == -1) perror("Write Thread: sendto operation failed");
                    cout << "Send Thread: " << sockaddr_toip(dataCenters[i].addr) << endl;
                }
            }
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

void security_protocol() {
    // check for ddos attack and password brute forcing.
    bool block = true;
    for(int i = 0; i < 2; i++) {
        if(recentClients[i] != recentClients[i+1]) {
            block = false;
            break;
        }
    }
    // add to empty slot of block list
    if(block) {
        bool added = false;
        for(int i = 0; i < blockDataCenterNo; i++) {
            if(recentClients[0] == blockedIP[i])  {
                added = true;
                break;
            }
        }
        if(!added) {
            for(int i = 0; i < dataCenterNo; i++) { // if rogue client is subscribed, remove from list
                if(recentClients[0] == sockaddr_toip(dataCenters[i].addr)) {
                    pthread_mutex_lock(&lock);
                    dataCenters[i].username[0] = 0;
                    dataCenterNo--;
                    pthread_mutex_unlock(&lock);
                }
            }
            blockedIP[blockDataCenterNo] = recentClients[0];
            blockDataCenterNo++;
            cout << "Adding " << recentClients[0] << " to the rogue client list" << endl;
        }
    }
}

void command(int fd, char* buf, struct sockaddr_in *claddr) {
    char* token;
    char* username;
    token = strtok(buf, ",");
    if(strcmp(token, "Subscribe") == 0) {
        if(dataCenterNo == MAX_CLIENT) return; // if data unit has maximum clients, exit (no more subscription)
        token = strtok(NULL, ",");
        username = token;
        token = strtok(NULL, ",");
        if(strcmp(token, PASSWORD) == 0) { // check password
            bool subscribed = false;
            for(int i = 0; i < dataCenterNo; i++) { // check if user is alreaday subscribed
                if(strcmp(username, dataCenters[i].username) == 0) {
                    cout << username << " already subscribed" << endl;
                    subscribed = true;
                }
            }
            if(!subscribed) { // add to list if not subscribed
                int len = sendto(fd, "Subscribed", 11, 0, (struct sockaddr *)claddr, sizeof(struct sockaddr_in));
                if(len == -1) perror("Read_thread: sendto operation failed");
                cout << "Data center " << username << " has subscribed." << endl;
                struct DataCenter datacenter;
                strcpy(datacenter.username, username);
                datacenter.addr = *claddr;

                bool added = false;
                // add to empty slots
                for(int i = 0; i < dataCenterNo; i++) {
                    if(dataCenters[i].username[0] == 0) {
                        pthread_mutex_lock(&lock);
                        dataCenters[i] = datacenter;
                        dataCenterNo++;
                        pthread_mutex_unlock(&lock);
                        added = true;
                    }
                }
                if(!added) {
                    pthread_mutex_lock(&lock);
                    dataCenters[dataCenterNo] = datacenter;
                    dataCenterNo++;
                    pthread_mutex_unlock(&lock);
                }
            }
        }
    }
    else if(strcmp(token, "Cancel") == 0) { // cancel subscription
        token = strtok(NULL, ",");
        username = token;
        for(int i = 0; i < MAX_CLIENT; i++) {
            if(strcmp(username, dataCenters[i].username) == 0) { // check if user is in subscribed list
                cout << "Data center " << username << " has unsubscribed." << endl;
                pthread_mutex_lock(&lock);
                dataCenters[i].username[0] = 0;
                dataCenterNo--;
                pthread_mutex_unlock(&lock);
            }
        }
    }
    else {
        cout << "Unkown Command: " << token << endl;
    }
}