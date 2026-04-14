#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int sudoku[9][9];
int validity[27];
int error_heatmap[9][9];

typedef struct {
    int index;
    int type;
} validate_arg_t;

void *validate(void *arg) {
    validate_arg_t *a = (validate_arg_t *)arg;
    int idx  = a->index;
    int type = a->type;
    int seen[10] = {0};
    int valid = 1;

    if (type == 0) {
        for (int j = 0; j < 9; j++) {
            int val = sudoku[idx][j];
            if (val < 1 || val > 9) { valid = 0; }
            else { seen[val]++; }
        }
    } else if (type == 1) {
        for (int i = 0; i < 9; i++) {
            int val = sudoku[i][idx];
            if (val < 1 || val > 9) { valid = 0; }
            else { seen[val]++; }
        }
    } else {
        int startRow = (idx / 3) * 3;
        int startCol = (idx % 3) * 3;
        for (int i = startRow; i < startRow + 3; i++) {
            for (int j = startCol; j < startCol + 3; j++) {
                int val = sudoku[i][j];
                if (val < 1 || val > 9) { valid = 0; }
                else { seen[val]++; }
            }
        }
    }

    if (valid) {
        for (int num = 1; num <= 9; num++) {
            if (seen[num] != 1) {
                valid = 0;
                break;
            }
        }
    }

    int vi;
    if (type == 0)       vi = idx;
    else if (type == 1)  vi = 9 + idx;
    else                 vi = 18 + idx;

    validity[vi] = valid;

    free(a);
    pthread_exit(NULL);
}

void *fill_heatmap_row(void *arg) {
    int row = *(int *)arg;

    for (int j = 0; j < 9; j++) {
        int subgrid = (row / 3) * 3 + (j / 3);
        error_heatmap[row][j] = (!validity[row])
                               + (!validity[9 + j])
                               + (!validity[18 + subgrid]);
    }

    free(arg);
    pthread_exit(NULL);
}

int main() {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            scanf("%1d", &sudoku[i][j]);

    pthread_t val_threads[27];

    for (int i = 0; i < 9; i++) {
        for (int type = 0; type < 3; type++) {
            validate_arg_t *arg = malloc(sizeof(validate_arg_t));
            arg->index = i;
            arg->type  = type;
            int tid = type * 9 + i;
            pthread_create(&val_threads[tid], NULL, validate, arg);
        }
    }

    for (int i = 0; i < 27; i++)
        pthread_join(val_threads[i], NULL);

    int all_valid = 1;
    for (int i = 0; i < 27; i++) {
        if (!validity[i]) {
            all_valid = 0;
            break;
        }
    }

    if (all_valid) {
        printf("VALID\n");
        return 0;
    }

    printf("INVALID\n");

    int valid_count = 0;
    for (int i = 0; i < 27; i++)
        valid_count += validity[i];
    printf("Validity Score: %.2f%%\n", (valid_count / 27.0) * 100.0);

    pthread_t hmap_threads[9];
    for (int i = 0; i < 9; i++) {
        int *row = malloc(sizeof(int));
        *row = i;
        pthread_create(&hmap_threads[i], NULL, fill_heatmap_row, row);
    }

    for (int i = 0; i < 9; i++)
        pthread_join(hmap_threads[i], NULL);

    printf("Error Heatmap:\n");
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (j > 0) printf(" ");
            printf("%d", error_heatmap[i][j]);
        }
        printf("\n");
    }

    return 0;
}