#include <stdlib.h>
#define MAX_PROCESS_SIZE 100

int numOfProcess = 0;
float meanOfTT = 0;
float meanOfWT = 0;

typedef struct Process{
    int pid;
    int AT;
    int queue;
    int numOfCycle;
    int currentCycle;
    int *CPU_BTArr;
    int *IO_BTArr;
    int sumOfCPU_BT;
    int sumOfIO_BT;
    int start_time;
    int finish_time;
    int turnaround_time;
    int waiting_time;
    //status = -1(not started yet) , 0(started) , 1(finished)
    int status;
} Process;

typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct Queue{
    Node *front;
    Node *rear; 
    int count;
} Queue;

typedef struct MFQ{
    Queue *q0;
    Queue *q1;
    Queue *q2;
    Queue *q3;
} MFQ;

// void InitQueue(Queue *queue);

// bool empty(Queue *queue);

// void push(Queue *queue, int data);

// void pop(Queue *queue);

// int front(Queue *queue);


typedef struct IOhandleInfo
{
    int pid;
    int destQueue;
    //this is priority
    long scheduled_time;
} IOhandleInfo;

typedef struct Priority_Queue{
    IOhandleInfo arr[MAX_PROCESS_SIZE];
    int count;
}Priority_Queue;

// void swap_IOhandleInfo(IOhandleInfo *a, IOhandleInfo *b);

// int push_priority(Priority_Queue *pri_queue, IOhandleInfo node);

// void pop_priority(Priority_Queue* pri_queue);

// IOhandleInfo top_priority(Priority_Queue *pri_queue);

// bool empty_priority(Priority_Queue *pri_queue);