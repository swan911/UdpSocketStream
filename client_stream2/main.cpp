#include "utils.h"

#include <iostream>
#include <arpa/inet.h>
 
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <vector>

using namespace std;

#define PORT    12348


unsigned char* getDataForSendOne(int& size) {
    int dsize = dataPackage::getMaxDataSize();
    size = 3 * dsize + 10;
    unsigned char* p = new unsigned char[size] {'\0'};
    for (int i = 0; i < size - 1; i++) {
        p[i] = 'y';
    }
    for (int i = 0; i < dsize - 1; i++) {
        p[i] = 'a';
        p[i + dsize] = 'b';
        p[i + 2 * dsize] = 'c';
    }
    return p;
}


void sendDataOne() {
    int send_data_size = 0;
    unsigned char* send_data = getDataForSendOne(send_data_size);
    std::byte id[8];
    for(int i = 0; i < 8; i++)
     id[i] = (std::byte)('a');
    
    std::vector<dataPackage*> packages = getListPackages(send_data, send_data_size, id);
    int size = packages.size();
    for (int i = 0; i < size / 2; i++) {
        sendData(*packages[size - i]);
        sendData(*packages[i]);
    }
    if (size % 2 != 0)
        sendData(*packages[size / 2]);
}


unsigned char* getDataForSendTwo(int& size) {
    int dsize = dataPackage::getMaxDataSize();
    size = 4 * dsize + 20;
    unsigned char* p = new unsigned char[size] {'\0'};
    for (int i = 0; i < size - 1; i++) {
        p[i] = '1';
    }
    for (int i = 0; i < dsize - 1; i++) {
        p[i] = 'x';
        p[i + dsize] = 'y';
        p[i + 2 * dsize] = 'z';
        p[i + 3 * dsize] = 'u';
    }
    return p;
}



void sendDataTwo() {
    int send_data_size = 0;
    unsigned char* send_data = getDataForSendTwo(send_data_size);
    std::byte id[8];
    for (int i = 0; i < 8; i++)
        id[i] = (std::byte)('a');
    id[0] = (std::byte)('b');
    std::vector<dataPackage*> packages = getListPackages(send_data, send_data_size, id);
    int size = packages.size();
    for (int i = 0; i < size/2; i++) {
        sendData(*packages[size - (i+1)]);
        sendData(*packages[i]);
    }
    if (size%2 != 0)
        sendData(*packages[size / 2]);
}

