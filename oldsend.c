#include <stdio.h>
#include <stdlib.h>

int main() {

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
