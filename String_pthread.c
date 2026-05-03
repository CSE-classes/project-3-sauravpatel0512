#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define NUM_THREADS 4
#define MAX 1024

void *sub_string(void *);
int readf(FILE *fp);

int total = 0;
int nlocal, n1, n2;
char *s1, *s2;
FILE *fp;
pthread_mutex_t total_lock;

int main(int argc, char *argv[])
{
    int i, rc;
    pthread_t threads[NUM_THREADS];

    pthread_mutex_init(&total_lock, NULL);

    if (readf(fp) < 0) {
        printf("ERROR: failed to read input file.\n");
        return -1;
    }

    for (i = 0; i < NUM_THREADS; i++) {
        rc = pthread_create(&threads[i], NULL, sub_string, (void *)(intptr_t)i);
        if (rc) {
            printf("ERROR: return error from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("ERROR: return error from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    printf("the occurences of s2 in s1 is %d\n", total);

    pthread_mutex_destroy(&total_lock);
    free(s1);
    free(s2);

    return 0;
}

int readf(FILE *fp)
{
    if ((fp = fopen("strings.txt", "r")) == NULL) {
        printf("ERROR: can't open strings.txt!\n");
        return -1;
    }

    s1 = (char *)malloc(sizeof(char) * MAX);
    if (s1 == NULL) {
        printf("ERROR: Out of memory!\n");
        fclose(fp);
        return -1;
    }

    s2 = (char *)malloc(sizeof(char) * MAX);
    if (s2 == NULL) {
        printf("ERROR: Out of memory\n");
        free(s1);
        fclose(fp);
        return -1;
    }

    if (fgets(s1, MAX, fp) == NULL || fgets(s2, MAX, fp) == NULL) {
        printf("ERROR: failed to read strings.\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    s1[strcspn(s1, "\n")] = '\0';
    s2[strcspn(s2, "\n")] = '\0';

    n1 = strlen(s1);
    n2 = strlen(s2);

    if (s1 == NULL || s2 == NULL || n1 < n2 || n2 == 0)
        return -1;

    nlocal = n1 / NUM_THREADS;

    return 0;
}

void *sub_string(void *threadid)
{
    int tid = (int)(intptr_t)threadid;
    int start = tid * nlocal;
    int end = start + nlocal - 1;
    int i, j;
    int local_count = 0;

    for (i = start; i <= end && i <= n1 - n2; i++) {
        int match = 1;

        for (j = 0; j < n2; j++) {
            if (s1[i + j] != s2[j]) {
                match = 0;
                break;
            }
        }

        if (match)
            local_count++;
    }

    pthread_mutex_lock(&total_lock);
    total += local_count;
    pthread_mutex_unlock(&total_lock);

    return NULL;
}
