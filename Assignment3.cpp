/*Carlos Salazar 
Operating systems, Paris, 2015
semphore and mutex in simulation of a post office
*/
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <semaphore.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <queue>
#include <sys/wait.h>
#include <string.h>


using namespace std;

class customer {
public: 
	string name;
	int waitTime;
	int processingTime; 
	customer(string,int,int);
};

customer::customer(string n, int w, int p)
{
	name = n;
	waitTime = w;
	processingTime = p;
}

int main(int argc,char **argv)
{
	//temporary variables to test our patiens
	//name, arrival delay, service time
	//name, sleep time (sleep outside), service time (sleep inside)
	int clerks;
	if (argc != 2){
		perror("Incorrect number of Arguments. Exit probgram\n");
		exit(4);
	}
	clerks = atoi(argv[1]);
	

	int numberOfCustomers = 0;
	string name;
	int delay;
	int process;

	queue<customer*> customersQueue;

	while ((cin>>name) && (cin>>delay) && (cin>>process))
	{
		numberOfCustomers ++;
		customer * tempCustomer = new customer(name,delay,process);
		customersQueue.push(tempCustomer);

	}


	//what we need for init. 
	long key_mem; // shared segment key
	int sid_mem; //shared memory id
	int i; // counter for loop
	int pid; //process id for forking
	int *pmem;
	int value; //value sent by child
		//semaphores
	sem_t *postOffice;
	sem_t *mutex;
		//semaphores names
	char sem_postOffice[] = "CS_postOffice";
	char sem_mutex[] = "CS_mutex";
	int semvalue; // stores value of sem_getvalue()

/*	cout<<"input the nubmer of clerks: must be 1-16"<<endl;
	cin>>clerks;*/

	//make postOffice, create, preventacces, clerks number. 
	postOffice = sem_open(sem_postOffice,O_CREAT,0600,clerks);

	if (postOffice == SEM_FAILED)
	{
		perror("unable to create CS_postOffice semaphore");
		sem_unlink(sem_postOffice);
		exit(1);
	}

	//show value of semaphore
	sem_getvalue(postOffice,&semvalue);
/*	cout<<"the initial value of CS_postOffice is: "
	<<semvalue<<endl;*/

	//create our mutex 
	// this var stores the number of processes that had to wait. 
	mutex = sem_open(sem_mutex,O_CREAT,0600, 1);
	if (mutex == SEM_FAILED)
	{
		perror("unable to create CS_mutex semaphore");
		sem_unlink(sem_mutex);
		exit(-1);
	}

	sem_getvalue(mutex,&semvalue);
/*	cout<<"the intial value of CS_mutex is "
	<<semvalue <<endl;*/

	//create out unique key
	key_mem = 4561235;
	sid_mem = shmget(key_mem,4, 0600|IPC_CREAT);

	//we specify the size of the shared memory segment (MEMSIZE)

	if (sid_mem == -1)
	{
		perror ("cannot get shared segment");
		exit (3);		
	}

	//store into pmem the address of teh shared memory segment
	//and set it to be the pointer to an integer
	pmem = (int *) shmat(sid_mem, NULL, 0);

	if (pmem == NULL)
	{
		perror("cannot get address of shared semgent");
		exit (24);		
	}



	/*what the program will be doing:
		read a line with customer's name, arrival delay, and service time
		sleep for arrival delay 
		fork a child
		wait till children have terminated, 
		print statistics*/

		//looping for number of children
	while(!customersQueue.empty())
	{
		customer * temp; 

		temp = customersQueue.front();
		customersQueue.pop();

		sleep(temp->waitTime); 		//we sleep for the ammount of time per proccess

		pid = fork();
		if (pid == -1)
		{
			perror ("could not fork");
			exit(2);
		} 

		else if (pid == 0)
		
		{
	
			//print message
			//get current value of postoffice
			//clerks are busy increment nubmer of wait
				/*do a p() on mutex
				increment number of customers who had to wait
				do a v() mutex*/
			//do a p() on postOffice
			//print message
			//sleep for service time seconds
			//print message
			// do v() on postOffice
			//terminate with _exit(0)
			cout<<temp->name<<" has arrived\n"; 

			sem_getvalue(postOffice,&semvalue); 
			//cout<<"semvalue of postOffice "<<semvalue<<endl;

			if (semvalue == 0) // office is full
			{
				//cout<<temp->name<<" process is going to have to wait"<<endl;
				sem_wait(mutex);
				//cout<<"entered mutex\n";
				*pmem = *pmem +1;
				//cout<<"leaves mutex\n";
				sem_post(mutex);

			}
			sem_wait(postOffice);
			cout<<temp->name<<" starts getting helped"<<endl;
			sleep(temp->processingTime);
			cout<<temp->name<<" leaves the post office"<<endl;
			sem_post(postOffice);
			_exit(0);

		}
	}


	// parent process area:

	//cout<<"parent process starts!"<<endl;

	for (int i = 0; i<numberOfCustomers;i++){
		wait(0);
		//cout<<"child process ended"<<endl;
	}

	//output the number of processes that had to wait: 
	//cout<<"the number of customers that had to wait is: "<<*pmem<<endl;
	cout<<endl<<"Total number of customers: "<<numberOfCustomers<<endl;
	cout<<numberOfCustomers - *pmem<<" customer(s) did not have to wait \n";
	cout<<*pmem<<" customer(s) had to wait\n";

	//removing our semaphore and freeing up the memory
	shmdt(pmem);
	shmctl(sid_mem,IPC_RMID,NULL);
	//sem_close(postOffice);
	//sem_close(mutex);
	sem_unlink(sem_postOffice);
	sem_unlink(sem_mutex);

	return 0;
}