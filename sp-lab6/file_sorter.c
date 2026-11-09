#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/* Print usage instructions */
static void print_usage(const char *prog_name)
{
    fprintf(stderr,
            "Використання: %s --source=<вихідний_шлях> --dest=<цільовий_шлях>\n"
            "   або:       %s --source <вихідний_шлях> --dest <цільовий_шлях>\n"
            "   або:       %s -s <вихідний_шлях> -d <цільовий_шлях>\n"
            "\n"
            "Параметри:\n"
            "  --source, -s  Шлях до вихідної директорії для сортування\n"
            "  --dest,   -d  Шлях до цільової директорії для розміщення категорій\n"
            "  --help,   -h  Показати цю довідку\n",
            prog_name, prog_name, prog_name);
}

/* Dual logger: writes message to stdout and appends to sort_report.log */
static void log_msg(FILE *log_fp, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    putchar('\n');
    va_end(args);

    if (log_fp != NULL)
    {
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args);
        fputc('\n', log_fp);
        fflush(log_fp);
        va_end(args);
    }
}

/* Remove trailing slashes from path in-place, except for root "/" */
static void trim_trailing_slash(char *path)
{
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/')
    {
        path[len - 1] = '\0';
        len--;
    }
}

/* Categorize a regular file based on its extension (case-insensitive) */
static const char *get_file_category(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    /* No extension or dot is leading char (.gitignore, etc.) */
    if (dot == NULL || dot == filename)
    {
        return "Other";
    }

    if (strcasecmp(dot, ".txt") == 0 || strcasecmp(dot, ".md") == 0)
    {
        return "Documents";
    }
    if (strcasecmp(dot, ".c") == 0 || strcasecmp(dot, ".h") == 0 || strcasecmp(dot, ".go") == 0)
    {
        return "Sources";
    }
    if (strcasecmp(dot, ".jpg") == 0 || strcasecmp(dot, ".png") == 0)
    {
        return "Images";
    }
    if (strcasecmp(dot, ".pdf") == 0)
    {
        return "PDFs";
    }

    return "Other";
}

/* Ensure directory exists; if not, create it and log */
static int ensure_directory(const char *path, FILE *log_fp)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return 0;
        }
        log_msg(log_fp, "Помилка: шлях '%s' існує, але не є директорією", path);
        return -1;
    }

    if (mkdir(path, 0755) != 0)
    {
        log_msg(log_fp, "Помилка: не вдалося створити директорію '%s': %s", path, strerror(errno));
        return -1;
    }

    log_msg(log_fp, "Створено папку: %s", path);
    return 0;
}

/* Copy file block by block (used as fallback for cross-filesystem EXDEV) */
static int copy_file_data(const char *src, const char *dest)
{
    FILE *in = fopen(src, "rb");
    if (in == NULL)
    {
        return -1;
    }

    FILE *out = fopen(dest, "wb");
    if (out == NULL)
    {
        fclose(in);
        return -1;
    }

    char buf[8192];
    size_t n = 0;
    int status = 0;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    {
        if (fwrite(buf, 1, n, out) != n)
        {
            status = -1;
            break;
        }
    }

    if (ferror(in))
    {
        status = -1;
    }

    fclose(in);
    if (fclose(out) != 0)
    {
        status = -1;
    }

    return status;
}

/* Move file safely: rename() with EXDEV fallback, logging errors if failed */
static int relocate_file(const char *src_path, const char *dest_path, FILE *log_fp)
{
    if (rename(src_path, dest_path) == 0)
    {
        log_msg(log_fp, "Переміщено: %s -> %s", src_path, dest_path);
        return 0;
    }

    if (errno == EXDEV)
    {
        if (copy_file_data(src_path, dest_path) == 0)
        {
            if (unlink(src_path) == 0)
            {
                log_msg(log_fp, "Переміщено: %s -> %s", src_path, dest_path);
                return 0;
            }
            log_msg(log_fp, "Помилка: не вдалося видалити '%s' після копіювання: %s", src_path, strerror(errno));
            return -1;
        }
        log_msg(log_fp, "Помилка: не вдалося скопіювати '%s' до '%s': %s", src_path, dest_path, strerror(errno));
        return -1;
    }

    log_msg(log_fp, "Помилка: не вдалося перемістити '%s' до '%s': %s", src_path, dest_path, strerror(errno));
    return -1;
}

