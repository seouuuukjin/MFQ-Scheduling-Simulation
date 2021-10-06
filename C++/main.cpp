#include<iostream>
#include<stdlib.h>
#include<queue>
#include"structure.h"

using namespace std;
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

bool checkProcessInit(Process * process_arr, int pid){
    //return true if the process is started already.
    if(process_arr[pid].status == -1)
        return true;
    else
        return false;
}

bool checkProcessDone(Process * process_arr , int length){
    for(int i=1; i<(length+1); i++){
        if(process_arr[i].status != 1)
            return false;
    }
    return true;
}

void markAsStarted(Process * process_arr, int pid, int current_cpu_time){
    process_arr[pid].start_time = current_cpu_time;
    process_arr[pid].status = 0;
}

void markAsFinished(Process * process_arr, int pid, int current_cpu_time){
    process_arr[pid].finish_time = current_cpu_time;
    process_arr[pid].status = 1;
}

int main(int argc, char* argv[]){

    long cpu_time_count= -1;

    if (argc != 2) {
        printf("Usage: %s <infile>\n", argv[0]);
        return 0;
    }

    char *fn = argv[1];
    FILE *fin = fopen(fn, "r");

    int numOfProcess;
    int pid, AT, initQueue, numOfCycle;
    MFQ* mfq = new MFQ();
    priority_queue<IOhandleInfo, vector<IOhandleInfo>, compare> IOHandleQueue;

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

        if(initQueue == 0)
            mfq->q0.push(pid);
        else if(initQueue == 1)
            mfq->q1.push(pid);
        else if(initQueue == 2)
            mfq->q2.push(pid);
        else if(initQueue == 3)
            mfq->q3.push(pid);

    }
    printProcessArr(process_arr, numOfProcess);
    //input reading is over & also all processes are now in the initial Queue of MFQ.
    //now we need to handle the processes to be done.

    int q1_count=0;
    int q2_count=0;
    cout << "start" << endl;

    // 1iteration of this loop == 1 cpu cyle(cpu time)
    while(1){

        if(cpu_time_count > 500)
            break;
        //exit condition
        if(checkProcessDone(process_arr, numOfProcess)){
            break;
        }

        cpu_time_count++;
        printf("cpu time = %d -->\n", cpu_time_count);

        //if there is any process which finished its own IO Burst time, make them get in the MFQ again.
        if(!(IOHandleQueue.empty()) && ((IOHandleQueue.top().scheduled_time) <= cpu_time_count)){
            cout << IOHandleQueue.top().pid << "/" << IOHandleQueue.top().destQueue << "/" << IOHandleQueue.top().scheduled_time << endl;
            int targetPid = IOHandleQueue.top().pid;
            printf("#%d's IO Burst Over, now(%ld) go into the MFQ\n", targetPid, cpu_time_count);
            switch (IOHandleQueue.top().destQueue){
            case 0:
                mfq->q0.push(targetPid);
                break;
            case 1:
                mfq->q1.push(targetPid);
                break;
            case 2:
                mfq->q2.push(targetPid);
                break;
            case 3:
                mfq->q3.push(targetPid);
                break;
            }
            IOHandleQueue.pop();
        }
        
        if(!(mfq->q0.empty())){
            printf("[q0]:");
            //time quantum is 1
            
            int targetPid = mfq->q0.front();
            if(!(checkProcessInit(process_arr, targetPid))){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
            }

            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                //use all time quantum for CPU Burst and pass the process to q1
                cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                mfq->q1.push(targetPid);
                mfq->q0.pop();
            }
            else{
                //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                //and put it in IOHandleQueue.
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                    
                    IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                    IOHandleQueue.push(tmp);
                    mfq->q0.pop();
                    printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                    process_arr[targetPid].currentCycle++;
                    process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                }
                else{
                    mfq->q0.pop();
                    markAsFinished(process_arr, targetPid, cpu_time_count);
                    printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                }
            }
            continue;
        }
        else if(!(mfq->q1.empty())){
            cout << "[q1]";
            //time quantum is 2

            int targetPid = mfq->q1.front();
            

            if(!(checkProcessInit(process_arr, targetPid))){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
            }

            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            if(q1_count == 1){
                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;

                    mfq->q1.pop();
                    mfq->q2.push(targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                        
                        IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        IOHandleQueue.push(tmp);
                        printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        mfq->q1.pop();
                        process_arr[targetPid].currentCycle++;
                        process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                    }
                    else{
                        mfq->q1.pop();
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                }
                q1_count = 0;
            }
            else{
                q1_count++;

                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                        
                        IOhandleInfo tmp = {targetPid, 0, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        IOHandleQueue.push(tmp);
                        printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        mfq->q1.pop();
                        process_arr[targetPid].currentCycle++;
                        process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                    }
                    else{
                        mfq->q1.pop();
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                    q1_count =0;
                }
            }
            continue;
        }
        else if(!(mfq->q2.empty())){
            cout << "[q2]";
            //time quantum is 4

            int targetPid = mfq->q2.front();
           
            if(!(checkProcessInit(process_arr, targetPid))){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
            }
            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            if(q2_count == 3){
                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;

                    mfq->q2.pop();
                    mfq->q3.push(targetPid);
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                        
                        IOhandleInfo tmp = {targetPid, 1, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        IOHandleQueue.push(tmp);
                        printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        mfq->q2.pop();
                        process_arr[targetPid].currentCycle++;
                        process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                    }
                    else{
                        mfq->q2.pop();
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                }
                q2_count = 0;
            }
            else{
                q2_count++;

                if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                    //use all time quantum for CPU Burst and pass the process to q1
                    cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
                }
                else{
                    //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                    //and put it in IOHandleQueue.
                    process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                    if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                        IOhandleInfo tmp = {targetPid, 1, 1 + cpu_time_count + process_arr[targetPid].IO_BTArr[currentIdxOfCycle]};
                        IOHandleQueue.push(tmp);
                        printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                        mfq->q2.pop();
                        process_arr[targetPid].currentCycle++;
                        process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                    }
                    else{
                        mfq->q2.pop();
                        markAsFinished(process_arr, targetPid, cpu_time_count);
                        printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                    }
                    q2_count = 0;
                }
            }
            continue;
        }
        else if(!(mfq->q3.empty())){
            cout << "[q3]";
            //FCFS scheduling
            // cpu_time_count += 1;

            int targetPid = mfq->q3.front();

            if(!(checkProcessInit(process_arr, targetPid))){
                //if this process's scheduling is first time(= if this is starting time of process)
                //then, Mark the process as they are started.
                markAsStarted(process_arr, targetPid, cpu_time_count);
            }
            //In this, scheduling queue, we need to spent all time for current CPU Burst time
            //and, pass it to IO handle Queue
            int currentIdxOfCycle = process_arr[targetPid].currentCycle;
            // cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
            if(process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] > 1){
                //use all time quantum for CPU Burst and pass the process to q1
                cout << "-<"  << targetPid << ":"<< currentIdxOfCycle << "> : "<< process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] << "-\n";
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle]--;
            }
            else{
                //if CPU Burst Time is cleared in this turn, take care of rest of CPU Burst time
                //and put it in IOHandleQueue.
                process_arr[targetPid].CPU_BTArr[currentIdxOfCycle] = 0;
                if(process_arr[targetPid].IO_BTArr[currentIdxOfCycle] != 0){
                    IOhandleInfo tmp = {targetPid, 2, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle])};
                    IOHandleQueue.push(tmp);
                    printf("#%d is pushed in to IO Queue. return time is %ld. dest is %d\n", targetPid, (1 + cpu_time_count + (long)process_arr[targetPid].IO_BTArr[currentIdxOfCycle]), tmp.destQueue);
                    mfq->q3.pop();
                    process_arr[targetPid].currentCycle++;
                    process_arr[targetPid].IO_BTArr[currentIdxOfCycle] = 0;
                }
                else{
                    mfq->q3.pop();
                    markAsFinished(process_arr, targetPid, cpu_time_count);
                    printf("#%d is finished. end time is %ld.\n", targetPid, cpu_time_count);
                }
            }
            continue;
        }
    }
    cout << "\ncount :" << cpu_time_count << endl;
    free(process_arr);
    free(mfq);

    return 0;
}