#include <unistd.h>
#include <stdio.h>
#define CRC 11

void *memcpy(void *dest, const void * src, size_t n);

struct file_info {
    FILE* fileptr;
    unsigned long int file_len;
    int file_position;

};

struct data_frame {
    char data[8];
    unsigned char r; /**Remainder of divided value*/
};

struct data_packet {
    unsigned char index;
    unsigned short int frame_count;
    struct data_frame frames[32];
};

struct data_buffer {
    int size;
    int space_used;
    char* data;
};





void write_data_to_packet(struct data_packet* packet, char* data, int size) {

    packet->frame_count = (size/8) + 1;
    int i;
    for(i = 0; i < packet->frame_count; i++) {
        memcpy(packet->frames[i].data, data+8*i,8);
        unsigned long int* datal = packet->frames[i].data;

        packet->frames[i].r = (unsigned short int) (*datal)%CRC;
    }
}

int read_data_from_packet(struct data_packet* packet, char* buffer, int size) {
    int transferred = 0;
    int frame = 0;
    while(transferred < size && frame < packet->frame_count) {
        if(size-transferred < 8) {
            memcpy(buffer+transferred, packet->frames[frame].data, size-transferred);
            transferred = size;
        } else {
            memcpy(buffer+transferred, packet->frames[frame].data,8);
            transferred += 8;
        }
        frame++;
    }
    return transferred;
}



void get_buffer(struct data_buffer* target, int size) {
    target->size = size;
    target->space_used = 0;
    target->data = malloc(size);
    return 0;
}

int check_packet_integrity(struct data_packet* packet) {
    int i = 0;
    int pass = 1;
    while(pass && i < packet->frame_count) {
        pass = check_frame(packet->frames[i]);
        i++;
    }
    return pass;
}

int check_frame(struct data_frame frame) {
    //unsigned long int* con = frame.data;
    return (unsigned short int)(*((unsigned long int*) frame.data))%CRC == frame.r;
}



int open_file(struct file_info* file, const char* filepath) {

    /**Open Specified File*/
    if((file->fileptr = fopen(filepath,"rb")) == 0) {
        perror("Open File Error: ");
        return -1;
    }


    /**Get Size of File*/
    if(fseek(file->fileptr,0,SEEK_END) == 0) file->file_len = ftell(file->fileptr); else {
        perror("Open File Error: ");

        return -2;
    }



    /**Return to start of File*///
    rewind(file->fileptr);
    file->file_position = 0;
    return 0;
}

int read_file(struct file_info* file, char* buffer, int data_size) {
    return fread(buffer, data_size, 1,file->fileptr);
}

int create_file(struct file_info* new_file, const char* file_path) {
    new_file->fileptr = fopen(file_path, "w");
    if(new_file->fileptr == 0) {
        perror("Create File Error: ");
        return -1;
    }
    new_file->file_len = 0;
    return 0;
}

void write_file(struct file_info* dest, char* data, long data_size) {
    int loaded = 0;

    fwrite(data, data_size, 1, dest->fileptr);
    /*while(loaded < data_size) {
    
        fputc(data[loaded], dest->fileptr);

        loaded += 1;
    }*/

}