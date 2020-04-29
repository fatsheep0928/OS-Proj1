#ifndef USEFUL_H
	#define USEFUL_H
	
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <time.h>
	#include <sys/wait.h>
	
	#ifndef _ULL_
		#define _ULL_
		#define ull unsigned long long int
	#endif
	
	
	//define CLOCK_SIMU 1000U
	#define CLOCK_SIMU 100000UL
	#define TIME_SLICE 500

	#define PRINT(X)  do{printf X; fflush(stdout);}while(0)

	void print_exit(int exit_value, char *message){
	    PRINT(("%s\n", message)); exit(exit_value);
	}

	void ull_to_string(ull num, char *string){
		int digit = 0; unsigned int buffer[128];
		while(num > 9ULL){
			buffer[digit] = (unsigned int) (num % 10ULL);
			num /= 10ULL; ++digit;
		}
		buffer[digit++] = (unsigned int)num;
		for(int i=0;i<digit;++i) string[i] = buffer[digit-i-1] + '0';
		string[digit] = '\0';
	}

	ull string_to_ull(char *string){
		ull out = 0ULL; int i = 0;
		while(string[i] != '\0'){
			out = out * 10ULL + (string[i] - '0');
			++i;
		}
		return out;
	}

	void get_time_now(ull *sec, ull *nsec){
	    struct timespec now;
	    if(clock_gettime(CLOCK_REALTIME, &now) != 0) print_exit(1, "Failed to clock_gettime");
		#ifdef MINUS_VER
			if(*getTimeFlag){
				PRINT(("Here\n"));
				*firstTimeS = now.tv_sec; *firstTimeNS = now.tv_nsec;
				*sec = *nsec = 0ULL; *getTimeFlag = 0;
			}else{
	    		if(now.tv_nsec >= *firstTimeNS){
					*sec = now.tv_sec - *firstTimeS; *nsec = now.tv_nsec - *firstTimeNS;
				}else{
					*sec = now.tv_sec - *firstTimeS - 1ULL; *nsec = 1000000000ULL - (*firstTimeNS - now.tv_nsec);
				}
			}
		#else
			*sec = now.tv_sec; *nsec = now.tv_nsec;
		#endif
	}
	
	void tick_tock(void){
	    volatile unsigned long int i; for(i=0;i<CLOCK_SIMU;++i);
	}
	
	pid_t fork_orphan(void){
	    pid_t child, gr_child;
	    if((child = vfork()) < 0) print_exit(1, "Failed to vfork");
	    else if(child == 0){
	        if((gr_child = fork()) < 0) print_exit(1, "Failed to fork.   Child exit");
	        else if(gr_child > 0) _exit(0); //child
	        else{ //grand_child
				
				
				
				
				return 0;
			}
	    }else{ //parent
	        int status; wait(&status);
	        if(gr_child < 0) print_exit(1, "Failed to fork.   Parent exit");
			
			
			
			
	        return gr_child;
	    }
	}

#endif
