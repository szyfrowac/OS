#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_N 10

struct thread_params {
    int thread_id;
    int start_index;
    int segment_size;
};

struct thread_result {
    int thread_id;
    double average;
    int minimum;
    int maximum;
};

int global_array[1 << MAX_N];
int sorted_array[1 << MAX_N];
int array_size;

static int cmp_int(const void *a, const void *b) {
    return *(int *)a - *(int *)b;
}

void *worker(void *arg) {
    struct thread_params *p = (struct thread_params *)arg;
    struct thread_result *res = malloc(sizeof(struct thread_result));

    qsort(global_array + p->start_index, p->segment_size, sizeof(int), cmp_int);

    int min = global_array[p->start_index];
    int max = global_array[p->start_index];
    double sum = 0.0;

    for (int i = p->start_index; i < p->start_index + p->segment_size; i++) {
        int v = global_array[i];
        sum += v;
        if (v < min) min = v;
        if (v > max) max = v;
    }

    res->thread_id = p->thread_id;
    res->average   = sum / p->segment_size;
    res->minimum   = min;
    res->maximum   = max;

    pthread_exit((void *)res);
}

typedef struct {
    int value, seg_id, index, end;
} HeapNode;

void push_down(HeapNode *heap, int size, int i) {
    while (1) {
        int s = i, l = 2*i+1, r = 2*i+2;
        if (l < size && heap[l].value < heap[s].value) s = l;
        if (r < size && heap[r].value < heap[s].value) s = r;
        if (s == i) break;
        HeapNode tmp = heap[i]; heap[i] = heap[s]; heap[s] = tmp;
        i = s;
    }
}

void kway_merge(int m, int seg_size) {
    HeapNode *heap = malloc(m * sizeof(HeapNode));
    int hs = m;

    for (int t = 0; t < m; t++) {
        int start = t * seg_size;
        heap[t] = (HeapNode){ global_array[start], t, start, start + seg_size };
    }
    for (int i = hs/2 - 1; i >= 0; i--) push_down(heap, hs, i);

    int out = 0;
    while (hs > 0) {
        sorted_array[out++] = heap[0].value;
        heap[0].index++;
        if (heap[0].index < heap[0].end)
            heap[0].value = global_array[heap[0].index];
        else
            heap[0] = heap[--hs];
        push_down(heap, hs, 0);
    }
    free(heap);
}

int main(void) {
    int n, m;

    printf("Enter n (2 <= n <= 10): ");
    scanf("%d", &n);
    array_size = 1 << n;

    srand(42);
    for (int i = 0; i < array_size; i++)
        global_array[i] = rand() % 100;

    printf("Enter the number of segments (m): ");
    scanf("%d", &m);

    int seg_size = array_size / m;

    printf("\nInitial Unsorted Array:");
    for (int i = 0; i < array_size; i++) printf(" %d", global_array[i]);
    printf("\n");

    pthread_t *tids = malloc(m * sizeof(pthread_t));
    struct thread_params *params = malloc(m * sizeof(struct thread_params));

    for (int t = 0; t < m; t++) {
        params[t] = (struct thread_params){ t, t * seg_size, seg_size };
        pthread_create(&tids[t], NULL, worker, &params[t]);
    }

    struct thread_result **results = malloc(m * sizeof(struct thread_result *));
    for (int t = 0; t < m; t++) {
        void *ret;
        pthread_join(tids[t], &ret);
        results[t] = (struct thread_result *)ret;
    }

    printf("\n--- Worker Thread Stats ---\n");
    for (int t = 0; t < m; t++)
        printf("Thread %d: Avg: %.2f | Min: %d | Max: %d\n",
               results[t]->thread_id, results[t]->average,
               results[t]->minimum, results[t]->maximum);

    double global_sum = 0.0;
    int global_min = results[0]->minimum;
    int global_max = results[0]->maximum;

    for (int t = 0; t < m; t++) {
        global_sum += results[t]->average * seg_size;
        if (results[t]->minimum < global_min) global_min = results[t]->minimum;
        if (results[t]->maximum > global_max) global_max = results[t]->maximum;
    }

    printf("\n--- Global Statistics ---\n");
    printf("Global Average: %.2f\n", global_sum / array_size);
    printf("Global Minimum: %d\n", global_min);
    printf("Global Maximum: %d\n", global_max);

    kway_merge(m, seg_size);

    printf("\nFinal Sorted Array:");
    for (int i = 0; i < array_size; i++) printf(" %d", sorted_array[i]);
    printf("\n");

    for (int t = 0; t < m; t++) free(results[t]);
    free(results);
    free(tids);
    free(params);

    return 0;
}
