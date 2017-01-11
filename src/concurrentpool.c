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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "concurrent.h"
#include "concurrent_util.h"
#include "concurrentqueue.h"
#include "concurrentpool.h"
#include "slinkedlist.h"
#include "log.h"
#include "stdlib_util.h"

#define TERMINATE_JOB_LOAD "terminate-job-load"
#define JOB_NAME_PREFIX "job-"
#define JOB_NAME_PREFIX_LEN 4

/* Structure to store job information. */
typedef struct _job_info {
    char * name;
    concurent_job ccj;
    void *ccj_data;
    size_t *requested_thread_id;
} job_info;

/* Structure to hold information for a thread in the pool. */
typedef struct _thread_pool_item_struct {
    size_t *current_thread_id; /* thread identification */
    concurrentqueue *job_queue; /* job queue */
    concurrentpool *cpool; /* reference to the concurrentpool it belongs */
} thread_pool_item;

/**
 * Dummy job to indicate termination of a concurrent pool.
 *
 * @param job_data pointer to data that will be passed to the function.
 */
static void
dummy_concurent_job(void *job_data);

/* Dummy job information to indicate termination of a concurrent pool */
static job_info dummy_job_info = {TERMINATE_JOB_LOAD, dummy_concurent_job, NULL};
/* Internal job index */
static int internal_id = 0;

/**
 * Dummy work to indicate termination of a concurrent pool.
 *
 * @param job_data pointer to data that will be passed to the function.
 */
static void
dummy_concurent_job(void *job_data) {
    /* nothing to do */
}

/**
 * Test whether the provided concurrentpool structure is initialized or not.
 *
 * @param cpool concurrentpool structure.
 *
 * @return Non-zero if concurrentpool structure is initialized, zero otherwise.
 */
static int
concurrentpool_initialized(concurrentpool * cpool) {
    int result = 0;

    if (cpool != NULL
    && cpool->name != NULL
    && cpool->thread_pool != NULL
    && cpool->job_queue != NULL
    && cpool->mutex != NULL
    && cpool->shutdown_signal != NULL) {
        result++;
    }

    return result;
}

/**
 * Function that executes jobs from the concurrent pool.
 *
 * @param job_data pointer to data that will be passed to the function.
 * @param thread_id identity of the thread executor responsible for executing
 *        the call-back.
 */
static void *
thread_function_cb(void *job_data, size_t * thread_id) {
    int send_term_signal = 0;
    thread_pool_item * tpi = (thread_pool_item *)job_data;

    concurrent_lock_mutex(tpi->cpool->mutex); /* lock concurrent pool mutex */
    tpi->cpool->active_thread_count++;
    concurrent_unlock_mutex(tpi->cpool->mutex); /* un-lock concurrent pool mutex */

    log_debug_format("pool %s thread %u started!",
        tpi->cpool->name, *(tpi->current_thread_id));

    while(1) {
        job_info *ji_p = (job_info *)concurrentqueue_get_item(tpi->job_queue);
        log_debug_format("pool %s thread %u got %s job!",
            tpi->cpool->name, *(tpi->current_thread_id), ji_p->name);
        /* check if the job load has been finished */
        if (strcmp(ji_p->name, TERMINATE_JOB_LOAD) == 0) {
            break;
        } else {
            /* perform job */
            ji_p->ccj(ji_p->ccj_data);
            /* free job resources */
            SAFE_FREE(ji_p->name);
            SAFE_FREE(ji_p);
        }
    }
    concurrent_lock_mutex(tpi->cpool->mutex); /* lock concurrent pool mutex */
    if (tpi->cpool->active_thread_count > 1) {
        tpi->cpool->active_thread_count--; /* decrease by one active thread count of the pool */
        concurrentqueue_put_item(tpi->job_queue, (void *)&dummy_job_info); /* re-insert */
    } else {
        tpi->cpool->active_thread_count--; /* last active thread of the pool */
        send_term_signal++; /* mark termination signal indication */
    }
    concurrent_unlock_mutex(tpi->cpool->mutex); /* un-lock concurrent pool mutex */

    log_debug_format("pool %s thread %u finished!",
        tpi->cpool->name, *(tpi->current_thread_id));

    if (send_term_signal) {
        log_debug_format("pool %s thread %u sends termination signal !",
            tpi->cpool->name, *(tpi->current_thread_id));
        concurrent_util_send_signal(tpi->cpool->shutdown_signal); /* send shutdown_signal */
    }

    return NULL;
}

/**
 * Method to create a new thread of a concurrent pool.
 *
 * @param cpool reference to the concurrent pool
 *        in which the thread belongs.
 *
 * @return reference to the created thread object.
 */
static thread_pool_item *
create_thread_pool_item(concurrentpool * cpool) {
    thread_pool_item * tpi =
        (thread_pool_item *)SAFE_MALLOC(sizeof(thread_pool_item));

    tpi->cpool = cpool;
    tpi->job_queue = (concurrentqueue *)cpool->job_queue;
    tpi->current_thread_id =
        concurrent_install_cb(100, thread_function_cb, tpi);

    return tpi;
}

