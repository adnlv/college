# Звіт з лабораторної роботи №7: Робота з системними сервісами в операційній системі Linux

**Виконав:** Данилов Артем, група 80, № за журналом 6

---

## 1. Тема та мета роботи

- **Тема:** Робота з системними сервісами в операційній системі Linux.
- **Мета:** Навчитись керувати системними сервісами за допомогою `systemd` (`systemctl`, `journalctl`), налаштовувати віддалений доступ і передачу файлів між вузлами локальної мережі через SSH та SCP (з паролем та безпарольною автентифікацією за ключами), а також створювати власні системні утиліти на мові C і планувати їх періодичне виконання через демон `cron`.

---

## 2. Хід виконання завдань

### Завдання 1. Підключення між двома комп'ютерами в локальній мережі за допомогою SSH

Для виконання завдання два комп'ютери в одній локальній мережі виступають у ролях **PC1** (сервер, до якого підключаються) та **PC2** (клієнт), після чого ролі змінюються.

1. **Визначення локальної IP-адреси на PC1:**

   ```bash
   ip addr show
   ```

   Команда `ip addr show` виводить мережеві інтерфейси системи та призначені їм IPv4/IPv6-адреси (наприклад, `192.168.1.10/24` на інтерфейсі `eth0` або `wlan0`), що дозволяє клієнту PC2 визначити адресу сервера для встановлення мережевого з'єднання.

2. **Перевірка та запуск демона SSH (`sshd`) на PC1:**

   ```bash
   systemctl status sshd
   # Якщо сервіс не активний:
   sudo systemctl start sshd
   systemctl is-active sshd
   ```

   За допомогою `systemctl status sshd` перевіряється поточний стан демона OpenSSH. Якщо він не перебуває у стані `active (running)`, команда `sudo systemctl start sshd` запускає його для прийому вхідних SSH-з'єднань на порту `22`.

3. **Підключення з PC2 до PC1 по SSH та перевірка робочої директорії:**

   ```bash
   ssh Student1@192.168.1.10
   # Після введення пароля користувача на PC1:
   pwd
   touch ~/test_pc2.txt
   ls -la ~/test_pc2.txt
   exit
   ```

   Команда `ssh <user>@<ip>` ініціює захищене шифроване з'єднання до PC1. Після успішної автентифікації команда `pwd` підтверджує, що сесія відкрилась у домашній директорії користувача на PC1 (`/home/Student1`). Команда `touch ~/test_pc2.txt` створює тестовий файл безпосередньо на віддаленому комп'ютері PC1, а `exit` коректно завершує SSH-сесію.

4. **Перегляд логів сервісу `sshd`:**

   ```bash
   journalctl -u sshd -n 30 --no-pager
   ```

   Утиліта `journalctl` із прапорцем `-u sshd` читає журнал `systemd` для SSH-демона, що дозволяє перевірити час підключення, IP-адресу клієнта, спосіб автентифікації та факт закриття сесії.

5. **Зворотне підключення (зміна ролей: з PC1 до PC2):**

   На PC2 аналогічно визначається IP-адреса (`ip addr show`, наприклад `192.168.1.50`) і запускається `sshd` (`sudo systemctl start sshd`), після чого з PC1 виконується підключення `ssh Student2@192.168.1.50`, перевірка `pwd`, створення файлу та аналіз логів `journalctl -u sshd`.

---

### Завдання 2. Передача файлів через SCP та налаштування SSH без пароля

1. **Генерація пари SSH-ключів на PC1 (алгоритм RSA):**

   ```bash
   ssh-keygen -t rsa -b 4096
   ls -la ~/.ssh/id_rsa ~/.ssh/id_rsa.pub
   ```

   Команда `ssh-keygen -t rsa` створює криптографічну пару ключів у `~/.ssh/`: приватний ключ `id_rsa`, який залишається лише на PC1 із правами `0600`, та публічний ключ `id_rsa.pub` для передачі на віддалені вузли.

2. **Копіювання публічного ключа на PC2 за допомогою `scp` та додавання до `authorized_keys`:**

   На PC1 виконуємо передачу публічного ключа у тимчасову директорію на PC2:

   ```bash
   scp ~/.ssh/id_rsa.pub Student2@192.168.1.50:/tmp/pc1_key.pub
   ```

   Далі на PC2 (або через SSH-сесію) додаємо ключ до списку довірених і встановлюємо відповідні права доступу, оскільки демон `sshd` вимагає суворих обмежень для `~/.ssh` (`700`) та `~/.ssh/authorized_keys` (`600`), щоб унеможливити підміну ключів іншими користувачами системи:

   ```bash
   mkdir -p ~/.ssh
   chmod 700 ~/.ssh
   cat /tmp/pc1_key.pub >> ~/.ssh/authorized_keys
   chmod 600 ~/.ssh/authorized_keys
   rm -f /tmp/pc1_key.pub
   cat ~/.ssh/authorized_keys
   ```

