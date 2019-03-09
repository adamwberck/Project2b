#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <ctype.h>
#include <stdbool.h>
#include <wait.h>
#include <dirent.h>
#include <sys/fcntl.h>


void my_cd(int count,char** args);

void malloc_check(const char* buffer);

void remove_newline_char(char **str);

void get_input(char* prompt,char **input);

char** parse_input(char* input,int count);

int get_count(char *input);

void execute(char **args,int count);

void my_error();

bool my_built_in(int count, char** args);

void put_shell_into_env();

int remove_exe_name(char **str);

char *get_prompt() ;

void my_dir(int count, char *const *args);

int has_write_redirect(char **args, int count);

void replace_other_whitespace(char **str);

int has_read_redirect(char **args, int count);

void perform_write_redirect(char *const *args, int count, int write_redirect);

//
extern char** environ;

char** paths;
int number_of_paths = 0;
bool is_parallel=false;

int main() {
    put_shell_into_env();
    int j=0;
    while(j<50) {
        j++;
        char *prompt = get_prompt();
        char *input;
        get_input(prompt, &input);
        int count = get_count(input);
        char **parsedInput = parse_input(input, count);
        execute(parsedInput,count);

        free(parsedInput);
    }
    //output
    return(0);
}


char *get_prompt() {
    char dir[PATH_MAX];
    if (getcwd(dir, sizeof(dir)) == NULL) {
        printf("getcwd failed\n");//never should run
    }
    //get position of / to find relative dir.
    size_t i=strlen(dir)-1;
    while(dir[i]!='/'&&i>=0){
        i--;
    }
    char * end_prompt = " ~:myshell> ";
    char *prompt = malloc(PATH_MAX* sizeof(char));
    strcpy(prompt,dir+i);
    strcat(prompt,end_prompt);
    return prompt;
}

void put_shell_into_env() {
    //allocate space to hold shell environment
    size_t buffer_size=PATH_MAX;
    char* temp = malloc(buffer_size* sizeof(char));
    readlink("/proc/self/exe",temp,buffer_size);
    //remove exe name
    remove_exe_name(&temp);
    //add '/myshell' to the end of name
    strcat(temp,"/myshell");
    //allocate space for shell
    char* shell=malloc((strlen(temp)+7)* sizeof(char));
    //prefix concat 'shell=' to environment
    strcpy(shell,"shell=");
    strcat(shell,temp);
    //put shell into environment
    if(putenv(shell)!=0){
        printf("putenv failed");
    }
    free(temp);//free temp memory;
}

int remove_exe_name(char **str) {
    char* s = *str;
    size_t len =  strlen(s);
    for(size_t i=len-1;i>=0;i--) {
        if(s[i] == '/'){
            s[i]='\0';
            return 0;
        }
    }
    return -1;
}

void execute(char **args,int count) {
    //checks if built in command
    if(my_built_in(count,args)){
        return;
    }
    char *tmp =  strdup(args[0]);
    bool working_path = false;
    int i=0;
    while(!working_path && i<number_of_paths){
        char *p = strdup(paths[i++]);
        strcat(p,tmp);
        //test if it exists and i have execute permission.
        if(access(p,X_OK)==0){
            working_path=true;
        }
    }
    if(!working_path) {
        printf("Command %s not found.\n",args[0]);
    } else{
        char *path = paths[i - 1];
        //put path on the begging of buffer
        strcpy(args[0], path);
        strcat(args[0], tmp);
        free(tmp);
        int write_redirect = has_write_redirect(args, count);
        if(write_redirect==-1){
            printf("Error: malformed write_redirect\n");
            return;
        }
        int read_redirect = has_read_redirect(args,count);
        pid_t pid = fork();
        if (pid == 0) {
            //this is child
            if (write_redirect>=1){
                perform_write_redirect(args, count, write_redirect);
                //remove write redirect from arguments
                args[count-1] = 0;
                args[count-2] = 0;
                count-=2;
            }
            if (read_redirect==1){
                //read only open of input file
                int f = open(args[count - 1], O_RDONLY);
                dup2(f,STDIN_FILENO);//stdout goto file;
                close(f);
                //remove read redirect from arguments
                args[count-1] = 0;
                args[count-2] = 0;
            }
            //run code
            execv(args[0], args);
            my_error();//only runs if execv fails; never should run
        } else if (pid < 0) {
            printf("fork failed");
        } else {
            //this is the parent
            if(!is_parallel) {
                wait(NULL);//wait until child is done
            }
        }
    }
}

