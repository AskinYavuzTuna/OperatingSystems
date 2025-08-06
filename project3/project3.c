#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h> 
//Defines the max number of lines and length 
#define MAX_LINES 100
#define MAX_LINE_LENGTH 256

//Creates array for lines
char lines[MAX_LINES][MAX_LINE_LENGTH];
int line_count = 0; //number of lines

int read_index = 0;
int upper_index = 0;
int replace_index = 0;
int write_index = 0;

//This array keeps info about lines
int processed_flags[MAX_LINES] = {0}; 


pthread_mutex_t read_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t upper_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replace_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_complete_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t process_complete_cond = PTHREAD_COND_INITIALIZER;

int read_complete = 0;
int process_complete = 0;

FILE *file_write;

//Thread functions
void* read_thread(void* arg);
void* upper_thread(void* arg);
void* replace_thread(void* arg);
void* write_thread(void* arg);

//char functions
void to_uppercase(char* str);
void replace_spaces(char* str);

int main(int argc, char* argv[]) {
  //Input checking
    if (argc != 8 || strcmp(argv[1], "-d") != 0 || strcmp(argv[3], "-n") != 0) {
        printf("Usage: %s -d <filename> -n <Read> <Upper> <Replace> <Write>\n", argv[0]);
        return 1;
    }
  //Initialization of the variables according to inputs
    char* filename = argv[2];
    int read_count = atoi(argv[4]);
    int upper_count = atoi(argv[5]);
    int replace_count = atoi(argv[6]);
    int write_count = atoi(argv[7]);


    FILE* file_read = fopen(filename, "r");
    if (!file_read) {
        perror("Error opening file for reading");
        return 1;
    }

    //Read lines 
    while (fgets(lines[line_count], MAX_LINE_LENGTH, file_read)) {
        lines[line_count][strcspn(lines[line_count], "\n")] = 0; // Remove newline
        line_count++;
    }
    fclose(file_read);


    file_write = fopen(filename, "w");
    if (!file_write) {
        perror("Error opening file for writing");
        return 1;
    }

  
    pthread_t read_threads[read_count];
    pthread_t upper_threads[upper_count];
    pthread_t replace_threads[replace_count];
    pthread_t write_threads[write_count];
  //Read part
    for (int i = 0; i < read_count; i++)
        pthread_create(&read_threads[i], NULL, read_thread, (void*)(intptr_t)i);

    for (int i = 0; i < read_count; i++)
        pthread_join(read_threads[i], NULL);


    pthread_mutex_lock(&read_mutex);
    read_complete = 1;
    pthread_cond_broadcast(&read_complete_cond);
    pthread_mutex_unlock(&read_mutex);
  //Upper and replace parts
    for (int i = 0; i < upper_count; i++)
    pthread_create(&upper_threads[i], NULL, upper_thread, (void*)(intptr_t)i);
    
    for (int i = 0; i < replace_count; i++)
    pthread_create(&replace_threads[i], NULL, replace_thread, (void*)(intptr_t)i);
    
    for (int i = 0; i < replace_count; i++)
    pthread_join(replace_threads[i], NULL);
    
    for (int i = 0; i < upper_count; i++)
    pthread_join(upper_threads[i], NULL);
    


    pthread_mutex_lock(&upper_mutex);
    process_complete = 1;
    pthread_cond_broadcast(&process_complete_cond);
    pthread_mutex_unlock(&upper_mutex);
   //Write part
    for (int i = 0; i < write_count; i++)
        pthread_create(&write_threads[i], NULL, write_thread, (void*)(intptr_t)i);

    for (int i = 0; i < write_count; i++)
        pthread_join(write_threads[i], NULL);


    fclose(file_write);
    //Destroying mutexes
    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&upper_mutex);
    pthread_mutex_destroy(&replace_mutex);
    pthread_mutex_destroy(&write_mutex);
    pthread_cond_destroy(&read_complete_cond);
    pthread_cond_destroy(&process_complete_cond);

    return 0;
}

