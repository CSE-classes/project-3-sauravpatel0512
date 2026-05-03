#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
int count = 0;
int done = 0;

pthread_mutex_t mutex_lock;
pthread_cond_t not_full;
pthread_cond_t not_empty;

void *producer(void *arg)
{
    FILE *fp = fopen("message.txt", "r");
    int ch;

    if (fp == NULL) {
        printf("ERROR: can't open message.txt!\n");

        pthread_mutex_lock(&mutex_lock);
        done = 1;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex_lock);

        return NULL;
    }

    while ((ch = fgetc(fp)) != EOF) {
        pthread_mutex_lock(&mutex_lock);

        while (count == BUFFER_SIZE)
            pthread_cond_wait(&not_full, &mutex_lock);

        buffer[in] = (char)ch;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex_lock);
    }

    fclose(fp);

    pthread_mutex_lock(&mutex_lock);
    done = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex_lock);

    return NULL;
}

void *consumer(void *arg)
{
    char ch;

    while (1) {
        pthread_mutex_lock(&mutex_lock);

        while (count == 0 && !done)
            pthread_cond_wait(&not_empty, &mutex_lock);

        if (count == 0 && done) {
            pthread_mutex_unlock(&mutex_lock);
            break;
        }

        ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex_lock);

        putchar(ch);
        fflush(stdout);
    }

    return NULL;
}

int main(void)
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    pthread_mutex_init(&mutex_lock, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex_lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