/*
 * Resolve destination file path within category folder.
 * If a file with the target name already exists, resolve collision by
 * appending a numerical suffix before the extension (e.g. name_1.ext).
 */
static int resolve_dest_path(const char *cat_dir, const char *orig_name, char *out_buf, size_t out_buf_size)
{
    int ret = snprintf(out_buf, out_buf_size, "%s/%s", cat_dir, orig_name);
    if (ret < 0 || (size_t)ret >= out_buf_size)
    {
        return -1;
    }

    struct stat st;
    if (stat(out_buf, &st) != 0)
    {
        /* Target does not exist, safe to use */
        return 0;
    }

    /* Collision detected: generate unique name */
    const char *dot = strrchr(orig_name, '.');
    char stem[256];
    const char *ext = "";

    if (dot != NULL && dot != orig_name)
    {
        size_t stem_len = (size_t)(dot - orig_name);
        if (stem_len >= sizeof(stem))
        {
            stem_len = sizeof(stem) - 1;
        }
        memcpy(stem, orig_name, stem_len);
        stem[stem_len] = '\0';
        ext = dot;
    }
    else
    {
        size_t name_len = strlen(orig_name);
        if (name_len >= sizeof(stem))
        {
            name_len = sizeof(stem) - 1;
        }
        memcpy(stem, orig_name, name_len);
        stem[name_len] = '\0';
    }

    int collision_idx = 1;
    while (collision_idx < 100000)
    {
        ret = snprintf(out_buf, out_buf_size, "%s/%s_%d%s", cat_dir, stem, collision_idx, ext);
        if (ret < 0 || (size_t)ret >= out_buf_size)
        {
            return -1;
        }

        if (stat(out_buf, &st) != 0)
        {
            return 0;
        }
        collision_idx++;
    }

    return -1;
}