void* read_thread(void* arg) {
    int thread_id = (int)(intptr_t)arg;

    while (1) {
        pthread_mutex_lock(&read_mutex);
        //After reading all lines, break will work.
        if (read_index >= line_count) {
            pthread_mutex_unlock(&read_mutex);
            break;
        }

        int line = read_index++;
        printf("Read_%d         Read_%d read the line %d which is %s\n", thread_id+1,thread_id+1, line, lines[line]);
        pthread_mutex_unlock(&read_mutex);
        //we use sleep because if we don't use it code is being executed very fast
        // so all of the functions are done by same thread.
        sleep(1);
    }

    return NULL;
}

void* upper_thread(void* arg) {
    int thread_id = (int)(intptr_t)arg;

    pthread_mutex_lock(&read_mutex);
    while (!read_complete) {
        pthread_cond_wait(&read_complete_cond, &read_mutex);
    }
    pthread_mutex_unlock(&read_mutex);

    while (1) {
        pthread_mutex_lock(&upper_mutex);
        //After using upper threads for all lines, break will work.
        if (upper_index >= line_count) {
            pthread_mutex_unlock(&upper_mutex);
            break;
        }

        int line = upper_index++;
        if (processed_flags[line] == 0) {
            processed_flags[line] = 1;
            to_uppercase(lines[line]);
            printf("Upper_%d        Upper_%d read the line %d which is %s\n", thread_id+1,thread_id+1, line, lines[line]);
            //we use sleep because if we don't use it code is being executed very fast
            // so all of the functions are done by same thread.
            sleep(1);
        }
        pthread_mutex_unlock(&upper_mutex);
    }

    return NULL;
}

void* replace_thread(void* arg) {
    int thread_id = (int)(intptr_t)arg;

    pthread_mutex_lock(&read_mutex);
    while (!read_complete) {
        pthread_cond_wait(&read_complete_cond, &read_mutex);
    }
    pthread_mutex_unlock(&read_mutex);

    while (1) {
        pthread_mutex_lock(&replace_mutex);
        //After replacing all lines, break will work.
        if (replace_index >= line_count) {
            pthread_mutex_unlock(&replace_mutex);
            break;
        }

        int line = replace_index++;
        if (processed_flags[line] == 1) {
            processed_flags[line] = 2;
            replace_spaces(lines[line]);
            printf("Replace_%d      Replace_%d read the line %d which is %s\n", thread_id+1,thread_id+1, line, lines[line]);
            //we use sleep because if we don't use it code is being executed very fast
            // so all of the functions are done by same thread.
            sleep(2);
        }
        pthread_mutex_unlock(&replace_mutex);
    }

    return NULL;
}

void* write_thread(void* arg) {
    int thread_id = (int)(intptr_t)arg;

    pthread_mutex_lock(&upper_mutex);
    while (!process_complete) {
        pthread_cond_wait(&process_complete_cond, &upper_mutex);
    }
    pthread_mutex_unlock(&upper_mutex);

    while (1) {
        pthread_mutex_lock(&write_mutex);
        //After writing all lines, break will work.
        if (write_index >= line_count) {
            pthread_mutex_unlock(&write_mutex);
            break;
        }

        int line = write_index++;
        if (processed_flags[line] == 2) {
            fprintf(file_write, "%s\n", lines[line]);
            printf("Write_%d        Write_%d read the line %d which is %s\n", thread_id+1,thread_id+1, line, lines[line]);
            fflush(file_write);
            //we use sleep because if we don't use it code is being executed very fast
            // so all of the functions are done by same thread.
            sleep(1);
        }
        pthread_mutex_unlock(&write_mutex);
    }

    return NULL;
}
//Line modifying functions
void to_uppercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void replace_spaces(char* str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == ' ') {
            str[i] = '_';
        }
    }
}
