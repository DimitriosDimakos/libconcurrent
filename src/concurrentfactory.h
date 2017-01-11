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
 * Implementation of a high level concurrent factory.
 */

#ifndef CONCURRENTFACTORY_H_
#define CONCURRENTFACTORY_H_

#ifdef  __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(__NO_PTHREAD_LIB)

#define __USE_WINAPI_THREAD_LIB

#endif /* defined(_WIN32) && defined(_NO_PTHREAD_LIB) */

#ifdef __USE_WINAPI_THREAD_LIB

#include <windows.h>

typedef HANDLE cthread_t;
typedef CRITICAL_SECTION cthread_mutex_t;

#if (_WIN32_WINNT >= NTDDI_VISTA)

/* Minimum system required for CONDITION_VARIABLE is Windows Vista */
typedef CONDITION_VARIABLE cthread_cond_t;

#else

/* use own implementation of a conditional variable. */
#define __USE_OWN_CONDVAR

#endif /* (_WIN32_WINNT >= NTDDI_VISTA) */

#ifdef __USE_OWN_CONDVAR

typedef struct _cthread_cond_t cthread_cond_t;

#endif /* __USE_OWN_CONDVAR */

#else

#include <pthread.h>

typedef pthread_t cthread_t;
typedef pthread_mutex_t cthread_mutex_t;
typedef pthread_cond_t cthread_cond_t;

#endif /* __USE_WINAPI_THREAD_LIB */

/**
 * Creates a thread which starts execution by invoking start_routine.
 * arg is passed as the sole argument of start_routine.
 *
 * @param start_routine the function to be called by the created thread.
 * @param arg argument to be passed to the start_routine function call.
 */
extern cthread_t *
concurrentfactory_create_thread(void *(*start_routine) (void *), void *arg);

/**
 * Marks the thread identified by thread as detached.
 *
 * @param pthread_id thread identity.
 *
 * @return Non zero if operation finished successfully, zero otherwise.
 */
extern int
concurrentfactory_detach_thread(cthread_t cthread_id);

/**
 * Waits for the thread specified by thread to terminate.
 *
 * @param cthread_id thread identity.
 *
 * @return Non zero if operation finished successfully, zero otherwise.
 */
extern int
concurrentfactory_join_thread(cthread_t cthread_id);

/**
 * Terminates/cancels the execution of the specified thread.
 *
 * @param cthread_id thread identity.
 *
 * @return Non zero if thread cancelled, zero otherwise.
 */
extern int
concurrentfactory_cancel_thread(cthread_t cthread_id);

/**
 * Create a mutex.
 *
 * @return created mutex upon successful creation, NULL otherwise
 */
extern cthread_mutex_t *
concurrentfactory_create_mutex(void);

/**
 * Destroy amutex.
 *
 * @param mutex the mutex to be destroyed.
 */
extern void
concurrentfactory_destroy_mutex(cthread_mutex_t *mutex);

/**
 * Lock the mutex object.
 *
 * @param mutex mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrentfactory_lock_mutex(cthread_mutex_t *mutex);

/**
 * Lock the mutex object. If the mutex is locked
 * by another thread the call shall return immediately
 * without locking the mutex.
 *
 * @param mutex mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrentfactory_trylock_mutex(cthread_mutex_t *mutex);

/**
 * Unlock the mutex object.
 *
 * @param mutex the mutex to be unlocked.
 *
 * @return Non-zero if mutex was unlocked, zero otherwise.
 */
extern int
concurrentfactory_unlock_mutex(cthread_mutex_t *mutex);

/**
 * Create a thread condition variable.
 *
 * @return created thread condition variable
 *         upon successful creation, NULL otherwise.
 */
extern cthread_cond_t *
concurrentfactory_create_condvar(void);

/**
 * Destroy a thread condition variable.
 *
 * @param condvar the thread condition variable to be destroyed.
 */
extern void
concurrentfactory_destroy_condvar(cthread_cond_t *condvar);

/**
 * Wait on a thread condition variable.
 *
 * @param condvar the thread condition variable.
 * @param mutex the mutex to be used during the wait process,
 *        which is unlocked prior to the wait call and re-locked
 *        upon return of the wait call.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentfactory_wait_condvar(cthread_cond_t *condvar, cthread_mutex_t *mutex);

/**
 * Wait on a thread condition variable for the specified timeout.
 *
 * @param condvar the thread condition variable.
 * @param mutex the mutex to be used during the wait process,
 *        which is unlocked prior to the wait call and re-locked
 *        upon return of the wait call.
 * @param timeout milliseconds to wait on the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero when timeout expires,
 *         negative upon error.
 */
extern int
concurrentfactory_timedwait_condvar(
    cthread_cond_t *condvar,
    cthread_mutex_t *mutex,
    unsigned long timeout);

/**
 * Unblock a thread waiting on a thread condition variable.
 *
 * @param condvar the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentfactory_condvar_signal(cthread_cond_t *condvar);

/**
 * Unblock all threads waiting on a thread condition variable.
 *
 * @param condvar the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentfactory_condvar_broadcast(cthread_cond_t *condvar);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENTFACTORY_H_ */
