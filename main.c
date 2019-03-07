#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <ctype.h>
#include <stdbool.h>
#include <wait.h>

void malloc_check(const char* buffer);

void remove_newline_char(char **str);

void get_input(char* prompt,char **buffer);

char** parse_input(char* input,int count);

void trim(char **str);

int get_count(char *input);

void execute(char **args);

void my_error();

int main() {
    char *input;
    get_input("myshell>",&input);
    int count = get_count(input);
    char **toks = parse_input(input,count);
    execute(toks);
    free(toks);
    //output
    return(0);
}

void execute(char **args) {
    pid_t pid = fork();
    if(pid == 0){
        //this is child
        //execv(args[0],args);
        char *const arg[]={};
        execv("./ls",NULL);
        my_error();//only runs if execv fails
    }else if(pid<0){
        printf("fork failed");
    } else{
        //this is the parent
        wait(NULL);//wait until child is done
    }
}

void my_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO,error_message,strlen(error_message));
    exit(1);
}

//counts the different words in input
int get_count(char *input) {
    bool part_of_word = false;
    int count=0;
    for(int i=0;i<strlen(input);i++){
        if(!isspace(input[i])&&!part_of_word){
            part_of_word=true;
            count++;
        }
        else{
            part_of_word=false;
        }
    }
    return count;
}

char** parse_input(char* input,int count){
    char** output    = 0;
    char del[2]=" ";

    output = malloc(sizeof(char*) * count);
    //malloc_check(output);
    int j  = 0;
    char* token = strtok(input, del);
    while (token)
    {
        *(output + j++) = strdup(token);
        token = strtok(0, del);
    }
    *(output+ j) = 0;

    return output;
}


//gets input from user
void get_input(char* prompt,char **buffer){
    size_t buffsize = 32;//aprox size of a command; can be changed by getline if not large enough
    *buffer = (char *) malloc(buffsize * sizeof(char)); //allocate space for buffer
    malloc_check(*buffer);//check to ensure space was allocated
    printf("%s",prompt);
    getline(buffer, &buffsize, stdin);//getline
    remove_newline_char(buffer);
    //trim(buffer);
}

void trim(char **str){
    char *s = *str;
    int i=0;
    while(isspace(s[i])){
        i++;
    }
    for(int j=i;j<strlen(s);j++){
        s[j]=s[j+1];
    }
    i=0;
    while(!isspace(s[i])){
        i++;
    }
    int j = i;
    while(s[j]!='\0'){
        j;
    }
}

void remove_newline_char(char **str){
    char *s = *str;//get pointer to char
    int i=0;
    //loop through string
    while(s[i]!='\0'){//stop looping when you get to null terminator
        //replace new line with null terminator
        if(s[i]=='\n'){
            s[i]='\0';
        }
        i++;
    }
}

void malloc_check(const char* buffer){
    if( buffer == NULL)
    {
        my_error();//malloc error
    }
}