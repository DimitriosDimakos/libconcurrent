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
 * Utility module supporting the concurrent functionality
 * by providing convenience functions
 */

#ifndef CONCURRENT_UTIL_H_
#define CONCURRENT_UTIL_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* structure to hold concurrent signal information */
typedef struct _concurrent_signal_struct {
    size_t * mutex; /* mutex for the queue */
    size_t * condvar; /* condition variable for the queue */
} concurrent_signal;

/**
 * Initialize a concurrent signal.
 *
 * @param signal concurrent signal structure.
 */
extern void
concurrent_util_init_signal(concurrent_signal * signal);

/**
 * Wait (thread blocks) on a concurrent signal.
 *
 * Note that the underneath implementation expects that
 * the mutex of the concurrent signal is already locked
 * prior to the call.
 * Once the call has been made the mutex shall be released
 * until a successful return of the function.
 * Upon successful return, the mutex shall have been locked and shall
 * be owned by the calling thread.
 * Thus the implementation of this function will lock the mutex
 * before making the actual call and unlock it after the call.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_util_wait_signal(concurrent_signal * signal);

/**
 * Wait (thread blocks) on a concurrent signal for the specified timeout.
 *
 * Note that the underneath implementation expects that
 * the mutex of the concurrent signal is already locked
 * prior to the call.
 * Once the call has been made the mutex shall be released
 * until a successful return of the function.
 * Upon successful return, the mutex shall have been locked and shall
 * be owned by the calling thread.
 * Thus the implementation of this function will lock the mutex
 * before making the actual call and unlock it after the call.
 *
 * @param signal concurrent signal structure.
 * @param timeout milliseconds to wait on the thread condition variable.
 *
 * @return Non-zero upon successful completion, zero when timeout expires,
 *         negative upon error.
 */
extern int
concurrent_util_timedwait_signal(concurrent_signal * signal, unsigned long timeout);

/**
 * Unblock a thread waiting on a concurrent signal.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_util_send_signal(concurrent_signal * signal);

/**
 * Unblock all threads waiting on a concurrent signal.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_util_broadcast_signal(concurrent_signal * signal);

/**
 * Destroy a concurrent signal.
 *
 * @param signal concurrent signal structure.
 */
extern void
concurrent_util_destroy_signal(concurrent_signal * signal);

/*
 * The suspend() function shall cause the calling thread to be suspended from execution
 * until the number of real-time seconds specified by the argument seconds has elapsed
 * The suspension time may be longer than requested due to the scheduling of other
 * activity by the system.
 *
 * @param seconds number of seconds to suspend.
 *
 * @return -1 on errors, != -1 otherwise.
 */
extern int
concurrent_util_suspend(long seconds);

/*
 * The suspend() function shall cause the calling thread to be suspended from execution
 * until the number of real-time milliseconds specified by the argument seconds has elapsed
 * The suspension time may be longer than requested due to the scheduling of other
 * activity by the system.
 *
 * @param mseconds number of milliseconds to suspend.
 *
 * @return -1 on errors, != -1 otherwise.
 */
extern int
concurrent_util_msuspend(long mseconds);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENT_UTIL_H_ */
