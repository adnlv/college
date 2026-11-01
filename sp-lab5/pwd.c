#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define VERSION 0x010000

static void print_help(void)
{
    printf("NAME\n"
           "\tpwd - print name of current/working directory\n"
           "\nSYNOPSIS\n"
           "\tpwd [OPTION]\n"
           "\nDESCRIPTION\n"
           "\tPrint the full filename of the current working directory.\n"
           "\n\t-L, --logical     use PWD from environment, even if it contains symlinks\n"
           "\n\t-P, --physical    resolve all symlinks\n"
           "\n\t-h, --help        display this help and exit\n"
           "\n\t-v, --version     output version information and exit\n"
           "\n\tIf no option is specified, -P is assumed.\n"
           "\nAUTHOR\n"
           "\tWritten by Artem Danylov\n"
           "\n");
}

static void print_version(void)
{
    printf("%" PRIu8 ".%" PRIu8 ".%" PRIu8 "\n",
           (VERSION & 0xFF0000) >> 16,
           (VERSION & 0x00FF00) >> 8,
           (VERSION & 0x0000FF));
}

static int print_physical(void)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    printf("%s\n", cwd);
    return 0;
}

static int print_logical(void)
{
    const char *cwd = getenv("ENV");
    if (cwd != NULL)
    {
        printf("%s\n", cwd);
        return 0;
    }

    return print_physical();
}

int main(int argc, char const *argv[])
{
    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments. Only one option available per call.\n");
        return 1;
    }

    if (argc == 1)
    {
        return print_physical();
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        print_help();
    }
    else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
    {
        print_version();
    }
    else if (strcmp(argv[1], "-L") == 0 || strcmp(argv[1], "--logical") == 0)
    {
        return print_logical();
    }
    else if (strcmp(argv[1], "-P") == 0 || strcmp(argv[1], "--physical") == 0)
    {
        return print_physical();
    }
    else
    {
        fprintf(stderr, "Bad argument '%s'. For more information, try '--help'.\n", argv[1]);
        return 1;
    }

    return 0;
}
