#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

// Функция для вывода сообщения об ошибке и завершения программы.
void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Преобразует структуру timespec в секунды с плавающей точкой.
double timespec_to_sec(const struct timespec *ts) {
    return ts->tv_sec + ts->tv_nsec / 1e9;
}

// Вычисляет разницу между двумя моментами времени.
double diff_timespec(const struct timespec *start, const struct timespec *end) {
    return timespec_to_sec(end) - timespec_to_sec(start);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        exit(EXIT_FAILURE);
    }

    // Замер начального времени с использованием CLOCK_MONOTONIC_RAW для большей точности.
    struct timespec t_start, t_end;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &t_start) == -1) {
        error_exit("clock_gettime (start)");
    }

    // Создаём дочерний процесс.
    pid_t pid = fork();
    if (pid < 0) {
        error_exit("fork");
    }  else if (pid == 0) {
        // Перенаправляем stdout и stderr в /dev/null, чтобы исключить лишний вывод.
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
        execvp(argv[1], &argv[1]);
        // Если execvp вернулся, значит произошла ошибка.
        fprintf(stderr, "execvp failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }else {
        int status;
        struct rusage ru_child;
        
        // Ожидание завершения дочернего процесса + получение статистики CPU.
        if (wait4(pid, &status, 0, &ru_child) == -1) {
            error_exit("wait4");
        }

        // Замер конечного времени.
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &t_end) == -1) {
            error_exit("clock_gettime (end)");
        }

        // Вычисляем общее время выполнения.
        double real_time = diff_timespec(&t_start, &t_end);

        // Вычисляем пользовательское и системное время процессора.
        double user_cpu_time = ru_child.ru_utime.tv_sec + ru_child.ru_utime.tv_usec / 1e6;
        double sys_cpu_time = ru_child.ru_stime.tv_sec + ru_child.ru_stime.tv_usec / 1e6;
        double total_cpu_time = user_cpu_time + sys_cpu_time;

        // Расчёт процентной загрузки CPU.
        double cpu_usage = real_time > 0 ? (total_cpu_time / real_time) * 100.0 : 0.0;

        // Выводим только два числа: время выполнения и загрузку процессора.
        printf("%.9f %.2f\n", real_time, cpu_usage);
    }

    return EXIT_SUCCESS;
}