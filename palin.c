#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
char *strrev(char *str);

int main(int argc, char *argv[]){
	int isPalin;
	time_t t;
	struct tm *timeInfo;
	time(&t);
	timeInfo = localtime(&t);
	time(&t);

	sem_t* sem;
	sem = sem_open("proj3sem",0);
	if(sem == SEM_FAILED){
		fprintf(stderr,"./palin: Error in sem_open");
		exit(1);
	}
	
	srand((int)time(&t) % getpid());
	//retrieve index for child process
	int index = atoi(argv[0]);
	int count = atoi(argv[1]);
	
	//retrieve mylist from shared mem
	char (*mylist)[count];
	
	key_t mem_key = ftok(".", 'a');
	int shmid2 = shmget(mem_key, count * sizeof(char), 0666);
	if(shmid2 < 0){
		printf("Error in palin shmget\n");
		exit(1);
	}


	mylist = shmat(shmid2,(void *)0,0);
	if((intptr_t)mylist == -1){
		printf("Error in palin shmat\n");
	}

	int i;
	for(i = 0; i < 5; i++){
		//In the case an empty element is decected break
		//before attempt at critical section
		if(strlen(mylist[index + i]) == 0){
			break;
		}
		int num = (rand()%4);
		sleep(num);//sleep
		t = time(0);
		fprintf(stderr,"Process:% d Will wait to enter critical section: %s",getpid() ,asctime(timeInfo));
		sem_wait(sem);//Start of critical section
		fprintf(stderr,"Process:% d Entered critical section: %s",getpid() ,asctime(timeInfo));

		//Check if palindrome
		char *temp = strdup(mylist[index + i]);
		FILE *fp;
		temp = strrev(temp);
		wait(2);
		//Write to files
		if(strcmp(mylist[index + i],temp) == 0){
			isPalin = 1;
			fp = fopen("palin.out","a");
		}
		else{
			isPalin = 0;
			fp = fopen("nopalin.out","a");
		}
		fprintf(fp,"PID: %d Index: %d String: %s\n",getpid(),(index + i), mylist[index + i]);
		fprintf(stderr,"Process:% d Is leaving the critical section: %s",getpid() ,asctime(timeInfo));
		
		free(temp);
		fclose(fp);
		wait(2);
		sem_post(sem);//End of critical section
	}


	//detach shared mem pointers
	shmdt((void*)mylist);
	return 0;
}

char *strrev(char *str){
	if(!str || !*str)
		return str;
	int i = strlen(str) -1;
	int k = 0;
	char c;
	
	while(i > k){
		c = str[i];
		str[i] = str[k];
		str[k] = c;
		i--;
		k++;
	}
	return str;
}
