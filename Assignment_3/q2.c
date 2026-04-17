#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5

typedef enum {EXPLORING, WAITING, CHARGING, EMERGENCY} state_t;

int battery[N];
state_t state[N];

pthread_mutex_t mutex;
pthread_cond_t cond[N];

int left(int i) { return (i + N - 1) % N; }
int right(int i) { return (i + 1) % N; }

void check(int i) {
    if ((state[i] == WAITING || state[i] == EMERGENCY) &&
        state[left(i)] != CHARGING &&
        state[right(i)] != CHARGING) {

        state[i] = CHARGING;
        pthread_cond_signal(&cond[i]);
    }
}

void take_cables(int i) {
    pthread_mutex_lock(&mutex);

    if (battery[i] < 20) {
        state[i] = EMERGENCY;
        printf("Rover %d ENTERING EMERGENCY (Battery: %d%%)\n", i, battery[i]);
    } else {
        state[i] = WAITING;
        printf("Rover %d waiting (Battery: %d%%)\n", i, battery[i]);
    }

    check(i);

    while (state[i] != CHARGING) {
        pthread_cond_wait(&cond[i], &mutex);
    }

    printf("Rover %d charging (Battery: %d%%)\n", i, battery[i]);

    pthread_mutex_unlock(&mutex);
}

void release_cables(int i) {
    pthread_mutex_lock(&mutex);

    printf("Rover %d finished charging (Battery: %d%%)\n", i, battery[i]);

    state[i] = EXPLORING;

    check(left(i));
    check(right(i));

    pthread_mutex_unlock(&mutex);
}

void* rover(void* arg) {
    int i = *(int*)arg;

    while (1) {
        // EXPLORING
        state[i] = EXPLORING;
        printf("Rover %d exploring (Battery: %d%%)\n", i, battery[i]);
        sleep(rand()%5 + 1);

        battery[i] -= 10;

        if (battery[i] <= 0) {
            printf("Rover %d STRANDED! Battery = 0%%\n", i);
            break;
        }

        // TAKE CABLES
        take_cables(i);

        sleep(rand()%3 + 1);

        battery[i] += 15;
        if (battery[i] > 100) battery[i] = 100;

        release_cables(i);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t tid[N];
    int id[N];

    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < N; i++) {
        pthread_cond_init(&cond[i], NULL);
        battery[i] = 50;
        id[i] = i;
    }

    for (int i = 0; i < N; i++) {
        pthread_create(&tid[i], NULL, rover, &id[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(tid[i], NULL);
    }

    return 0;
}