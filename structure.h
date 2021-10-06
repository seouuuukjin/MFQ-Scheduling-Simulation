#include<queue>

using namespace std;

typedef struct Process
{
    int pid;
    int AT;
    int queue;
    int numOfCycle;
    int currentCycle;
    int *CPU_BTArr;
    int *IO_BTArr;
    int start_time;
    int finish_time;

    //status = -1(not started yet) , 0(started) , 1(finished)
    int status;
} Process;

typedef struct Node {
    int data;
    struct Node *next;
}Node;

typedef struct Queue{
    Node *front;
    Node *rear; 
    int count;
}Queue;

void InitQueue(Queue *queue){
    queue->front = queue->rear = NULL;
    queue->count = 0;
}

bool empty(Queue *queue){
    if(queue->count == 0)  
        return true;
    else
        return false;
}
void push(Queue *queue, int data){
    Node *now = (Node *)malloc(sizeof(Node));
    now->data = data;
    now->next = NULL;
 
    if (empty(queue)){
        queue->front = now;     
    }
    else{
        queue->rear->next = now;
    }
    queue->rear = now;
    queue->count++;
}
void pop(Queue *queue)
{
    Node *now;
    if (empty(queue)){
        return;
    }
    now = queue->front;
    queue->front = now->next;
    free(now);
    queue->count--;
}
int front(Queue *queue){
    return queue->front->data;
}

typedef struct MFQ
{
    queue<int> q0;
    queue<int> q1;
    queue<int> q2;
    queue<int> q3;
};

typedef struct IOhandleInfo
{
    int pid;
    int destQueue;
    long scheduled_time;
};

struct compare{
    bool operator()(IOhandleInfo &a, IOhandleInfo&b){
        if(a.scheduled_time != b.scheduled_time)
            return a.scheduled_time > b.scheduled_time;
        return a.destQueue > b.destQueue;
    }
};
//print whole information of All the current Processes
void printProcessArr(Process * process_arr, int numOfProcess);
//Check whether the process is started already
bool checkProcessInit(Process * process_arr, int pid);
//Check if the process is finished
bool checkProcessDone(Process * process_arr , int length);
//mark the process as it is now started
void markAsStarted(Process * process_arr, int pid, int current_cpu_time);
//mark the process as it is now finished
void markAsFinished(Process * process_arr, int pid, int current_cpu_time);