#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "priority_queue.h"
#include "rr_queue.h"
#include "useful.h"
#include "proinfo.h"

#define __USE_GNU
#include <sched.h>

Pro_Info *pros;
char scheduling_policy[6];
ull process_num, process_left,
	*ctimeleft, realctimeleft, timeleft,  //for PSJF
    RR_clock;   //for RR
pid_t main_process_pid;

Priority_Queue *ready_queue;
rr_queue * rr_ready_queue;
ull running_id;
int pfd[2][2];

void handle(ull jobNext, int sig);
void psjfhandle(ull jobNext);
void usr1(int signo);
void usr2(int signo);
void usr3(int signo);


int main(){
	/*===============================get input====================================*/
	scanf("%s%llu", scheduling_policy, &process_num);
	pros = (Pro_Info *)malloc(sizeof(Pro_Info)*process_num);
	if(pros == NULL) print_exit(1, "Failed to malloc pros");
	
	for(ull k=0ULL;k<process_num;++k)
		scanf("%s%llu%llu", pros[k].name, &(pros[k].readyT), &(pros[k].execT));
	
	/*=================================signal====================================*/
	struct sigaction sig_usr1;
	struct sigaction sig_usr2;
	sig_usr1.sa_handler = (scheduling_policy[0] == 'R')? usr3 : usr1;
	sigemptyset(&sig_usr1.sa_mask);
	sig_usr1.sa_flags = 0;
	if(sigaction(SIGUSR1,&sig_usr1,NULL) < 0)
		print_exit(3, "sigerr");

	sig_usr2.sa_handler = usr2;
	sigemptyset(&sig_usr2.sa_mask);
	sig_usr2.sa_flags = 0;
	if(sigaction(SIGUSR2,&sig_usr2,NULL) < 0)
		print_exit(4, "sigerr");
	
	sigset_t parent_newmask, parent_oldmask, child_newmask, child_oldmask;
	sigemptyset(&parent_newmask); sigemptyset(&child_newmask);
	sigaddset(&parent_newmask, SIGUSR1); sigaddset(&child_newmask, SIGUSR2);
	
	
	/*===============================prepare parent===============================*/
	main_process_pid = getpid();
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(1, &mask);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &mask) < 0){
		perror("can't set affinity\n"); exit(1);
	}
	ctimeleft = &realctimeleft;	ready_queue = init_pq();
	running_id = process_left = process_num; //means no process running
	/*=============================start sort============================*/
	sort_pros(pros, process_num);
	#ifdef DEBUG
		for(ull k=0ULL;k<process_num;++k)
			print_ProInfo(pros, k);
	#endif
    /*========================start simulation============================*/
	ull simu_clock_time, jobNext;
	switch(scheduling_policy[0]){
		case 'F':
			#ifdef DEBUG
			PRINT(("Start FIFO scheduling\n"));
			#endif
			for(simu_clock_time=0ULL, jobNext=0ULL; jobNext<process_num; ++simu_clock_time){
				while(pros[jobNext].readyT <= simu_clock_time){
					#ifdef DEBUG
					PRINT(("Handle %llu   at clock_time %llu\n", jobNext, simu_clock_time));
					#endif
					handle(jobNext, 0);
					if(++jobNext == process_num) break;
				}
				tick_tock();
			}
	  		exit(0);
		case 'S':
			#ifdef DEBUG
			PRINT(("Start SJF scheduling\n"));
			#endif
			for(simu_clock_time=0ULL, jobNext = 0ULL; jobNext<1; ++simu_clock_time){
				if(pros[jobNext].readyT <= simu_clock_time){
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					#ifdef DEBUG
					PRINT(("Handle %llu   at clock_time %llu\n", jobNext, simu_clock_time));
					#endif
					handle(jobNext, 1);
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					++jobNext; break;
				}
				tick_tock();
			}
	  		#ifdef DEBUG
			PRINT(("Out first for\n"));
			#endif
			for(; jobNext<process_num; ++simu_clock_time){
				while(pros[jobNext].readyT <= simu_clock_time){
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					push(ready_queue, pros[jobNext].readyT, pros[jobNext].execT, jobNext);
					handle(jobNext, 1);		    
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					if(++jobNext == process_num) break;
				}
				tick_tock();
			}
			#ifdef DEBUG
			PRINT(("Out secont for\n"));
			#endif
			break;
		case 'P':
			#ifdef DEBUG
			PRINT(("Start PSJF scheduling\n"));
			#endif
			//create pipe for child to send ctimeleft to main
			pipe(pfd[0]); pipe(pfd[1]);
			for(simu_clock_time=0ULL, jobNext = 0ULL; jobNext<1; ++simu_clock_time){
				if(pros[jobNext].readyT <= simu_clock_time){
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					#ifdef DEBUG
					PRINT(("Handle %llu   at clock_time %llu\n", jobNext, simu_clock_time));
					#endif
					psjfhandle(jobNext);
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					++jobNext; break;
				}
				tick_tock();
			}
			#ifdef DEBUG
			PRINT(("Out first for\n"));
			#endif
			for(; jobNext<process_num; ++simu_clock_time){
				while(pros[jobNext].readyT <= simu_clock_time){
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					#ifdef DEBUG
					PRINT(("2Handle %llu   at clock_time %llu\n", jobNext, simu_clock_time));
					#endif
					//......fork()  .....    note that priorities are all same
					psjfhandle(jobNext);		    
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					/*--------------------------------------------*/
					if(++jobNext == process_num) break;
				}
				tick_tock();
			}
			#ifdef DEBUG
			PRINT(("Out second for\n"));
			#endif
			break;

		case 'R':
			#ifdef DEBUG
			PRINT(("Start RR scheduling\n"));
			#endif
			rr_ready_queue = rr_init();
			for(RR_clock=0ULL, simu_clock_time=0ULL, jobNext = 0ULL; jobNext<1; ++RR_clock, ++simu_clock_time){
				if(pros[jobNext].readyT <= simu_clock_time){
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					#ifdef DEBUG
					PRINT(("Handle %llu   at clock_time %llu\n", jobNext, simu_clock_time));
					#endif
					handle(jobNext, 2); running_id = jobNext;
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					++jobNext; break;
				}
				tick_tock();
			}
	  		#ifdef DEBUG
			PRINT(("Out first for\n"));
			#endif
			for(; jobNext<process_num; ++simu_clock_time){
				while(pros[jobNext].readyT <= simu_clock_time){
					/*--------------------------------------------*/
					//put into ready queue
					//sigprocmask(SIG_BLOCK, &parent_newmask, &parent_oldmask);
					#ifdef DEBUG
						PRINT(("2Handle %llu   at clock_time %llu   ", jobNext, simu_clock_time));
					#endif
					rr_push_by_id(rr_ready_queue, jobNext);
					handle(jobNext, 2);
					//sigprocmask(SIG_SETMASK, &parent_oldmask, NULL);
					/*--------------------------------------------*/
					if(++jobNext == process_num) break;
				}
				tick_tock();
				if(++RR_clock % TIME_SLICE == 0ULL){
					#ifdef DETAILED
					PRINT(("_____TIME___ %llu\n", simu_clock_time));
					#endif
					RR_clock = 0ULL;
					rrNode *rrnode = rr_pop(rr_ready_queue);
					if(rrnode != NULL){ //nonempty rr_ready_queue
						ull tempull = running_id; running_id = rrnode->id;
						rrnode->id = tempull; rr_push(rr_ready_queue, rrnode);
						struct sched_param param;
						param.sched_priority = sched_get_priority_min(SCHED_FIFO)+1;
						if(sched_setscheduler(pros[running_id].pid, SCHED_FIFO, &param) < 0){
							perror("can't set scheduler\n"); exit(2);
						}
						-- param.sched_priority;
						if(sched_setscheduler(pros[tempull].pid, SCHED_FIFO, &param) < 0){
							perror("can't set scheduler\n"); exit(2);
						}
					}
				}
			}
			#ifdef DEBUG
			PRINT(("Out secont for\n"));
			#endif
			break;
	}

	/*================================finish===============================*/
	//printf("Finish\n\n\n");
	//fflush(stdout);
	//free(pros);
    while(1);
}

