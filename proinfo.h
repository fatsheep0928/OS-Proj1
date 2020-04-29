#ifndef PRO_INFO_H
	#define PRO_INFO_H
	#include <stdlib.h>
	#include <stdio.h>
	#include "useful.h"

	typedef struct Process_Info{
	  char name[32];
	  ull readyT, execT;
	  pid_t pid;
	}Pro_Info;

	void print_ProInfo(Pro_Info *pros, ull k){
	    PRINT(("%s\n%llu %llu\n", pros[k].name, pros[k].readyT, pros[k].execT));
	}

	int FIFO_cmp(const void *data1, const void *data2){
	    ull rt1 = ((Pro_Info *)data1)->readyT, rt2 = ((Pro_Info *)data2)->readyT,
	         et1 = ((Pro_Info *)data1)->execT, et2 = ((Pro_Info *)data2)->execT;
	    if(rt1 < rt2) return -1;
	    else if(rt1 > rt2) return 1;
	    return (et1 <= et2)? -1:1;
	}

	void sort_pros(Pro_Info *pros, ull process_num){
	    ull k, l, choose, timeNow=0ULL, first_ready_time;
	    Pro_Info temp;
	    qsort(pros, process_num, sizeof(Pro_Info), FIFO_cmp);
	}
#endif
