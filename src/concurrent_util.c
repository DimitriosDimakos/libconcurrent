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

#include <stdlib.h>
#include <stdio.h>

#include "concurrent_util.h"
#include "concurrent.h"
#include "time_util.h"

/**
 * Initialize a concurrent signal.
 *
 * @param signal concurrent signal structure.
 */
extern void
concurrent_util_init_signal(concurrent_signal * signal) {
    signal->mutex = concurrent_create_mutex();
    signal->condvar = concurrent_create_condvar();
}

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
concurrent_util_wait_signal(concurrent_signal * signal) {
    int r1, r2, r3;

	r1 = concurrent_lock_mutex(signal->mutex); /* lock mutex */
    r2 = concurrent_wait_condvar(signal->condvar, signal->mutex);
    r3 = concurrent_unlock_mutex(signal->mutex); /* un-lock mutex */
    return  r1 && r2 && r3;
}

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
concurrent_util_timedwait_signal(concurrent_signal * signal, unsigned long timeout) {
    int r1, r2, r3;

	r1 = concurrent_lock_mutex(signal->mutex); /* lock mutex */
	r2 = concurrent_timedwait_condvar(signal->condvar, signal->mutex, timeout);
	r3 = concurrent_unlock_mutex(signal->mutex); /* un-lock mutex */
    if (r1 && r3) {
        return r2;
    } else {
        return -1;
    }
}

/**
 * Unblock a thread waiting on a concurrent signal.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_util_send_signal(concurrent_signal * signal) {
    return (concurrent_condvar_signal(signal->condvar));
}

/**
 * Unblock all threads waiting on a concurrent signal.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrent_util_broadcast_signal(concurrent_signal * signal) {
	return (concurrent_condvar_broadcast(signal->condvar));
}

/**
 * Destroy a concurrent signal.
 *
 * @param signal concurrent signal structure.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern void
concurrent_util_destroy_signal(concurrent_signal * signal) {
    concurrent_destroy_mutex(signal->mutex);
    concurrent_destroy_condvar(signal->condvar);
}

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
concurrent_util_suspend(long seconds) {
    return suspend(seconds);
}

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
concurrent_util_msuspend(long mseconds) {
    return msuspend(mseconds);
}