/*=============================functions============================*/

void handle(ull jobNext, int sig){
	pid_t pid; char buff[4];
	int fd[2];  pipe(fd);
	if((pros[jobNext].pid = pid = fork_orphan()) < 0) print_exit(1, "Failed to fork_orphan\n");
	else if(pid == 0){
	//child process
		char buff[7], buffer[256];  close(fd[1]);
		read(fd[0], buff, sizeof(buff));
		pid = getpid();
		#ifdef DETAILED
		PRINT(("%s %d  started\n", pros[jobNext].name, pid));
		#else
		PRINT(("%s %d\n", pros[jobNext].name, pid));
		#endif
		ull startSec, startNSec, endSec, endNSec;
		get_time_now(&startSec, &startNSec);
		while(pros[jobNext].execT-- > 0ULL)
		  tick_tock();
		get_time_now(&endSec, &endNSec);
		syscall(333, pid, startSec, startNSec, endSec, endNSec);
		if(sig != 0 ){
			#ifdef DEBUG
			PRINT(("Send signal by %d\n", pid));
			#endif
			kill(main_process_pid, SIGUSR1);
			if(sig == 2) read(pfd[0][0], buff, 3);
		}
		#ifdef DETAILED
		PRINT(("%s %d  exited\n", pros[jobNext].name, pid));
		#endif
		exit(0);
	}else{
		//parent process
	    cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(0, &mask);
		if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask) < 0){
		        perror("can't set affinity\n"); exit(1);
		}	      
		struct sched_param param;
		param.sched_priority = sched_get_priority_min(SCHED_FIFO);
		if(sig == 2 && running_id == process_num) ++param.sched_priority;
		if(sched_setscheduler(pid, SCHED_FIFO, &param) < 0){
			perror("can't set scheduler\n"); exit(2);
		}
		char buff[7] = "sssset\0";
		close(fd[0]);
		write(fd[1], buff, sizeof(buff)); fsync(fd[1]);
		close(fd[1]);
	}
}

