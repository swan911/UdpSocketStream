#include "manager.h"

#include <iostream>
// inet_addr
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
#include <chrono>
#include <thread>


using namespace std;
/*
int main(int argc, char **argv) {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
*/
#define PORT    12348


#define MAXLINE 1472
   
int main() {
    ManagerPackages mng;
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
       
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
       
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    socklen_t len;
  int n;
   
    len = sizeof(cliaddr);  //len is value/result
    int counter = 0;
   while(true){
    /*   if(counter % 3 == 0){
            std::cout<<"!!!sleep"<<std::endl;           
           std::this_thread::sleep_for(std::chrono::seconds(10));
           std::cout<<"!!! after sleep"<<std::endl;           
       }*/
       counter++;
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
    dataPackage* recv = readData(buffer, n);
    std::cout<<"Client recv:"<<std::endl;
    std::cout<<"ret->seq_number="<<recv->seq_number<<std::endl;
    std::cout<<"ret->seq_total="<<recv->seq_total<<std::endl;
    std::cout<<"ret->data_size="<<recv->data_size<<std::endl;
    
    dataPackage* send_package = mng.getReplyDataPackage(recv);
    char* data = getDataForSend(*send_package);
    int size = send_package->data_size + send_package->getSizeHeader();
    
    std::cout<<"Server send:"<<std::endl;
    std::cout<<"send_package->id="<<(char)send_package->id[0] <<std::endl;
    std::cout<<"send_package->seq_number="<<send_package->seq_number<<std::endl;
    std::cout<<"send_package->seq_total="<<send_package->seq_total<<std::endl;
    std::cout<<"send_package->data_size="<<send_package->data_size<<std::endl;
    std::cout<<"counter="<<counter<<std::endl;
    if(send_package->seq_total == recv->seq_total)
        std::cout<<"!!!!!send_package->data="<<(char*)send_package->data<<std::endl;
    
    
    sendto(sockfd, (const char *)data, size, 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
   }
   close(sockfd);
       
    return 0;
}




