3. **Перевірка безпарольного SSH-підключення з PC1 до PC2:**

   ```bash
   ssh Student2@192.168.1.50
   ```

   Оскільки публічний ключ PC1 уже міститься в `authorized_keys` на PC2, сервер перевіряє володіння приватним ключем через криптографічний виклик-відповідь і надає доступ без запиту пароля користувача.

4. **Передача файлу `pc1_test.txt` з PC1 на PC2 через `scp`:**

   На PC1:

   ```bash
   echo "Тестовий файл з PC1" > ~/pc1_test.txt
   scp ~/pc1_test.txt Student2@192.168.1.50:~/
   ```

   На PC2 перевіряємо наявність файлу та логи `sshd`:

   ```bash
   ls -la ~/pc1_test.txt
   cat ~/pc1_test.txt
   journalctl -u sshd -n 20 --no-pager
   ```

5. **Створення директорії `pc2_dir` на PC2 та рекурсивне копіювання на PC1:**

   На PC2:

   ```bash
   mkdir -p ~/pc2_dir
   echo "Тестовий файл з PC2" > ~/pc2_dir/pc2_test.txt
   ```

   На PC1 завантажуємо директорію без використання пароля за допомогою прапорця `-r`, який забезпечує рекурсивну передачу всієї директорії разом із вкладеними файлами поверх захищеного SSH-каналу:

   ```bash
   scp -r Student2@192.168.1.50:~/pc2_dir ~/
   ls -la ~/pc2_dir
   cat ~/pc2_dir/pc2_test.txt
   journalctl -u sshd -n 20 --no-pager
   ```

6. **Повторення операцій у зворотному напрямку (починаючи з PC2):**

   На PC2 генерується пара RSA-ключів (`ssh-keygen -t rsa`), публічний ключ передається на PC1 (`scp ~/.ssh/id_rsa.pub Student1@192.168.1.10:/tmp/pc2_key.pub`) і додається у `~/.ssh/authorized_keys` на PC1, після чого повторюються безпарольний вхід, копіювання файлу та директорії в зворотному напрямку.

---

### Завдання 3. Планування завдань з `cron`

1. **Розробка C-утиліти `current_time_80_6`:**

   У файлі [`current_time_80_6.c`](./current_time_80_6.c) реалізовано POSIX-сумісну утиліту на стандарті C11, яка:
   - визначає шлях до лог-файлу за замовчуванням як `$HOME/current_time_80_6.log` (із резервним шляхом `/tmp/current_time_80_6.log`, якщо змінна `HOME` не задана), або приймає явний шлях через `-o` / `--output`;
   - отримує поточний системний час за допомогою `time(NULL)` та перетворює його в локальний час через потокобезпечну функцію `localtime_r()`;
   - форматує дату й час через `strftime()` у формат `[YYYY-MM-DD HH:MM:SS TZ]`;
   - відкриває лог-файл у режимі дозапису (`"a"`) та додає рядок із часовою міткою й ідентифікатором процесу (`getpid()`), перевіряючи коректність запису та закриття файлу.

   Використання `$HOME/current_time_80_6.log` замість відносного шляху є важливим для запуску під `cron`, оскільки демон `cron` запускає завдання з мінімальним оточенням і не прив'язаний до робочої теки репозиторію.

2. **Компіляція та глобальне встановлення утиліти:**

   Для автоматизації збірки створено [`Makefile`](./Makefile) зі строгими прапорцями компілятора (`-std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g`) та цілями `all`, `debug`, `install`, `uninstall`, `clean`:

   ```bash
   make
   sudo make install
   which current_time_80_6
   ```

   Команда `make` компілює бінарний файл `current_time_80_6`, а `sudo make install` встановлює його у `/usr/local/bin/current_time_80_6` із правами `0755`, роблячи команду доступною глобально в системі.

