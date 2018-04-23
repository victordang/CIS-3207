/*
 Victor Dang
 CIS 3207
 Lab 2 - Writing your own linux/unix shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>


//declarations of functions before main
char **parse(char*input); //parse user input
void prompt(char *input); //prompt = current working directory
int exec(char *input); //execute normally , no pipe, redirect , or background
int builtIn(char **args , char *input); //built in commands
int piping(char *input); //piping
int bg(char *input); //background



//main function
int main (){
    
    //outputs prompt , and current working directory
    printf("Welcome to my shell\n");
    char input[1024];
    
    prompt(input); //1st prompt of user input
    
    
    //main loop , until exit
    
    while (1){
        
        int i = 0;

        //doing the special cases
        
        // \0 is NULL character
        // while user input is not null
        while (input[i]!= '\0'){
            
            //normal execution, check builtins
            
            if (input[i+1] == '\0') {
                if (exec(input) != 0) {
                    printf("Error\n");
                }
                break;
            }
            
            //background process case
            
            else if (input[strlen(input)-1] == '&') {
                if (bg(input) != 0) {
                    printf("Error\n");
                }
                break;
            }
           
            //piping
            else if (input[i] == '|') {
                if (piping(input) != 0) {
                    printf("Error\n");
                }
                break;
            }
            i++;
            
        }//end of special cases loop
        
         
    prompt(input); //prompt of user in main loop
        
        
    }//end of main loop
    
    return 0;
}


//prompts user the current working directory , using getcwd
void prompt(char*input){
    char *getcwd(char *buf , size_t size);
    char cwd[1024]; //cwd = current working directory
    getcwd(cwd,sizeof(cwd));
    
    if (getcwd(cwd,sizeof(cwd)) != NULL){
        printf("%s>",cwd);
        
        fgets(input,1024,stdin);

    }
}//end of prompt


//parsing user input
char **parse(char *input){
    char **args = malloc(1024 * sizeof(char*));
    int i=0;
    char *arg;
    //delimited by white space , space or tabs
    const char wspace[10] = "  \t\n";
    //strtok is delimiter
    arg = strtok(input, wspace);
    while (arg != NULL) {
        args[i] = arg;
        arg = strtok(NULL, wspace);
        i++;
    }
    
    args[i] = NULL;
    return args;
    
}//end of parse




//normal execution, checks for builtin functions
int exec(char *input) {
    
    //taking in parsed user input & parameters
    char **args = parse(input);
    char *path = args[0];
    char *param[20];
    
    int i=0;
    while (args[i] != NULL) {
        param[i] = args[i];
        i++;
    }
    param[i] = NULL;
    
    //checking builtin
    if (builtIn(args,input) == 0) {
        return 0;
    }
    
    //fork , execute and wait
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(path, param) != 0) {
            printf("Command not recognized. Try again\n");
            exit(0); // so you dont need mult quits to quit
        }
    }
    else {
        waitpid(pid, NULL, 0);
    }
    return 0;
}



//builtin functions , originally had as separate functions but easier to write it in one cause piping and background checks for built in

int builtIn(char **args , char *input){
    
    //empty space, do nothing
    if(args[0] == NULL){
        printf("Command not recognized. Try Again\n");
        return 0;
    }
    
    //if user types quit, exit entire program
    if (strcmp(args[0],"quit") == 0){
        printf("Exiting Shell\n");
        exit(0);
    }
    //if user types in pause , pause shell until user presses enter
    else if (strcmp(args[0],"pause") == 0){
        int j = 0;
        char c;
        printf("Shell is paused until you press ENTER\n");
        while( (c = getchar() ) != '\n'){
        }
        return 0;
    }
    //if user types help, outputs readme to user
    else if (strcmp(args[0],"help") == 0){
        FILE *help;
        char fileN[50]; //file name
        char k;
        
        //open readme file for reading
        help = fopen("README","r");
        if (help == NULL){
            printf("File not found\n");
        }
        else{
            k = fgetc(help);
            
            //outputs to terminal character by character until end of file
            while (k!= EOF){
                printf("%c",k);
                k = fgetc(help);
            }
        }
        fclose(help);
        return 0;
    }
    
    //if user types in clr, clears entire terminal
    else if (strcmp(args[0],"clr") == 0){
        printf("\e[1;1H\e[2J");
        return 0;
    }
    
    //if user types in environ, list terminal environment
    else if (strcmp(args[0],"environ") == 0){
        system("printenv");
        return 0;
    }
    
    //if user types in dir, lists entire directory's content , like ls
    else if (strcmp(args[0],"dir") == 0){
        char *path = getenv("PWD");
        DIR *a; //dp
        struct dirent *b; //ep
        
        if (strcmp(path,"\n")!=0){
            path[strcspn(path,"\n")] = 0;
        }
        if((a = opendir(path)) == NULL){
            printf("Failure to open directory\n");
        }
        
        else{
            int k = 0;
            while((b= readdir(a)) != NULL){
                printf("%s\n", b->d_name);
                k++;
            }
            closedir(a);
        }
        return 0;
    }
    
    //if user types in echo followed by a comment , ex) echo hello world , the terminal will output hello world
    else if (strcmp(args[0],"echo") == 0){
        int i = 1;
        while(args[i] != NULL){
            printf("%s ", args[i]);
            i++;
        }
        printf("\n");
        return 0;
    }
    
    
    //if user types in cd , it will change directory , cd .. will go back one directory , doesn't work 100% correctly
    else if (strcmp(args[0],"cd") == 0){
        // if no argument , then go to home directory
        if (args[1] == NULL) {
            return 0;
        }
        else {
            if (chdir(args[1]) != 0) {
                printf("error can't find directory\n");
            }
        }
    
        return 0;
        
    }
    
    return -1;
}
//background , piping , couldn't do redirection
    
//background execution
int bg(char *input){
    //taking in parsed user input
    char **args = parse(input);
    char *path = args[0];
    char *param[20];
    
    int i=0;
    while (args[i][0] != '&' ) {
        param[i] = args[i];
        i++;
    }
    param[i] = NULL;
    
    //checking built ins
    if (builtIn(args, input) == 0) {
        return 0;
    }
    
    
    //fork , execute
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(path, param) != 0) {
            printf("Error in background process\n");
            exit(0);
        }
    }
    return 0;
}
    

int piping(char* input) {
    //taking in parsed user input & parameters for piping
    char** args = parse(input);
    char *path1 = args[0];
    char* param1[20];
    char* param2[20];
    
    int n = 0;
    //first parameter is whatever is upto |
    while (args[n][0] != '|') {
        param1[n] = args[n];
        n++;
    }
    param1[n] = NULL;
    n++;
    
    int m = 0;
    char *path2 = args[m];
    //2nd parameter
    while (args[n] != NULL) {
        param2[m] = args[n];
        m++;
        n++;
    }
    param2[m] = NULL;
    
    
    //fork , execute & wait
    int fd[2];
    pipe(fd);
    int pid = fork();
    if (pid == 0) {
        
        //duplicate child descriptors
        dup2(fd[0], 0);
        close(fd[1]);
        if (execvp(path2, param2) != 0) {
            printf("Error in Child process\n");
            exit(0);
        }
    } else {
        int pid2 = fork();
        if (pid2 == 0) {
            dup2(fd[1], 1);
            close(fd[0]);
            if (execvp(path1, param1) != 0) {
                printf("Error in Parent process\n");
                exit(0);
            }
        } else {
            waitpid(pid2,NULL,0);
        }
        waitpid(pid, NULL, 0);
    }
    return 0;
}

//couldn't get redirection to work


    
    
    


