#include <stdlib.h>
#define MAX_PROCESS_SIZE 100

int numOfProcess = 0;
float meanOfTT = 0;
float meanOfWT = 0;

typedef struct Process{
    int pid;
    int AT; //Arrival Time
    int queue; //Process's Queue
    int numOfCycle;
    int currentCycle;

    int *CPU_BTArr; //Array to save BTs
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

typedef struct Node { //node of Queue
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

typedef struct IOhandleInfo{ 
    //node of Priority Queue

    int pid;
    int destQueue; //destination queue to get in after finishing IO
    long scheduled_time; //decide priority by this variable
} IOhandleInfo;

typedef struct Priority_Queue{
    IOhandleInfo arr[MAX_PROCESS_SIZE];
    int count;
}Priority_Queue;