dataPackage* sendToServer(int sockfd, dataPackage* package) {
    dataPackage* ret = nullptr;
    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    int ret_code;
    int n;
    socklen_t len;
    char* buffer = new char[package->size_data_package];
    static int recv_counter = 0;
    while(ret == nullptr) { 
        ret_code = sendto(sockfd, (const char *)getDataForSend(*package), package->data_size + package->getSizeHeader(), MSG_CONFIRM, (const struct sockaddr *) &servaddr,
            sizeof(servaddr));
        std::cout<<"errno="<< errno <<std::endl;
        std::cout<<"ret_code="<< ret_code <<std::endl;
        if(ret_code == -1){
            std::cout<<"ERROR! sendto: resend package"<< n <<std::endl;
            continue;
        }
        std::cout<<"message sent:"<<std::endl;
        std::cout<<"package->id="<< (char)package->id[0]<<std::endl;
        std::cout<<"package->seq_number="<< package->seq_number<<std::endl;
        std::cout<<"package->seq_total="<< package->seq_total<<std::endl;
    
        n = recvfrom(sockfd, (char *)buffer, package->size_data_package, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
        std::cout<<"errno="<< errno <<std::endl;
        std::cout<<"n="<< n <<std::endl;
        if(n == -1){
            std::cout<<"ERROR! recvfrom: resend package"<<std::endl;
            continue;
        }
        ret = readData(buffer, n);
        std::cout<<"Server recv:"<<std::endl;
        std::cout<<"ret->id="<< (char)ret->id[0]<<std::endl;
        std::cout<<"ret->seq_number="<<ret->seq_number<<std::endl;
        std::cout<<"ret->seq_total="<<ret->seq_total<<std::endl;
        std::cout<<"ret->data_size="<<ret->data_size<<std::endl;
        recv_counter++;
        std::cout<<"recv_counter="<<recv_counter<<std::endl;
    }
    return ret;
}

void test1(){
    int send_data_size1 = 0;
    unsigned char* send_data1 = getDataForSendOne(send_data_size1);
    uint32_t check_sum1 = crc32c(crc32c_crc, send_data1, send_data_size1);
    size_t len1 = 0;
    char* buf1 = getBufCheckSum(check_sum1, len1);
    std::cout << "INF buf1="<< buf1 << std::endl;
    std::byte id1[8];
    for (int i = 0; i < 8; i++)
        id1[i] = (std::byte)('a');
    std::vector<dataPackage*> packages1 = getListPackages(send_data1, send_data_size1, id1);
    int size1 = packages1.size();

    int send_data_size2 = 0;
    unsigned char* send_data2 = getDataForSendTwo(send_data_size2);
    uint32_t check_sum2 = crc32c(crc32c_crc, send_data2, send_data_size2);
    size_t len2 = 0;
    char* buf2 = getBufCheckSum(check_sum2, len2);
    std::cout << "INF buf2="<< buf2 << std::endl;
    std::byte id2[8];
    for (int i = 0; i < 8; i++)
        id2[i] = (std::byte)('a');
    id2[0] = (std::byte)('b');
    std::vector<dataPackage*> packages2 = getListPackages(send_data2, send_data_size2, id2);
    int size2 = packages2.size();
    
    int sockfd;
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("setsockopt failed");
            exit(EXIT_FAILURE);
        }
        
    bool lastPackage1 = false;
    dataPackage* recv1 = nullptr;
    bool lastPackage2 = false;
    dataPackage* recv2 = nullptr;
    int recv_count2 = 0;
    for (int i = 0; i < size1 / 2; i++) {
        // 1
        dataPackage* send_pack = packages1[size1 - (i + 1)];
        recv1 = sendToServer(sockfd, send_pack);
        if (recv1->data_size != 0) {
                std::cout << "ERROR WTF4???" << std::endl;
        }
        //2
        dataPackage* send_pack2 = packages2[size1 - (i + 1)];
        recv2 = sendToServer(sockfd, send_pack2);
        std::cout << "recv2->seq_total=" << recv2->seq_total << std::endl;
        if (recv2->data_size != 0) {
            std::cout << "ERROR WTF3???" << std::endl;
        }
        recv_count2++;
        //1
        send_pack = packages1[i];
        recv1 = sendToServer(sockfd, send_pack);
        if (recv1->data_size != 0)
            lastPackage1 = true;
        //2
        send_pack2 = packages2[i];
        recv2 = sendToServer(sockfd, send_pack2);
        std::cout << "recv2->seq_total=" << recv2->seq_total << std::endl;
        if (recv2->data_size != 0)
            lastPackage2 = true;
        recv_count2++;
    }
    bool eq2 = true;
    for (int i = recv_count2; i < size2; i++) {
        dataPackage* send_pack2 = packages2[i];
        recv2 = sendToServer(sockfd, send_pack2);
        std::cout << "recv2->seq_total=" << recv2->seq_total << std::endl;
        if (recv2->data_size != 0) {
            lastPackage2 = true;
            for (size_t i = 0; i < recv2->data_size; i++)
                if (recv2->data[i] != (std::byte)buf2[i]) {
                    std::cout << "ERROR buf2="<< buf2 << std::endl;
                    std::cout << "ERROR recv2->data="<< (char*)recv2->data << std::endl;
                    eq2 = false;
                    break;
                }
        }
    }   
     
    if (!lastPackage1 && size1 % 2 != 0) {
        recv1 = sendToServer(sockfd, packages1[size1 / 2]);
        if (recv1->data_size != 0)
            lastPackage1 = true;
    }
    bool eq1 = true;
    if (!lastPackage1 || recv1 == nullptr || recv1->data_size != len1)
        std::cout << "ERROR WTF2???" << std::endl;
    else {
        for(size_t i = 0; i < recv1->data_size; i++)
            if ((char)recv1->data[i] != buf1[i]) { 
                eq1 = false; 
                std::cout << "ERROR i="<< i << std::endl;
                std::cout << "ERROR (char)recv1->data[i]="<< (char)recv1->data[i] << std::endl;
                std::cout << "ERROR buf1[i]="<< buf1[i] << std::endl;
                std::cout << "ERROR buf1="<< buf1 << std::endl;
                std::cout << "ERROR recv1->data="<< (char*)recv1->data << std::endl;
                break;
            }
    }

    if (!eq1)
        std::cout << "ERROR WTF1???" << std::endl;
   
    close(sockfd);
}
   
int main(int argc, char* argv[]) {
    test1();
    
    return 0;
}



#define MAXLINE 1472
int main13() {
    int sockfd;
    char buffer[MAXLINE];
    const char *hello = "Hello from client";
    struct sockaddr_in     servaddr;
   
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
       
    int n;
    socklen_t len;
    
    int send_data_size1 = 0;
    unsigned char* send_data1 = getDataForSendOne(send_data_size1);
    uint32_t check_sum1 = crc32c(crc32c_crc, send_data1, send_data_size1);
    size_t len1 = 0;
    char* buf1 = getBufCheckSum(check_sum1, len1);
    std::byte id1[8];
    for (int i = 0; i < 8; i++)
        id1[i] = (std::byte)('a');
    std::vector<dataPackage*> packages1 = getListPackages(send_data1, send_data_size1, id1);
    int size1 = packages1.size();
    
    sendto(sockfd, (const char *)getDataForSend(*(packages1[0])), packages1[0]->data_size + packages1[0]->getSizeHeader(),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
    std::cout<<"Hello message sent."<<std::endl;
           
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    //dataPackage* ret = readData(buffer, n);
    //std::cout<<"Server recv:"<<std::endl;
    //std::cout<<"ret->seq_number="<<ret->seq_number<<std::endl;
    //std::cout<<"ret->seq_total="<<ret->seq_total<<std::endl;
    //std::cout<<"ret->data_size="<<ret->data_size<<std::endl;
    
    
    
    
    
    buffer[n] = '\0';
    std::cout<<"Server :"<<buffer<<std::endl;
   
    close(sockfd);
    return 0;
}
