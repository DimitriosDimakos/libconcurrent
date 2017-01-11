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
 * Implementation of a high level thread library.
 */

#include <stdio.h>

#include "concurrent.h"
#include "concurrentfactory.h"
#include "hashmap.h"
#include "stdlib_util.h"
#include "time_util.h"
#include "log.h"

/* structure to store information of concurrent call-backs. */
typedef struct _concurrent_cb_info{
    long unsigned int interval;
    concurent_cb cb;
    void *cb_data;
    int periodic_cb;
    int strict_timing_enabled;
    size_t * local_thread_id;
    cthread_t * threadlib_thread_id;
    int executing;
} concurrent_cb_info;

/* indicates whether concurrent module is initialized or not */
static int concurrent_initialized = 0;
/* hash map to store concurrent call-back information */
static hashmap *callback_map;
/* hash map to store mutexes */
static hashmap *mutex_map;
/* hash map to store thread conditional variables */
static hashmap *condvar_map;
/* next internal thread identity */
static size_t next_thread_ident = 1;
/* next internal mutex identity */
static size_t next_mutex_ident = 1;
/* next internal thread conditional variable identity */
static size_t next_condvar_ident = 1;
/* mudule initialization timestamp */
static long int time_reference;

/**
 * Allocates next available identifier.
 *
 * @param next_ident_count_in pointer to the next
 *        available identifier value.
 *
 * @return pointer to the allocated identifier.
 */
static size_t *
allocate_next_ident(size_t * next_ident_count_in){
    size_t * ni = (size_t *)SAFE_MALLOC(sizeof(size_t));

    *ni = *next_ident_count_in;
    *next_ident_count_in = *next_ident_count_in + 1;

    return ni;
}

/**
 * Test whether two size_t values are equal.
 *
 * @param l pointer to the first size_t value.
 * @param r pointer to the second size_t value.
 *
 * @return Non-zero if equal, zero otherwise.
 */
static int
hash_map_size_t_equal(const void *l, const void *r) {
    int result = 0;
    size_t l1, r1;

    l1 = *((size_t *)l);
    r1 = *(size_t *)r;
    if (l1 == r1) {
        result = 1;
    }

    return result;
}

/**
 * Function to be called by a thread.
 *
 * @param arg parameter to be passed during the function call.
 */
static void *
thread_start_routine(void * arg) {
    concurrent_cb_info * concurrent_cb_info_ptr =
        (concurrent_cb_info *)arg;
    long int next, now, rem;

    concurrent_cb_info_ptr->executing = 1;
    now = get_ms_since_epoch();
    next = now + concurrent_cb_info_ptr->interval;
    rem = concurrent_cb_info_ptr->interval;
    do {
        msuspend(rem); /* execute call-back after specified delay */
        concurrent_cb_info_ptr->cb(
            concurrent_cb_info_ptr->cb_data,
            concurrent_cb_info_ptr->local_thread_id);
        /* in case of a strict periodic call-back
           re-calculate the time for the next
           call-back execution
         */
        if (concurrent_cb_info_ptr->strict_timing_enabled) {
            next = next + concurrent_cb_info_ptr->interval;
            now = get_ms_since_epoch();
            if (next > now) {
                rem = (next - now);
            } else {
                if (concurrent_cb_info_ptr->periodic_cb) {
                    log_warn_format(
                        "concurrent: call-back execution for thread %lu takes more than specified interval",
                        concurrent_cb_info_ptr->local_thread_id);
                    rem = concurrent_cb_info_ptr->interval;
                }
            }
        }
    } while(concurrent_cb_info_ptr->periodic_cb); /* continue execution if call-back is periodic */
    concurrent_cb_info_ptr->executing = 0;

    return NULL;
}

/**
 * Install a call-back to be called after a specified time interval.
 *
 * @param interval time internal in milliseconds.
 * @param cb call-back function.
 * @param cb_data pointer to data that will be passed to the call-back.
 * @param periodic indicates whether call-back is periodic or not.
 * @param strict_timing_enabled indicates whether periodic string timing
 *        is enabled or not.
 *
 * @return identity of the thread executor that will perform the call.
 */