void psjfhandle(ull jobNext){
	ull cleft; char buffer[2000];
	int flag = -1;  //flag == 1   means should preempt,  flag == -1  means there was no process running currently
	//signal the child to send back its remaining time units
	if(running_id != process_num){ //have process running
		pid_t running_pid = pros[running_id].pid;
		#ifdef DEBUG
		PRINT(("Sending signal to running process %d\n", running_pid));
		#endif
		kill(running_pid, SIGUSR2);
		//get running process's left time
		read(pfd[0][0], buffer, sizeof(buffer));
		cleft = string_to_ull(buffer);
		flag = (pros[jobNext].execT < cleft)? 1:0;
	}
	pid_t pid;
	int fd[2];  pipe(fd);
	if((pros[jobNext].pid = pid = fork_orphan()) < 0) print_exit(1, "Failed to fork_orphan\n");
	else if(pid == 0){
	//child process
		char buff[7];  close(fd[1]);
		read(fd[0], buff, sizeof(buff));
		pid = getpid();
		#ifdef DEBUG
		PRINT(("\n\n=====%s %d\n\n", pros[jobNext].name, pid));
		#endif
		#ifdef DETAILED
		PRINT(("%s %d  started\n", pros[jobNext].name, pid));
		#else
		PRINT(("%s %d\n", pros[jobNext].name, pid));
		#endif
		ull startSec, startNSec, endSec, endNSec;
		get_time_now(&startSec, &startNSec);
		while(pros[jobNext].execT-- > 0){
			#ifdef DEBUG
			if(pros[jobNext].execT % 1000 == 0){
				PRINT(("%llu %llu TIME LEFT\n",timeleft, pros[jobNext].execT));
			}
			#endif
			tick_tock(); timeleft = pros[jobNext].execT;
		}
		get_time_now(&endSec, &endNSec);
		syscall(333, pid, startSec, startNSec, endSec, endNSec);
		kill(main_process_pid, SIGUSR1);
		#ifdef DETAILED
		PRINT(("%s %d  exited\n", pros[jobNext].name, pid));
		#endif
		exit(0);
	}else{
		//parent process
		cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(0, &mask);
		if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask) < 0){
			perror("can't set affinity\n"); exit(1);
		}	      
		struct sched_param param;
		param.sched_priority = sched_get_priority_min(SCHED_FIFO);
		switch(flag){
			case 1:  //preempt
				#ifdef DEBUG
				PRINT(("Preempted!   %llu < %llu\n", pros[jobNext].execT, cleft));
				#endif
				if(sched_setscheduler(pros[running_id].pid, SCHED_FIFO, &param) < 0){
					perror("can't set scheduler\n"); exit(2);
				}
				++ param.sched_priority;
				if(sched_setscheduler(pid, SCHED_FIFO, &param) < 0){
					perror("can't set scheduler\n"); exit(2);
				}
				push(ready_queue, pros[running_id].readyT, cleft, running_id);
				running_id = jobNext; break;
			case 0:  //can't preempt
				#ifdef DEBUG 
				PRINT(("Can't preempt   %llu >= %llu\n", pros[jobNext].execT, cleft));
				#endif
				if(sched_setscheduler(pid, SCHED_FIFO, &param) < 0){
					perror("can't set scheduler\n"); exit(2);
				}
				push(ready_queue, pros[jobNext].readyT, pros[jobNext].execT, jobNext);
				break;
			case -1:  //no process running
				#ifdef DEBUG
				PRINT("No process was running\n");
				#endif
				++ param.sched_priority;
				if(sched_setscheduler(pid, SCHED_FIFO, &param) < 0){
					perror("can't set scheduler\n"); exit(2);
				}
				running_id = jobNext; break;
		}
		write(pfd[1][1], "USR2 over\n", 10);
		char buff[7] = "allset\0";
		close(fd[0]);
		write(fd[1], buff, sizeof(buff));
		close(fd[1]);
	}
}

