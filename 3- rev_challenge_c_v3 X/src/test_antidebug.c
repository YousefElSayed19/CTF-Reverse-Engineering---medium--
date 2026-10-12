#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>

static int is_debugger_present(void) {
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        return 1;
    }
    ptrace(PTRACE_DETACH, 0, NULL, NULL);
    return 0;
}

int main() {
    int pipefd[2];
    if (pipe(pipefd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();

    if (pid == 0) {
        close(pipefd[0]);
        char go = 'x';
        write(pipefd[1], &go, 1);
        close(pipefd[1]);

        usleep(200000);

        int detected = is_debugger_present();
        printf("[CHILD] is_debugger_present() = %d (expected 1, since parent attached)\n", detected);
        _exit(detected == 1 ? 0 : 1);
    } else {
        close(pipefd[1]);
        char buf;
        read(pipefd[0], &buf, 1);
        close(pipefd[0]);

        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
            perror("PTRACE_ATTACH failed");
            return 1;
        }
        int status;
        waitpid(pid, &status, 0);
        ptrace(PTRACE_CONT, pid, NULL, NULL);

        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int child_result = WEXITSTATUS(status);
            printf("[PARENT] child exited normally with code %d (0 = anti-debug correctly detected attachment)\n", child_result);
            return child_result;
        } else if (WIFSIGNALED(status)) {
            printf("[PARENT] child was killed by signal %d\n", WTERMSIG(status));
            return 2;
        } else if (WIFSTOPPED(status)) {
            printf("[PARENT] child stopped again by signal %d, continuing once more...\n", WSTOPSIG(status));
            ptrace(PTRACE_CONT, pid, NULL, NULL);
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                int child_result = WEXITSTATUS(status);
                printf("[PARENT] (2nd try) child exited normally with code %d\n", child_result);
                return child_result;
            }
            return 3;
        }
        printf("[PARENT] unknown status: %d\n", status);
        return 4;
    }
}
