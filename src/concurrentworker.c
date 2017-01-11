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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "concurrent.h"
#include "concurrent_util.h"
#include "concurrentqueue.h"
#include "concurrentworker.h"
#include "log.h"
#include "stdlib_util.h"

#define TERMINATE_WORK_LOAD "terminate-work-load"
#define WORK_NAME_PREFIX "work-"
#define WORK_NAME_PREFIX_LEN 5

/* Structure to store work information. */
typedef struct _work_info {
    char * name;
    concurent_work ccw;
    void *ccw_data;
} work_info;

/* Structure to hold work load information for a concurrent worker. */
typedef struct _work_load_struct {
    concurrentqueue * work_queue; /* work queue */
    concurrent_signal *shutdown_signal; /* signal that the thread function finished */
    size_t *thread_id; /* thread identification */
} work_load;

/**
 * Dummy work to indicate termination of a concurrent worker.
 *
 * @param work_data pointer to data that will be passed to the function.
 */
static void
dummy_concurent_work(void *work_data);

/* Dummy work information to indicate termination of a concurrent worker */
static work_info dummy_work_info = {TERMINATE_WORK_LOAD, dummy_concurent_work, NULL};

/* Internal concurrent worker index */
static int internal_id = 0;

/**
 * Dummy work to indicate termination of a concurrent worker.
 *
 * @param client_data pointer to data that will be passed to the function.
 */
static void
dummy_concurent_work(void *client_data) {
    /* nothing to implement */
}

/**
 * Method to clear the contents of the provided work concurrent queue.
 *
 * @param work_queue concurrentqueue structure.
 */
static void
clear_work_queue(concurrentqueue * work_queue) {
    if (!concurrentqueue_empty(work_queue)) {
        while(!concurrentqueue_empty(work_queue)) {
            work_info *wi_p = (work_info *)concurrentqueue_get_item(work_queue);
            SAFE_FREE(wi_p->name);
            SAFE_FREE(wi_p);
        }
    }
}

/**
 * Test whether the provided concurrentworker structure is initialized or not.
 *
 * @param cworker concurrentworker structure.
 *
 * @return Non-zero if concurrentworker structure is initialized, zero otherwise.
 */
static int
concurrentworker_initialized(concurrentworker *cworker) {
    int result = 0;

    if (cworker != NULL && cworker->name > 0 && cworker->work_load != NULL) {
        result++;
    }

    return result;
}

/**
 * Function that executes the work load of a concurrent worker.
 *
 * @param work_data pointer to data that will be passed to the function.
 * @param thread_id identity of the thread executor responsible for handling the call-back.
 */
static void *
thread_function_cb(void *work_data, size_t * thread_id) {
    concurrentworker * cw = (concurrentworker *)work_data;
    work_load * wl = (work_load *)cw->work_load;

    log_debug_format("worker %s thread %u started!", cw->name, *(wl->thread_id));

    while(1) {
        work_info *wi_p = (work_info *)concurrentqueue_get_item(wl->work_queue);
        log_debug_format("worker %s got %s work!", cw->name, wi_p->name);
        /* check if the work load has been finished */
        if (strcmp(wi_p->name, TERMINATE_WORK_LOAD) == 0) {
            break;
        } else {
            /* perform work */
            wi_p->ccw(wi_p->ccw_data);
            /* free work resources */
            SAFE_FREE(wi_p->name);
            SAFE_FREE(wi_p);
        }
    }

    log_debug_format("worker %s thread %u finished!", cw->name, *(wl->thread_id));
    concurrent_util_send_signal(wl->shutdown_signal); /* send shutdown signal */

    return NULL;
}

/**
 * Allocates memory for the concurrent worker structure.
 *
 * @return new concurrent worker structure reference.
 */
extern concurrentworker *
concurrentworker_create(void) {
    concurrentworker * cp =
        (concurrentworker *)SAFE_MALLOC(sizeof(concurrentworker));

    return cp;
}

/**
 * Initialize the concurrent worker.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 * @param name name of the concurrent worker.
 */
extern void
concurrentworker_init(concurrentworker *cworker, char *name) {
    /* use concurrent worker field thread_worker as a slinkedlist */
    if (cworker != NULL && name != NULL) {
        work_load * wl = (work_load *)SAFE_MALLOC(sizeof(work_load));
        cworker->name = strdup(name);
        cworker->work_load = (void *)wl;
        wl->work_queue = (concurrentqueue *)SAFE_MALLOC(sizeof(concurrentqueue));
        concurrentqueue_init(wl->work_queue, 7);
        wl->shutdown_signal = (concurrent_signal *)SAFE_MALLOC(sizeof(concurrent_signal));
        concurrent_util_init_signal(wl->shutdown_signal);
        wl->thread_id = concurrent_install_cb(100, thread_function_cb, cworker);
    }
}

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
    void *work_data) {

    if (concurrentworker_initialized(cworker)) {
        size_t len;
        work_load * wl;
        work_info * wi_p = (work_info *)SAFE_MALLOC(sizeof(work_info));
        internal_id++;
        len = WORK_NAME_PREFIX_LEN + 64;
        wi_p->name = (char *)SAFE_MALLOC(len * sizeof(char));
        snprintf(wi_p->name, len, "%s%d", WORK_NAME_PREFIX, internal_id);
        wi_p->ccw = work;
        wi_p->ccw_data = work_data;
        wl = (work_load *)cworker->work_load;
        concurrentqueue_put_item(wl->work_queue, (void *)wi_p);
    }
}

/**
 * Free the allocated memory for concurrent worker contents.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 */
extern void
concurrentworker_free(concurrentworker *cworker) {
    if (concurrentworker_initialized(cworker)) {
        work_load * wl = (work_load *)cworker->work_load;

        /* clear work queue */
        clear_work_queue(wl->work_queue);

        /* insert dummy_work_info to initialize shutdown */
        concurrentqueue_put_item(wl->work_queue, (void *)&dummy_work_info);
        /* wait until thread finished */
        concurrent_util_wait_signal(wl->shutdown_signal);

        /* free resources */
        concurrent_util_destroy_signal(wl->shutdown_signal);
        SAFE_FREE(wl->shutdown_signal);

        concurrent_uninstall_cb(wl->thread_id);

        concurrentqueue_destroy(wl->work_queue);

        SAFE_FREE(wl->work_queue);
        SAFE_FREE(wl);
        free(cworker->name);
    }
}

/**
 * Free the allocated memory for concurrent worker structure reference.
 *
 * @param cworker concurrent worker structure reference (not NULL).
 */
extern void
concurrentworker_destroy(concurrentworker *cworker) {
    concurrentworker_free(cworker);
    if (cworker != NULL) {
        SAFE_FREE(cworker);
    }
}
