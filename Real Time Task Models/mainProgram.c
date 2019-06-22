#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<string.h>
#include<semaphore.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<string.h>
#include<sched.h>
#include<sys/time.h>
#include<time.h>
#define MAX 10
#define MOUSEPATH "/dev/input/mice"
#define LOG_FILE "realTime.log"


// Semaphore used for Periodic threads declared globally
sem_t sema;

//Mutex variable declared globally
pthread_mutex_t event0, event1;

//Universal Flag to take care of iterations
int universal = 1;

//File Variable to address the log file in user space
FILE *logFile;

struct tm *ttt;

time_t curtime;

long int totalIter;


/* Structure declaration for the Task variable that covers all
inputs as attributes to create the basis of threads */
struct Task
{
    char type[20];
    int priority;
    int period;
    int min;
    int max;
};


/* Function to initiate Opening of the log file in User Space */
int initiateLogFile(){
    logFile = fopen(LOG_FILE, "w");
    if(logFile == NULL){
        printf("Couldn't open the Log File");
        return -1;
    } else {
    //If opens successfully
        return 0;
    }
}

//Function to print the TimeStamps in the Log File
void getDateStamp(){
    time(&curtime);
    ttt = localtime(&curtime);
    fprintf(logFile,"%s   -  ",asctime(ttt));
}


// Function to uniformly select a random number between two given Limits
int randomGen(int min, int max){
    int ran;
    srand(time(0));
    do {
        ran = rand()%max+min;
    }while(ran < min || ran > max);
    return ran;
}


// The Templates for Periodic Threads
void *periodicTask(void* task){
    struct timespec next, period;
    struct Task *data = (void*) task;
    int j;
    long int nsec;
    //variable Iter defines the number of iterations for the computation body
    int iter = randomGen(data->min, data->max);
    fprintf(logFile,"Entered the Thread\n\n");
    clock_gettime(CLOCK_MONOTONIC,&next);

   while(universal > 0){

        // Periodic task body template for every task defined
        sem_wait(&sema);

        getDateStamp();
        fprintf(logFile,"Entered the Periodic Thread Computation\n");

        for(j=0; j < iter; j++){
            totalIter++;
        }

        nsec = nsec + (data->period)*1000000;
        period.tv_nsec = (next.tv_nsec + nsec) % 1000000000;
        period.tv_sec = next.tv_sec + (nsec / 1000000000);

        getDateStamp();
        fprintf(logFile,"Exiting the Periodic Thread Computation\n");
        sem_post(&sema);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,&period, 0);
        totalIter = 0;
    }

    return NULL;
}


// Template for the mouse thread which is a periodic thread and reads the mouse button activity till the end of the program
void *mouseThreadBody(void* args){

    int fd, hexBytes;
    unsigned char data[3];
    const char *mouseDevPath = MOUSEPATH;
    fprintf(logFile,"Entered Mouse Thread\n");

    // Open MouseFile
    fd = open(mouseDevPath, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening the path - %s\n", mouseDevPath);
    }

    int left, right, flag;
    while(universal > 0)
    {
        // Read Mouse
        hexBytes = read(fd, data, sizeof(data));

        if(hexBytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            if(left == 1 && right == 0){
            flag = 1;
            } else if(left == 0 && right == 2){
            flag  = 2;
            } else if (flag == 1 && left == 0 && right == 0){
                flag = 0;
                printf("Left button Released\n");
                pthread_mutex_unlock(&event0);
            } else if (flag == 2 && left == 0 && right == 0){
                flag = 0;
                printf("Right button Released\n");
                pthread_mutex_unlock(&event1);
            }
        }
    }
    return NULL;
}

