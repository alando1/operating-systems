/*--------------------------------
| Alan Rodriguez  Sara Savitz    |
| U86831061       U37713110      |
| COP4600.002F17  Final Project  |
----------------------------------
|  THE CIGARETTE SMOKER PROBLEM  |
---------------------------------*/

#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// Max Number of Resources 
#define SIZE 2	

// Declare Semaphores
sem_t mutex, agent, paper, match, tobacco;

// Resource Buffer 
char buffer[SIZE];

// Smoker Names (used for printing)
char* smokers[] = { "Matches", "Paper", "Tobacco"};

// Thread Functions
void* agent_func(void* p);
void* smoker_func(void* p);

int main() 
{
	// Semaphore init
	sem_init(&mutex, 0, 1);
	sem_init(&agent, 0, 0);
	sem_init(&match, 0, 0);
	sem_init(&paper, 0, 0);
	sem_init(&tobacco, 0, 0);

	// Process ID for threads 
	pthread_t a, m, p, t;
	pthread_attr_t attribute;

	// pthread functions initalize threads
	fflush(stdout);
	
	// Required to schedule thread independently
	pthread_attr_init(&attribute);
	pthread_attr_setscope(&attribute, PTHREAD_SCOPE_SYSTEM);
	// End to schedule thread independently

	// Create Threads
	pthread_create(&a, &attribute, agent_func, (void *)0);
	pthread_create(&m, &attribute, smoker_func, (void *)0);
	pthread_create(&p, &attribute, smoker_func, (void *)1);
	pthread_create(&t, &attribute, smoker_func, (void *)2);

	// Wait for all threads to terminate 
	pthread_join(t, NULL);
	pthread_join(m, NULL);
	pthread_join(p, NULL);
	pthread_join(a, NULL);

	printf("\n");

	// Destroy semaphores
	sem_destroy(&mutex);
	sem_destroy(&match);
	sem_destroy(&paper);
	sem_destroy(&tobacco);

	// Terminate threads 
    pthread_exit(NULL);
    
	return 0;
}

void* agent_func(void* p)
{
	srand(time(NULL));   // Should only be called once

	while(1)
	{
		int r = rand()%3;	// Get a random number 0-2
		printf("r = %d\n", r);

		// Entering critical section 
		sem_wait(&mutex);

		switch(r)
		{
			case 0:// Agent puts Tobacco and Paper on table and signals smoker with matches
				buffer[0] = 'p'; buffer[1] = 't';
				printf("Agent puts Paper and Tobacco on table.\n");			
				printf("Agent wakes smoker with Matches.\n");
				sem_post(&match);
				break;

			case 1:// Agent puts Matches and Tobacco on table and signals smoker with paper
				buffer[0] = 'm'; buffer[1] = 't';
				printf("Agent puts Match and Tobacco on table.\n");					
				printf("Agent wakes smoker with Papers.\n");			
				sem_post(&paper);
				break;

			case 2:// Agent puts Matches and Paper on table and signals smoker with tobacco
				buffer[0] = 'm'; buffer[1] = 'p';
				printf("Agent puts Match and Paper on table.\n");						
				printf("Agent wakes smoker with Tobacco.\n");
				sem_post(&tobacco);				
				break;
		}

		sem_post(&mutex); 	// Exiting critical section
		sem_wait(&agent);	// Put agent to sleep
	}
}

// 0 - signal smoker with matches
// 1 - signal smoker with paper
// 2 - signal smoker with tobacco
void* smoker_func(void* p)
{
	// Identify thread
	int n = (int)p;	 
	char* name = smokers[n];

	while(1)
	{
		// Make each smoker thread wait until semaphore is signaled by agent
		switch(n)
		{
			case 0: sem_wait(&match);	break;
			case 1: sem_wait(&paper);	break;
			case 2: sem_wait(&tobacco);	break;
			default:					break;
		}

		// Entering critical section
		sem_wait(&mutex);

		// Read and print resources placed on table by agent thread
		int i;
		for(i=0; i<2; i++)
		{
			char c = buffer[i];

			switch (c)
			{
				case 'p': printf("%c - %s removed paper from table.\n", c, name); break;
				case 'm': printf("%c - %s removed match from table.\n", c, name); break;
				case 't': printf("%c - %s removed tobacco from table.\n", c, name); break;
				default:  printf("%c - %s removed unknown item.\n", c, name);
			}
		}

		// Exiting critical section 
		sem_post(&mutex);
		
		// Smoke for 1 sec
		printf("Smoker with %s smokes cigarette.\n", name);
		sleep(1);

		// Wake up agent thread 
		sem_post(&agent);
	}
}
