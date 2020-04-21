/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "simulator.h"


void pageit(Pentry q[MAXPROCESSES]) {


    /* Static vars */
    static int init = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */

    int proctmp;
    int pagetmp;


  //Either going to swap in or keep these pages
	int PAGE_OLD;
	int PAGE_OLDEST;

	int i;


  //Init vars
	if(!init){
		for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
			for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
				timestamps[proctmp][pagetmp] = 0;
			}
		}

		init = 1;
	}

  //LRU implementation
	for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++){
	   //Check if the current process is active
		if(q[proctmp].active){


      //Get a page
			pagetmp = q[proctmp].pc / PAGESIZE;

			//If this pagetmp is not in memory, swap it in and find the LRU page to remove
			if(!q[proctmp].pages[pagetmp] && !pagein(proctmp, pagetmp)){
				//The oldest page is equal to the current number of ticks
				PAGE_OLDEST = tick;
				//Loop through all the pages in the current process to see which page is being removed
				for(i = 0; i < MAXPROCPAGES; i++){

					//If a page is newer than the oldest pgae, keep it
					if(q[proctmp].pages[i] && timestamps[proctmp][i] < PAGE_OLDEST){

            // reassign the oldest page
						PAGE_OLDEST = timestamps[proctmp][i];
						// curent page will be removed
						PAGE_OLD = i;
					}
				}
				//Removeal of page
				pageout(proctmp, PAGE_OLD);
				break;
 			}

			timestamps[proctmp][pagetmp] = tick;
		}
	}

    tick++;
}
