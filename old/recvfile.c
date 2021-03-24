#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    //while(!clean_packet) {

    while(bytes_received != sizeof(struct data_packet)) {
        if((bytes_received = recvfrom(*sock,recv_packet,sizeof(struct data_packet),0,source_addr,source_len)) == -1) {
            perror("Bytes Received\n");
            return -1;
        }
    }

    return check_packet_integrity(recv_packet) ? read_data_from_packet(recv_packet,buffer,bytes) : -1;

}

int main(int argc, char* argv[]) {



    if(argc < 3) {printf("Format: recvfile -p <recv_port>\n"); return -1;}

    char* port = argv[2];

    int sock;

    struct addrinfo hints, *p;

    int numbytes;
    struct sockaddr_storage their_addr;

    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

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
    recv_data(&sock,file_name_len,8,(struct sockaddr*)&their_addr,&addr_len, &index);
    void* ack = malloc(1);
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    printf("Filename Length: %d\n",*file_name_len);

    /**Receive File Name*/
    const char* file_name = malloc(*file_name_len);
    recv_data(&sock,file_name,*file_name_len,(struct sockaddr*)&their_addr,&addr_len, &index);
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    printf("Filename: %s\n",file_name);

    /**Receive File Size*/
    long unsigned int* file_size = malloc(8);
    recv_data(&sock,file_size,8,(struct sockaddr*)&their_addr,&addr_len,&index);
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    printf("File size: %ul\n",*file_size);
    //a
    /**Receive File*/
    struct file_info new_file;
    create_file(&new_file,file_name);
    long unsigned int bytes_received = 0;
    char* file_holder;
        file_holder = malloc(*file_size);
        while(bytes_received < *file_size) {
            if(*file_size - bytes_received > 256) {
                recv_data(&sock,file_holder+bytes_received,256,(struct sockaddr*)&their_addr,&addr_len,&index);
                bytes_received += 256;
                printf("%ul bytes received\n",bytes_received);
                printf("Sending Acknowledgement\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
            } else {
                bytes_received = *file_size;
                recv_data(&sock,file_holder+bytes_received,(*file_size) - bytes_received,(struct sockaddr*)&their_addr,&addr_len,&index);
                printf("%ul bytes received\n",bytes_received);
                printf("Sending Final Acknowledgement\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
            }
        }
        write_file(&new_file,file_holder,*file_size);
        fclose(new_file.fileptr);

    /*else {
        unsigned long int total_bytes_recv = 0;
        while(total_bytes_recv < *file_size) {
            int file_block = (*file_size) - total_bytes_recv > 1000000 ? 1000000 : (*file_size) - new_file.file_len;

            printf("File Block: %d\n",file_block);
            file_holder = malloc(file_block);
            printf("File Holder Allocated\n");
            while(bytes_received < file_block) {

                if(*file_size - bytes_received > 256) {
                    recv_data(&sock,file_holder+bytes_received,256,(struct sockaddr*)&their_addr,&addr_len,&index);
                    bytes_received += 256;
                    printf("%ul bytes received\n",total_bytes_recv + bytes_received);
                    printf("Sending Acknowledgement\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
                } else {
                    recv_data(&sock,file_holder+bytes_received,file_block - bytes_received,(struct sockaddr*)&their_addr,&addr_len,&index);
                    bytes_received = file_block;
                    printf("%ul bytes received\n",total_bytes_recv + bytes_received);
                    printf("Sending Acknowledgement\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
                }
            }

            printf("Writing File\n");
            write_file(&new_file,file_holder,file_block);
            printf("Free File Holder\n");
            total_bytes_recv += bytes_received;
            if(total_bytes_recv > *file_size) {
                fclose(new_file.fileptr);
                printf("File Closed\n");
            }
            free(file_holder);

            bytes_received = 0;//
            printf("Total Bytes: %d\n",total_bytes_recv);
        }
        //printf("Expected File Size: %ul\n", *file_size);
        //printf("FIle Size: %ul\n",new_file.file_len);
    }*/



    return 0;

}