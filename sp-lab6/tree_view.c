#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

static int dir_count = 0;
static int file_count = 0;

int print_dir_deep(const char *name, int depth)
{
    DIR *dir = opendir(name);
    if (dir == NULL)
    {
        fprintf(stderr, "Failed to open '%s': %s\n", name, strerror(errno));
        return -1;
    }

    const int space_count = depth * 2;

    struct dirent *ent = NULL;
    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_name[0] == '.')
        {
            continue;
        }

        for (int i = 0; i < space_count; i++)
        {
            putchar(i % 2 == 0 ? '|' : ' ');
        }
        printf("L %s\n", ent->d_name);

        if (ent->d_type == DT_DIR)
        {
            dir_count++;

            char nested_path[1024];
            snprintf(nested_path, sizeof(nested_path), "%s/%s", name, ent->d_name);

            int res = print_dir_deep(nested_path, depth + 1);
            if (res != 0)
            {
                closedir(dir);
                return res;
            }
        }
        else
        {
            file_count++;
        }
    }

    closedir(dir);
    return 0;
}

int main(int argc, char const *argv[])
{
    const char *dir_name = ".";
    if (argc >= 2)
    {
        dir_name = argv[1];
    }

    print_dir_deep(dir_name, 0);

    printf("Directories: %d, Files: %d\n", dir_count, file_count);
    return 0;
}
