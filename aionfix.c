/*
 * This fixes extremely low FPS in WINE when tap-tabbing targets in some legacy
 * Aion game clients (used by some private servers). The issue seems to be that
 * the client during tab-tabbing issues an excessive number of calls to
 * NtReadVirtualMemory() which in WINE is implemented in WINE server using
 * ptrace attack/read/detach to read the client memory which causes massive
 * stalls.
 *
 * The workaround presented here basically ptraces all its children processes,
 * which effectively causes the WINE server's ptrace() call to fail (there can be
 * only one ptrace master process). This consequentially causes the
 * NtReadVirtualMemory() call from the client to fail, but it doesn't seem to
 * cause too much harm and boosts the FPS to _normal_ levels.
 *
 * Presumably the reason why the client is issuing a lot of
 * NtReadVirtualMemory() calls is because of some anti-cheat code (it doesn't
 * make sense to use that function on itself otherwise).
 *
 * strace could be used instead, but since it traces all system calls it also
 * induces noticable jitter.
 */
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ptrace_loop(bool trace_forks)
{
    int wstatus;
    int rc;
    pid_t pid;

    printf("Dummy ptracing ..");
    do
    {
        printf("."); fflush(stdout);
        if ((pid = waitpid(-1, &wstatus, 0)) < 0)
        {
            if (errno == ECHILD)
            {
                printf("\nAION process exited.\n");
            }
            break;
        }

        if (WIFSTOPPED(wstatus))
        {
            printf("*"); fflush(stdout);
            int signal = WSTOPSIG(wstatus);
            if (signal == SIGSTOP || signal == SIGTRAP)
            {
                signal = 0;

                /* Trace subprocesses */
                if (trace_forks &&
                        ptrace(PTRACE_SETOPTIONS, pid , NULL , PTRACE_O_TRACEFORK) < 0)
                {
                    fprintf(stderr, "TRACEFORK failed.\n");
                }
            }

            rc = ptrace(PTRACE_CONT, pid, 0, signal);
            if (rc < 0)
            {
                fprintf(stderr, "Error resuming process\n");
                break;
            }
        }
    }
    while (true);
}

bool ptrace_start(void)
{
    return ptrace(PTRACE_TRACEME, 0, 0, 0) >= 0;
}

int main(int argc, char *argv[])
{
    printf("Installing WINE NtReadVirtualMemory() workaround.\n");

    if (fork() == 0)
    {
        if (!ptrace_start())
        {
            fprintf(stderr, "Error executing PTRACE_TRACEME\n");
            exit(1);
        }

        /* Execute the subprocess specified in argv[1...N] */
        printf("Tracing process %s\n", argv[1]);
        execvp(argv[1], argv + 1);
        fprintf(stderr, "ERROR executing process\n");
        exit(1);
    }

    ptrace_loop(true);
    printf("Exit.\n");
}
