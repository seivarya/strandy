#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "executor.h"

#define threads 4
#define ITEMS 200
#define TASKS 100000

void work_trace(void *arg);
void work_increment(void *arg);
void work_heavy(void *arg);
long now_ms(void);
void work_heavy_count(void *arg);
void test_basic(executor *exec);
void work_sleep(void *arg);
void test_trace(executor *exec);
void test_correctness(executor *exec); 
void test_stress(executor *exec);
void test_stress_seq(void); 
void test_destroy(executor *exec);
void sleep_ms(int ms);

// utils 

long now_ms(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void sleep_ms(int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

//  work functions 

void work_trace(void *arg) {
	int id = *(int*)arg;
	printf("[work_trace]: start id=%d thread=%lu time=%ld\n", id, (unsigned long)pthread_self(), now_ms());


	sleep_ms(rand() % 5); // 0–4 ms

	printf("[work_trace]: end   id=%d thread=%lu time=%ld\n", id, (unsigned long)pthread_self(), now_ms());


}

void work_increment(void *arg) {
	int *v = (int*)arg;
	(*v)++;
}

// cpu heavy work (no shared state)
void work_heavy(void *arg) {
	double x = 0.0;
	int n = *(int*)arg;

	for (int i = 0; i < 20000; i++) {
		x += (i * 0.5) / (n + 1);
		x = x * 1.0000001;
	}

	if (x < 0) printf("impossible\n");


}

// cpu heavy ahd correctness counter

void work_heavy_count(void *arg) {
	int *counter = (int*)arg;

	double x = 0.0;
	for (int i = 0; i < 20000; i++) {
		x += i * 0.5;
	}

	__sync_fetch_and_add(counter, 1);


}

void work_sleep(void *arg) {
	(void)arg;
	sleep_ms(5); // 5 ms
}

//  tests 
//
void test_basic(executor *exec) {
	int *vals = calloc(ITEMS, sizeof(int));


	for (int i = 0; i < ITEMS; i++) {
		vals[i] = i;
		exec_add_work(exec, work_increment, &vals[i]);
	}

	exec_wait(exec);

	int fail = 0;
	for (int i = 0; i < ITEMS; i++) {
		if (vals[i] != i + 1) {
			printf("[test_basic]: fail index=%d got=%d\n", i, vals[i]);
			fail = 1;
		}
	}

	if (!fail) printf("[test_basic]: pass\n");

	free(vals);


}

void test_trace(executor *exec) {
	int ids[ITEMS];


	for (int i = 0; i < ITEMS; i++) {
		ids[i] = i;
		exec_add_work(exec, work_trace, &ids[i]);
	}

	exec_wait(exec);


}

void test_correctness(executor *exec) {
	printf("\n[test_correctness]: start\n");


	int counter = 0;

	for (int i = 0; i < TASKS; i++) {
		exec_add_work(exec, work_heavy_count, &counter);
	}

	exec_wait(exec);

	printf("[test_correctness]: result=%d expected=%d\n", counter, TASKS);

	if (counter == TASKS)
		printf("[test_correctness]: pass\n");
	else
		printf("[test_correctness]: fail\n");


}

// performance test (parallel)
void test_stress(executor *exec) {
	printf("\n[test_stress]: start\n");


	int *inputs = malloc(sizeof(int) * TASKS);

	for (int i = 0; i < TASKS; i++) {
		inputs[i] = i;
	}

	long t1 = now_ms();

	for (int i = 0; i < TASKS; i++) {
		exec_add_work(exec, work_heavy, &inputs[i]);
	}

	exec_wait(exec);

	long t2 = now_ms();

	printf("[test_stress]: time=%ldms TASKS=%d\n",
			(t2 - t1), TASKS);

	free(inputs);
	printf("[test_stress]: pass\n");


}

// sequential baseline
void test_stress_seq(void) {
	printf("\n[test_stress_seq]: start\n");


	int *inputs = malloc(sizeof(int) * TASKS);

	for (int i = 0; i < TASKS; i++) {
		inputs[i] = i;
	}

	long t1 = now_ms();

	for (int i = 0; i < TASKS; i++) {
		work_heavy(&inputs[i]);
	}

	long t2 = now_ms();

	printf("[test_stress_seq]: time=%ldms TASKS=%d\n",
			(t2 - t1), TASKS);

	free(inputs);
	printf("[test_stress_seq]: pass\n");


}

void test_destroy(executor *exec) {
	printf("\n[test_destroy]: start\n");


	for (int i = 0; i < 50; i++) {
		exec_add_work(exec, work_sleep, NULL);
	}

	printf("[test_destroy]: calling destroy (no wait)\n");
	exec_destroy(exec);

	printf("[test_destroy]: pass\n");


}

//  main func
int main(void) {
	srand((unsigned int)time(NULL));


	printf("[main]: start\n");

	executor *exec = exec_create(threads);
	if (!exec) {
		printf("[main]: fail create\n");
		return 1;
	}

	test_basic(exec);
	test_trace(exec);

	test_correctness(exec);

	test_stress_seq();
	test_stress(exec);

	test_destroy(exec);

	printf("[main]: end\n");
	return 0;

}