void usr1(int signo){
	#ifdef DEBUG
	PRINT(("Main process was signaled by %d\n", pros[running_id].pid));
	#endif
	--process_left;
	Item fringe;
    if(pop(ready_queue, &fringe) == 0){
		if(process_left == 0){
			destroy_pq(ready_queue); exit(0);
		}else running_id = process_num;
	}
    running_id = fringe.id;
    pid_t pid = pros[running_id].pid;
	struct sched_param param;
    param.sched_priority = sched_get_priority_min(SCHED_FIFO)+1;
    sched_setscheduler(pid, SCHED_FIFO, &param);
}
void usr2(int signo){
	char buff[2000];
	ull_to_string(timeleft, buff);
	#ifdef DEBUG
	PRINT(("Running process was signaled  send buff %s\n", buff));
	#endif
    //write timeleft to pipe
	write(pfd[0][1], buff, strlen(buff)+1);
	read(pfd[1][0], buff, sizeof(buff));
}

void usr3(int signo){  //for RR
	#ifdef DEBUG
	PRINT(("Main process was signaled by %d\n", pros[running_id].pid));
	#endif
	--process_left;
	rrNode *fringe;
    if((fringe = rr_pop(rr_ready_queue)) == NULL){ //empty rr_ready_queue
		if(process_left == 0){
			exit(0);
		}else
			running_id = process_num; return;
	}
    running_id = fringe->id;
    pid_t pid = pros[running_id].pid;
	struct sched_param param; char buff[8];
    param.sched_priority = sched_get_priority_min(SCHED_FIFO)+1;
    if(sched_setscheduler(pid, SCHED_FIFO, &param) < 0) print_exit(1, "Failed to setcheduler");
	write(pfd[0][1], "Die", 3);
}
