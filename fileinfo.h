struct file_info {
    FILE* fileptr;
    unsigned long int file_len;
    int file_position;

};


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



    /**Return to start of File*/
    rewind(file->fileptr);
    file->file_position = 0;
    return 0;
}

int read_file(struct file_info* file, char* buffer, int data_size) {
    return fread(buffer, 1, data_size, file->fileptr);
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
    while(loaded < data_size) {
        fputc(data[loaded], dest->fileptr);
        loaded += 1;
    }
}