/* Recursively traverse source directory and relocate regular files */
static int sort_directory_recursive(const char *current_src,
                                    const char *dest_root,
                                    dev_t dest_dev,
                                    ino_t dest_ino,
                                    bool dest_has_inode,
                                    FILE *log_fp)
{
    DIR *dir = opendir(current_src);
    if (dir == NULL)
    {
        log_msg(log_fp, "Помилка: не вдалося відкрити директорію '%s': %s", current_src, strerror(errno));
        return -1;
    }

    struct dirent *ent = NULL;
    int overall_status = 0;

    while ((ent = readdir(dir)) != NULL)
    {
        /* Skip dotfiles and dotdirs */
        if (ent->d_name[0] == '.')
        {
            continue;
        }

        char src_item_path[PATH_MAX];
        int ret = snprintf(src_item_path, sizeof(src_item_path), "%s/%s", current_src, ent->d_name);
        if (ret < 0 || (size_t)ret >= sizeof(src_item_path))
        {
            log_msg(log_fp, "Помилка: переповнення буфера шляху для '%s/%s'", current_src, ent->d_name);
            continue;
        }

        struct stat st;
        if (lstat(src_item_path, &st) != 0)
        {
            log_msg(log_fp, "Помилка: не вдалося отримати stat для '%s': %s", src_item_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            /* Prevent recursing into destination if destination is inside source */
            if (dest_has_inode && st.st_dev == dest_dev && st.st_ino == dest_ino)
            {
                continue;
            }

            int sub_res = sort_directory_recursive(src_item_path, dest_root, dest_dev, dest_ino, dest_has_inode, log_fp);
            if (sub_res != 0)
            {
                overall_status = sub_res;
            }
        }
        else if (S_ISREG(st.st_mode))
        {
            const char *category = get_file_category(ent->d_name);

            char cat_dir[PATH_MAX];
            ret = snprintf(cat_dir, sizeof(cat_dir), "%s/%s", dest_root, category);
            if (ret < 0 || (size_t)ret >= sizeof(cat_dir))
            {
                log_msg(log_fp, "Помилка: переповнення буфера для категорії '%s'", category);
                continue;
            }

            if (ensure_directory(cat_dir, log_fp) != 0)
            {
                overall_status = -1;
                continue;
            }

            char target_path[PATH_MAX];
            if (resolve_dest_path(cat_dir, ent->d_name, target_path, sizeof(target_path)) != 0)
            {
                log_msg(log_fp, "Помилка: не вдалося створити цільовий шлях для '%s'", ent->d_name);
                overall_status = -1;
                continue;
            }

            if (relocate_file(src_item_path, target_path, log_fp) != 0)
            {
                overall_status = -1;
            }
        }
    }

    closedir(dir);
    return overall_status;
}

int main(int argc, char *argv[])
{
    const char *source_arg = NULL;
    const char *dest_arg = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "--source=", 9) == 0)
        {
            source_arg = argv[i] + 9;
        }
        else if (strcmp(argv[i], "--source") == 0 || strcmp(argv[i], "-s") == 0)
        {
            if (i + 1 < argc)
            {
                source_arg = argv[++i];
            }
            else
            {
                fprintf(stderr, "Помилка: прапорець '%s' вимагає вказання шляху.\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
        }
        else if (strncmp(argv[i], "-s=", 3) == 0)
        {
            source_arg = argv[i] + 3;
        }
        else if (strncmp(argv[i], "--dest=", 7) == 0)
        {
            dest_arg = argv[i] + 7;
        }
        else if (strcmp(argv[i], "--dest") == 0 || strcmp(argv[i], "-d") == 0)
        {
            if (i + 1 < argc)
            {
                dest_arg = argv[++i];
            }
            else
            {
                fprintf(stderr, "Помилка: прапорець '%s' вимагає вказання шляху.\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
        }
        else if (strncmp(argv[i], "-d=", 3) == 0)
        {
            dest_arg = argv[i] + 3;
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        else
        {
            fprintf(stderr, "Помилка: невідомий параметр '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (source_arg == NULL || dest_arg == NULL)
    {
        fprintf(stderr, "Помилка: обидва параметри --source та --dest є обов'язковими.\n");
        print_usage(argv[0]);
        return 1;
    }

    char src_clean[PATH_MAX];
    char dest_clean[PATH_MAX];

    size_t src_len = strlen(source_arg);
    size_t dest_len = strlen(dest_arg);

    if (src_len >= sizeof(src_clean) || dest_len >= sizeof(dest_clean))
    {
        fprintf(stderr, "Помилка: занадто довгий шлях.\n");
        return 1;
    }

    memcpy(src_clean, source_arg, src_len + 1);
    memcpy(dest_clean, dest_arg, dest_len + 1);

    trim_trailing_slash(src_clean);
    trim_trailing_slash(dest_clean);

    struct stat src_st;
    if (stat(src_clean, &src_st) != 0)
    {
        fprintf(stderr, "Помилка: не вдалося отримати доступ до вихідної директорії '%s': %s\n",
                src_clean, strerror(errno));
        return 1;
    }

    if (!S_ISDIR(src_st.st_mode))
    {
        fprintf(stderr, "Помилка: вихідний шлях '%s' не є директорією.\n", src_clean);
        return 1;
    }

    FILE *log_fp = fopen("sort_report.log", "a");
    if (log_fp == NULL)
    {
        fprintf(stderr, "Попередження: не вдалося відкрити 'sort_report.log': %s\n", strerror(errno));
    }

    /* Opening log line */
    log_msg(log_fp, "--- Програма сортування запущена ---");

    /* Ensure destination root directory exists */
    if (ensure_directory(dest_clean, log_fp) != 0)
    {
        log_msg(log_fp, "Сортування завершено! Див. 'sort_report.log'.");
        if (log_fp != NULL)
        {
            fclose(log_fp);
        }
        return 1;
    }

    struct stat dest_st;
    bool dest_has_inode = false;
    if (stat(dest_clean, &dest_st) == 0 && S_ISDIR(dest_st.st_mode))
    {
        dest_has_inode = true;
        if (src_st.st_dev == dest_st.st_dev && src_st.st_ino == dest_st.st_ino)
        {
            log_msg(log_fp, "Помилка: вихідна та цільова директорії є однаковими.");
            log_msg(log_fp, "Сортування завершено! Див. 'sort_report.log'.");
            if (log_fp != NULL)
            {
                fclose(log_fp);
            }
            return 1;
        }
    }

    /* Recursively scan and sort files */
    int sort_res = sort_directory_recursive(src_clean,
                                           dest_clean,
                                           dest_st.st_dev,
                                           dest_st.st_ino,
                                           dest_has_inode,
                                           log_fp);

    /* Concluding log line */
    log_msg(log_fp, "Сортування завершено! Див. 'sort_report.log'.");

    if (log_fp != NULL)
    {
        fclose(log_fp);
    }

    return (sort_res == 0) ? 0 : 1;
}
