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
#define MYPORT "18005"
#define MAXBUFLEN 100
#define CRCR 11
struct packet {
    char data[41];
};

void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

    /**Receive File Size*/
    char* buf = malloc(9);
    while(numbytes != 9) {
        if((numbytes = recvfrom(sock, buf, MAXBUFLEN-1,0,(struct sockaddr*)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            printf("Error\n");
        }
    }


    printf("recvfile: got size packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s,sizeof s));

    printf("recvfile: packet is %d bytes long\n", numbytes);
    //buf[numbytes] = '\0';
    unsigned int fSize = *((unsigned long int*)buf);
    printf("File Size: %ld\n",fSize);

    unsigned char receivedCRC = *(((unsigned char*)buf)+8);
    unsigned char expectedCRC = (unsigned char) (fSize%CRCR);

    int CRCPass = (receivedCRC == expectedCRC);
    printf("CRC Pass: %d\n", CRCPass);


    void* ack = malloc(1);
    
    printf("Sending %d\n",sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr)));
    perror("Sending Acknowledgement");

    realloc(buf,2); /**Max File Size is 255, thus small enough for a short*/
    printf("Waiting for Filename Length\n");
    while(numbytes != 3) {
    if((numbytes = recvfrom(sock, buf, MAXBUFLEN-1,0,(struct sockaddr*)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        printf("Error\n");
    }
    }
    short int* file_name_len = (short int*) buf;
    char filename[*file_name_len];
    printf("File Name Length: %d\n", *file_name_len);
    expectedCRC = (*file_name_len)%11;
    receivedCRC = *((unsigned char*) (buf+2));
    int crc = expectedCRC == receivedCRC;
    if(crc) sendto(sock, &ack, 1,0,(struct sockaddr*)&their_addr, sizeof(their_addr));
    free(buf);
    close(sock);
    return 0;
}