//Template for the Aperiodic threads
void *aPeriodicThread(void* task){
    struct Task *data = (void*) task;
    int j;
    int iter = randomGen(data->min, data->max);

    while(universal > 0){
        if(data->period == 1){
            pthread_mutex_lock(&event1);

            // Task body for Mouse Right Release
            getDateStamp();
            fprintf(logFile,"Entered the Right Button Release Thread Computation\n");

            for(j=0; j < iter; j++){
                totalIter++;

            }
            getDateStamp();
            fprintf(logFile,"Exiting the Right Button Release Thread Computation\n");

        }else{

            pthread_mutex_lock(&event0);

            // Task body for Mouse Left Release

            getDateStamp();
            fprintf(logFile,"Entered the Left Button Release Thread Computation\n");

            for(j=0; j < iter; j++){
                totalIter++;
            }

            getDateStamp();
            fprintf(logFile,"Exiting the Left Button Release Thread Computation\n");
        }
        totalIter= 0;
    }
    return NULL;
}


// Signal Handler to execute when the total time of the program is out
void signalHandler(){
    printf("Entered the Signal Handler");
    universal = 0;
}



// THE MAIN FUNCTION
int main()
{
    struct Task task[MAX];
    //Parameters to copy priority
    struct sched_param param, taskParam[MAX];
    //Defining threads
    pthread_t thread[MAX], mouseThread;
    //Defining thread Attributes
    pthread_attr_t attr[MAX], mouseAttr;
    cpu_set_t mask;
    int i, nTask, totalTime;

    pid_t pid = getpid();
    printf("Process Id - %d \n", pid);
    //Receiving Inputs such as the number of process
    scanf("%d %d",&nTask, &totalTime);

//Creating a CPU affinity mask for running threads in a single cpu
    CPU_ZERO(&mask);
    CPU_SET(0,&mask);
    sched_setaffinity(0,sizeof(mask),&mask);

    initiateLogFile();

    // Reading individual Task params
    for(i = 0; i < nTask; i++ )
    {
        scanf("%s", task[i].type);
        scanf("%d", &task[i].priority);
        scanf("%d", &task[i].period);
        scanf("%d", &task[i].min);
        scanf("%d", &task[i].max);

    }

    fprintf(logFile,"\n The Execution Starts \n");

    //Defining the total time of the Process Execution
    alarm(totalTime/1000);
    signal(SIGALRM, signalHandler);

    // Mouse Thread Execution
    pthread_attr_init(&mouseAttr);
    param.sched_priority = 10;
    pthread_attr_setinheritsched(&mouseAttr, PTHREAD_INHERIT_SCHED);
    pthread_attr_setschedpolicy(&mouseAttr, SCHED_FIFO);
    pthread_attr_setschedparam(&mouseAttr, &param);

    pthread_create(&mouseThread,&mouseAttr, (void*)mouseThreadBody,NULL);

    // Defining semaphore locks for Periodic threads
    sem_init(&sema,0,1);
    sem_wait(&sema);

    //Initiating Mutexes for two types of events
    pthread_mutex_init(&event0,NULL);
    pthread_mutex_init(&event1,NULL);

    //Locking Mutex parts for Events
    pthread_mutex_lock(&event0);
    pthread_mutex_lock(&event1);


//Generating Individual threads both periodic and Aperiodic
    for(i = 0; i < nTask; i++ )
    {
        pthread_attr_init(&attr[i]);

        taskParam[i].sched_priority = task[i].priority;
        pthread_attr_setinheritsched(&attr[i], PTHREAD_INHERIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
        pthread_attr_setschedparam(&attr[i], &taskParam[i]);
        pthread_attr_setaffinity_np(&attr[i],sizeof(mask),&mask);

        if(strcmp(task[i].type,"A") == 0){
        //Creates Aperiodic Threads
            pthread_create(&thread[i], &attr[i], aPeriodicThread, &task[i]);
        }else {
        //Creates periodic threads
            pthread_create(&thread[i], &attr[i], (void*)periodicTask, &task[i]);
        }


    }

    //Activating the threads at the same time
    sem_post(&sema);

    //Joining the thread execution of mouse threads
    pthread_join(mouseThread, NULL);
    sem_destroy(&sema);
    return 0;
}
