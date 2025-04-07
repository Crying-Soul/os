/******************************************************************************
 *  
 *  Цель:
 *      Демонстрация обмена данными между двумя потоками (писателем и читателем)
 *      с использованием разделяемой памяти и средств синхронизации потоков
 *      (мьютекс и условная переменная).
 *
 *  Описание:
 *      Программа создает два потока: писатель и читатель. Поток-писатель
 *      записывает сообщения в разделяемую память, а поток-читатель считывает
 *      эти сообщения.  Синхронизация доступа к разделяемой памяти
 *      обеспечивается мьютексом и условной переменной.
 *
 *  Основные компоненты:
 *      - Структура `shared_data`:  Содержит разделяемые данные, мьютекс,
 *        условную переменную и флаг готовности данных.
 *      - Мьютекс `pthread_mutex_t`: Обеспечивает взаимное исключение при
 *        доступе к разделяемой памяти.
 *      - Условная переменная `pthread_cond_t`: Используется для уведомления
 *        потоков о готовности данных.
 *      - Функция `writer()`:  Поток-писатель, записывающий данные в
 *        разделяемую память.
 *      - Функция `reader()`:  Поток-читатель, считывающий данные из
 *        разделяемой памяти.
 *
 *  Схема работы:
 *      1. Инициализация мьютекса и условной переменной.
 *      2. Создание потоков писателя и читателя.
 *      3. Поток-писатель захватывает мьютекс, ожидает, пока данные будут
 *         считаны (флаг `ready == 0`), записывает данные, устанавливает флаг
 *         `ready = 1` и сигнализирует условной переменной.
 *      4. Поток-читатель захватывает мьютекс, ожидает, пока данные будут
 *         готовы для чтения (флаг `ready == 1`), считывает данные,
 *         устанавливает флаг `ready = 0` и сигнализирует условной переменной.
 *      5. Потоки повторяют этот процесс заданное количество раз.
 *      6. Уничтожение мьютекса и условной переменной.
 *
 *  Использование:
 *      ./thread_shared_memory <количество_итераций>
 *
 *  Примечания:
 *      - Для компиляции требуется библиотека pthread:  `gcc -o thread_shared_memory thread_shared_memory.c -lpthread`
 *      - Размер разделяемой памяти ограничен SHM_SIZE.
 *      - Код предполагает, что потоки не будут отменены (pthread_cancel).
 *
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SHM_SIZE 1024

typedef struct {
    char data[SHM_SIZE];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ready;       // Флаг готовности данных: 0 – данные не готовы, 1 – данные готовы для чтения
    int iterations;  // Количество итераций
} shared_data;

void* writer(void *arg) {
    shared_data *shdata = (shared_data*) arg;
    for (int i = 0; i < shdata->iterations; i++) {
        if (pthread_mutex_lock(&shdata->mutex) != 0) {
            perror("pthread_mutex_lock (writer)");
            pthread_exit(NULL);
        }
        // Ждем, пока данные не будут прочитаны (ready == 0)
        while (shdata->ready != 0) {
            if (pthread_cond_wait(&shdata->cond, &shdata->mutex) != 0) {
                perror("pthread_cond_wait (writer)");
                pthread_mutex_unlock(&shdata->mutex);
                pthread_exit(NULL);
            }
        }
        snprintf(shdata->data, SHM_SIZE, "Сообщение %d", i + 1);
        printf("Писатель отправил: %s\n", shdata->data);
        shdata->ready = 1;
        if (pthread_cond_signal(&shdata->cond) != 0) {
            perror("pthread_cond_signal (writer)");
            pthread_mutex_unlock(&shdata->mutex);
            pthread_exit(NULL);
        }
        if (pthread_mutex_unlock(&shdata->mutex) != 0) {
            perror("pthread_mutex_unlock (writer)");
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}

void* reader(void *arg) {
    shared_data *shdata = (shared_data*) arg;
    for (int i = 0; i < shdata->iterations; i++) {
        if (pthread_mutex_lock(&shdata->mutex) != 0) {
            perror("pthread_mutex_lock (reader)");
            pthread_exit(NULL);
        }
        // Ждем, пока данные не будут готовы для чтения (ready == 1)
        while (shdata->ready != 1) {
            if (pthread_cond_wait(&shdata->cond, &shdata->mutex) != 0) {
                perror("pthread_cond_wait (reader)");
                pthread_mutex_unlock(&shdata->mutex);
                pthread_exit(NULL);
            }
        }
        printf("Читатель получил: %s\n", shdata->data);
        shdata->ready = 0;
        if (pthread_cond_signal(&shdata->cond) != 0) {
            perror("pthread_cond_signal (reader)");
            pthread_mutex_unlock(&shdata->mutex);
            pthread_exit(NULL);
        }
        if (pthread_mutex_unlock(&shdata->mutex) != 0) {
            perror("pthread_mutex_unlock (reader)");
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <количество_итераций>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        fprintf(stderr, "Количество итераций должно быть положительным числом\n");
        return EXIT_FAILURE;
    }

    shared_data shdata;
    shdata.ready = 0;
    shdata.iterations = iterations;
    
    if (pthread_mutex_init(&shdata.mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }
    if (pthread_cond_init(&shdata.cond, NULL) != 0) {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&shdata.mutex);
        return EXIT_FAILURE;
    }

    pthread_t writer_tid, reader_tid;
    int ret;
    
    ret = pthread_create(&writer_tid, NULL, writer, &shdata);
    if (ret != 0) {
        errno = ret;
        perror("pthread_create (writer)");
        pthread_cond_destroy(&shdata.cond);
        pthread_mutex_destroy(&shdata.mutex);
        return EXIT_FAILURE;
    }
    
    ret = pthread_create(&reader_tid, NULL, reader, &shdata);
    if (ret != 0) {
        errno = ret;
        perror("pthread_create (reader)");
        pthread_cancel(writer_tid);
        pthread_join(writer_tid, NULL);
        pthread_cond_destroy(&shdata.cond);
        pthread_mutex_destroy(&shdata.mutex);
        return EXIT_FAILURE;
    }
    
    pthread_join(writer_tid, NULL);
    pthread_join(reader_tid, NULL);

    pthread_cond_destroy(&shdata.cond);
    pthread_mutex_destroy(&shdata.mutex);

    return EXIT_SUCCESS;
}