static size_t *
concurrent_install_cb_internal(
    unsigned int interval,
    concurent_cb cb,
    void *cb_data,
    int periodic,
    int strict_timing_enabled) {

    size_t *thread_id = NULL;
    if (concurrent_initialized) {
        cthread_t * threadlib_thread_id = NULL;
        concurrent_cb_info *info =
            (concurrent_cb_info *) SAFE_MALLOC(sizeof(concurrent_cb_info));
        thread_id = allocate_next_ident(&next_thread_ident);

        info->interval = interval;
        info->cb = cb;
        info->cb_data = cb_data;
        info->periodic_cb = periodic;
        if (periodic) {
            info->strict_timing_enabled = strict_timing_enabled;
        } else {
            info->strict_timing_enabled = 0;
        }
        info->local_thread_id = thread_id;
        info->executing = 0;
        threadlib_thread_id =
            concurrentfactory_create_thread(thread_start_routine, (void *) info);
        if (threadlib_thread_id == NULL) {
            SAFE_FREE(info);
            SAFE_FREE(thread_id);
        } else {
            info->threadlib_thread_id = threadlib_thread_id;
            hashmap_put(callback_map, thread_id, info);
        }
    }

    return thread_id;
}

/**
 * Initializes the concurrent library module.
 */
extern void
concurrent_init(void) {
    if (!concurrent_initialized) {
        callback_map = SAFE_MALLOC(sizeof(hashmap));
        mutex_map = SAFE_MALLOC(sizeof(hashmap));
        condvar_map = SAFE_MALLOC(sizeof(hashmap));
        hashmap_init(callback_map, 17, hash_map_size_t_equal, NULL);
        hashmap_init(mutex_map, 17, hash_map_size_t_equal, NULL);
        hashmap_init(condvar_map, 17, hash_map_size_t_equal, NULL);
        concurrent_initialized++;
        time_reference = get_ms_since_epoch();
    }
}

/**
 * Install a call-back to be executed after a specified time interval.
 *
 * @param interval initial time interval in milliseconds before call-back execution.
 * @param cb call-back function to be executed.
 * @param cb_data pointer to data that will be passed to the call-back function.
 *
 * @return identity of the thread executor that will execute the call.
 */
extern size_t *
concurrent_install_cb(
    long unsigned int interval,
    concurent_cb cb,
    void *cb_data) {
    return concurrent_install_cb_internal(interval, cb, cb_data, 0, 0);
}

/**
 * Install a call-back to be executed periodically after a specified time interval.
 *
 * @param interval time interval in milliseconds.
 * @param cb call-back function to be executed.
 * @param client_data pointer to data that will be passed to the call-back function.
 *
 * @return identity of the thread executor that will execute the periodic calls.
 */
extern size_t *
concurrent_install_periodic_cb(
    long unsigned int interval,
    concurent_cb cb,
    void *cb_data) {

    return concurrent_install_cb_internal(interval, cb, cb_data, 1, 0);
}

/**
 * Install a call-back to be executed periodically after a specified time interval.
 * Timeout calculation takes into account the specified interval as well as the
 * call-back execution time.
 *
 * @param interval time interval in milliseconds.
 * @param cb call-back function to be executed.
 * @param client_data pointer to data that will be passed to the call-back function.
 *
 * @return identity of the thread executor that will execute the periodic calls.
 */
extern size_t *
concurrent_install_strict_periodic_cb(
    long unsigned int interval,
    concurent_cb cb,
    void *cb_data) {

    return concurrent_install_cb_internal(interval, cb, cb_data, 1, 1);
}

/**
 * Un-install a previously install (periodic) call-back.
 *
 * @param thread_id identity of the thread executor
 *        responsible for executing the function call(s).
 */
extern void
concurrent_uninstall_cb(size_t * thread_id) {
    if (concurrent_initialized && thread_id != NULL) {
        hashmap_entry * entry =
            (hashmap_entry *)hashmap_remove(callback_map, thread_id);
        if (entry != NULL) {
            concurrent_cb_info *info = (concurrent_cb_info *)entry->value;
            if (info->executing) {
                /* cancel thread execution */
                concurrentfactory_cancel_thread(*(info->threadlib_thread_id));
            }
            /* detach to release claimed resources */
            concurrentfactory_detach_thread(*(info->threadlib_thread_id));

            SAFE_FREE(info->threadlib_thread_id);

            SAFE_FREE(entry->value);
            SAFE_FREE(entry->key);
            SAFE_FREE(entry);
        }
    }
}

