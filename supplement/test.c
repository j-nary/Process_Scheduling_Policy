#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
    pid_t child_pid[21];
    for (int i = 0; i < 21; i++) {
        child_pid[i] = fork();
        if (child_pid[i] < 0) {
            perror("Fail fork");
            exit(EXIT_FAILURE);
        }

        if (child_pid[i] == 0) {
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
            if (syscall(453, (int)getpid()) == 0) {
                printf("PID %d : Success\n", (int)getpid());
		system("chrt -p $$");
            }
            exit(EXIT_SUCCESS);
        }
    }

    //parent process
    for (int i = 0; i < 21; i++) {
        waitpid(child_pid[i], NULL, 0);
    }

    return 0;
}
