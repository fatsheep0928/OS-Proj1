#ifndef RR_QUEUE_H
	#define RR_QUEUE_H
	#include <stdio.h>
	#include <stdlib.h>
	#include "useful.h"
	#ifndef _ULL_
		#define _ULL_
		#define ull unsigned long long int
	#endif
	
	typedef struct rrNode{
		ull id;
		struct rrNode *next;
	}rrNode;
	
	typedef struct rr_queue{
		rrNode *head, *tail;
	}rr_queue;

	rr_queue *rr_init(void){
		rr_queue *ret;
		if((ret = (rr_queue *)malloc(sizeof(rr_queue))) == NULL) print_exit(1, "Failed to malloc rr_queue");
		ret->head = ret->tail = NULL;
		return ret;
	}
	
	void rr_push(rr_queue *rr, rrNode *topush){
		if(rr->head == NULL){
			rr->head = topush;
		}else
			rr->tail->next = topush;
		topush->next = NULL; rr->tail = topush;
	}
	
	void rr_push_by_id(rr_queue *rr, ull id){
		rrNode *topush;
		if((topush = (rrNode *)malloc(sizeof(rrNode))) == NULL)	print_exit(1, "Failed to malloc rrNode");
		topush->id = id; rr_push(rr, topush);
	}
	
	rrNode *rr_pop(rr_queue *rr){
		if(rr->head == NULL) return NULL;
		rrNode *temp = rr->head;
		rr->head = temp->next;
		return temp;
	}

#endif
