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
#define SERVERPORT "18005"
int main(int argc, char*argv[]) {
    /**Copy file*/
    FILE *fileptr = fopen("flick.png","rb");
    fseek(fileptr,0,SEEK_END);
    long filelen = ftell(fileptr);
    rewind(fileptr);
    char *buffer = (char*) malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, fileptr);
    printf("File: %s\n",buffer);

    printf("Enter name of target file\n");
    char tfile[25];
    gets(tfile);
    FILE *cop = fopen(tfile, "w");
    char* n = malloc(1);
    if( cop == NULL )
    {
        fclose(fileptr);
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
    int nci = 0;
    char c = ' ';
    while( nci < filelen ){

        c = buffer[nci];
        //printf("%c",c);
        fputc(c, cop);

        //ns[nsi] = ch;
        nci++;
        //realloc(ns, nsi+1);
    }
    printf("\n");
    //printf("File: %s\n",nc);
    printf("File copied successfully.\n");
    fclose(fileptr);
    fclose(cop);

    return 0;
    int sock;
    struct addrinfo hints, *servinfo, *p;


    int numbytes;

    if(argc != 3) {
        fprintf(stderr, "usage:talker hostname message\n");
        exit(1);
    }
    printf("Memset\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;


    int rv;
    printf("Hostname: %s\n",argv[1]);
    if((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
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

    if((numbytes = sendto(sock, argv[2],strlen(argv[2]),0,p->ai_addr,p->ai_addrlen)) == -1) {
        perror("Sendfile: sendto\n");
        exit(1);
    }

    freeaddrinfo(servinfo);
    printf("Successfully sent %d bytes to recvfile\n",numbytes);
    close(sock);


    return 0;
    char ch, source_file[25], target_file[25];

    FILE *source, *target;

    printf("Enter name of file to copy\n");
    gets(source_file);

    source = fopen(source_file, "r");

    if( source == NULL )
    {
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter name of target file\n");
    gets(target_file);
    target = fopen(target_file, "w");
    char* ns = malloc(1);
    if( target == NULL )
    {
        fclose(source);
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
    int nsi = 0;
    while( ( ch = fgetc(source) ) != EOF ){
        fputc(ch, target);

        ns[nsi] = ch;
        nsi++;
        realloc(ns, nsi+1);
    }

    printf("File: %s\n",ns);
    printf("File copied successfully.\n");
    fclose(source);
    fclose(target);

    return 0;

}
