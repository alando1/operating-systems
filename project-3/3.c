/*******************
| Alan Rodriguez   |
| U86831061        |
*******************/
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
#define SIZE 15

sem_t mutex, full, empty;
char* buffer;
FILE* fp;

//target functions for semaphores
void *producer(void *p);
void *consumer(void *p);

int main() 
{

	//open file for reading
	fp = fopen("mytest.dat", "r");
	int shmid;

	//creates shared memory segment
	if((shmid = shmget(IPC_PRIVATE, sizeof(char)*SIZE, IPC_CREAT | 0666) < 0))
	{
    	perror ("shmget failed. Exiting program.\n");
    	exit (1);     
  	}

	//shared memory is attached to buffer
	buffer = shmat(shmid, NULL, 0);

	//semaphore init
	sem_init(&full, 0, 0);
	sem_init(&mutex, 0, 1);
	sem_init(&empty, 0, SIZE);

	//Process id for threads 
	pthread_t p, c;				
	pthread_attr_t attribute;

	//pthread functions initalize threads
	fflush(stdout);
	
	//Required to schedule thread independently
	pthread_attr_init(&attribute);
	pthread_attr_setscope(&attribute, PTHREAD_SCOPE_SYSTEM);
	//End to schedule thread independently

	//Create the threads
	pthread_create(&p, &attribute, producer, buffer);
	pthread_create(&c, &attribute, consumer, buffer);

	//Wait for target thread to terminate 
	pthread_join(p, NULL);
	pthread_join(c, NULL);

	printf("\n");
	//destroy semaphores
	sem_destroy(&mutex);
	sem_destroy(&full);
	sem_destroy(&empty);


	//Terminate threads 
    pthread_exit(NULL);
    
	//detaches shared memory
	if(shmdt(buffer) == -1)
	{
		perror("shmdt");
		exit(-1);
	}

	//remove shared memory
	if(shmctl(shmid, IPC_RMID, NULL) < 0)
		printf("\nFailed to remove shared memory ID %d \n", shmid);

	//closes file and ends
	fclose(fp);
	//printf("\nend");
	return 0;
}


//producer function
void *producer(void *p) 
{
	int i = 0;
	char ch;
	while((i++ < 150) && ((ch = getc(fp)) != EOF)) 
	{
		//decrements (locks) semaphore
		sem_wait(&empty);
		sem_wait(&mutex);

		buffer[i%SIZE] = ch;

		//increments (unlocks) semaphores
		sem_post(&mutex);
		sem_post(&full);
	}

	//decrements (locks) semaphore
	sem_wait(&empty);
	sem_wait(&mutex);

	//producer informs consumer of last char in buffer  
	buffer[i%SIZE] = '*';

	//increments (unlocks) semaphores
	sem_post(&mutex);
	sem_post(&full);
}

//consumer function
void *consumer(void *p)
{

	int i = 0;
	char ch = '0';

	while((i++ < 150) && (ch != '*')) 
	{

		//decrements (locks) semaphore
		sem_wait(&full);
		sem_wait(&mutex);

		ch = buffer[i%SIZE];
		if(ch != '*')
			printf("%c", ch);
		
		//increments (unlocks) semaphores
		sem_post(&mutex);
		sem_post(&empty);
	}
}