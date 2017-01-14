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

#include "concurrentfactory.h"

#ifdef __USE_WINAPI_THREAD_LIB

#include <windows.h>

#else

#include <errno.h>
#include <time.h>
#include <pthread.h>

#ifdef _WIN32

#if !defined(ETIMEDOUT)
#  define ETIMEDOUT 10060 /* Same as WSAETIMEDOUT */
#endif /* ETIMEDOUT */

#endif /* _WIN32 */

#endif

#include "time_util.h"
#include "stdlib_util.h"
#include "log.h"

#ifdef __USE_OWN_CONDVAR

struct _cthread_cond_t {
    HANDLE hSemaphore;
    cthread_mutex_t *waiters_mutex;
    int waiters_no;
};

#endif /* __USE_OWN_CONDVAR */

/**
 * Creates a thread which starts execution by invoking start_routine.
 * arg is passed as the sole argument of start_routine.
 *
 * @param start_routine the function to be called by the created thread.
 * @param arg argument to be passed to the start_routine function call.
 */
extern cthread_t *
concurrentfactory_create_thread(void *(*start_routine) (void *), void *arg) {
#ifdef __USE_WINAPI_THREAD_LIB

    HANDLE * cthread_id = (HANDLE *)SAFE_MALLOC(sizeof(HANDLE));
    HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_routine, arg, 0, NULL);

    if (h == NULL) {
        log_error_message(
            "concurrentfactory.concurrentfactory_create_thread.CreateThread() failed!");
        SAFE_FREE(cthread_id);
        cthread_id = NULL;
    } else {
        *cthread_id = h;
    }

#else

    pthread_t * cthread_id = (pthread_t *)SAFE_MALLOC(sizeof(pthread_t));
    int result = pthread_create(cthread_id, NULL, start_routine, arg);

    if (result != 0) {
        log_error_message(
            "concurrentfactory.concurrentfactory_create_thread.pthread_create() failed!");
        SAFE_FREE(cthread_id);
        cthread_id = NULL;
    }

#endif /* __USE_WINAPI_THREAD_LIB */

    return cthread_id;
}

/**
 * Marks the thread identified by thread as detached.
 *
 * @param pthread_id thread identity.
 *
 * @return Non zero if operation finished successfully, zero otherwise.
 */
