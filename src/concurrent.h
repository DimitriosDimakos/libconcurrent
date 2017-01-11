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

#ifndef CONCURRENT_H_
#define CONCURRENT_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * The function type that should be supplied as a call-back to the thread context.
 *
 * @param cb_data pointer to data that will be passed
 *        to the function.
 * @param thread_id identity of the thread executor
 *        responsible for handling the call-back.
 */
typedef void *(*concurent_cb) (void *cb_data, size_t * thread_id);

/*
 * Initializes the concurrent library module.
 */
extern void
concurrent_init(void);

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
    void *cb_data);

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
    void *cb_data);

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
    void *cb_data);

/**
 * Un-install a previously install (periodic) call-back.
 *
 * @param thread_id identity of the thread executor
 *        responsible for executing the function call(s).
 */
extern void
concurrent_uninstall_cb(size_t * thread_id);

/**
 * Test whether a call-back is being executed or not
 *
 * @param thread_id identity of the thread executor responsible
 *        for executing the call-back.
 *
 * @return Non-zero if call-back is being executed, zero otherwise.
 */
extern int
concurrent_cb_executing(size_t * thread_id);

/**
 * Create a mutex.
 *
 * @return identity of the mutex.
 */
extern size_t *
concurrent_create_mutex(void);

/**
 * Destroy a mutex.
 *
 * @param mutex_id identity of the mutex.
 */
extern void
concurrent_destroy_mutex(size_t * mutex_id);

/**
 * Lock the mutex object.
 *
 * @param mutex_id identity of the mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrent_lock_mutex(size_t * mutex_id);

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
concurrent_trylock_mutex(size_t * mutex_id);

/**
 * Unlock the mutex object.
 *
 * @param mutex_id identity of the mutex to be unlocked.
 *
 * @return Non-zero if mutex was unlocked, zero otherwise.
 */
extern int
concurrent_unlock_mutex(size_t * mutex_id);

/**
 * Create a thread condition variable.
 *
 * @return identity of the thread condition variable.
 */
extern size_t *
concurrent_create_condvar(void);

/**
 * Destroy a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 */
extern void
concurrent_destroy_condvar(size_t * condvar_id);

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
concurrent_wait_condvar(size_t * condvar_id, size_t * mutex_id);

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
concurrent_timedwait_condvar(size_t * condvar_id, size_t * mutex_id, unsigned long timeout);

/**
 * Unblock a thread waiting on a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_condvar_signal(size_t * condvar_id);

/**
 * Unblock all threads waiting on a thread condition variable.
 *
 * @param condvar_id identity of the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_condvar_broadcast(size_t * condvar_id);

/**
 * Shutdown the concurrent library module, freeing all dynamic allocated memory.
 */
extern void
concurrent_shutdown(void);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENT_H_ */
