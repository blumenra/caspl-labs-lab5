#include "JobControl.h"


void myPrintJob(job* j, int count);

int main(int argc, char** argv){

	job* jobs[5] = {0, 0, 0, 0, 0};
	char* cmd1 = "ls | grep .o\0";
	char* cmd2 = "du | grep .o\0";
	job* job1 = addJob(jobs, cmd1);
	job* job2 = addJob(jobs, cmd2);
	printJobs(jobs);

	// job* jobToFind = findJobByIndex(job1, 1);
	// myPrintJob(job1, 1);
	
	// jobToFind = findJobByIndex(job1, 2);
	// myPrintJob(jobToFind, 1);

	// jobToFind = findJobByIndex(job1, 3);
	// myPrintJob(jobToFind, 1);

	freeJobList(jobs);

	return 0;
}


void myPrintJob(job* j, int count){

	if(j == NULL){

		fprintf(stderr, "The job is NULL\n");
	}
	else{

		printf("\n");
		printf("Job %d:\n", count);
		printf("\tcmd: %s\n", j->cmd);
		printf("\tidx: %d\n", j->idx);
		printf("\tpgid: %d\n", (int) j->pgid);
		printf("\tstatus: %d\n", j->status);
		printf("\ttmodes: %d\n", tcgetattr(0, j->tmodes));
		// printf("tmodes: %d\n", tcgetattr(1, j->tmodes));

		if(j->next != NULL){

			count++;
			myPrintJob(j->next, count);
		}
	}
}


/**
* Receive a pointer to a job list and a new command to add to the job list and adds it to it.
* Create a new job list if none exists.
**/
job* addJob(job** job_list, char* cmd){
	job* job_to_add = initializeJob(cmd);

	if (*job_list == NULL){
		*job_list = job_to_add;
		job_to_add -> idx = 1;
	}	
	else{
		int counter = 2;
		job* list = *job_list;
		while (list -> next !=NULL){
			printf("adding %d\n", list->idx);
			list = list -> next;
			counter++;
		}
		job_to_add ->idx = counter;
		list -> next = job_to_add;
	}
	return job_to_add;
}


/**
* Receive a pointer to a job list and a pointer to a job and removes the job from the job list 
* freeing its memory.
**/
void removeJob(job** job_list, job* tmp){
	if (*job_list == NULL)
		return;
	job* tmp_list = *job_list;
	if (tmp_list == tmp){
		*job_list = tmp_list -> next;
		freeJob(tmp);
		return;
	}
		
	while (tmp_list->next != tmp){
		tmp_list = tmp_list -> next;
	}
	tmp_list -> next = tmp -> next;
	freeJob(tmp);
	
}

/**
* receives a status and prints the string it represents.
**/
char* statusToStr(int status)
{
  static char* strs[] = {"Done", "Suspended", "Running"};
  return strs[status + 1];
}


/**
*   Receive a job list, and print it in the following format:<code>[idx] \t status \t\t cmd</code>, where:
    cmd: the full command as typed by the user.
    status: Running, Suspended, Done (for jobs that have completed but are not yet removed from the list).
  
**/
void printJobs(job** job_list){

	updateJobList(job_list, FALSE);
	job* tmp = *job_list;

	while (tmp != NULL){
		printf("[%d]\t %s \t\t %s", tmp->idx, statusToStr(tmp->status),tmp -> cmd); 
		
		if (tmp -> cmd[strlen(tmp -> cmd)-1]  != '\n'){
			printf("\n");
		}
		job* job_to_remove = tmp;
		tmp = tmp -> next;
		if (job_to_remove->status == DONE)
			removeJob(job_list, job_to_remove);
		
	}
 
}


/**
* Receive a pointer to a list of jobs, and delete all of its nodes and the memory allocated for each of them.
*/
void freeJobList(job** job_list){
	while(*job_list != NULL){
		job* tmp = *job_list;
		*job_list = (*job_list) -> next;
		freeJob(tmp);
	}
	
}


/**
* receives a pointer to a job, and frees it along with all memory allocated for its fields.
**/
void freeJob(job* job_to_remove){

	
	if(job_to_remove != NULL){
		
		free(job_to_remove->cmd);
		free(job_to_remove->tmodes);
		free(job_to_remove);
	}
}



/**
* Receive a command (string) and return a job pointer. 
* The function needs to allocate all required memory for: job, cmd, tmodes
* to copy cmd, and to initialize the rest of the fields to NULL: next, pigd, status 
**/

job* initializeJob(char* cmd){
	
	job* newJob = (job*) (malloc(sizeof(job)));

	newJob->cmd = (char*) (malloc((sizeof(char)*(strlen(cmd))) + 1));
	memcpy(newJob->cmd, cmd, (strlen(cmd)));
	newJob->cmd[(strlen(cmd))] = '\0';

	newJob->idx = 0;
	newJob->pgid = 0;
	newJob->status = 0;
	newJob->tmodes = (struct termios*) (malloc(sizeof(struct termios)));
	newJob->next = NULL;

	return newJob;
}


/**
* Receive a job list and and index and return a pointer to a job with the given index, according to the idx field.
* Print an error message if no job with such an index exists.
**/
job* findJobByIndex(job* job_list, int idx){

	job* tempJob = job_list;

	while(tempJob != NULL){

		if(tempJob->idx == idx){

			return tempJob;
		}

		if(tempJob->next != NULL)
			tempJob = tempJob->next;
		else
			break;
	}

	fprintf(stderr, "A job with index %d does not exist!\n", idx);

	return NULL;
}


/**
* Receive a pointer to a job list, and a boolean to decide whether to remove done
* jobs from the job list or not. 
**/
void updateJobList(job **job_list, int remove_done_jobs){

	job* tmp = *job_list;

	while(tmp != NULL){
		
		job* nextJob = tmp->next;

		waitpid(tmp->pgid, &(tmp->status), WNOHANG);

		if(tmp->status == SUSPENDED){

			tmp->status = DONE;

			if(remove_done_jobs){

				printf("[%d]\t %s \t\t %s", tmp->idx, statusToStr(tmp->status),tmp -> cmd); 
		
				if (tmp -> cmd[strlen(tmp -> cmd)-1]  != '\n')
					printf("\n");

				removeJob(job_list, tmp);
			}
		}

		tmp = nextJob;
	}
}

/** 
* Put job j in the foreground.  If cont is nonzero, restore the saved terminal modes and send the process group a
* SIGCONT signal to wake it up before we block.  Run updateJobList to print DONE jobs.
**/

void runJobInForeground (job** job_list, job *j, int cont, struct termios* shell_tmodes, pid_t shell_pgid){
 
}

/** 
* Put a job in the background.  If the cont argument is nonzero, send
* the process group a SIGCONT signal to wake it up.  
**/

void runJobInBackground (job *j, int cont){	
 
}
