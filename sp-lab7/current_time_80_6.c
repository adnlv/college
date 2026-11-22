#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define DEFAULT_LOG_FILENAME "current_time_80_6.log"
#define FALLBACK_LOG_PATH "/tmp/" DEFAULT_LOG_FILENAME

/* Print usage instructions to the specified stream. */
static void print_usage(FILE *stream, const char *prog_name)
{
    fprintf(stream,
            "Використання: %s [ПАРАМЕТРИ]\n"
            "\n"
            "Записує поточний локальний час у лог-файл (за замовчуванням ~/%s).\n"
            "\n"
            "Параметри:\n"
            "  -o, --output <шлях>  Шлях до лог-файлу (за замовчуванням $HOME/%s,\n"
            "                       або %s, якщо HOME не задано)\n"
            "  -h, --help           Показати цю довідку\n",
            prog_name, DEFAULT_LOG_FILENAME, DEFAULT_LOG_FILENAME, FALLBACK_LOG_PATH);
}

/* Parse CLI arguments; returns 0 on success, -1 on invalid input. */
static int parse_args(int argc, char *argv[], const char **output_path, int *show_help)
{
    *output_path = NULL;
    *show_help = 0;

    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            *show_help = 1;
            return 0;
        }

        if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
        {
            if (i + 1 >= argc || argv[i + 1][0] == '\0')
            {
                fprintf(stderr, "Помилка: параметр '%s' потребує непорожнього шляху до файлу\n", arg);
                return -1;
            }
            *output_path = argv[++i];
            continue;
        }

        if (strncmp(arg, "--output=", 9) == 0)
        {
            const char *val = arg + 9;
            if (val[0] == '\0')
            {
                fprintf(stderr, "Помилка: параметр '--output=' потребує непорожнього шляху\n");
                return -1;
            }
            *output_path = val;
            continue;
        }

        fprintf(stderr, "Помилка: невідомий параметр '%s'\n", arg);
        return -1;
    }

    return 0;
}

/* Resolve the default log file path into buf. */
static int resolve_default_log_path(char *buf, size_t buf_size)
{
    const char *home = getenv("HOME");
    int written;

    if (home != NULL && home[0] != '\0')
    {
        size_t home_len = strlen(home);
        const char *sep = (home[home_len - 1] == '/') ? "" : "/";
        written = snprintf(buf, buf_size, "%s%s%s", home, sep, DEFAULT_LOG_FILENAME);
    }
    else
    {
        written = snprintf(buf, buf_size, "%s", FALLBACK_LOG_PATH);
    }

    if (written < 0 || (size_t)written >= buf_size)
    {
        fprintf(stderr, "Помилка: шлях до лог-файлу перевищує буфер (%zu байт)\n", buf_size);
        return -1;
    }

    return 0;
}

/* Format current local wall-clock time into buf. */
static int format_timestamp(char *buf, size_t buf_size)
{
    time_t now = time(NULL);
    if (now == (time_t)-1)
    {
        fprintf(stderr, "Помилка: не вдалося отримати поточний час: %s\n", strerror(errno));
        return -1;
    }

    struct tm local_tm;
    if (localtime_r(&now, &local_tm) == NULL)
    {
        fprintf(stderr, "Помилка: не вдалося перетворити час у локальний\n");
        return -1;
    }

    if (strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S %Z", &local_tm) == 0)
    {
        fprintf(stderr, "Помилка: недостатній розмір буфера для форматування часу\n");
        return -1;
    }

    return 0;
}

/* Append a formatted timestamp entry to log_path. */
static int append_time_log(const char *log_path, const char *timestamp)
{
    FILE *fp = fopen(log_path, "a");
    if (fp == NULL)
    {
        fprintf(stderr, "Помилка: не вдалося відкрити файл '%s' для запису: %s\n",
                log_path, strerror(errno));
        return -1;
    }

    int write_ok = fprintf(fp, "[%s] current_time_80_6 (pid=%ld): logged current time\n",
                           timestamp, (long)getpid());
    int close_rc = fclose(fp);

    if (write_ok < 0)
    {
        fprintf(stderr, "Помилка: збій запису у файл '%s': %s\n", log_path, strerror(errno));
        return -1;
    }

    if (close_rc != 0)
    {
        fprintf(stderr, "Помилка: не вдалося коректно закрити файл '%s': %s\n",
                log_path, strerror(errno));
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    const char *cli_path = NULL;
    int show_help = 0;

    if (parse_args(argc, argv, &cli_path, &show_help) != 0)
    {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    if (show_help)
    {
        print_usage(stdout, argv[0]);
        return EXIT_SUCCESS;
    }

    char default_path[PATH_MAX];
    const char *log_path = cli_path;
    if (log_path == NULL)
    {
        if (resolve_default_log_path(default_path, sizeof(default_path)) != 0)
        {
            return EXIT_FAILURE;
        }
        log_path = default_path;
    }

    char timestamp[64];
    if (format_timestamp(timestamp, sizeof(timestamp)) != 0)
    {
        return EXIT_FAILURE;
    }

    if (append_time_log(log_path, timestamp) != 0)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
