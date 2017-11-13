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
#include  <sys/sem.h>
#include <sys/wait.h>

// key number
#define SHMKEY ((key_t) 1497)
// semaphore key
#define SEMKEY ((key_t) 400L)
// number of semaphores being created
#define NSEMS 1

// GLOBAL

int sem_id;// semaphore id
int status;
// semaphore buffers
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};
struct sembuf *P =&OP;
struct sembuf *V =&OV;

// semapore union used to generate semaphore
typedef union{
	int val;		
	struct semid_ds *buf;
	ushort *array;
} semunion;

// POP (wait()) function for semaphore to protect critical section
int POP()
{	
	status = semop(sem_id, P,1);
	return status;
}

// VOP (signal()) function for semaphore to release protection
int VOP()
{	
	status = semop(sem_id, V,1);
	return status;
}

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
	{
		POP();
		total->value++;
		VOP();
	}
	printf("From Process 1: counter = %d\n", total->value);
	exit(1);
}

void process2()
{
	int i;
	for(i = 0; i < 200000; i++)
	{
		POP();
		total->value++;
		VOP();
	}
	printf("From Process 2: counter = %d\n", total->value);
	exit(1);
}

void process3()
{
	int i;
	for(i = 0; i < 300000; i++)
	{
		POP();
		total->value++;
		VOP();
	}
	printf("From Process 3: counter = %d\n", total->value);
	exit(1);
}
void process4()
{
	int i;
	for(i = 0; i < 500000; i++)
	{
		POP();
		total->value++;
		VOP();
	}
	printf("From Process 4: counter = %d\n", total->value);
	exit(1);
}

int main()
{
	//process ids
	int shmid, pid1, pid2, pid3, pid4;
	char* shmadd;
	shmadd = (char*) 0;

	int   value, value1, semnum = 0;
	semunion semctl_arg;
	semctl_arg.val = 1;

	/* Create semaphores */
	sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
	if(sem_id < 0) printf("Error in creating the semaphore.\n");

	/* Initialize semaphore */
	value1 =semctl(sem_id, semnum, SETVAL, semctl_arg);
	value =semctl(sem_id, semnum, GETVAL, semctl_arg);
	if (value < 1) printf("Error detected in SETVAL.\n");



	
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

	/* De-allocate semaphore */
	semctl_arg.val = 0;
	status =semctl(sem_id, 0, IPC_RMID, semctl_arg);
	if( status < 0) printf("Error in removing the semaphore.\n");

	printf("End of Program. \n");
	return 0;
}