3. **Перевірка локальної роботи утиліти:**

   Утиліту скомпільовано (включно з санітайзером `-fsanitize=address,undefined`) та перевірено локально без жодних попереджень компілятора:

   ```text
   $ cc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g current_time_80_6.c -o current_time_80_6
   $ ./current_time_80_6 --help
   Використання: ./current_time_80_6 [ПАРАМЕТРИ]

   Записує поточний локальний час у лог-файл (за замовчуванням ~/current_time_80_6.log).

   Параметри:
     -o, --output <шлях>  Шлях до лог-файлу (за замовчуванням $HOME/current_time_80_6.log,
                          або /tmp/current_time_80_6.log, якщо HOME не задано)
     -h, --help           Показати цю довідку

   $ HOME=/tmp/lab7_test_home ./current_time_80_6_debug
   $ HOME=/tmp/lab7_test_home ./current_time_80_6_debug
   $ cat /tmp/lab7_test_home/current_time_80_6.log
   [2026-11-22 16:46:10 EET] current_time_80_6 (pid=57): logged current time
   [2026-11-22 16:46:10 EET] current_time_80_6 (pid=60): logged current time
   ```

4. **Перевірка та запуск демона `crond`:**

   ```bash
   systemctl status crond
   # Якщо демон не встановлений (наприклад, на Arch/CachyOS):
   # sudo pacman -S cronie
   sudo systemctl enable --now crond
   systemctl status crond
   ```

   На дистрибутивах Arch/CachyOS планувальник `cron` надає пакет `cronie` (сервіс `cronie.service` / `crond.service`), на RHEL/Fedora використовується `crond.service`, а на Debian/Ubuntu — `cron.service`.

5. **Планування виконання у `crontab` кожні 2 хвилини:**

   Відкриваємо таблицю завдань користувача для редагування:

   ```bash
   crontab -e
   ```

   Додаємо правило із вказанням повного шляху до встановленого бінарного файлу:

   ```cron
   */2 * * * * /usr/local/bin/current_time_80_6
   ```

   Перевіряємо збережену конфігурацію командою `crontab -l`.

6. **Очікування 4–6 хвилин, перевірка лог-файлу та логів `crond`:**

   Після 4–6 хвилин очікування (`cron` виконує завдання 2–3 рази з інтервалом у 2 хвилини) перевіряємо вміст лог-файлу та системний журнал:

   ```bash
   cat ~/current_time_80_6.log
   journalctl -u crond -n 20 --no-pager
   ```

---

## 3. Повний програмний код

### Файл `current_time_80_6.c`

```c
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
```

### Файл `Makefile`

```makefile
CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Wconversion -O2 -g
SAN_FLAGS = -fsanitize=address,undefined

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

TARGET = current_time_80_6
SRC = current_time_80_6.c

.PHONY: all debug install uninstall clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@

debug: $(SRC)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $< -o $(TARGET)_debug $(SAN_FLAGS)

install: $(TARGET)
	install -d "$(DESTDIR)$(BINDIR)"
	install -m 0755 "$(TARGET)" "$(DESTDIR)$(BINDIR)/$(TARGET)"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/$(TARGET)"

clean:
	rm -f $(TARGET) $(TARGET)_debug
```

---

## 4. Відповіді на контрольні запитання

Повні відповіді на всі 5 контрольних запитань винесено у файл [`questions.md`](./questions.md):

1. **Етапи підключення по SSH без паролю:** генерація пари ключів `ssh-keygen`, передача публічного ключа на сервер (`scp` / `ssh-copy-id`), додавання до `~/.ssh/authorized_keys` із коректними правами (`700` / `600`), перевірка `PubkeyAuthentication yes` на сервері та вхід через `ssh`.
2. **Команда `systemctl` для перевірки стану `cron`:** `systemctl status crond` (або `cronie` / `cron` залежно від дистрибутиву), а також `systemctl is-active crond`.
3. **Команда перевірки логів системного сервісу:** `journalctl -u <назва_сервісу>` (наприклад, `journalctl -u sshd`, `journalctl -u crond`).
4. **Ключова перевага SSH-ключів перед паролем:** значно вища ентропія (стійкість до brute-force) та криптографічна автентифікація виклик-відповідь без передачі приватного секрету мережею чи серверу.
5. **Параметри для створення SSH-тунелю:** специфікація перенаправлення портів (`-L` для локального, `-R` для зворотного, `-D` для SOCKS-проксі), адреса користувача та сервера (`user@host`), а також прапорці `-N` (без інтерактивної оболонки), `-f` (фоновий режим), `-p` (порт SSH), `-i` (приватний ключ).