void perform_write_redirect(char *const *args, int count, int write_redirect) {
    int f=0;
    //truncate
    if(write_redirect==1) {
        f = open(args[count - 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    //append
    }else{
        f = open(args[count - 1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    }
    dup2(f,STDOUT_FILENO);//stdout goto file;
    dup2(f,STDERR_FILENO);//std error goto
    close(f);
}

int has_read_redirect(char **args, int count) {
    //check if has write redirect to change argument check
    if(has_write_redirect(args,count)>=1){
        count-=2;
    }
    int is_redirect=0;
    for(int i=0;i<count;i++){
        char *temp = args[i];
        if(temp[0]=='<'&&strlen(temp)==1){
            //checks if redirect is malformed
            if(is_redirect == 0 && count-2==i) {
                is_redirect = 1;
            }else{
                return -1;
            }
        }
    }
    return is_redirect;
}

//returns 1 if redirect replace,2 if redirect append 0 if not, and -1 if error;
int has_write_redirect(char **args, int count) {
    int is_redirect=0;
    for(int i=0;i<count;i++){
        char *temp = args[i];
        if(temp[0]=='>'&&strlen(temp)<=2){
            //checks if redirect is malformed
            if(is_redirect == 0 && count-2==i) {
                //check if append or replace
                if(temp[1]=='>' && strlen(temp)==2){
                    is_redirect = 2;//append
                }else {
                    is_redirect = 1;//replace
                }
            }else{
                return -1;
            }
        }
    }
    return is_redirect;
}

void my_cd(int count,char** args){
    //if count more than two error
    if (count>2){
        printf("cd takes one argument\n");
    }
        //other wise run cd
    else{
        char *new_dir;
        //one argument give own directory
        if(count==1){
            new_dir=".";
            //otherwise give first argument
        }else{
            new_dir=args[1];
        }
        if(chdir(new_dir)==-1) {
            //cd fails
            my_error();
        }else{
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("Current directory : %s\n", cwd);
                char* PWD = malloc((strlen(cwd)+7)* sizeof(char));
                //prefix concat 'shell=' to environment
                strcpy(PWD,"PWD=");
                strcat(PWD,cwd);
                putenv(PWD);
            } else {
                perror("getcwd() error");
            }
        }
    }
}

void my_dir(int count, char *const *args) {
    if(count == 2) {
        DIR *d;
        struct dirent *dir;
        d = opendir(args[1]);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
            }
            closedir(d);
        }
    }else{
        printf("Correct usage: dir <directory>");
    }
}

bool my_built_in(int count, char** args) {
    char *cmd = args[0];
    //exit or quit
    if(strcmp(cmd,"exit")==0 || strcmp(cmd,"quit")==0){
        exit(0);
    //cd
    }else if(strcmp(cmd,"cd")==0) {
        my_cd(count,args);
    //path
    }else if(strcmp(cmd,"path")==0) {
        //handle memory allocation for paths
        free(paths);
        paths = malloc((size_t) (count) * sizeof(char));
        //write each argument as a path
        for (int i = 1; i < count; i++) {
            paths[i - 1] = malloc(strlen(args[i]) * sizeof(char));
            paths[i - 1] = args[i];//set paths
            number_of_paths = count - 1;
        }
    //clears screen
    } else if(strcmp(cmd,"clr")==0){
        //goto position(1,1) then clear the screen
        printf("\e[1;1H\e[2J");
    //list contents of directory
    } else if(strcmp(cmd,"dir")==0){
        my_dir(count, args);
    //environ
    } else if(strcmp(cmd,"environ")==0){
        //list environment vars
        int i=0;
        while(environ[i]){
            printf("%s\n", environ[i++]);
        }
    //echo
    } else if(strcmp(cmd,"echo")==0){
        for(int i=1;i<count;i++) {
            printf("%s ",args[i]);
        }
        printf("\n");
    } else if(strcmp(cmd,"help")==0) {
        //TODO manual
    //pause
    } else if(strcmp(cmd,"pause")==0) {
        printf("Paused. Press Enter to Continue.");
        //stays in while until enter
        while (getchar() != '\n'&& getchar() != '\r');
    } else{
        //not built in
        return false;
    }
    //was built in don't execute a command
    return true;
}

void my_error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO,error_message,strlen(error_message));
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
void get_input(char* prompt,char **input){
    size_t buffer_size = 32;//aprox size of a command; can be changed by getline if not large enough
    *input = (char *) malloc(buffer_size * sizeof(char)); //allocate space for input
    malloc_check(*input);//check to ensure space was allocated
    printf("%s",prompt);
    getline(input, &buffer_size, stdin);//getline
    remove_newline_char(input);
    replace_other_whitespace(input);
}

void replace_other_whitespace(char **str) {
    char *s = *str;//get pointer to char
    int i=0;
    //loop through string
    while(s[i]!='\0'){//stop looping when you get to null terminator
        //replace h-tab v-tab and form feed with space
        if(s[i]=='\t'||s[i]=='\v'||s[i]=='\f'){
            s[i]=' ';
        }
        i++;
    }
}


void remove_newline_char(char **str){
    char *s = *str;//get pointer to char
    int i=0;
    //loop through string
    while(s[i]!='\0'){//stop looping when you get to null terminator
        //replace newline with null terminator
        if(s[i]=='\n'||s[i]=='\r'){
            s[i]='\0';
        }
        i++;
    }
}

void malloc_check(const char* buffer){
    if( buffer == NULL) {
        my_error();//malloc error
        exit(1);
    }
}