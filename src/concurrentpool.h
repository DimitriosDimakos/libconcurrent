/*
 * Copyright 2016 Dimitrios Dimakos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implementation of a thread pool.
 */

#ifndef CONCURRENTPOOL_H_
#define CONCURRENTPOOL_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * The function type that should be supplied as a call-back to the concurrent pool.
 *
 * @param job_data pointer to data that will be passed to the function.
 */
typedef void (*concurent_job) (void *job_data);

/* Structure to hold concurrent pool information. */
typedef struct _concurrentpool_struct {
    char *name; /* name of concurrent pool */
    void *thread_pool; /* thread pool */
    int active_thread_count; /* number of active threads */
    void *job_queue; /* thread pool */
    size_t * mutex; /* mutex for the concurrent pool */
    void * shutdown_signal; /* internal signal used to shutdown the pool */
} concurrentpool;

/**
 * Allocates memory for the concurrent pool structure.
 *
 * @return new concurrent pool structure reference.
 */
extern concurrentpool *
concurrentpool_create(void);

/**
 * Initialize the concurrent pool.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 * @param capacity number of threads in the concurrent pool.
 * @param name name of the concurrent pool.
 */
extern void
concurrentpool_init(concurrentpool *cpool, int capacity, char *name);

/**
 * Submit a job for the concurrent pool.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 * @param job job (function) to be performed (called) by the concurrent pool.
 * @param job_data pointer to data that will be passed to the job(function).
 */
extern void
concurrentpool_submit_job(concurrentpool *cpool, concurent_job job, void *job_data);

/**
 * Free the allocated memory for concurrent pool contents.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 */
extern void
concurrentpool_free(concurrentpool *cpool);

/**
 * Free the allocated memory for concurrent pool structure reference.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 */
extern void
concurrentpool_destroy(concurrentpool *cpool);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENTPOOL_H_ */