/**
 * Test whether a call-back is being executed or not
 *
 * @param thread_id identity of the thread executor responsible
 *        for executing the call-back.
 *
 * @return Non-zero if call-back is being executed, zero otherwise.
 */
extern int
concurrent_cb_executing(size_t * thread_id) {
    int result = 0;

    if (concurrent_initialized && thread_id != NULL) {
        concurrent_cb_info * concurrent_cb_info_ptr = hashmap_get(callback_map, thread_id);
        if (concurrent_cb_info_ptr != NULL) {
            result = concurrent_cb_info_ptr->executing;
        }
    }

    return result;
}

/**
 * Create a mutex.
 *
 * @return identity of the mutex.
 */
extern size_t *
concurrent_create_mutex(void) {
    size_t *mutex_id = NULL;

    if (concurrent_initialized) {
        cthread_mutex_t *mutex = concurrentfactory_create_mutex();
        if (mutex != NULL) {
            mutex_id = allocate_next_ident(&next_mutex_ident);
            hashmap_put(mutex_map, mutex_id, mutex);
        }
    }

    return mutex_id;
}

/**
 * Destroy a mutex.
 *
 * @param mutex_id identity of the mutex.
 */
extern void
concurrent_destroy_mutex(size_t * mutex_id) {
    if (concurrent_initialized && mutex_id != NULL) {
        hashmap_entry * entry = (hashmap_entry *)hashmap_remove(mutex_map, mutex_id);
        if (entry != NULL) {
            cthread_mutex_t *mutex = (cthread_mutex_t *)entry->value;
            concurrentfactory_destroy_mutex(mutex);
            SAFE_FREE(entry->key);
            SAFE_FREE(entry);
        }
    }
}

/**
 * Lock the mutex object.
 *
 * @param mutex_id identity of the mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrent_lock_mutex(size_t * mutex_id) {
    int result = 0;

    if (concurrent_initialized && mutex_id != NULL) {
        cthread_mutex_t *cthread_mutex =
            (cthread_mutex_t *)hashmap_get(mutex_map, mutex_id);
        if (cthread_mutex != NULL) {
            result = concurrentfactory_lock_mutex(cthread_mutex);
        }
    }

    return result;
}

/**
 * Try to lock the mutex object. If the mutex is locked
 * by another thread the call shall return immediately
 * without locking the mutex.
 *
 * @param mutex_id identity of the mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrent_trylock_mutex(size_t * mutex_id) {
    int result = 0;

    if (concurrent_initialized && mutex_id != NULL) {
        cthread_mutex_t *cthread_mutex =
            (cthread_mutex_t *)hashmap_get(mutex_map, mutex_id);
        if (cthread_mutex != NULL) {
            result = concurrentfactory_trylock_mutex(cthread_mutex);
        }
    }

    return result;
}

/**
 * Unlock the mutex object.
 *
 * @param mutex_id identity of the mutex to be unlocked.
 *
 * @return Non-zero if mutex was unlocked, zero otherwise.
 */
extern int
concurrent_unlock_mutex(size_t * mutex_id) {
    int result = 0;

    if (concurrent_initialized && mutex_id != NULL) {
        cthread_mutex_t *cthread_mutex =
            (cthread_mutex_t *)hashmap_get(mutex_map, mutex_id);
        if (cthread_mutex != NULL) {
            result = concurrentfactory_unlock_mutex(cthread_mutex);
        }
    }

    return result;
}

/**
 * Create a thread condition variable.
 *
 * @return identity of the thread condition variable.
 */
extern size_t *
concurrent_create_condvar(void) {
    size_t *condvar_id = NULL;

    if (concurrent_initialized) {
        cthread_cond_t *condvar = concurrentfactory_create_condvar();
        if (condvar != NULL) {
            condvar_id = allocate_next_ident(&next_condvar_ident);
            hashmap_put(condvar_map, condvar_id, condvar);
        }
    }

    return condvar_id;
}

