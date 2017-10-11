/***************
*Alan Rodriguez*
*   U86831061  *
***************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// key number
#define SHMKEY ((key_t) 1497)

typedef struct
{
  int value;
} shared_mem;

shared_mem* total;

//processes increment shared memory
void process1()
{	
	int i;
	for(i = 0; i < 100000; i++)
		total->value++;
	printf("From Process 1: counter = %d\n", total->value);
	exit(1);
}

void process2()
{
	int i;
	for(i = 0; i < 200000; i++)
		total->value++;
	printf("From Process 2: counter = %d\n", total->value);
	exit(1);
}

void process3()
{
	int i;
	for(i = 0; i < 300000; i++)
		total->value++;
	printf("From Process 3: counter = %d\n", total->value);
	exit(1);
}
void process4()
{
	int i;
	for(i = 0; i < 500000; i++)
		total->value++;
	printf("From Process 4: counter = %d\n", total->value);
	exit(1);
}

int main()
{
	//process ids
	int shmid, pid1, pid2, pid3, pid4;
	char* shmadd;
	shmadd = (char*) 0;
	
	//create shared memory with key
	if((shmid = shmget(SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0)
    {
		perror("shmget");
		exit(1);     
	}

	//attach to shared memory
	if((total = (shared_mem*) shmat(shmid, shmadd, 0)) == (shared_mem*) -1) 
	{
		perror("shmat");
		exit(0);
	}

	//fork and handle child processes.
	total->value = 0;
	if((pid1 = fork()) == 0)
		process1();
	if((pid2 = fork()) == 0)
		process2();
	if((pid3 = fork()) == 0)
		process3();
	if((pid4 = fork()) == 0)
		process4();

	//parent waits for child processes to finish.
	waitpid(pid1, NULL, 0); 
	printf("Child %d pid has just exited.\n", pid1);
	waitpid(pid2, NULL, 0); 
	printf("Child %d pid has just exited.\n", pid2);
	waitpid(pid3, NULL, 0); 
	printf("Child %d pid has just exited.\n", pid3);
	waitpid(pid4, NULL, 0); 
	printf("Child %d pid has just exited.\n", pid4);

	//detach shared memory.
	if(shmdt(total) == -1)	
	{	
		perror("shmdt");
		exit(-1);
	}
	
	//shared memory is removed.
	shmctl(shmid, IPC_RMID, NULL);	

	printf("End of Program. \n");
	return 0;
}
