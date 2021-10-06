#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include"DataStructures.h"
#include"Functions.h"

//Process Array printing function for testing 
void printProcessArr(Process * process_arr, int numOfProcess){
    for(int i=1;i<numOfProcess + 1; i++){
        printf("%d %d %d %d\n", process_arr[i].pid, process_arr[i].AT, process_arr[i].queue, process_arr[i].numOfCycle);
        for(int j=0; j<process_arr[i].numOfCycle;j++){
            printf("(%d, %d) ", process_arr[i].CPU_BTArr[j], process_arr[i].IO_BTArr[j]);
        }
        // printf("(%d, _ )\n", process_arr[i].CPU_BTArr[process_arr[i].numOfCycle - 1]);
        printf("!\n");
    }
}
//return status code of the process
int checkProcessInit(Process * process_arr, int pid){
    //return true if the process is started already.
    if(process_arr[pid].status == -1)
        return -1;
    else if (process_arr[pid].status == 0)
        return 0;
    else
        return 1;
}
//return true if All processes are done(=marked as finished), else false.
bool checkProcessDone(Process * process_arr , int length){
    for(int i=1; i<(length+1); i++){
        if(process_arr[i].status != 1)
            return false;
    }
    return true;
}

//change the status of Process as "started" and record the start_time
void markAsStarted(Process * process_arr, int pid, int current_cpu_time){
    process_arr[pid].start_time = current_cpu_time;
    process_arr[pid].status = 0;
    printf("process #%d Initial Start\n", pid);
}

//change the status of Process as "finished" and record the finish_time
void markAsFinished(Process * process_arr, int pid, int current_cpu_time){
    process_arr[pid].finish_time = current_cpu_time + 1;
    process_arr[pid].status = 1;
    process_arr[pid].turnaround_time = process_arr[pid].finish_time - process_arr[pid].AT;
    process_arr[pid].waiting_time = process_arr[pid].finish_time - process_arr[pid].sumOfIO_BT - process_arr[pid].sumOfCPU_BT;
    printf("process #%d OUT (Finished)\n", pid);
}
//return Sum of CPU Burst
int sumOfCPUBurst(Process * process_arr, int pid){
    int length = process_arr[pid].numOfCycle;
    int sum = 0;
    for(int i=0; i<length;i++){
        sum += process_arr[pid].CPU_BTArr[i];
    }
    return sum;
}
//return Sum of IO Burst
int sumOfIOBurst(Process * process_arr, int pid){
    int length = process_arr[pid].numOfCycle;
    int sum = 0;
    for(int i=0; i<length;i++){
        sum += process_arr[pid].IO_BTArr[i];
    }
    return sum;
}

