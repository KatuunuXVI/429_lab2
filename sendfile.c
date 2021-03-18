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

struct file_info {
    FILE* fileptr;
    long file_len;
};

char* oferror(int error_code) {
    switch(error_code) {
        case -1: return "File Does Not Exist";
        case -2: return "File could not be measured";
        default: return "Invalid Error Code";
    }
}
int open_file(struct file_info* file, const char* filepath) {

    /**Open Specified File*/
    if((file->fileptr = fopen(filepath,"rb")) == 0) return(-1);


    /**Get Size of File*/
    if(fseek(file->fileptr,0,SEEK_END) == 0) file->file_len = ftell(file->fileptr); else return(-2);



    /**Return to start of File*/
    rewind(file->fileptr);
    return 0;
}

int read_file(struct file_info* file, char* buffer, int data_size) {
    return fread(buffer, data_size, 1, file->fileptr);
}

void create_file(struct file_info* new_file, const char* file_path) {
    new_file->fileptr = fopen(file_path, "w");
    new_file->file_len = 0;
}

void write_file(struct file_info* dest, char* data, long data_size) {
    int loaded = 0;
    while(loaded < data_size) {
        fputc(data[loaded], dest->fileptr);
        loaded += 1;
    }
}


void open(struct file_info* original, const char* filepath) {
    //original = malloc(sizeof(struct file_info));


}


int main(int argc, char*argv[]) {
    int error_code;

    struct file_info original;

    if((error_code = open_file(&original, "navyseals.txt")) != 0) {
        fprintf(stderr, "Open File Error: %s\n",oferror(error_code));
        return 1;
    }
    char* navyseals = malloc(original.file_len);
    printf("Read %d bytes\n",read_file(&original,navyseals,original.file_len+1));
    printf("%s\n",navyseals);

    struct file_info copy;
    create_file(&copy,"newseals.txt");

    write_file(&copy,navyseals,original.file_len);


    return 0;
    if(argc != 5) {
        fprintf(stderr, "usage:sendfile -r <recv host>:<recv port> -f <subdir>/<filename>\n");
        exit(1);
    }

    /**Open file*/
    FILE *fileptr = fopen("flick.png","rb");

    /**Get Size of File*/
    fseek(fileptr,0,SEEK_END);

    long filelen = ftell(fileptr);
    /**Return to start of file*/
    rewind(fileptr);
    /**Allocate space the same size of file*/
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
