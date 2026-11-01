#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>

#define __USE_POSIX
#include <signal.h>

#define BACKTRACE_BUF_SIZE 32

void sighandler(int sig)
{
    (void)sig;

    void *buf[BACKTRACE_BUF_SIZE];
    int n = backtrace(buf, sizeof(buf));
    backtrace_symbols_fd(buf, n, STDOUT_FILENO);
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    const int pid = getpid();
    printf("kill -3 %d\n", pid);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sighandler;
    if (sigaction(SIGQUIT, &sa, NULL) != 0)
    {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    printf("Waiting for SIGQUIT ...\n");
    while (1)
    {
        pause();
    }

    return 0;
}
