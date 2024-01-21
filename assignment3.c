#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include <sched.h>
#include <sys/resource.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

void printResult(pid_t pid, struct timespec start, struct timespec end, double elapsedTime, int nice) {
	printf("PID: %d ", pid);
	if (nice != -1) printf("| NICE : %d ", nice);

	char buf[100];
	struct tm tstart, tend;
	localtime_r(&start.tv_sec, &tstart);
	localtime_r(&end.tv_sec, &tend);
	strftime(buf, sizeof(buf), "%H:%M:%S", &tstart);
	printf("| Start time: %s.%09ld ", buf, start.tv_nsec);

	strftime(buf, sizeof(buf), "%H:%M:%S", &tend);
	printf("| End time: %s.%09ld ", buf, end.tv_nsec);

	printf("| Elapsed time: %f\n", elapsedTime);
	fflush(stdout);
}

void setSchedulingPolicy(int policy, int priority, int timeSlice, int pid) {
	struct sched_param param;
	param.sched_priority = priority;
	if (sched_setscheduler(pid, policy, &param) == -1) {
		perror("sched_setscheduler");
		exit(EXIT_FAILURE);
	}

	if (policy == SCHED_RR) {
		FILE *fp;
		const char *filename = "/proc/sys/kernel/sched_rr_timeslice_ms";
		fp = fopen(filename, "w");
		if (fp == NULL) {
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}
		
		fprintf(fp, "%d", timeSlice);
		fclose(fp);
	}
}

int main() {
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &set) == -1) {
		perror("sched_setaffinity");
		exit(EXIT_FAILURE);
	}
	printf("Input the Scheduling Polity to apply:\n1. CFS_DEFAULT\n2. CFS_NICE\n3. RT_FIFO\n4. RT_RR\n0. Exit\n");
	int pip[21][2];		//pipe
	pid_t child_pid[21];
	for (int i = 0; i < 21; i++) {
		if (pipe(pip[i]) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}

	}

	int option, nice = -1, timeSlice = -1, policy;
	scanf("%d", &option);
	if (!option) return 0;
	if (option < 0 || option > 4) {
		printf("Invalid option, exiting...\n");
		return 0;
	}

	int priority = 0;
	switch(option) {
		case 3:		//RT_FIFO
			policy = SCHED_FIFO;
			priority = sched_get_priority_max(SCHED_FIFO);
			break;
		case 4:		//RT_RR
			policy = SCHED_RR;
			priority = sched_get_priority_max(SCHED_RR);
			printf("Input the Time Slice to apply(10, 100, 1000): ");
			scanf("%d", &timeSlice);
			break;
	}


	struct timespec start, end;
	for (int i = 0; i < 21; i++) {
		child_pid[i] = fork();
		if (option == 2) {
			if (i < 7) nice = 19;
			else if (i < 14) nice = 0;
			else nice = -20;
		}

		if (child_pid[i] == 0) {		//child process
			if (sched_setaffinity(0, sizeof(cpu_set_t), &set) == -1) {
				perror("sched_setaffinity");
				exit(EXIT_FAILURE);
			}
			if (option == 3 || option == 4) {
				setSchedulingPolicy(policy, priority, timeSlice, getpid());
			}
			if (option == 2) {
				if (setpriority(PRIO_PROCESS, 0, nice) < 0) {
					perror("Fail setpriority");
					exit(EXIT_FAILURE);
				}
			}


			if (close(pip[i][0]) == -1) {
				perror("Fail read pipe in child process");
				exit(EXIT_FAILURE);
			}

			clock_gettime(CLOCK_REALTIME, &start);

			int count = 0, k, l, j;
			int result[102][102], A[102][102], B[102][102];
			memset(result, 0, sizeof(result));
			memset(A, 0, sizeof(A));
			memset(B, 0, sizeof(B));
			while(count < 100) {
				for (k = 0; k < 100; k++) {
					for(l = 0; l < 100; l++) {
						for(j = 0; j < 100; j++) {
							result[k][j] += A[k][l] * B[l][j];
						}
					}
				}
				count++;
			}

			clock_gettime(CLOCK_REALTIME, &end);
			double elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

			write(pip[i][1], &elapsedTime, sizeof(elapsedTime));

			close(pip[i][1]);
			printResult(getpid(), start, end, elapsedTime, nice);
			exit(EXIT_SUCCESS);

		} else if (child_pid[i] < 0) {
			perror("fail fork");
			exit(EXIT_FAILURE);
		}
	}

	//Parent process
	double total_elapsed = 0;
	for (int i = 0; i < 21; i++) {
		int status;
		pid_t wpid;
		do {
			wpid = waitpid(child_pid[i], &status, 0);
		} while (wpid == -1);
		if (WIFEXITED(status)) {
			double elapsed;
			close(pip[i][1]);
			if (read(pip[i][0], &elapsed, sizeof(elapsed)) > 0) {
				total_elapsed += elapsed;
			} else {
				perror("Fail read in parent process");
				exit(EXIT_FAILURE);
			}
		}
		close(pip[i][0]);
	}

	double avgTime = total_elapsed / 21;
	printf("Scheduling Policy: ");
	switch(option) {
		case 1:
			printf("CFS_DEFAULT | ");
			break;
		case 2:
			printf("CFS_NICE | ");
			break;
		case 3:
			printf("RT_FIFO | ");
			break;
		case 4:
			printf("RT_RR | ");
			break;
	}
	if (timeSlice != -1) printf("Time Quantum: %d ms | ", timeSlice);
	printf("Average elapsed time: %f\n", avgTime);
}
