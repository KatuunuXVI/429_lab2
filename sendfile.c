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
#include <string.h>
#include "fileinfo.h"
#define SERVERPORT "18005"
#define MAXBUFLEN 100
#define CRC 11



int clean_send(int* sock, char* data, int bytes, const struct sockaddr* dest_addr, socklen_t dest_len) {
    int bytes_sent = 0;
    int ack_received = 0;
    void* ackPacket = malloc(1);
    while(!ack_received) {
        while(bytes_sent != sizeof(struct data_packet)) {
            if((bytes_sent = sendto(*sock, data,sizeof(struct data_packet),0,dest_addr,dest_len)) == -1) {
                perror("sendto");
                printf("Error\n");
                return -1;
            }
            if(bytes_sent != bytes) printf("Error: Incorrect Bytes Sent: %d\n",bytes_sent);
        }

        if(recvfrom(*sock, ackPacket, 1,0,NULL, dest_len) == -1) {
            perror("recvfrom");
            printf("Error\n");
            return -1;
        } else {
            ack_received = 1;
        }
    }
    free(ackPacket);

    return 0;
}

int send_data(int* sock, char* data, int bytes, const struct sockaddr* dest_addr, socklen_t dest_len, unsigned char* index) {
    if(bytes > 256) {
        printf("Error: Can only send 32 frames (256 bytes) at a time");
        return -1;
    }
    struct data_packet packet;
    packet.index = index;
    write_data_to_packet(&packet,data,bytes);
    int bytes_sent = 0;
    int ack_received = 0;
    void* ackPacket = malloc(1);
    while(!ack_received) {
        while(bytes_sent != sizeof(packet)) {

            if((bytes_sent = sendto(*sock, &packet,sizeof(packet),0,dest_addr,dest_len)) == -1) {
                perror("sendto");
                printf("Error\n");
                return -1;
            }
            if(bytes_sent != sizeof(packet)) {
                printf("Error: Incorrect Bytes Sent: %d\n",bytes_sent);
            }
        }
        printf("Receiving\n");
        if(recvfrom(*sock, ackPacket, 1,0,NULL, dest_len) == -1) {
            perror("recvfrom");
            printf("Error\n");
            return -1;
        } else {
            ack_received = 1;
            printf("Send Succesful\n");
        }
    }
    free(ackPacket);
    (*index)++;

    return *index;
}


int main(int argc, char*argv[]) {



    const char* sample = "Your mother is upset with you anon\n";


    /*a
    int error_code;/
    struct file_info original;
    if((error_code = open_file(&original, "../ur/command.wmv")) != 0) return -1;


    printf("FIle Size: %d\n", original.file_len);
    //printf("%s\n",navyseals);

    struct file_info copy;
    printf("Creating Ur\n");
    if(create_file(&copy,"../ur/numand.wmv") != 0) return -1;
    printf("Ur Created at %d\n", copy.fileptr);
    int read = 0;
    int prev_read;
    char* data;
    while(read < original.file_len) {
        prev_read = read;
        data = malloc(1000000);
        read += read_file(&original,data,1000000);
        write_file(&copy,data,read-prev_read);
        free(data);
    }



    return 0;*/
    if(argc != 5) {
        fprintf(stderr, "usage:sendfile -r <recv host>:<recv port> -f <subdir>/<filename>\n");
        exit(1);
    }



    /**Open File*/
    struct file_info to_send;
    open_file(&to_send,argv[4]);

    int sock;
    struct addrinfo hints, *servinfo, *p;

    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;


    int rv;
    printf("Hostname: %s\n",argv[2]);
    if((rv = getaddrinfo(argv[2], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sock = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
            perror("sendfile: socket error\n");
            continue;
        } else {
            //printf("Address: %ul\n",p->addr->sin_addr.s_addr);
            break;
        }
    }

    if(p == NULL) {
        fprintf(stderr,"sendfile: Failed to create socket\n");
        return 2;
    }



    /**Set Timeout Window*/
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    /**Send Filename Length*/
    //send_data(int* sock, char* data, int bytes, const struct sockaddr* dest_addr, socklen_t dest_len, unsigned char index) {
    const char* filename =  argv[4];
    char* file_name_len = malloc(8);
    *((unsigned long int*)file_name_len) = strlen(filename);
    unsigned char send_index = 0;
    send_data(&sock,file_name_len,8,p->ai_addr,p->ai_addrlen, &send_index);


    /**Send Filename*/
    send_data(&sock,filename,*file_name_len,p->ai_addr,p->ai_addrlen, &send_index);
    free(file_name_len);

    /**Send Filesize*/
    char* file_size = malloc(8);
    *((unsigned long int*)file_size) = to_send.file_len;
    printf("File Size: %ul\n",to_send.file_len);
    send_data(&sock,file_size,8,p->ai_addr,p->ai_addrlen, &send_index);
    free(file_size);

    /**Send File*/
    if(to_send.file_len < 1000000) {
        char* file_holder = malloc(to_send.file_len);
        unsigned long int bytes_sent = 0;
        read_file(&to_send,file_holder,to_send.file_len);
        while(bytes_sent < to_send.file_len) {
            if(to_send.file_len - bytes_sent > 256) {
                send_data(&sock,file_holder+bytes_sent,256,p->ai_addr,p->ai_addrlen,&send_index);
                bytes_sent += 256;
                printf("%ul bytes sent\n",bytes_sent);
            } else {
                bytes_sent += send_data(&sock,file_holder+bytes_sent,to_send.file_len - bytes_sent,p->ai_addr,p->ai_addrlen,&send_index);
                bytes_sent = to_send.file_len;
                printf("%ul bytes sent\n",bytes_sent);
            }
        }

    } else {
        //TODO: Manage Heavy files
        char* file_holder = malloc(1000000);
        return 0;
    }

    return 0;



}
