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

void execute(char **args,int count);

void my_error();

int built_in(char *string,char* path);


char** paths;
int nubmer_of_paths = 0;

int main() {
    int i=0;
    while(i<50) {
        i++;
        char *input;
        get_input("myshell> ", &input);
        int count = get_count(input);
        char **toks = parse_input(input, count);
        execute(toks,count);
        free(toks);
    }
    //output
    return(0);
}

void execute(char **args,int count) {
    char* path = "/bin/";
    int builtin = built_in(args[0],path);//checks if built in cmd
    if(builtin==0){
        exit(0);
    } else if(builtin==1){//cd
        char* new_dir =  args[1];
        chdir(new_dir);
    } else if(builtin==2){//path
        //handle memory allocation for paths
        free(paths);
        paths = malloc((size_t) (count)*sizeof(char));
        for(int i=1;i<count;i++){
            paths[i-1]=malloc(strlen(args[i])* sizeof(char));
            paths[i-1]=args[i];//set paths
        }
        nubmer_of_paths=count-1;
    }
    //not built in command run exe
    else {
        pid_t pid = fork();
        char *temp = *args;
        /*
        for(int i=0;i<10;i++){
            printf("%c",temp[i]);
        }*/
        if (pid == 0) {
            //this is child
            //put path on the begging of buffer
            char *tmp =  strdup(args[0]);
            strcpy(args[0],path);
            strcat(args[0],tmp);
            free(tmp);
            execv(args[0], args);
            my_error();//only runs if execv fails
        } else if (pid < 0) {
            printf("fork failed");
        } else {
            //this is the parent
            wait(NULL);//wait until child is done
        }
    }
}

int built_in(char *string,char *path) {
    char *cmd = (string+strlen(path));
    if(strcmp(cmd,"exit")==0){
        return 0;
    }else if(strcmp(cmd,"cd")==0){
        return 1;
    }else if(strcmp(cmd,"path")==0){
        return 2;
    }
    return -1;
}

void my_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO,error_message,strlen(error_message));
    exit(1);
}

//counts the different arguments in input including command
int get_count(char *input) {
    bool part_of_word = false;
    int count=0;
    for(int i=0;i<strlen(input);i++){
        if(!isspace(input[i])&&!part_of_word){
            part_of_word=true;
            count++;
        }
        else if(isspace(input[i])){
            part_of_word=false;
        }
    }
    return count;
}

char** parse_input(char* input,int count){
    char** output    = 0;
    char del[2]=" ";

    output = malloc(sizeof(char*) * (count+2));
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
    char* path = "/bin/";
    size_t buffsize = 32+strlen(path);//aprox size of a command; can be changed by getline if not large enough
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
    if( buffer == NULL) {
        my_error();//malloc error
    }
}