extern int
concurrentfactory_detach_thread(cthread_t cthread_id) {
    int result = 0;

#ifdef __USE_WINAPI_THREAD_LIB
    if (CloseHandle(cthread_id)) {
#else
    if (pthread_detach(cthread_id) == 0) {
#endif /* __USE_WINAPI_THREAD_LIB */
        result++;
    } else {
#ifdef __USE_WINAPI_THREAD_LIB
        log_error_message(
            "concurrentfactory.concurrentfactory_detach_thread.CloseHandle() failed!");
#else
        log_error_message(
            "concurrentfactory.concurrentfactory_detach_thread.pthread_detach() failed!");
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Waits for the thread specified by thread to terminate.
 *
 * @param cthread_id thread identity.
 *
 * @return Non zero if operation finished successfully, zero otherwise.
 */
extern int
concurrentfactory_join_thread(cthread_t cthread_id) {
    int result = 0;

#ifdef __USE_WINAPI_THREAD_LIB
    if (WaitForSingleObject(cthread_id, INFINITE) == 0) {
        /* CloseHandle(cthread_id); */
#else
    if (pthread_join(cthread_id, NULL) == 0) {
#endif /* __USE_WINAPI_THREAD_LIB */
        result++;
    } else {
#ifdef __USE_WINAPI_THREAD_LIB
        log_error_message(
            "concurrentfactory.concurrentfactory_detach_thread.WaitForSingleObject() failed!");
#else
        log_error_message(
            "concurrentfactory.concurrentfactory_detach_thread.pthread_join() failed!");
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Terminates/cancels the execution of the specified thread.
 *
 * @param cthread_id thread identity.
 *
 * @return Non zero if thread canceled, zero otherwise.
 */
extern int
concurrentfactory_cancel_thread(cthread_t cthread_id) {
    int result = 0;

#ifdef __USE_WINAPI_THREAD_LIB
    if (TerminateThread(cthread_id, 0)) {
#else
    if (pthread_cancel(cthread_id) == 0) {
#endif /* __USE_WINAPI_THREAD_LIB */
        result++;
    } else {
#ifdef __USE_WINAPI_THREAD_LIB
        log_error_message(
            "concurrentfactory.concurrentfactory_cancel_thread.TerminateThread() failed!");
#else
        log_error_message(
            "concurrentfactory.concurrentfactory_cancel_thread.pthread_cancel() failed!");
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Create a mutex.
 *
 * @return created mutex upon successful creation, NULL otherwise
 */
extern cthread_mutex_t *
concurrentfactory_create_mutex(void) {
#ifdef __USE_WINAPI_THREAD_LIB

    CRITICAL_SECTION * mutex = (CRITICAL_SECTION *)SAFE_MALLOC(sizeof(CRITICAL_SECTION));

    InitializeCriticalSectionAndSpinCount(mutex, 0);

#else    

    pthread_mutex_t *mutex = (pthread_mutex_t *)SAFE_MALLOC(sizeof(pthread_mutex_t));

    if (pthread_mutex_init(mutex, NULL) != 0) {
        log_error_message(
            "concurrentfactory.concurrentfactory_create_mutex.pthread_mutex_init() failed!");
        SAFE_FREE(mutex);
    }

#endif /* __USE_WINAPI_THREAD_LIB */

    return mutex;
}

/**
 * Destroy a mutex.
 *
 * @param mutex the mutex to be destroyed.
 */
extern void
concurrentfactory_destroy_mutex(cthread_mutex_t *mutex) {
    if (mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
        DeleteCriticalSection(mutex);
#else
        if (pthread_mutex_destroy(mutex) != 0) {
            log_error_message(
                "concurrentfactory.concurrentfactory_destroy_mutex.pthread_mutex_destroy.mutex() failed!");
        }
#endif /* __USE_WINAPI_THREAD_LIB */
        SAFE_FREE(mutex);
    }
}

/**
 * Lock the mutex object.
 *
 * @param mutex mutex to be locked.
 *
 * @return Non-zero if mutex was locked, zero otherwise.
 */
extern int
concurrentfactory_lock_mutex(cthread_mutex_t *mutex) {
    int result = 0;

    if (mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
        EnterCriticalSection(mutex);
        result++;
#else
        if (pthread_mutex_lock(mutex) == 0) {
            result++;
        } else {
            log_error_message(
                "concurrentfactory.concurrentfactory_lock_mutex.pthread_mutex_lock() failed!");
        }
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

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
concurrentfactory_trylock_mutex(cthread_mutex_t *mutex) {
    int result = 0;

    if (mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
        if (TryEnterCriticalSection(mutex)) {
#else
        if (pthread_mutex_trylock(mutex) == 0) {
#endif /* __USE_WINAPI_THREAD_LIB */
        result++;
        }  else {
#ifdef __USE_WINAPI_THREAD_LIB
            log_error_message(
                "concurrentfactory.concurrentfactory_trylock_mutex.TryEnterCriticalSection() failed!");
#else
            log_error_message(
                "concurrentfactory.concurrentfactory_trylock_mutex.pthread_mutex_trylock() failed!");
#endif /* __USE_WINAPI_THREAD_LIB */
        }
    }

    return result;
}

/**
 * Unlock the mutex object.
 *
 * @param mutex the mutex to be unlocked.
 *
 * @return Non-zero if mutex was unlocked, zero otherwise.
 */
extern int
concurrentfactory_unlock_mutex(cthread_mutex_t *mutex) {
    int result = 0;

    if (mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
        LeaveCriticalSection(mutex);
        result++;
#else
        if (pthread_mutex_unlock(mutex) == 0) {
            result++;
        }  else {
            log_error_message(
                "concurrentfactory.concurrentfactory_unlock_mutex.pthread_mutex_unlock() failed!");
        }
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Create a thread condition variable.
 *
 * @return created thread condition variable
 *         upon successful creation, NULL otherwise.
 */
extern cthread_cond_t *
concurrentfactory_create_condvar(void) {
#ifdef __USE_WINAPI_THREAD_LIB

#ifdef __USE_OWN_CONDVAR

    cthread_cond_t *condvar = (cthread_cond_t *)SAFE_MALLOC(sizeof(cthread_cond_t));
    /* Create the semaphore */
    condvar->hSemaphore = CreateSemaphore(
        NULL,   /* default security attributes */
        0,      /* initial count */
        0xffff, /* maximum count (!) */
        NULL);

    if (condvar->hSemaphore == NULL) {
        log_error_message(
            "concurrentfactory.concurrentfactory_create_condvar.CreateSemaphore() failed!");
        log_error_format("CreateSemaphore() error: %lu\n", GetLastError());
        SAFE_FREE(condvar);
        condvar = NULL;
    } else {
        /* Create waiters mutex */
        condvar->waiters_mutex = concurrentfactory_create_mutex();

        if (condvar->waiters_mutex == NULL) {
            log_error_message(
                "concurrentfactory.concurrentfactory_create_condvar.concurrentfactory_create_mutex() failed!");
            CloseHandle(condvar->hSemaphore);
            SAFE_FREE(condvar);
            condvar = NULL;
        } else {
            condvar->waiters_no = 0;
        }
    }

#else

    CONDITION_VARIABLE *condvar =
        (CONDITION_VARIABLE *)SAFE_MALLOC(sizeof(CONDITION_VARIABLE));
    InitializeConditionVariable(condvar);

#endif /* __USE_OWN_CONDVAR */

#else

    pthread_cond_t *condvar = (pthread_cond_t *)SAFE_MALLOC(sizeof(pthread_cond_t));

    if (pthread_cond_init(condvar, NULL) != 0) {
        log_error_message(
            "concurrentfactory.concurrentfactory_create_condvar.pthread_cond_init() failed!");
        SAFE_FREE(condvar);
        condvar = NULL;
    }

#endif /* __USE_WINAPI_THREAD_LIB */

    return condvar;
}

/**
 * Destroy a thread condition variable.
 *
 * @param condvar the thread condition variable to be destroyed.
 */
extern void
concurrentfactory_destroy_condvar(cthread_cond_t *condvar) {
    if (condvar != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
#ifdef __USE_OWN_CONDVAR
        if (!CloseHandle(condvar->hSemaphore)) {
            log_error_message(
                "concurrentfactory.concurrentfactory_destroy_condvar.CloseHandle() failed!");
        }
        concurrentfactory_destroy_mutex(condvar->waiters_mutex);
#endif /* __USE_OWN_CONDVAR */
#else
        if (pthread_cond_destroy(condvar) != 0) {
            log_error_message(
                "concurrentfactory.concurrentfactory_destroy_condvar.pthread_cond_destroy() failed!");
        }
#endif /* __USE_WINAPI_THREAD_LIB */
        SAFE_FREE(condvar);
    }
}

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
concurrentfactory_wait_condvar(cthread_cond_t *condvar, cthread_mutex_t *mutex) {
    int result = 0;

    if (condvar != NULL && mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
#ifdef __USE_OWN_CONDVAR
        if (concurrentfactory_unlock_mutex(mutex)) { /* unlock external mutex */
            concurrentfactory_lock_mutex(condvar->waiters_mutex);
            condvar->waiters_no++;
            concurrentfactory_unlock_mutex(condvar->waiters_mutex);
            if(WaitForSingleObject(condvar->hSemaphore, INFINITE) == WAIT_OBJECT_0) {
                concurrentfactory_lock_mutex(condvar->waiters_mutex);
                condvar->waiters_no--;
                concurrentfactory_unlock_mutex(condvar->waiters_mutex);
                if (concurrentfactory_lock_mutex(mutex)) { /* re-lock external mutex */
                    result++;
                }
            }
        }
#else
        if (SleepConditionVariableCS(condvar,  mutex, INFINITE)) {
            result++;
        }
#endif /* __USE_OWN_CONDVAR */
#else
        if (pthread_cond_wait(condvar, mutex) == 0) {
            result++;
        }
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

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
    unsigned long timeout) {
    int result = 0;

    if (timeout == 0L) {
        return 0;
    }

    if (condvar != NULL && mutex != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
#ifdef __USE_OWN_CONDVAR
        if (concurrentfactory_unlock_mutex(mutex)) { /* unlock external mutex */
        	unsigned long wait_result;
        	concurrentfactory_lock_mutex(condvar->waiters_mutex);
            condvar->waiters_no++;
            concurrentfactory_unlock_mutex(condvar->waiters_mutex);

            wait_result = WaitForSingleObject(condvar->hSemaphore, timeout);

            if(wait_result == WAIT_OBJECT_0 || wait_result == WAIT_TIMEOUT) {
                concurrentfactory_lock_mutex(condvar->waiters_mutex);
                condvar->waiters_no--;
                concurrentfactory_unlock_mutex(condvar->waiters_mutex);
                if (concurrentfactory_lock_mutex(mutex)) { /* re-lock external mutex */
                    if (wait_result == WAIT_OBJECT_0) {
                	    result++;
                    }
                }
            } else {
                result = -1;
            }
        }
#else
        result = SleepConditionVariableCS(condvar,  mutex, timeout);
        if (result) {
            result++;
        } else {
            if (GetLastError() == ERROR_TIMEOUT) {
            	result = 0;
            } else {
            	result = -1;
            }
        }
#endif /* __USE_OWN_CONDVAR */
#else
        struct timespec time_out;
        time_val_struct time_val;

        get_time_since_epoch(&time_val);
        time_out.tv_sec = time_val.tv_sec + timeout/1000L;
        time_out.tv_nsec = (time_val.tv_usec) * 1000L;

        result = pthread_cond_timedwait(condvar, mutex, &time_out);

        if (result == 0) {
            result++;
        } else {
        	if (result == ETIMEDOUT) {
            	result = 0;
            } else {
                result = -1;
            }
        }
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Unblock a thread waiting on a thread condition variable.
 *
 * @param condvar the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentfactory_condvar_signal(cthread_cond_t *condvar) {
    int result = 0;
    if (condvar != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
#ifdef __USE_OWN_CONDVAR
        if (ReleaseSemaphore(condvar->hSemaphore, 1, NULL)) {
            result++;
        } else {
            log_error_message(
                "concurrentfactory.concurrentfactory_condvar_signal.ReleaseSemaphore() failed!");
            log_error_format("ReleaseSemaphore() error: %lu\n", GetLastError());
        }
#else
        WakeConditionVariable(condvar);
        result++;
#endif /* _USE_OWN_CONDVAR */

#else
        if (pthread_cond_signal(condvar) == 0) {
            result++;
        } else {
            log_error_message(
                "concurrentfactory.concurrentfactory_condvar_signal.pthread_cond_signal() failed!");
        }
#endif /* _USE_WINAPI_THREAD_LIB */
    }

    return result;
}

/**
 * Unblock all threads waiting on a thread condition variable.
 *
 * @param condvar the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentfactory_condvar_broadcast(cthread_cond_t *condvar) {
    int result = 0;
    if (condvar != NULL) {
#ifdef __USE_WINAPI_THREAD_LIB
#ifdef __USE_OWN_CONDVAR
        concurrentfactory_lock_mutex(condvar->waiters_mutex);
        if (ReleaseSemaphore(condvar->hSemaphore, condvar->waiters_no, NULL)) {
            result++;
        } else {
            log_error_message(
                "concurrentfactory.concurrentfactory_condvar_broadcast.ReleaseSemaphore() failed!");
            log_error_format("ReleaseSemaphore() error: %lu\n", GetLastError());
        }
        concurrentfactory_unlock_mutex(condvar->waiters_mutex);
#else
        WakeAllConditionVariable(condvar);
        result++;
#endif /* __USE_OWN_CONDVAR */

#else
        if (pthread_cond_broadcast(condvar) == 0) {
            result++;
        } else {
            log_error_message(
                "concurrentfactory.concurrentfactory_condvar_broadcast.pthread_cond_broadcast() failed!");
        }
#endif /* __USE_WINAPI_THREAD_LIB */
    }

    return result;
}
