#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "executor.h"

#define THREADS 4
#define ITEMS 20

// ---------- worker functions ----------

void work_increment(void *arg) {
    int *v = (int*)arg;
    (*v)++;
}

void work_sleep(void *arg) {
    (void)arg;
    usleep(1000);
}

// ---------- main ----------
void work_atomic(void *arg) {
        int *c = (int*)arg;
        __sync_fetch_and_add(c, 1);
    }

int main() {
    printf("[main] starting\n");

    executor *exec = exec_create(THREADS);
    if (!exec) {
        printf("failed to create executor\n");
        return 1;
    }

    // ---- Test 1: basic correctness ----
    printf("\n[TEST 1] basic\n");

    int *vals = calloc(ITEMS, sizeof(int));
    for (int i = 0; i < ITEMS; i++) {
        vals[i] = i;
        exec_add_work(exec, work_increment, &vals[i]);
    }

    exec_wait(exec);

    for (int i = 0; i < ITEMS; i++) {
        if (vals[i] != i + 1) {
            printf("[FAIL] index %d → got %d\n", i, vals[i]);
        }
    }
    printf("[PASS] basic done\n");

    free(vals);

    // ---- Test 2: stress ----
    printf("\n[TEST 2] stress\n");

    int counter = 0;
    int tasks = 50000;

    
    for (int i = 0; i < tasks; i++) {
        exec_add_work(exec, work_atomic, &counter);
    }

    exec_wait(exec);

    printf("[RESULT] counter = %d (expected %d)\n", counter, tasks);

    // ---- Test 3: destroy without wait ----
    printf("\n[TEST 3] destroy without wait\n");

    for (int i = 0; i < 50; i++) {
        exec_add_work(exec, work_sleep, NULL);
    }

    exec_destroy(exec);

    printf("[PASS] destroy completed\n");

    return 0;
}
