#include "enzyme.h"

int please_quit;
int use_yield;
int workperformed;

// The code each enzyme executes.
void *run_enzyme(void *data) {
	/* This function should :
	1. cast the void* pointer to thread_info_t*
	2. initialize the swapcount to zero
	3. Set the cancel type to PTHREAD_CANCEL_ASYNCHRONOUS
	4. If the first letter of the string is a C then call pthread_cancel on this thread.
	5. Create a while loop that only exits when please_quit is nonzero
	6. Within this loop: if the first character of the string has an ascii value greater than the second (s[0] >s[1]) then -
		Set workperformed=1, increment swapcount for this thread, then swap the two characters around
		If "use_yield" is nonzero then call pthread_yield at the end of the loop.
	7. Return a pointer to the updated structure.
	*/
       	int cancel_state;
	thread_info_t *dt = (struct thread_info_t*) data; 
	dt->swapcount = 0;
	cancel_state = pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//Set the thread to be canceled at any time
	if(dt->string[0]=='C')
	{
		pthread_cancel(pthread_self());
	}
	while(please_quit==0) {

		if(dt->string[0]>dt->string[1])
		{
			//printf("String before:%s \n",dt->string);
			char temp = dt->string[0];
			dt->string[0] = dt->string[1];
			dt->string[1] = temp;
			workperformed = 1;
			dt->swapcount++;		
			//printf("String after:%s \n",dt->string);
		}
		if(use_yield != 0)
		{
			sched_yield();
		}
	};
	return dt;
}


// Make threads to sort string.
// Returns the number of threads created.
// There is a memory bug in this function.
// enzymes[] stores thread ID, string is a string to be swapped, fp is and function to be run 
int make_enzyme_threads(pthread_t * enzymes, char *string, void *(*fp)(void *)) {
	int i,rv,len;
	thread_info_t *info;	//declaring ponter to struct thread_info_t 
	len = strlen(string);

	for(i=0;i<len-1;i++) {
	    info = (thread_info_t *)malloc(sizeof(thread_info_t));	//allocating memory for info 
	    info->string = string+i;
	    rv = pthread_create(enzymes+i,NULL,fp,info);
	    //Thread created. Stores ID to enzyme[i],default attribute, passes info to enzymerun 
	    if (rv) {   //pthread_create returns 0 for successful completion otherwise non zero 
	        fprintf(stderr,"Could not create thread %d : %s\n",
			i,strerror(rv));
		exit(1);
	    }
	}  
	return len-1;
}




// Join all threads at the end.
// Returns the total number of swaps.
int join_on_enzymes(pthread_t *threads, int n) {
	int i;
	int totalswapcount = 0;
	int whatgoeshere=0; // just to make the code compile 
	                    // you will need to edit the code below
	for(i=0;i<n;i++) {
	    void *status;
	    int rv = pthread_join(threads[i],&status);
	    // whatgoeshere = rv;
	
	
        if(rv) {
	    fprintf(stderr,"Can't join thread %d:%s.\n",i,strerror(rv));
	    continue;
	}
	/*		
        if(whatgoeshre) {
	    fprintf(stderr,"Can't join thread %d:%s.\n",i,strerror(rv));
	    continue;
	}
	*/
	if ((void*)status == PTHREAD_CANCELED) {
	    continue;
	} else if (status == NULL) {
	    printf("Thread %d did not return anything\n",i);
	    } else {
	      printf("Thread %d exited normally: ",i);// Don't change this line
	      //int threadswapcount = ((struct thread_info_t*) status).swapcount ; 
	      thread_info_t *dt = (struct thread_t*) status;
	      // Hint - you will need to cast something.
	      int threadswapcount = dt->swapcount;
	      printf("%d swaps.\n",threadswapcount); // Don't change this line
	      totalswapcount += threadswapcount;// Don't change this line
	    }
	}	
	return totalswapcount;
}

/* Wait until the string is in order. Note, we need the workperformed flag just in case a thread is in the middle of swapping characters
so that the string temporarily is in order because the swap is not complete.
*/
void wait_till_done(char *string, int n) {
	int i;
	while(1) {
	    sched_yield();	//stops executing current thread, places itself to the end of queue and lets new thread  to run
	    workperformed=0;
	    for(i=0;i<n;i++) 
	        if (string[i] > string[i+1]) {
	            workperformed=1;
	    	}
	    if(workperformed==0) break;
	}
}

void * sleeper_func(void *p) {
	sleep( (int) p); 
	// Actually this may return before p seconds because of signals. 
	// See man sleep for more information
	printf("sleeper func woke up - exiting the program\n");
	exit(1);
}

int smp2_main(int argc, char **argv) {
	pthread_t enzymes[MAX]; //creating thread named enzyme;
	int n,totalswap;	
	char string[MAX];
	
	if (argc <= 1) {
	    fprintf(stderr,"Usage: %s <word>\n",argv[0]);
	    exit(1);
	}
	strncpy(string,argv[1],MAX); // Why is this necessary? Why cant we give argv[1] directly to the thread functions?
	
	please_quit = 0;
	use_yield =1;
	
	printf("Creating threads...\n");
	n = make_enzyme_threads(enzymes,string,run_enzyme); //
	printf("Done creating %d threads.\n",n);
	
	pthread_t sleeperid;
	pthread_create(&sleeperid,NULL,sleeper_func,(void*)5);

	wait_till_done(string,n);
	please_quit = 1;
	printf("Joining threads...\n");
	totalswap = join_on_enzymes(enzymes, n);
	printf("Total: %d swaps\n",totalswap);
	printf("Sorted string: %s\n",string);
	
	exit(0);
}



