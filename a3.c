
#include<stdio.h>
#include <stddef.h>
#include<string.h>
#include<assert.h>
#include<pthread.h>
#include<time.h>
#include<stdlib.h>
#include<sys/queue.h>
#include<stdint.h>
#include <unistd.h>

///////////////////SLEEP and time FUNC////////////////////////
#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000

static void microsleep(unsigned int usecs)
{
    long seconds = usecs / USEC_PER_SEC;
    long nanos   = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = { .tv_sec = seconds, .tv_nsec = nanos };
    int ret;
    do
    {
        ret = nanosleep( &t, &t );
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}

typedef struct timespec timespec;

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

uint64_t as_nanoseconds(struct timespec* ts) {
    return ts->tv_sec * (uint64_t)1000000000L + ts->tv_nsec;
}

////////////////////*******///////////////////////////

#define MAX_LINE 100  /// max length of line in text file
#define QUEUE_QUANTUM 50  /// quantum length for all queues
#define TIME_ALLOTMENT 200  // time allotment for priority reduction
#define HI_QUEUE 2 /// index of the highest priority queue


typedef struct task_info
{
    int id;
    char name[MAX_LINE/4];
    int time_length;
    int io_odds;
    
    ///extra paramaters
    int allotLeft;   /// timeAllotment left at current priority
    int curr_priority; /// Queue priority level 
    //  time 
    timespec arrival; // arrival on  system 
    timespec start; /// first time on CPU
    timespec end;  // done time 
    int firstRun; /// if firstTime ran on CPU -1 else 0

    //time needed start,end,firstRun
    TAILQ_ENTRY(task_info) entries;
}tasks;

TAILQ_HEAD(Queue,task_info);

struct Queue readyQueue; /// ready to schedule queue 
struct Queue doneQueue; // finished tasks queue
struct Queue runnungQueues[HI_QUEUE + 1];  //// Queue's with 3 priority levels

///locks and conditional variables
pthread_mutex_t readyLock,doneLock,runningLock;
pthread_cond_t readyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t CPU_wait = PTHREAD_COND_INITIALIZER;
pthread_cond_t boost_start = PTHREAD_COND_INITIALIZER;


int totalTasks = 0; // num of total tasks inputted
int finishedTasks = 0; /// num of tasks finished their time 
int runningTask= 0;  // num of currently task running 

int S; // value of S
int numOfCPUs; // number of CPU in work
int available_CPUs = 0; // idle CPUs

int boosted = 0; // num of times boosted - debug variable

pthread_t reader,scheduler,booster_thread;

///booleans
int doneReading = 0; /// indicates if we done reading the file
int no_task = 0;  // no task left to schedule
int first_delay = 0; /// if first delay found
int priorityBoost = 0;/// cheecks if priority boost in progress 

/// function declarations
void* reading_thread(void*);
void* scheduler_thread();
void* CPU_thread();
void* checkPriorityBoost(); /// sleeps and wakes up to do the priority boost


int main(int argc,char* argv[]){

    // must the 3 cmd arguments - numOfCPUs, intervalTime S, tasks fileName
    assert(argc == 4);
    numOfCPUs = atoi(argv[1]);
    S = atoi(argv[2]);
    char* fileName = argv[3];


    /// initializing locks and queues
    TAILQ_INIT(&readyQueue);
    TAILQ_INIT(&doneQueue);
    pthread_mutex_init(&readyLock,NULL);
    pthread_mutex_init(&doneLock,NULL);
    pthread_mutex_init(&runningLock,NULL);
    for(int i=0;i<=HI_QUEUE;i++){
        TAILQ_INIT(&runnungQueues[i]);
    }

    /// CPU threads
    pthread_t cpu_threads[numOfCPUs];
    for(int i =0;i<numOfCPUs;i++){
        pthread_create(&cpu_threads[i],NULL,CPU_thread,NULL); // start CPU thread then wait for task to load on scheduler
    }

    pthread_create(&reader,NULL,reading_thread,fileName); // run reading thread
    pthread_create(&scheduler,NULL,scheduler_thread,NULL); // run scheduler 
    pthread_create(&booster_thread,NULL,checkPriorityBoost,NULL); /// run booster thread which will be waiting for scheduler to start
    
    // join to main thread
    pthread_join(reader,NULL);
    pthread_join(scheduler,NULL);
    for (int i = 0; i < numOfCPUs; i++)
    {
        pthread_join(cpu_threads[i],NULL);
    }
    

    //// report
    int numOfids = 4;

    int iDs[numOfids];
    uint64_t total_time[numOfids];
    uint64_t res_time[numOfids];

    for(int i=0;i<numOfids;i++){
        iDs[i] = 0;
        total_time[i] = 0;
        res_time[i] = 0;
    }

    tasks* temp = TAILQ_FIRST(&doneQueue);
    tasks* prev = NULL;

    while(temp != NULL){
        int curr_id = temp->id;
        timespec time= diff(temp->arrival,temp->end);
        total_time[curr_id] += as_nanoseconds(&time) ;
        time= diff(temp->arrival,temp->start);
        res_time[curr_id] += as_nanoseconds(&time);
        iDs[curr_id] += 1;

        prev = temp;
        temp = TAILQ_NEXT(temp,entries);
        TAILQ_REMOVE(&doneQueue,prev,entries);
        free(prev);
    }

    printf("Num of CPUs: %d\nS: %d\nfile: %s\n",numOfCPUs,S,fileName);
    printf("\nAverage turnaround times:\n");
    for(int i=0;i<numOfids;i++){
        printf("Type %d: %lu usec\n",i,(total_time[i]/1000)/iDs[i]);
    }

    printf("\nAverage response times:\n");
    for(int i=0;i<numOfids;i++){
        printf("Type %d: %lu usec\n",i,(res_time[i]/1000)/iDs[i]);
    }


    return EXIT_SUCCESS;
}

void* reading_thread(void* fileName){

    fileName = (char*)fileName;
    FILE* fileptr = fopen(fileName,"r");

    if(fileptr == NULL){
        printf("Couldn't open file %s \n",fileName);
        pthread_exit(NULL);
    }

    char buffer[MAX_LINE];

    while(fgets(buffer,MAX_LINE,fileptr) != NULL){
        char* token = strtok(buffer," ");

        if(strcmp(token,"DELAY") == 0){
            //printf("found delay sleep reading process\n");
            first_delay = 1;  // set flag
            pthread_cond_signal(&readyCond);  // start scheduler
            // delay
            token = strtok(NULL," ");
            int delayTime = atoi(token);
            microsleep(delayTime * 1000); /// mili to micro seconds
        }
        else{
            tasks* newTask = (tasks*) malloc(sizeof(tasks));
            strcpy(newTask->name,token);
            token = strtok(NULL," ");
            newTask->id = atoi(token);
            token = strtok(NULL," ");
            newTask->time_length = atoi(token);
            token = strtok(NULL," ");
            newTask->io_odds = atoi(token);
            
            // add to readyQueue 
            pthread_mutex_lock(&readyLock);
            TAILQ_INSERT_TAIL(&readyQueue,newTask,entries);
            totalTasks++;
            pthread_mutex_unlock(&readyLock);
        }   
    }

    doneReading = 1; // set flag reading done
    pthread_exit(NULL);

}


void* scheduler_thread(){
    
    while (!no_task){  /// run untill all tasks finished
        if(!doneReading){
            pthread_mutex_lock(&readyLock);
            while(first_delay == 0 ){ // wait for 1st delay in reader
                pthread_cond_wait(&readyCond,&readyLock);
                pthread_cond_signal(&boost_start);
            }
            /// add new tasks from readyQueue to top Queue
            tasks* currSched = TAILQ_FIRST(&readyQueue);  
            while(TAILQ_EMPTY(&readyQueue) == 0){
                TAILQ_REMOVE(&readyQueue,currSched,entries);
                
                timespec curr_time;
                clock_gettime(CLOCK_REALTIME, &curr_time);
                currSched->arrival = curr_time;
                currSched->firstRun = -1;
                //printf("new task- %s\n",currSched->name);

                pthread_mutex_lock(&runningLock);
                TAILQ_INSERT_TAIL(&runnungQueues[HI_QUEUE],currSched,entries);
                pthread_mutex_unlock(&runningLock);
                currSched = TAILQ_FIRST(&readyQueue);
            }
            
            pthread_mutex_unlock(&readyLock);
        }

        if(available_CPUs > 0){ /// wake up all CPU to perform tasks
            pthread_mutex_lock(&runningLock);
            available_CPUs--;
            pthread_cond_signal(&CPU_wait);
            pthread_mutex_unlock(&runningLock);

        }

        /// if all tasks done and no new task to read
        if(finishedTasks == totalTasks && doneReading){  //  if all tasks finished and no new task to read then exit
            no_task = 1;
            while(available_CPUs < numOfCPUs); // wait for all CPU to check in
            pthread_cond_broadcast(&CPU_wait); /// sends signal to exit all
        }
    }

    pthread_join(booster_thread,NULL); // wait for booster thread to finish
    //printf("Scheduler done\n");
    pthread_exit(NULL);
}


/// CPU thread
void* CPU_thread(){

 /// check in if CPU up and waits for scheduler to load tasks in Queue
    pthread_mutex_lock(&runningLock);
    available_CPUs++;
    //printf("CPU UP and available CPUs %d \n",available_CPUs);
    pthread_cond_wait(&CPU_wait,&runningLock);
    pthread_mutex_unlock(&runningLock);

    int done = 0; // all done 

    while(!done){
        
        int currQueue_idx = -1;
        tasks* currTask = NULL;
        timespec curr_time;

        /// look for task to perform from highest to lowest Queue
        for(int i=HI_QUEUE;i>=0 && currQueue_idx == -1;i--){
            pthread_mutex_lock(&runningLock);
            if(!TAILQ_EMPTY(&runnungQueues[i])){
                currQueue_idx = i;
                currTask = TAILQ_FIRST(&runnungQueues[i]); 
                TAILQ_REMOVE(&runnungQueues[i],currTask,entries);  
            }
            pthread_mutex_unlock(&runningLock);
        }
        

        if(currTask != NULL && currQueue_idx != -1){ 
            
            if(currTask->firstRun == -1){ // if a task enters CPU for the first time
                pthread_mutex_lock(&runningLock);
                runningTask++;
                pthread_mutex_unlock(&runningLock);

                /// initial task values
                currTask->firstRun = 0;
                currTask->allotLeft = TIME_ALLOTMENT;
                currTask->curr_priority = currQueue_idx;
                clock_gettime(CLOCK_REALTIME, &curr_time);
                currTask->start = curr_time;
            }
            else{
                currQueue_idx = currTask->curr_priority; // if it ran before it could have different priority if IO was performed recently
            }
            

            int time_slice = QUEUE_QUANTUM; // default time Queue quantum
            // if time left is less than quantum
            if(currTask->time_length < QUEUE_QUANTUM ){ 
                time_slice = currTask->time_length;
            }
            // if time left in current priority is less than time need to perform and task is not in lowest priority -- need to check with prof
            if(time_slice > currTask->allotLeft && currQueue_idx> 0){ 
                time_slice = currTask->allotLeft;
            }
            
            ///checking for IO
            int io = 0;
            int random_io = rand() % 101;
            if(random_io <= currTask->io_odds){
                io = 1;
                time_slice = rand() % (time_slice+1);
            }
            
            microsleep(time_slice); // excute task
           
            // update time left and allotment left 
            currTask->time_length -= time_slice; 
            currTask->allotLeft -= time_slice;

            if(currTask->allotLeft <= 0){ // if alloted time done then reduce priority level
                currTask->allotLeft = TIME_ALLOTMENT; // reset 
                if(currQueue_idx > 0){ // if already not in low priority
                    //printf("task reducing priority %s\n",currTask->name);
                    currQueue_idx -= 1;
                    currTask->curr_priority = currQueue_idx;
                }
            }

            if(currTask->time_length == 0){//done task
                clock_gettime(CLOCK_REALTIME, &curr_time);
                currTask->end = curr_time;
                
                pthread_mutex_lock(&doneLock);
                TAILQ_INSERT_TAIL(&doneQueue,currTask,entries);
                finishedTasks++;
                pthread_mutex_unlock(&doneLock);

                pthread_mutex_lock(&runningLock);
                runningTask--;
                pthread_mutex_unlock(&runningLock);
            }
            else{ /// add to Queue and schedule again
                pthread_mutex_lock(&runningLock);
                if(io){ /// added to first for immediate scheduling if IO was performed
                    TAILQ_INSERT_HEAD(&runnungQueues[HI_QUEUE],currTask,entries);
                }
                else{
                    TAILQ_INSERT_TAIL(&runnungQueues[currTask->curr_priority],currTask,entries);
                }
                pthread_mutex_unlock(&runningLock);
            }
        }
        pthread_mutex_lock(&runningLock);
        if(no_task){ // if no task available to schedule then wait for scheduler signal to exit
            available_CPUs++;
            pthread_cond_wait(&CPU_wait,&runningLock);
            done = 1;
        }
        pthread_mutex_unlock(&runningLock);      
    }
    //printf("CPU exited\n");
    pthread_exit(NULL);
}


/// sleep and wake up after S and boosts the tasks priority
void* checkPriorityBoost(){   
    // wait to start scheduling
    pthread_mutex_lock(&readyLock);
    while(first_delay == 0){
        pthread_cond_wait(&boost_start,&readyLock);
    }
    pthread_mutex_unlock(&readyLock);

    int s_in_micro = S * 1000;

    while(!no_task){
        //printf("sleeping\n");
        microsleep(s_in_micro);
        //printf("BOOST\n");
        pthread_mutex_lock(&runningLock);
        priorityBoost = 1;
        boosted++;
        /// adding all tasks to the HI-QUEUE
        for(int i = 0;i<HI_QUEUE;i++){
            tasks* temp = TAILQ_FIRST(&runnungQueues[i]);
            tasks* curr = NULL;
            while(temp != NULL){
                curr = temp;
                temp = TAILQ_NEXT(temp,entries);
                TAILQ_REMOVE(&runnungQueues[i],curr,entries);
                ///reset time allotment and priority
                curr->allotLeft = TIME_ALLOTMENT; 
                curr->curr_priority = HI_QUEUE;
                /// add to top queue
                TAILQ_INSERT_TAIL(&runnungQueues[HI_QUEUE],curr,entries);
            }
        }
        priorityBoost = 0;
        pthread_mutex_unlock(&runningLock);
    }
    pthread_exit(NULL);
}