/**
 * Destroy a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 */
extern void
concurrent_destroy_condvar(size_t * condvar_id) {
    if (concurrent_initialized && condvar_id != NULL) {
        hashmap_entry * entry =
            (hashmap_entry *)hashmap_remove(condvar_map, condvar_id);
        if (entry != NULL) {
            cthread_cond_t *cthread_condvar = (cthread_cond_t *)entry->value;
            concurrentfactory_destroy_condvar(cthread_condvar);
            SAFE_FREE(entry->key);
            SAFE_FREE(entry);
        }
    }
}

/**
 * Wait on a thread condition variable. It shall be called with mutex
 * locked by the calling thread. Once the call has been made the mutex
 * shall be released until a successful return of the function.
 * Upon successful return, the mutex shall have been locked and shall
 * be owned by the calling thread.
 *
 * @param condvar_id identity of the thread condition variable.
 * @param mutex_id identity of the mutex to be used during the process of waiting.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_wait_condvar(size_t * condvar_id, size_t * mutex_id) {
    int result = 0;

    if (concurrent_initialized && condvar_id != NULL && mutex_id != NULL) {
        cthread_cond_t *cthread_condvar =
            (cthread_cond_t *)hashmap_get(condvar_map, condvar_id);
        cthread_mutex_t *cthread_mutex =
            (cthread_mutex_t *)hashmap_get(mutex_map, mutex_id);

        if (cthread_condvar != NULL && cthread_mutex != NULL) {
            result = concurrentfactory_wait_condvar(cthread_condvar, cthread_mutex);
        }
    }

    return result;
}

/**
 * Wait on a thread condition variable for the specified timeout.
 * It shall be called with mutex locked by the calling thread.
 * Once the call has been made the mutex shall be released until
 * a successful return of the function.
 * Upon successful return, the mutex shall have been locked and shall
 * be owned by the calling thread.
 *
 * @param condvar_id identity of the thread condition variable.
 * @param mutex_id identity of the mutex to be used during the process of waiting.
 * @param timeout milliseconds to wait on the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero when timeout expires,
 *         negative upon error.
 */
extern int
concurrent_timedwait_condvar(size_t * condvar_id, size_t * mutex_id, unsigned long timeout) {
    int result = 0;

    if (concurrent_initialized && condvar_id != NULL && mutex_id != NULL) {
        cthread_cond_t *cthread_condvar =
            (cthread_cond_t *)hashmap_get(condvar_map, condvar_id);
        cthread_mutex_t *cthread_mutex =
            (cthread_mutex_t *)hashmap_get(mutex_map, mutex_id);

        if (cthread_condvar != NULL && cthread_mutex != NULL) {
            result = concurrentfactory_timedwait_condvar(cthread_condvar, cthread_mutex, timeout);
        }
    }

    return result;
}

/**
 * Unblock a thread waiting on a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_condvar_signal(size_t * condvar_id) {
    int result = 0;

    if (concurrent_initialized && condvar_id != NULL) {
        cthread_cond_t *cthread_condvar =
            (cthread_cond_t *) hashmap_get(condvar_map, condvar_id);
        if (cthread_condvar != NULL) {
            result = concurrentfactory_condvar_signal(cthread_condvar);
        }
    }

    return result;
}

/**
 * Unblock all threads waiting on a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_condvar_broadcast(size_t * condvar_id) {
    int result = 0;

    if (concurrent_initialized && condvar_id != NULL) {
        cthread_cond_t *cthread_condvar =
            (cthread_cond_t *) hashmap_get(condvar_map, condvar_id);
        if (cthread_condvar != NULL) {
            result = concurrentfactory_condvar_broadcast(cthread_condvar);
        }
    }

    return result;
}

/**
 * Shutdown the concurrent library module, freeing all dynamic allocated memory.
 */
extern void
concurrent_shutdown(void) {
    if (concurrent_initialized) {
        hashmap_free(callback_map);
        hashmap_free(mutex_map);
        hashmap_free(condvar_map);
    }
}