int main(int argc, char* argv[]){

    long cpu_time_count= -1;

    if (argc != 2) {
        printf("Usage: %s <infile>\n", argv[0]);
        return 0;
    }

    char *fn = argv[1];
    FILE *fin = fopen(fn, "r");

    int pid, AT, initQueue, numOfCycle;

    MFQ *mfq = (MFQ *)malloc(sizeof(MFQ));

    mfq->q0 = malloc(sizeof(Queue));
    mfq->q1 = malloc(sizeof(Queue));
    mfq->q2 = malloc(sizeof(Queue));
    mfq->q3 = malloc(sizeof(Queue));
    InitQueue(mfq->q0);
    InitQueue(mfq->q1);
    InitQueue(mfq->q2);
    InitQueue(mfq->q3);

    Priority_Queue * IOhandleQueue;
    IOhandleQueue = (Priority_Queue *)malloc(sizeof(Priority_Queue));
    IOhandleQueue->count = 0;

    //read number of process and make an array with type of Process
    fscanf(fin, "%d\n", &numOfProcess);
    Process *process_arr = (Process *)malloc(sizeof(Process) * (numOfProcess+1));

    //read rest of inputs and save it on process array we just had made
    while (fscanf(fin, "%d %d %d %d ", &pid, &AT, &initQueue, &numOfCycle) > 0) {
        process_arr[pid].AT = AT;
        process_arr[pid].pid = pid;
        process_arr[pid].queue = initQueue;
        process_arr[pid].numOfCycle = numOfCycle;
        process_arr[pid].CPU_BTArr = (int *)malloc(sizeof(int) * numOfCycle);
        process_arr[pid].IO_BTArr = (int *)malloc(sizeof(int) * numOfCycle);
        process_arr[pid].start_time = -1;
        process_arr[pid].status = -1;;
        process_arr[pid].currentCycle = 0;

        for(int i=0; i<(numOfCycle - 1); i++){
            fscanf(fin, "%d %d ", &process_arr[pid].CPU_BTArr[i], &process_arr[pid].IO_BTArr[i]);
        }
        fscanf(fin, "%d\n", &process_arr[pid].CPU_BTArr[numOfCycle-1]);
        process_arr[pid].IO_BTArr[numOfCycle - 1] = 0;

        process_arr[pid].sumOfCPU_BT = sumOfCPUBurst(process_arr, pid);
        process_arr[pid].sumOfIO_BT = sumOfIOBurst(process_arr, pid);

        if(initQueue == 0)
            push(mfq->q0, pid);
            // mfq->q0.push(pid);
        else if(initQueue == 1)
            push(mfq->q1, pid);
            // mfq->q1.push(pid);
        else if(initQueue == 2)
            push(mfq->q2, pid);
            // mfq->q2.push(pid);
        else if(initQueue == 3)
            push(mfq->q3, pid);
            // mfq->q3.push(pid);

    }

    //input reading is over & also all processes are now in the initial Queue of MFQ.
    //now we need to handle the processes to be done.
    int q1_count=0; //variable for count time quantum of Q1
    int q2_count=0; //variable for count time quantum of Q2

    printf("Scheduling Start\n");
    printf("-----------------------------------------------------\n");
    // 1 iteration of this loop == 1 cpu cyle(cpu time)
    while(1){
        //exit condition
        if(checkProcessDone(process_arr, numOfProcess)){
            break;
        }

        cpu_time_count++;
        printf("cpu-time = %-10ld", cpu_time_count);
        printf("|");
        //if there is any process which finished its own IO Burst time, make them get in the MFQ again.
        if(!(empty_priority(IOhandleQueue)) && ((top_priority(IOhandleQueue).scheduled_time) <= cpu_time_count)){
            int targetPid = top_priority(IOhandleQueue).pid;
            switch (top_priority(IOhandleQueue).destQueue){
            case 0:
                push(mfq->q0, targetPid);
                break;
            case 1:
                push(mfq->q1, targetPid);
                break;
            case 2:
                push(mfq->q2, targetPid);
                break;
            case 3:
                push(mfq->q3, targetPid);
                break;
            }
            pop_priority(IOhandleQueue);
        }
        
        if(!(empty(mfq->q0))){
            //time quantum is 1
            int targetPid = front(mfq->q0);
            bool flag = false;
            if(checkProcessInit(process_arr, targetPid) == -1){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
                flag = true;
            }

            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                //use all time quantum for CPU Burst and pass the process to q1
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                push(mfq->q1, targetPid);
                pop(mfq->q0);
                if(checkProcessInit(process_arr, targetPid) == 0  && flag == false)
                    printf("process #%d\n", targetPid);
            }
            else{
                //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                //and put it in IOHandleQueue.
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){ //if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                   //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때, 
                    IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                    push_priority(IOhandleQueue, tmp);
                    pop(mfq->q0);
                    startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                }
                else{ //if this index is the end of process
                    //즉, 이번 차례의 IO burst가 0이어서, process가 처리해야할 Burst들을 모두 처리한 상황일 때,
                    pop(mfq->q0);
                    markAsFinished(process_arr, targetPid, cpu_time_count);

                }
            }
            continue;
        }
        else if(!(empty(mfq->q1))){
            //time quantum is 2

            int targetPid = front(mfq->q1);
            bool flag = false;

            if(checkProcessInit(process_arr, targetPid) == -1){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
                flag = true;
            }

            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            if(q1_count == 1){
                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                    pop(mfq->q1);
                    push(mfq->q2, targetPid);
                    if(checkProcessInit(process_arr, targetPid) == 0 && flag == false)
                        printf("process #%d\n", targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){//if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                        //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때,
                        
                        IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        push_priority(IOhandleQueue, tmp);
                        pop(mfq->q1);
                        startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                    }
                    else{
                        pop(mfq->q1);
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                    }
                }
                q1_count = 0;
            }
            else{
                q1_count++;

                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                    if(checkProcessInit(process_arr, targetPid) == 0 && flag == false)
                        printf("process #%d\n", targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){//if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                        //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때,
                        
                        IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        push_priority(IOhandleQueue, tmp);
                        pop(mfq->q1);;
                        startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                    }
                    else{
                        pop(mfq->q1);
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                    }
                    q1_count =0;
                }
            }
            continue;
        }
        else if(!(empty(mfq->q2))){
            // printf("[q2]");
            //time quantum is 4

            int targetPid = front(mfq->q2);
            bool flag = false;

            if(checkProcessInit(process_arr, targetPid) == -1){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
                flag = true;
            }
            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            //printf("-<%d:%d> : %d-\n", targetPid, currentIdxOfCycle, process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]);
            if(q2_count == 3){
                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    //printf("-<%d:%d> : %d-\n", targetPid, currentIdxOfCycle, process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]);
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                    pop(mfq->q2);
                    push(mfq->q3, targetPid);
                    if(checkProcessInit(process_arr, targetPid) == 0 && flag == false)
                        printf("process #%d\n", targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){//if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                        //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때,
                        
                        IOhandleInfo tmp = {targetPid, 1, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        push_priority(IOhandleQueue, tmp);
                        //printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        pop(mfq->q2);
                        startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                    }
                    else{
                        pop(mfq->q2);
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        //printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                }
                q2_count = 0;
            }
            else{
                q2_count++;

                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    //printf("-<%d:%d> : %d-\n", targetPid, currentIdxOfCycle, process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]);
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                    if(checkProcessInit(process_arr, targetPid) == 0 && flag == false)
                        printf("process #%d\n", targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                        //if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                        //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때,

                        IOhandleInfo tmp = {targetPid, 1, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        push_priority(IOhandleQueue, tmp);
                        //printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        pop(mfq->q2);
                        startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                    }
                    else{
                        pop(mfq->q2);
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        //printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                    q2_count = 0;
                }
            }
            continue;
        }
        else if(!(empty(mfq->q3))){
            // printf("[q3]");
            //FCFS scheduling
            int targetPid = front(mfq->q3);
            bool flag = false;

            if(checkProcessInit(process_arr, targetPid) == -1){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
                flag = true;
            }
            //In this, scheduling queue, we need to spent all time for current CPU Burst time
            //and, pass it to IO handle Queue
            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            //printf("-<%d:%d> : %d-\n", targetPid, currentIdxOfCycle, process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]);
            if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                //use all time quantum for CPU Burst and pass the process to q1
                //printf("-<%d:%d> : %d-\n", targetPid, currentIdxOfCycle, process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]);
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                if(checkProcessInit(process_arr, targetPid) == 0 && flag == false)
                    printf("process #%d\n", targetPid);
            }
            else{
                //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                //and put it in IOHandleQueue.
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                    //if curren index of IO Burst is not ZERO (즉, 이번 차례의 IO burst가 0이 아닐때)
                   //다시 말하면, 이번차례의 IO Burst가 0이 아니어서 Process 중간의 IOburst일 때,
                   
                    IOhandleInfo tmp = {targetPid, 2, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle])};
                    push_priority(IOhandleQueue, tmp);
                    //printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                    pop(mfq->q3);
                    startProcessIO(process_arr, targetPid, currentIdxOfCycle);
                }
                else{
                    pop(mfq->q3);
                    markAsFinished(process_arr, targetPid, cpu_time_count);
                    //printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                }
            }
            continue;
        }
        else{
            printf("currently All processes are in IO\n");
        }
    }

    for(int i =1; i<numOfProcess+1; i++){
        printf("Process #%-3d [TT: %-7d WT: %-7d Start Time: %-7d Finish Time: %-7d]\n", i , process_arr[i].turnaround_time, process_arr[i].waiting_time, process_arr[i].start_time, process_arr[i].finish_time);
        meanOfTT += process_arr[i].turnaround_time;
        meanOfWT += process_arr[i].waiting_time;
    }
    meanOfWT /= numOfProcess;
    meanOfTT /= numOfProcess;
    printf("mean of Turn Aroundtime: %.2f , mean of Waiting Time: %f\n", meanOfTT, meanOfWT);
    
    free(process_arr);
    free(mfq->q0);
    free(mfq->q1);
    free(mfq->q2);
    free(mfq->q3);
    free(mfq);
    free(IOhandleQueue);
    return 0;
}