/*----------------------------.
| Alan Rodriguez  Sara Savitz |
| U86831061       U           |
'----------------------------*/
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
#define SIZE 2

sem_t mutex, agent, paper, match, tobacco;
char buffer[2];

char* smokers[] = { "Matches", "Paper", "Tobacco"};

//target functions for semaphores
void* Agent();
void* smoker(void *p);

int main() 
{

	//semaphore init
	sem_init(&mutex, 0, 1);
	sem_init(&agent, 0, 0);
	sem_init(&match, 0, 0);
	sem_init(&paper, 0, 0);
	sem_init(&tobacco, 0, 0);

	//Process id for threads 
	pthread_t a, m, p, t;
	pthread_attr_t attribute;

	//pthread functions initalize threads
	fflush(stdout);
	
	//Required to schedule thread independently
	pthread_attr_init(&attribute);
	pthread_attr_setscope(&attribute, PTHREAD_SCOPE_SYSTEM);
	//End to schedule thread independently

	//Create the threads
	pthread_create(&a, &attribute, Agent, buffer);
	pthread_create(&m, &attribute, smoker, (void *)0);
	pthread_create(&p, &attribute, smoker, (void *)1);
	pthread_create(&t, &attribute, smoker, (void *)2);
	//Wait for target thread to terminate 
	pthread_join(t, NULL);
	pthread_join(m, NULL);
	pthread_join(p, NULL);
	pthread_join(a, NULL);

	printf("\n");
	//destroy semaphores
	sem_destroy(&mutex);
	sem_destroy(&match);
	sem_destroy(&paper);
	sem_destroy(&tobacco);

	//Terminate threads 
    pthread_exit(NULL);
    
	return 0;
}

// 0 - signal Matches smoker
// 1 - signal Papers smoker
// 2 - signal Tobacco smoker
void* Agent(void *p)
{
	srand(time(NULL));   //should only be called once

	while(1)
	{
		int r = rand()%3;
		printf("r = %d\n", r);
		sem_wait(&mutex);

		switch(r)
		{
			case 0://agent puts Tobacco and Paper on table and signals Match smoke
				buffer[0] = 'p'; buffer[1] = 't';
				printf("Agent puts Paper and Tobacco on table.\n");			
				printf("Agent wakes Matches smoker.\n");
				sem_post(&match);
				break;

			case 1://agent puts Match and Tobacco on table and signals Paper smoker
				buffer[0] = 'm'; buffer[1] = 't';
				printf("Agent puts Match and Tobacco on table.\n");					
				printf("Agent wakes Papers smoker.\n");			
				sem_post(&paper);
				break;

			case 2://agent puts Match and Paper on table and signals Tobacco smoker
				buffer[0] = 'm'; buffer[1] = 'p';
				printf("Agent puts Match and Paper on table.\n");						
				printf("Agent wakes Tobacco smoker.\n");
				sem_post(&tobacco);				
				break;
		}

		sem_post(&mutex);
		sem_wait(&agent);
	}
}

void* smoker(void *p)
{
	int n = (int)p;
	char* name = smokers[n];

	while(1)
	{
		switch(n)
		{
			case 0: sem_wait(&match);	break;
			case 1: sem_wait(&paper);	break;
			case 2: sem_wait(&tobacco);	break;
			default:					break;
		}

		//decrements (locks) semaphore
		sem_wait(&mutex);
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
		//increments (unlocks) semaphores
		sem_post(&mutex);
		
		printf("Smoker with %s smokes cigarette.\n", name);
		sleep(1);

		sem_post(&agent);
	}
}
