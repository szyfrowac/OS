#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_READERS 4
#define NUM_WRITERS 2
#define ITERATIONS 5

static int shared_resource = 0;
static int read_count = 0;
static int write_count = 0;

/* Writer-preference synchronization semaphores */
static sem_t resource;
static sem_t read_try;
static sem_t rmutex;
static sem_t wmutex;

typedef struct ThreadInfo {
	int id;
	unsigned int seed;
} ThreadInfo;

static void random_sleep(unsigned int *seed, int max_microseconds) {
	int duration = rand_r(seed) % (max_microseconds + 1);
	usleep((useconds_t)duration);
}

static void *reader(void *arg) {
	ThreadInfo *info = (ThreadInfo *)arg;

	for (int i = 0; i < ITERATIONS; i++) {
		int local_readers;

		sem_wait(&read_try);
		sem_wait(&rmutex);
		read_count++;
		local_readers = read_count;
		if (read_count == 1) {
			sem_wait(&resource);
		}
		sem_post(&rmutex);
		sem_post(&read_try);

		printf("[Reader %d] Reading resource: %d (Active Readers: %d)\n",
			   info->id,
			   shared_resource,
			   local_readers);
		fflush(stdout);

		random_sleep(&info->seed, 1000000);

		sem_wait(&rmutex);
		read_count--;
		if (read_count == 0) {
			sem_post(&resource);
		}
		sem_post(&rmutex);

		random_sleep(&info->seed, 3000000);
	}

	return NULL;
}

static void *writer(void *arg) {
	ThreadInfo *info = (ThreadInfo *)arg;

	for (int i = 0; i < ITERATIONS; i++) {
		sem_wait(&wmutex);
		write_count++;
		if (write_count == 1) {
			sem_wait(&read_try);
		}
		sem_post(&wmutex);

		sem_wait(&resource);
		shared_resource += 5;
		printf("[Writer %d] Writing to resource: %d\n", info->id, shared_resource);
		fflush(stdout);
		random_sleep(&info->seed, 1500000);
		sem_post(&resource);

		sem_wait(&wmutex);
		write_count--;
		if (write_count == 0) {
			sem_post(&read_try);
		}
		sem_post(&wmutex);

		random_sleep(&info->seed, 5000000);
	}

	return NULL;
}

int main(void) {
	pthread_t readers[NUM_READERS];
	pthread_t writers[NUM_WRITERS];
	ThreadInfo reader_info[NUM_READERS];
	ThreadInfo writer_info[NUM_WRITERS];

	sem_init(&resource, 0, 1);
	sem_init(&read_try, 0, 1);
	sem_init(&rmutex, 0, 1);
	sem_init(&wmutex, 0, 1);

	srand((unsigned int)time(NULL));

	for (int i = 0; i < NUM_READERS; i++) {
		reader_info[i].id = i + 1;
		reader_info[i].seed = (unsigned int)(rand() ^ (i + 17));
		pthread_create(&readers[i], NULL, reader, &reader_info[i]);
	}

	for (int i = 0; i < NUM_WRITERS; i++) {
		writer_info[i].id = i + 1;
		writer_info[i].seed = (unsigned int)(rand() ^ (i + 101));
		pthread_create(&writers[i], NULL, writer, &writer_info[i]);
	}

	for (int i = 0; i < NUM_READERS; i++) {
		pthread_join(readers[i], NULL);
	}

	for (int i = 0; i < NUM_WRITERS; i++) {
		pthread_join(writers[i], NULL);
	}

	sem_destroy(&resource);
	sem_destroy(&read_try);
	sem_destroy(&rmutex);
	sem_destroy(&wmutex);

	return 0;
}
