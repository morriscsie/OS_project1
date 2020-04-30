#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHILD 1
#define PARENT 0
#define SIZE 25
int queue[SIZE];
int front = 0, rear = 0;
//FIFO 0
//SJF 1
//PSJF 2
//RR 3
int totaltime = 0;
int running = -1;
int finish = 0;
int t_last;
typedef struct Process{
	pid_t pid;
	char name[50];
	int readytime;
	int exectime;
}process;
void unit(){						
	volatile unsigned long i;		
	for (i = 0; i < 1000000UL; i++);	
}
int isFull() {
	if(front == (rear + 1) %SIZE)
		return 1;
	else
		return 0;
}
int isEmpty() {
	if (front == rear)
		return 1;
	else
		return 0;
}
void enQueue(int element){
	if (isFull())
		printf("Queue is full!!\n");
	else{
		queue[rear] = element;
		rear = (rear + 1) % SIZE;
	}
}
int deQueue(){
	int element;
	if(isEmpty()) {
		//printf("Queue is empty !!\n");
		return (-1);
	} 
	else{
		element = queue[front];
		front = (front + 1) % SIZE;
	}
	return (element);
}
int next_process(process *P, int num_of_process, int scheduler){
	int i;

	if((running!=-1)&&((scheduler==0)||(scheduler==1)))
		return running;
	int index = -1;
	if(scheduler==0){
		for(i=0;i<num_of_process;i++) {
			if((P[i].pid==-1)||(P[i].exectime==0))
				continue;
			if((index==-1)||(P[i].readytime<P[index].readytime))
				index = i;
		}
    }
	else if((scheduler==1)||(scheduler==2)){
		for (i=0;i<num_of_process;i++){
			if((P[i].pid==-1)||(P[i].exectime==0))
				continue;
			if ((index==-1)||(P[i].exectime<P[index].exectime))
				index = i;
		}
	}
	
	else if (scheduler==3){
		
		if (running==-1){
			index = deQueue();
			
			while((P[index].exectime==0)&&(index!=-1))
			index = deQueue();
		}
		else if ((totaltime-t_last)>=500){
			enQueue(running);
			index = deQueue();
			while((P[index].exectime==0)&&(index!=-1))
				index = deQueue();
		}
		else
			index = running;
	}


return index;
}						
void assign_cpu(int pid, int corenum){
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(corenum, &set);
	sched_setaffinity(pid, sizeof(set), &set);
}

int proc_exec(process *pro){
	pid_t pid;
	int i;
	pid = fork();
	if(pid==0){		//childprocess
		unsigned long int ss,sn,es,en;
		char dmesgmessage[100];
		struct timespec *k = (struct timespec*)malloc(sizeof(struct timespec));
		syscall(335,k);
		ss = k->tv_sec;
		sn = k->tv_nsec;
		for(i=1;i<=pro->exectime;i++)
			unit();
		syscall(335,k);
		es = k->tv_sec;
		en = k->tv_nsec;
		sprintf(dmesgmessage, "[Project1] %d %lu.%09lu %lu.%09lu\n", getpid(),ss,sn,es,en);
		syscall(334,dmesgmessage);
		exit(0);
	}
	else if(pid>0){			//parentprocess
		assign_cpu(pid,CHILD);
		return pid;
	}
}
void lower_priority(int pid){
	struct sched_param p;
	p.sched_priority = 0;
	sched_setscheduler(pid, SCHED_IDLE, &p);
}

void higher_priority(int pid){
	struct sched_param p;
	p.sched_priority = 0;
	sched_setscheduler(pid,SCHED_OTHER,&p);
}
int cmp(const void *a, const void *b) {
	process *data1 = (process*)a;
	process *data2 = (process*)b;
	if(data1->readytime<data2->readytime)
		return -1;
	else if(data1->readytime>data2->readytime)
		return 1;
	else if(data1->readytime==data2->readytime)
		return 0;
}
int main(void){
	int num_of_process;
	int scheduler;
	int i;
	
	
	char policy[100];
    	scanf("%s",policy);
	scanf("%d",&num_of_process);
	process P[num_of_process];
	for(i=0;i<num_of_process;i++)
		scanf("%s%d%d",P[i].name,&P[i].readytime,&P[i].exectime);
	
	if(policy[0]=='F')
		scheduler = 0;
	else if(policy[0]=='S')
		scheduler = 1;
	else if(policy[0]=='P')
		scheduler = 2;
	else if(policy[0]=='R')
		scheduler = 3;
	

	qsort(P,num_of_process,sizeof(process),cmp);
	for(i=0;i<num_of_process;i++)
		P[i].pid = -1;

	
	assign_cpu(getpid(),PARENT);
	higher_priority(getpid());
	while(1){
		if((running!=-1)&&(P[running].exectime==0)){
			waitpid(P[running].pid,NULL,0);
			printf("%s %d\n",P[running].name,P[running].pid);
			running = -1;
			finish++;
		if(finish==num_of_process)
			break;
		}
		for(i=0;i<num_of_process;i++){
			if(P[i].readytime==totaltime){
				P[i].pid = proc_exec(&P[i]);
				lower_priority(P[i].pid);
				enQueue(i);
				
			}
		}
		
	int nextprocess = next_process(P,num_of_process,scheduler);
	if(running!=nextprocess){
		lower_priority(P[running].pid);
		higher_priority(P[nextprocess].pid);
		running = nextprocess;
		t_last = totaltime;
	}
	unit();
	totaltime++;
	if (running != -1)
		P[running].exectime--;
}

	exit(0);
}
