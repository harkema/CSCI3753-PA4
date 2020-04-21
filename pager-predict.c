/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"

int LRU_SEARCH_PAGE(int timestamps[MAXPROCESSES][MAXPROCPAGES], Pentry q[MAXPROCESSES], int proctmp, int tick){
	int PAGE_OLDEST = tick;
	int PAGE_OLD;

	int i;
	for(i = 0; i < MAXPROCPAGES; i++){
		//If page is oldest, need to swqmp in new page
		if(q[proctmp].pages[i] && timestamps[proctmp][i] < PAGE_OLDEST){
			// reassign the oldest, and set the current page for removal
			PAGE_OLDEST = timestamps[proctmp][i];
			PAGE_OLD = i;
		}
	}
	return PAGE_OLD;
}

void swap(int *l, int *r){
	int lptr = *l;
	*l = *r;
	*r = lptr;
}


void pageit(Pentry q[MAXPROCESSES]) {

  /* This file contains the stub for a predictive pager */
	/* You may need to add/remove/modify any part of this file */

	//static vars
	static int init = 0;
	static int tick = 1; // artificial time


  //Counter Array
	static int PROC_CNTR[MAXPROCESSES];

  //Last used process's process counter
	static int LAST_PC[MAXPROCESSES];

  //Timestamps
	static int timestamps[MAXPROCESSES][MAXPROCPAGES];
	// Using the page timestamps, number, and frequency of use to determine when it should be replaced

	// use 3d array so each element has something else it is pointing to for another flow pattern
	static int *CTRL_FLOW_timestamps[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

  //Store the process, the page, and the other page in the control flow
	static int CTRL_FLOW_PG_NUM[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];
	static int CTRL_FLOW_FREQ[MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES];

	int **PRED_timestamps;
	int *PRED_PG_NUM;
	int *PRED_FREQ;


	int proctmp;
	int pagetmp;
	int CURR_PG_NUM;
	int LAST_PG_NUM;
	int NUM_PG_FILLED;
	int i, j, k;

//Init vars
	if(!init){
		for(i = 0; i < MAXPROCESSES; i++){
			for(j = 0; j < MAXPROCESSES; j++){
				for(k = 0; k < MAXPROCESSES; k++){
					/* loop through elems in 3D array */
					CTRL_FLOW_FREQ[i][j][k] = -1;
					CTRL_FLOW_PG_NUM[i][j][k] = -1;
					CTRL_FLOW_timestamps[i][j][k] = NULL;
				}
			}
		}
		for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++){
			for(pagetmp = 0; pagetmp < MAXPROCPAGES; pagetmp++){
				timestamps[proctmp][pagetmp] = 0;
			}
			PROC_CNTR[proctmp] = 0;
		}

		init = 1;
	}

	/* TODO: Implement Predictive Paging */
  //Update control flow for each process
	for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++){
		if(q[proctmp].active && LAST_PG_NUM != -1){
      //Get the last page of the queue
			LAST_PG_NUM = LAST_PC[proctmp] / PAGESIZE;
			//Update last page if the queue to be the current page
			LAST_PC[proctmp] = q[proctmp].pc;
			//Update current page number
			CURR_PG_NUM = LAST_PC[proctmp] / PAGESIZE;
      //Iterate to next pagetmp
      pagetmp = (q[proctmp].pc - 1) / PAGESIZE;

			timestamps[proctmp][pagetmp] = tick;

      //As long as the current page is not the last page in the queue
			if(CURR_PG_NUM != LAST_PG_NUM){


				pageout(proctmp, LAST_PG_NUM);

				for(i = 0; i < MAXPROCPAGES; i ++){

					if(CURR_PG_NUM == CTRL_FLOW_PG_NUM[proctmp][LAST_PG_NUM][i]){
					   //Increase the access frequency
						CTRL_FLOW_FREQ[proctmp][LAST_PG_NUM][i]++;
						break;
					}

					if(CTRL_FLOW_PG_NUM[proctmp][LAST_PG_NUM][i] == -1)
          {
						CTRL_FLOW_FREQ[proctmp][LAST_PG_NUM][i] = 1;
						CTRL_FLOW_PG_NUM[proctmp][LAST_PG_NUM][i] = CURR_PG_NUM;
						CTRL_FLOW_timestamps[proctmp][LAST_PG_NUM][i] = &(timestamps[proctmp][i]);
						break;
					}
				}
			}
		}
	}

	for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++){

		if(q[proctmp].active){
			//Temp page will be the next page required by the process
			pagetmp = (q[proctmp].pc) / PAGESIZE;

			if(q[proctmp].pages[pagetmp] != 1){
				//Process control == 0 when no longer waiting for page


        //If swap is successful
				if(pagein(proctmp, pagetmp))
        {
          //If there is page in the queue
					if(PROC_CNTR[proctmp])
          {
						PROC_CNTR[proctmp] = 0;


            //Do an LRU search
						if(LRU_SEARCH_PAGE(timestamps, q, proctmp, tick))
            {
              //INdicate there is page in the queue
							PROC_CNTR[proctmp] = 1;
						}
					}
				}
			}
		} else {

      //The process is didle
			for(pagetmp = 0; pagetmp < MAXPROCPAGES; pagetmp++)
      {
				pageout(proctmp, pagetmp);
			}
		}
	}

	for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++)
  {
		if(q[proctmp].active)
    {
			NUM_PG_FILLED = 0;
			PRED_timestamps = CTRL_FLOW_timestamps[proctmp][(q[proctmp].pc + 101) / PAGESIZE];
			PRED_FREQ = CTRL_FLOW_FREQ[proctmp][(q[proctmp].pc + 101) / PAGESIZE];
			PRED_PG_NUM = CTRL_FLOW_PG_NUM[proctmp][(q[proctmp].pc + 101) / PAGESIZE];
			//Look for empty pages first to reduce page faults
			for(i = 0; i < MAXPROCPAGES; i++)
      {
				if((NUM_PG_FILLED < MAXPROCPAGES) && (PRED_PG_NUM[NUM_PG_FILLED] != -1)){
					NUM_PG_FILLED++;
				}
			}



			for(i = 0; i < MAXPROCPAGES; i++)
      {
				for(int j = 1; j < NUM_PG_FILLED; j++)
        {
					if((*PRED_timestamps[j]) > (*PRED_timestamps[j - 1]))
          {
						if((PRED_FREQ[j]) > (PRED_FREQ[j - 1]))
            {
							swap(PRED_PG_NUM + (j - 1), PRED_PG_NUM + j);
							swap(*PRED_timestamps + (j - 1), *PRED_timestamps + j);
							swap(PRED_FREQ + (j - 1), PRED_FREQ + j);
						}
					}
				}
			}
			for(i = 0; i < NUM_PG_FILLED; i++)
      {
				pagein(proctmp, PRED_PG_NUM[i]);
			}
		}
	}




	tick++;
}
