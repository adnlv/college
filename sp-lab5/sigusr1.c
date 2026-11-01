#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define __USE_POSIX
#include <signal.h>

#define LVL_DEBUG 0
#define LVL_INFO 1
#define LVL_WARNING 2
#define LVL_ERROR 3

static int level = LVL_DEBUG;
static int interval = 0;
static int is_running = 1;

static int load_config(void)
{
    FILE *fp = fopen("config.txt", "r");
    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    char buf[8];
    fscanf(fp, "%7s %d", buf, &interval);

    if (strcmp(buf, "debug") == 0)
    {
        level = LVL_DEBUG;
    }
    else if (strcmp(buf, "info") == 0)
    {
        level = LVL_INFO;
    }
    else if (strcmp(buf, "warning") == 0)
    {
        level = LVL_WARNING;
    }
    else if (strcmp(buf, "error") == 0)
    {
        level = LVL_ERROR;
    }
    else
    {
        fprintf(stderr, "Bad level: %s. Fallback to %d", buf, level);
        return 1;
    }

    printf("Config loaded: level %d, interval: %d\n", level, interval);
    return 0;
}

void sighandler(int sig)
{
    (void)sig;

    load_config();
}

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv;

    int pid = getpid();
    printf("kill -USR1 %d\n", pid);

    if (load_config() < 0)
    {
        return 1;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = sighandler;
    if (sigaction(SIGUSR1, &sa, NULL) != 0)
    {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    while (is_running)
    {
        if (interval > 0)
        {
            sleep((unsigned int)interval);
        }

        if (level <= LVL_DEBUG)
        {
            printf("[DBG] Debug message\n");
        }
        if (level <= LVL_INFO)
        {
            printf("[INF] Info message\n");
        }
        if (level <= LVL_WARNING)
        {
            printf("[WRN] Warning message\n");
        }
        if (level <= LVL_ERROR)
        {
            printf("[ERR] Error message\n");
        }
    }

    return 0;
}
