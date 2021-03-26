#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "fileinfo.h"
#define MYPORT "18005"
#define MAXBUFLEN 100

struct packet {
    char data[41];
};

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



int recv_data(int* sock, char* buffer, int bytes, const struct sockaddr* source_addr, socklen_t* source_len, unsigned char* index) {
    int bytes_received = 0;
    char* ackPacket = malloc(1);
    *ackPacket = 'a';
    int clean_packet = 0;
    struct data_packet* recv_packet = malloc(sizeof(struct data_packet));
    //char* rp = malloc(sizeof(struct data_packet));

    while(bytes_received != sizeof(struct data_packet)) {
        bytes_received = recvfrom(*sock,recv_packet,sizeof(struct data_packet),0,source_addr,source_len);
        if(bytes_received == -1) {
            printf("Error: Incorrect Packet Size %d\n", bytes_received);
        } else if(bytes_received != sizeof(struct data_packet)) {
            printf("Error: Incorrect Packet Size %d\n", bytes_received);
        }
    }
    if((!check_packet_integrity(recv_packet)) || (*index < recv_packet->index)) {
        printf("Error: Packet Compromised\n");
        return -1;
    } else if(*index > recv_packet->index) {
        printf("Error Incorrect Index: expected %d, received %d\n",*index,recv_packet->index);
        return -2;
    } else {
        read_data_from_packet(recv_packet,buffer,bytes);
        (*index)++;
        return 0;
    }
}

int main(int argc, char* argv[]) {

    if(argc < 3) {printf("Format: recvfile -p <recv_port>\n"); return -1;}

    char* port = argv[2];

    int sock;

    struct addrinfo hints, *p;

    struct sockaddr_storage their_addr;

    socklen_t addr_len;


    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    struct addrinfo *servinfo; //Server Address
    if((rv = getaddrinfo(NULL,port,&hints,&servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    //printf("Server IP Address: %s\n",servinfo->ai_addr->sin_addr->s_addr);
    for(p = servinfo; p!= NULL; p= p->ai_next) {
        if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if(bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("listener: bind");
            continue;
        }
        break;
    }

    if(p == NULL) {
        fprintf(stderr, "recvfile: failed to bind socket\n");
        return 2;
    }

    //freeaddrinfo(servinfo);

    printf("recvfile: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;

    /**Receive File Name Length*/
    unsigned char index = 0;
    long unsigned int* file_name_len = malloc(8);
    while(recv_data(&sock,file_name_len,8,(struct sockaddr*)&their_addr,&addr_len, &index) != 0) printf("Re-Receiving Packet\n");
    void* ack = malloc(1);
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    printf("Filename Length: %d\n",*file_name_len);

    /**Receive File Name*/
    const char* file_name = malloc(*file_name_len);
    while(recv_data(&sock,file_name,*file_name_len,(struct sockaddr*)&their_addr,&addr_len, &index) != 0) printf("Re-Receiving Packet\n");
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    printf("Filename: %s\n",file_name);

    /**Receive File Size*/
    long unsigned int* file_size = malloc(8);
    while(recv_data(&sock,file_size,8,(struct sockaddr*)&their_addr,&addr_len,&index) != 0) printf("Re-Receiving Packet\n");
    sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
    printf("File size: %lul\nAcknowledgement Sent\n",*file_size);

    /**Receive File*/
    struct file_info new_file;
    create_file(&new_file,"flick.png");
    long unsigned int bytes_received = 0;
    char* file_holder;

    file_holder = malloc(*file_size);
    while(bytes_received < *file_size) {
        int rv;
        if(*file_size - bytes_received > 256) {
            while((rv = recv_data(&sock,file_holder+bytes_received,256,(struct sockaddr*)&their_addr,&addr_len,&index)) != 0) {
                if(rv == -2) {
                    printf("Sender Index Out of Line - Resending Acknowledgement");
                    sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
                }
                printf("Re-Receiving Packet\n");

            };
            printf("[Receive Data] %lu (%d)\n",bytes_received,256);
            bytes_received += 256;
            sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
            //printf("Sending Acknowledgement\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
        } else {
            while(recv_data(&sock,file_holder+bytes_received,256,(struct sockaddr*)&their_addr,&addr_len,&index) != 0) {
                if(rv == -2) {
                    printf("Sender Index Out of Line - Resending Acknowledgement");
                    sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
                }
                printf("Re-Receiving Packet\n");
            }
            printf("[Receive Data] %lu (%d)\n",bytes_received,*file_size - bytes_received);
            bytes_received = *file_size;
            sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
            //printf("Sending Acknowledgement\n",);
        }
    }
    write_file(&new_file,file_holder,*file_size);
    fclose(new_file.fileptr);



    return 0;

}