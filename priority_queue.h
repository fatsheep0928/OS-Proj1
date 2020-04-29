#ifndef PRIORITY_QUEUE_H
	#define PRIORITY_QUEUE_H
	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include "useful.h"
	#ifdef RB_TREE_VER
		/*===========================Red Black Tree Version============================*/
		#include "redblacktree.h"

		typedef RBRoot Priority_Queue;
	
		Priority_Queue *init_pq(void){
			return create_rbtree();
		}

		void destroy_pq(Priority_Queue *pq){
			destroy_rbtree(pq);
		}

		void push(Priority_Queue *pq, ull rT, ull tL, ull id){
			Item item = {rT, tL, id};
			Node *node = create_rbtree_node(item, NULL, NULL, NULL);
			rbtree_insert(pq, node);
		}

		int pop(Priority_Queue *pq, Item *out){
			Node *minNode;
			if((minNode = minimum(pq->node)) == NULL) return 0; //empty pq
			*out = minNode->key;
			rbtree_delete(pq, minNode);
			return 1;
		}
	#else
		/*==========================Brute Force Version===============================*/
		#ifndef _ULL_
			#define _ULL_
			#define ull unsigned long long int
		#endif
		#ifndef _ITEM_
			#define _ITEM_
			typedef struct item{
				ull ready_time;
				ull time_left;
				ull id;
			}Item;
		#endif
		
		typedef struct Priority_Queue{
			Item items[100];
			int num;
		}Priority_Queue;
		
		int item_compare(Item a, Item b){ //equals to a_is_smaller()
		    if(a.time_left < b.time_left) return 1;
		    else if(a.time_left > b.time_left) return 0;
		    return (a.ready_time <= b.ready_time) ? 1:0;
		}

		int cmp_func(const void *a, const void *b){
			return (item_compare(*(Item *)a, *(Item *)b))? 1:-1;
		}
		#ifdef DEBUG
			void print_pq(Priority_Queue *pq){
				if(pq->num == 0) PRINT(("Empty pq"));
				else{
					PRINT(("Have %d\n", pq->num));
					for(int i=0;i<pq->num;++i)
						PRINT(("(%llu %llu %llu)  ", pq->items[i].ready_time, pq->items[i].time_left, pq->items[i].id));
				}
				PRINT(("\n"));			
			}
		#endif

		Priority_Queue *init_pq(void){
			Priority_Queue *ret;
			if((ret = (Priority_Queue *)malloc(sizeof(Priority_Queue))) == NULL) print_exit(1, "Failed to init_pq");
			ret->num = 0; return ret;
		}

		void destroy_pq(Priority_Queue *pq){
			free(pq);
		}
		
		void push(Priority_Queue *pq, ull rT, ull tL, ull id){
			Item item = {rT, tL, id};
			pq->items[pq->num ++] = item;
			qsort(pq->items, pq->num, sizeof(Item), cmp_func);
			#ifdef DEBUG
			print_pq(pq);
			#endif
		}
		
		int pop(Priority_Queue *pq, Item *out){
			if(pq->num == 0) return 0; //empty pq
			*out = pq->items[-- pq->num];
			#ifdef DEBUG
			print_pq(pq);
			#endif
			return 1;
		}
		
	#endif
#endif
