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
 * Utility module providing time related operations.
 */

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_

#ifdef  __cplusplus
extern "C" {
#endif

/* structure to store time value. */
typedef struct _time_val_struct {
    long tv_sec;
    long tv_usec;
} time_val_struct;

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
suspend(long seconds);

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
msuspend(long mseconds);

/*
 * The get_ms_since_epoch() function returns the milliseconds passed since the Epoch.
 *
 * @return milliseconds passed since the Epoch.
 */
extern long long
get_ms_since_epoch(void);

/*
 * The get_time_since_epoch() function returns the time passed since the Epoch
 * as a time_val_struct structure.
 *
 * @param time_val reference to a time_val_struct (not NULL) structure
 *        to store the time value.
 */
extern void
get_time_since_epoch(time_val_struct * time_val);

/**
 * Get process time (in milliseconds) since the provided time reference.
 *
 * @param t_ref reference time.
 * @return time passed (in milliseconds) since t_ref.
 */
extern long long
get_wall_time_ms(long long t_ref);

#ifdef  __cplusplus
}
#endif

#endif /* TIME_UTIL_H_ */
