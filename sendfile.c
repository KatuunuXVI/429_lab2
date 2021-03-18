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
#define CRCR 11

struct file_packet {
    short int index;
    char data[400];
};
/*
int pack_file(struct file_info* fi, struct file_packet* fp,int in,int offset) {
   fp.index = in;
   fread(fp->data,400,1,fi->fileptr)
}*/

int main(int argc, char*argv[]) {

    /*
    int error_code;
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
    tv.tv_sec = 8;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    /**Send Filesize*/
    void* sendPacket = malloc(9);
    unsigned long int* fsize = (unsigned long int*) sendPacket;
    *fsize = to_send.file_len;
    unsigned char* remainder = (unsigned char*) sendPacket + 8;
    *remainder = (unsigned char) (to_send.file_len%11);
    printf("File Size: %ld\n",*((unsigned long int*) sendPacket));
    printf("CRC Value: %d\n",*remainder);
    while(numbytes != 9) { /**long plus */
        if((numbytes = sendto(sock, sendPacket,9,0,p->ai_addr,p->ai_addrlen)) == -1) {
            perror("Error Sending File: sendto\n");
            exit(1);
        }
    }
    printf("Sent %d bytes to recvfile\n",numbytes);
    free(sendPacket);

    void* ackPacket = malloc(2);
    printf("Awaiting Response\n");
    if((numbytes = recvfrom(sock, ackPacket, 2,0,p->ai_addr, p->ai_addrlen)) == -1) {
        perror("recvfrom");
        printf("Error\n");
    }
    short int ack = (short int*) ackPacket;
    if(ack == 0) printf("Filesize Acknowledgement\n"); else printf("Bad Size Sent\n");



    /*
    if((numbytes = sendto(sock, argv[2],strlen(argv[2]),0,p->ai_addr,p->ai_addrlen)) == -1) {
        perror("Error Sending File: sendto\n");
        exit(1);
    }*/


    freeaddrinfo(servinfo);
    close(sock);


    return 0;

}