/**
 * Method to destroy a thread of a concurrent pool.
 *
 * @param data pointer to data that will be passed to the function.
 */
static void
destroy_thread_pool_item(void * data) {
    thread_pool_item * tpi = (thread_pool_item *)data;

    concurrent_uninstall_cb(tpi->current_thread_id);
    SAFE_FREE(tpi);
}

/**
 * Method to clear the contents of the provided job queue.
 *
 * @param job_queue concurrentqueue structure.
 */
static void
clear_job_queue(concurrentqueue * job_queue) {
    if (!concurrentqueue_empty(job_queue)) {
        while(!concurrentqueue_empty(job_queue)) {
            job_info *ji_p = (job_info *)concurrentqueue_get_item(job_queue);
            SAFE_FREE(ji_p->name);
            SAFE_FREE(ji_p);
        }
    }
}

/**
 * Allocates memory for the concurrent pool structure.
 *
 * @return new concurrent pool structure reference.
 */
extern concurrentpool *
concurrentpool_create(void) {
    concurrentpool * cp =
        (concurrentpool *)SAFE_MALLOC(sizeof(concurrentpool));

    return cp;
}

/**
 * Initialize the concurrent pool.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 * @param capacity number of threads in the concurrent pool.
 * @param name name of the concurrent pool.
 */
extern void
concurrentpool_init(concurrentpool *cpool, int capacity, char *name) {
    /* use concurrent pool field thread_pool as a slinkedlist */
    if (cpool != NULL && (capacity > 0) && name != NULL) {
        int i;
        concurrent_signal *shutdown_signal; /* signal that all threads are finished */
        /* initialize the concurrent pool */
        cpool->name = strdup(name);
        cpool->thread_pool = (void *)slinkedlist_create();
        cpool->active_thread_count = 0;
        cpool->job_queue = (concurrentqueue *)SAFE_MALLOC(sizeof(concurrentqueue));
        concurrentqueue_init(cpool->job_queue, 17);
        cpool->mutex = concurrent_create_mutex();
        shutdown_signal = (concurrent_signal *)SAFE_MALLOC(sizeof(concurrent_signal));
        concurrent_util_init_signal(shutdown_signal);
        cpool->shutdown_signal = (void *)shutdown_signal;
        /* initialize threads and add them in the thread pool */
        for (i=0; i< capacity; i++) {
            slinkedlist_append(cpool->thread_pool, create_thread_pool_item(cpool));
        }
    }
}

/**
 * Submit job for the concurrent pool.
 *
 * @param cworker concurrent pool structure reference (not NULL).
 * @param job job (function) to be performed (called) by the concurrent pool.
 * @param job_data pointer to data that will be passed to the job(function).
 */
extern void
concurrentpool_submit_job(concurrentpool *cpool, concurent_job job, void *job_data) {
    if (concurrentpool_initialized(cpool)) {
        size_t len;
        job_info * ji_p = (job_info *)SAFE_MALLOC(sizeof(job_info));

        internal_id++;
        len = JOB_NAME_PREFIX_LEN + 64;
        ji_p->name = (char *)SAFE_MALLOC(len * sizeof(char));
        snprintf(ji_p->name, len, "%s%d", JOB_NAME_PREFIX, internal_id);
        ji_p->ccj = job;
        ji_p->ccj_data = job_data;
        concurrentqueue_put_item((concurrentqueue *)cpool->job_queue, (void *)ji_p);
    }
}

/**
 * Free the allocated memory for concurrent pool contents.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 */
extern void
concurrentpool_free(concurrentpool *cpool) {
    if (concurrentpool_initialized(cpool)) {
        /* clear job queue */
        clear_job_queue(cpool->job_queue);

        /* insert dummy_job_info to initialize shutdown */
        concurrentqueue_put_item(cpool->job_queue, (void *)&dummy_job_info);

        /* wait until all threads finished */
        concurrent_util_wait_signal(cpool->shutdown_signal);
        log_debug_format("pool %s threads finshed!", cpool->name);

        /* free resources */
        slinkedlist_free(cpool->thread_pool, destroy_thread_pool_item);
        SAFE_FREE(cpool->thread_pool);

        concurrent_util_destroy_signal(cpool->shutdown_signal);
        SAFE_FREE(cpool->shutdown_signal);

        concurrentqueue_destroy(cpool->job_queue);
        SAFE_FREE(cpool->job_queue);

        cpool->active_thread_count = 0;
        concurrent_destroy_mutex(cpool->mutex);

        free(cpool->name);
    }
}

/**
 * Free the allocated memory for concurrent pool structure reference.
 *
 * @param cpool concurrent pool structure reference (not NULL).
 */
extern void
concurrentpool_destroy(concurrentpool *cpool) {
    concurrentpool_free(cpool);
    if (cpool != NULL) {
        SAFE_FREE(cpool);
    }
}
