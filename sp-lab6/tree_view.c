#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_DEPTH 256

static int dir_count = 0;
static int file_count = 0;

/* Comparison function for sorting directory entries alphabetically */
static int compare_entries(const void *a, const void *b)
{
    const char *const *str_a = (const char *const *)a;
    const char *const *str_b = (const char *const *)b;
    return strcmp(*str_a, *str_b);
}

/* Free a dynamically allocated list of entry names */
static void free_entries(char **entries, size_t count)
{
    if (entries == NULL)
    {
        return;
    }
    for (size_t i = 0; i < count; i++)
    {
        free(entries[i]);
    }
    free(entries);
}

/* Recursive traversal and tree drawing */
static int print_tree(const char *dir_path, int depth, bool is_last_ancestor[MAX_DEPTH])
{
    if (depth >= MAX_DEPTH)
    {
        fprintf(stderr, "Error: Maximum directory depth (%d) exceeded at '%s'\n", MAX_DEPTH, dir_path);
        return -1;
    }

    DIR *dir = opendir(dir_path);
    if (dir == NULL)
    {
        fprintf(stderr, "Failed to open directory '%s': %s\n", dir_path, strerror(errno));
        return -1;
    }

    char **entries = NULL;
    size_t count = 0;
    size_t capacity = 0;

    struct dirent *ent = NULL;
    while ((ent = readdir(dir)) != NULL)
    {
        /* Skip dotfiles and dotdirs (. and .. and hidden files) */
        if (ent->d_name[0] == '.')
        {
            continue;
        }

        if (count >= capacity)
        {
            size_t new_cap = (capacity == 0) ? 16 : capacity * 2;
            char **new_entries = (char **)realloc(entries, new_cap * sizeof(char *));
            if (new_entries == NULL)
            {
                fprintf(stderr, "Memory allocation error\n");
                free_entries(entries, count);
                closedir(dir);
                return -1;
            }
            entries = new_entries;
            capacity = new_cap;
        }

        entries[count] = strdup(ent->d_name);
        if (entries[count] == NULL)
        {
            fprintf(stderr, "Memory allocation error\n");
            free_entries(entries, count);
            closedir(dir);
            return -1;
        }
        count++;
    }

    if (closedir(dir) != 0)
    {
        fprintf(stderr, "Error closing directory '%s': %s\n", dir_path, strerror(errno));
    }

    if (count > 0 && entries != NULL)
    {
        qsort(entries, count, sizeof(char *), compare_entries);
    }

    for (size_t i = 0; i < count; i++)
    {
        bool is_last = (i == count - 1);

        /* Print ancestor indentation */
        for (int d = 0; d < depth; d++)
        {
            if (is_last_ancestor[d])
            {
                printf("    ");
            }
            else
            {
                printf("|   ");
            }
        }

        /* Print branch connector */
        if (is_last)
        {
            printf("`-- %s\n", entries[i]);
        }
        else
        {
            printf("|-- %s\n", entries[i]);
        }

        char full_path[PATH_MAX];
        int ret = snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entries[i]);
        if (ret < 0 || (size_t)ret >= sizeof(full_path))
        {
            fprintf(stderr, "Path buffer overflow for '%s/%s'\n", dir_path, entries[i]);
            continue;
        }

        struct stat st;
        if (lstat(full_path, &st) != 0)
        {
            fprintf(stderr, "Cannot stat '%s': %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            dir_count++;
            is_last_ancestor[depth] = is_last;
            print_tree(full_path, depth + 1, is_last_ancestor);
        }
        else
        {
            file_count++;
        }
    }

    free_entries(entries, count);
    return 0;
}

int main(int argc, char const *argv[])
{
    const char *dir_name = ".";
    if (argc >= 2)
    {
        dir_name = argv[1];
    }

    struct stat st;
    if (stat(dir_name, &st) != 0)
    {
        fprintf(stderr, "Cannot access '%s': %s\n", dir_name, strerror(errno));
        return 1;
    }

    if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Path '%s' is not a directory\n", dir_name);
        return 1;
    }

    /* Print root directory name */
    printf("%s\n", dir_name);

    bool is_last_ancestor[MAX_DEPTH] = {false};
    int ret = print_tree(dir_name, 0, is_last_ancestor);

    printf("\n%d directories, %d files\n", dir_count, file_count);

    return (ret == 0) ? 0 : 1;
}
