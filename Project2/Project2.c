#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_INPUT 128
#define MAX_ARGS 32
#define HISTORY_SIZE 10
#define MAX_BG_PROCESSES 128

char *history[HISTORY_SIZE];
int history_count = 0;
pid_t foreground_pid = -1;
pid_t background_pids[MAX_BG_PROCESSES];
int bg_count = 0;


void add_to_history(const char *cmd);
void show_history();
void execute_history(int index, char input[], char *args[], int *background);
int execute_command(char *args[], int background);
void handle_redirection(char *args[]);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);
void add_background_process(pid_t pid);
void remove_background_process(pid_t pid);
int has_running_background_processes();
void fg_command(const char *arg);

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    int background = 0;

   
    signal(SIGINT, sigint_handler);    
    signal(SIGTSTP, sigtstp_handler); 
    signal(SIGCHLD, sigchld_handler); 

    while (1) {
        background = 0;

        
        printf("myshell: ");
        fflush(stdout);

     
        if (!fgets(input, MAX_INPUT, stdin)) {
            printf("\n");
            break; 
        }

       
        input[strcspn(input, "\n")] = '\0';

        
        if (strlen(input) == 0) continue;

        
        char input_copy[MAX_INPUT];
        strncpy(input_copy, input, MAX_INPUT);

        
        int arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        
        if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
            background = 1;
            args[arg_count - 1] = NULL;
        } else {
            background = 0;
        }

        
        if (strcmp(args[0], "exit") == 0) {
            if (has_running_background_processes()) {
                printf("There are background processes still running. Terminate them first.\n");
                continue;
            }
            exit(0);
        } else if (strcmp(args[0], "history") == 0) {
            if (arg_count > 1 && strcmp(args[1], "-i") == 0) {
                if (arg_count > 2) {
                    int index = atoi(args[2]);
                    execute_history(index, input, args, &background);
                } else {
                    printf("Usage: history -i <num>\n");
                }
            } else {
                show_history();
            }
            continue;
        } else if (strcmp(args[0], "fg") == 0) {
            if (arg_count > 1) {
                fg_command(args[1]);
            } else {
                printf("Usage: fg %%<num>\n");
            }
            continue;
        }

        
        if (execute_command(args, background) == 0) {
            add_to_history(input_copy);
        }
    }

    return 0;
}

void add_to_history(const char *cmd) {
    if (history_count == HISTORY_SIZE) {
        free(history[HISTORY_SIZE - 1]);  
        history_count--;  
    }

    
    for (int i = history_count; i > 0; i--) {
        history[i] = history[i - 1];
    }

    
    history[0] = strdup(cmd);
    history_count++;
}

void show_history() {
    if (history_count == 0) {
        printf("No commands in history.\n");
        return;
    }

    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i, history[i]);
    }
}

void execute_history(int index, char input[], char *args[], int *background) {
    if (index < 0 || index >= history_count) {
        fprintf(stderr, "Invalid history index: %d\n", index);
        return;
    }

    
    printf("Executing command from history: %s\n", history[index]);
    strncpy(input, history[index], MAX_INPUT);
    input[MAX_INPUT - 1] = '\0'; 

    
    add_to_history(history[index]);

    
    int arg_count = 0;
    char *token = strtok(input, " ");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " ");
    }
    args[arg_count] = NULL;

    
    if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0) {
        *background = 1;
        args[arg_count - 1] = NULL;
    } else {
        *background = 0;
    }

    
    execute_command(args, *background);
}

int execute_command(char *args[], int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) { 
        
        setpgid(0, 0);

        handle_redirection(args);
        execvp(args[0], args); 
        perror("execvp");
        exit(1);
    } else { 
        
        setpgid(pid, pid);

        if (background) {
            printf("Started background process with PID %d\n", pid);
            add_background_process(pid);
            return 0; 
        } else {
            foreground_pid = pid;
            int status;
            waitpid(pid, &status, 0);
            foreground_pid = -1;
            return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1; 
        }
    }
}

void fg_command(const char *arg) {
    int index = atoi(arg + 1); 
    if (index < 0 || index >= bg_count) {
        printf("Invalid background process index: %d\n", index);
        return;
    }

    pid_t pid = background_pids[index];
    printf("Bringing process %d to the foreground.\n", pid);

    foreground_pid = pid;
   

    
    kill(pid, SIGCONT);  

    int status;
    waitpid(pid, &status, WUNTRACED);  
    foreground_pid = -1;

    remove_background_process(pid);

    
    printf("\nmyshell: ");
    fflush(stdout);
}

void handle_redirection(char *args[]) {
    int i;
    int input_fd = -1, output_fd = -1, error_fd = -1;

    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            
            output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);  
            close(output_fd);
            args[i] = NULL;  
        } 
        else if (strcmp(args[i], ">>") == 0) {
           
            output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (output_fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(output_fd, STDOUT_FILENO);  
            close(output_fd);
            args[i] = NULL;  
        } 
        else if (strcmp(args[i], "<") == 0) {
           
            input_fd = open(args[i + 1], O_RDONLY);
            if (input_fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(input_fd, STDIN_FILENO);  
            close(input_fd);
            args[i] = NULL; 
        } 
        else if (strcmp(args[i], "2>") == 0) {
            
            error_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (error_fd < 0) {
                perror("open");
                exit(1);
            }
            dup2(error_fd, STDERR_FILENO);  
            close(error_fd);
            args[i] = NULL;  
        }
    }
}

void sigint_handler(int sig) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGKILL);
        printf("\nForeground process terminated.\n");
    }
}

void sigtstp_handler(int sig) {
    if (foreground_pid > 0) {
        printf("\nTerminating foreground process group %d.\n", foreground_pid);
        kill(-foreground_pid, SIGKILL); 
        foreground_pid = -1;
    }
}

void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        remove_background_process(pid);
    }
}

void add_background_process(pid_t pid) {
    if (bg_count < MAX_BG_PROCESSES) {
        background_pids[bg_count++] = pid;
    }
}

void remove_background_process(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (background_pids[i] == pid) {
            background_pids[i] = background_pids[--bg_count];
            break;
        }
    }
}

int has_running_background_processes() {
    return bg_count > 0;
}