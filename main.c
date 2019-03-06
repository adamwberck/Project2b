#include <stdio.h>
#include <stdlib.h>

void malloc_check(char* buffer);

int main() {
    char *buffer;//pointer to char setup
    size_t buffsize = 32;//aprox size of a command; can be changed by getline if not large enough
    buffer = (char *) malloc(buffsize * sizeof(char)); //allocate space for buffer
    malloc_check(buffer);//check to ensure space was allocated
    printf("Type something: ");
    //return for getline stating number of characters entered
    size_t characters;
    characters = (size_t) getline(&buffer, &buffsize, stdin);

    //output
    printf("%zu characters were read.\n",characters);
    printf("You typed: '%s'\n",buffer);

    return(0);
}

void malloc_check(char* buffer){
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
}