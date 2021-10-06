
//print whole information of All the current Processes
void printProcessArr(Process * process_arr, int numOfProcess);
//Check whether the process is started already
int checkProcessInit(Process * process_arr, int pid);
//Check if the process is finished
bool checkProcessDone(Process * process_arr , int length);
//mark the process as it is now started
void markAsStarted(Process * process_arr, int pid, int current_cpu_time);
//mark the process as it is now finished
void markAsFinished(Process * process_arr, int pid, int current_cpu_time);

int sumOfCPUBurst(Process * process_arr, int pid);

int sumOfIOBurst(Process * process_arr, int pid);

//initialize the Queue
void InitQueue(Queue *queue){
    queue->front = NULL;
    queue->rear = NULL;
    queue->count = 0;
}

//return true if the Queue is empty
bool empty(Queue *queue){
    // printf("empty?\n");
    // printf("%d", queue->count);
    // printf("!\n");
    if(queue->count == 0)  
        return true;
    else
        return false;
}

//insert in the Queue at Rear position
void push(Queue *queue, int data){
    // printf("data *%d inserting\n", data);
    Node *current_idx = (Node *)malloc(sizeof(Node));
    current_idx->data = data;
    current_idx->next = NULL;
    // printf("inseting start\n");
    if (empty(queue)){
        // printf("empty\n");
        queue->front = current_idx;     
    }
    else{
        // printf("not empty\n");
        queue->rear->next = current_idx;
    }
    queue->rear = current_idx;
    queue->count++;
}

//pop from the Queue
void pop(Queue *queue){
    Node *current_Node;
    if (empty(queue)){
        // printf("popping, but the queue is already empty!\n");
        return;
    }
    current_Node = queue->front;
    queue->front = current_Node->next;
    free(current_Node);
    queue->count--;
}

//return Front element of the Queue
int front(Queue *queue){
    return queue->front->data;
}

//swap between two Struct variable(which is IOHandleInfo)
void swap_IOhandleInfo(IOhandleInfo *a, IOhandleInfo *b){
    IOhandleInfo tmp;
    tmp.pid = a->pid;
    tmp.scheduled_time = a->scheduled_time;
    tmp.destQueue = a->destQueue;

    a->destQueue = b->destQueue;
    a->pid = b->pid;
    a->scheduled_time = b->scheduled_time;

    b->pid = tmp.pid;
    b->scheduled_time = tmp.scheduled_time;
    b->destQueue = tmp.destQueue;
}

//insert in the Priority Queue at proper position based on priority
int push_priority(Priority_Queue *pri_queue, IOhandleInfo node){

    if(pri_queue->count > MAX_PROCESS_SIZE){
        return -1;
    }
	int current_idx = pri_queue->count;
    // pri_queue->arr[current_idx] = node;
    pri_queue->arr[current_idx].pid = node.pid;
    pri_queue->arr[current_idx].destQueue = node.destQueue;
    pri_queue->arr[current_idx].scheduled_time = node.scheduled_time;

	int parent = (current_idx - 1) / 2;

	while (current_idx > 0 && pri_queue->arr[current_idx].scheduled_time < pri_queue->arr[parent].scheduled_time) {
		swap_IOhandleInfo(&pri_queue->arr[current_idx], &pri_queue->arr[parent]);
		current_idx = parent;
		parent = (current_idx - 1) / 2;
    }
	pri_queue->count++;

    return 0;
}

//pop top of the element from the Priority Queue
void pop_priority(Priority_Queue* pri_queue) {
	if (pri_queue->count <= 0) 
        return;
	pri_queue->count--;
	pri_queue->arr[0] = pri_queue->arr[pri_queue->count];
	int current_idx = 0, left = 1, right = 2;

	while (current_idx <= pri_queue->count) {

		int change = current_idx;

		if (left <= pri_queue->count && pri_queue->arr[current_idx].scheduled_time > pri_queue->arr[left].scheduled_time) 
            change = left;
		if (right <= pri_queue->count && pri_queue->arr[current_idx].scheduled_time > pri_queue->arr[right].scheduled_time                                              ) 
            change = right;
		if (change == current_idx) 
            break;

		swap_IOhandleInfo(&pri_queue->arr[current_idx], &pri_queue->arr[change]);
		current_idx = change;
		left = current_idx * 2 + 1;
		right = current_idx * 2 + 2;
	}
}

//return TOP element from the Priority Queue
IOhandleInfo top_priority(Priority_Queue *pri_queue){
    return pri_queue->arr[0];
}

//return true if the Priority Queue is empty
bool empty_priority(Priority_Queue *pri_queue){
    if(pri_queue->count == 0)
        return true;
    else
        return false;
}

//프로세스를 IOHandleQueue에 넣으면서, process_arr에 저장된 해당 차례의 IO Burst 정보를 소모했다고 기록한다.
void startProcessIO(Process * process_arr, int pid, int currentIdxOfCycle){
    process_arr[pid].IO_BTArr[currentIdxOfCycle] = 0;
    process_arr[pid].currentCycle++;
    printf("process #%d OUT (IO)\n", pid);
}