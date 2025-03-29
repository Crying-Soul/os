#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define SHM_SIZE 1024

typedef struct {
    char data[SHM_SIZE];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int ready;
    int iterations;  // Добавлено поле для хранения количества итераций
} shared_data;

void* writer(void *arg) {
    shared_data *shdata = (shared_data*) arg;
    for (int i = 0; i < shdata->iterations; i++) {  // Используем переданное количество итераций
        pthread_mutex_lock(&shdata->mutex);
        while (shdata->ready != 0) {
            pthread_cond_wait(&shdata->cond, &shdata->mutex);
        }
        snprintf(shdata->data, SHM_SIZE, "Сообщение %d", i+1);
        printf("Писатель отправил: %s\n", shdata->data);
        shdata->ready = 1;
        pthread_cond_signal(&shdata->cond);
        pthread_mutex_unlock(&shdata->mutex);
    }
    return NULL;
}

void* reader(void *arg) {
    shared_data *shdata = (shared_data*) arg;
    for (int i = 0; i < shdata->iterations; i++) {  // Используем переданное количество итераций
        pthread_mutex_lock(&shdata->mutex);
        while (shdata->ready != 1) {
            pthread_cond_wait(&shdata->cond, &shdata->mutex);
        }
        printf("Читатель получил: %s\n", shdata->data);
        shdata->ready = 0;
        pthread_cond_signal(&shdata->cond);
        pthread_mutex_unlock(&shdata->mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Использование: %s <количество_итераций>\n", argv[0]);
        return 1;
    }
    
    int iterations = atoi(argv[1]);
    if (iterations <= 0) {
        printf("Количество итераций должно быть положительным числом\n");
        return 1;
    }

    shared_data shdata = {
        .ready = 0,
        .iterations = iterations  // Устанавливаем количество итераций
    };
    pthread_mutex_init(&shdata.mutex, NULL);
    pthread_cond_init(&shdata.cond, NULL);

    pthread_t writer_tid, reader_tid;
    pthread_create(&writer_tid, NULL, writer, &shdata);
    pthread_create(&reader_tid, NULL, reader, &shdata);

    pthread_join(writer_tid, NULL);
    pthread_join(reader_tid, NULL);

    pthread_mutex_destroy(&shdata.mutex);
    pthread_cond_destroy(&shdata.cond);

    return 0;
}