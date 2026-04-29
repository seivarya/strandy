# strandy

a lightweight, strictly **posix-compliant** thread pool written in raw c. designed to handle concurrent task execution without the bloat of heavy abstractions. **strandy** gets straight to the point: enqueue function pointers, wake up worker threads, and execute jobs asynchronously.

## why strandy?
- **zero dependencies**: relies exclusively on standard `<pthread.h>`.
- **event-driven workers**: threads sleep on condition variables (`pthread_cond_t`) until work is explicitly signaled, eliminating spin-locking cpu waste.
- **dynamic work queue**: thread-safe linked-list based fifo queue dynamically scales with pending workload.

> [!IMPORTANT]
> strandy leaves lifecycle management in your hands. ensure you call `exec_wait()` and `exec_destroy()` to safely reap threads and prevent resource leaks.

## table of contents
- [architecture & concurrency](#architecture--concurrency)
- [project structure](#project-structure)
- [core api](#core-api)

***

## architecture & concurrency
strandy is constructed around two primary primitives: a synchronized **fifo queue** and the **executor** thread pool.

during initialization via `exec_create()`, the executor allocates and spawns a predefined pool of `pthread_t` workers. each thread immediately enters a blocked state (`pthread_cond_wait`), waiting on the `non_empty` condition variable. 

when a job is submitted via `exec_add_work()`:
1. the executor acquires the shared `pthread_mutex_t`.
2. the job (a function pointer and a `void*` argument context) is pushed to the tail of the queue.
3. the `pending_count` is incremented.
4. a `pthread_cond_signal()` is emitted to wake a single idle worker.

workers fetch from the queue head, process the payload, and update the `working_count` state, ultimately signaling back to `exec_wait()` when the queue drains completely.

> [!NOTE]
> concurrency guarantees are maintained by strict mutex locking boundaries around queue mutations. avoid submitting blocking io operations into the pool unless sized accordingly, as it will exhaust the available workers.

## project structure
```text
strandy/
├── include/
│   ├── executor.h       # thread pool logic, mutexes, and condvars
│   └── queue.h          # fifo task queue interface
├── src/
│   ├── executor.c       # core synchronization and worker routines
│   └── queue.c          # queue memory management
├── tests/               # validation and concurrent execution tests
├── makefile             # build instructions
└── readme.md
```

## core api

| interface | description |
|-----------|-------------|
| `exec_create(size_t num)` | allocates pool state, initializes mutexes/condvars, and spawns `num` worker threads. |
| `exec_add_work(executor*, exec_func, void*)` | safely enqueues a `void (*)(void*)` callback and signals a sleeping thread. |
| `exec_wait(executor*)` | blocks the caller until `pending_count` and `working_count` reach zero. |
| `exec_destroy(executor*)` | triggers the `stop` flag, broadcasts a wake-up to all threads, and reaps them via `pthread_join()`. |
