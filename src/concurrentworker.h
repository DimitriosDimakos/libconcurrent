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
 * Implementation of a thread worker.
 */

#ifndef CONCURRENTWORKER_H_
#define CONCURRENTWORKER_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * The function type that should be supplied as a call-back to the concurrent worker.
 *
 * @param work_data pointer to data that will be passed to the function.
 */
typedef void (*concurent_work) (void *work_data);

/* Structure to hold concurrent worker information. */
typedef struct _concurrentworker_struct {
    char *name; /* name of concurrent worker */
    void *work_load; /* work information to be performed by the concurrent worker */
} concurrentworker;

/**
 * Allocates memory for the concurrent worker structure.
 *
 * @return new concurrent worker structure reference.
 */
extern concurrentworker *
concurrentworker_create(void);

/**
 * Initialize the concurrent worker.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 * @param name name of the concurrent worker.
 */
extern void
concurrentworker_init(concurrentworker *cworker, char *name);

/**
 * Submit work for the concurrent worker.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 * @param work work (function) to be performed (called) by the concurrent worker.
 * @param work_data pointer to data that will be passed to the work(function).
 */
extern void
concurrentworker_submit_work(
    concurrentworker *cworker,
    concurent_work work,
    void *work_data);

/**
 * Free the allocated memory for concurrent worker contents.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 */
extern void
concurrentworker_free(concurrentworker *cworker);

/**
 * Free the allocated memory for concurrent worker structure reference.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 */
extern void
concurrentworker_destroy(concurrentworker *cworker);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENTWORKER_H_ */
