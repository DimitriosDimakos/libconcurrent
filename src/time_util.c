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

#ifdef _WIN32

#include <windows.h>

#else

#include <time.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#ifdef HAVE_CLOCK_GETTIME
#define __USE_CLOCK_GETTIME
#endif /* HAVE_FTIME */
#endif /* HAVE_CONFIG_H*/

#ifndef __USE_CLOCK_GETTIME
/* use old gettimeofday instead */
#include <sys/time.h>
#endif /* __USE_CLOCK_GETTIME */

#include <sys/select.h>

#endif /* _WIN32 */

#include "time_util.h"

#ifdef _WIN32

#ifndef _TIMEVAL_DEFINED

/* define timeval structure */
struct timeval {
    long tv_sec;
    long tv_usec;
};

#endif /* _TIMEVAL_DEFINED */

/* Epoch Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
 * Implementation of gettimeofday for Windows platform. Timezone information
 * is not relevant so it is not used.
 *
 * @param tp reference to a timeval structure to store time value.
 *
 * @return always zero.
 */
static int
gettimeofday(struct timeval * tp) {
    FILETIME  file_time;
    SYSTEMTIME system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

    return 0;
}

#endif /* _WIN32 */

/*
 * Wrapper function for the gettimeofday function
 *
 * @param tp reference to a timeval structure to be passed
          to the gettimeofday function
 *
 * @return result of gettimeofday function call.
 */
static int
time_util_gettimeofday(struct timeval * tp) {
#ifdef _WIN32
    return gettimeofday(tp);
#else
    return gettimeofday(tp, NULL);
#endif /* _WIN32 */
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
suspend(long seconds) {
    if (seconds <= 0L) {
        return (-1);
    }
    return msuspend(seconds * 1000L);
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
msuspend(long mseconds) {
    if (mseconds <= 0L) {
        return (-1);
    }
#ifdef _WIN32
    Sleep(mseconds);

    return 1;
#else
    struct timeval tp;
    tp.tv_sec = mseconds / 1000L;
    tp.tv_usec = (mseconds % 1000L) * 1000L;

    return select(0, NULL, NULL, NULL, &tp);
#endif /* _WIN32 */
}

/*
 * The get_ms_since_epoch() function returns the milliseconds passed since the Epoch.
 *
 * @return milliseconds passed since the Epoch.
 */
extern long long
get_ms_since_epoch(void) {
    long long t_now;
#ifdef __USE_CLOCK_GETTIME
    struct timespec timespec_now;

    clock_gettime(CLOCK_REALTIME, &timespec_now);
    t_now = timespec_now.tv_sec * 1000LL + timespec_now.tv_nsec / 1000000000LL;
#else
    struct timeval timeval_now;

    time_util_gettimeofday(&timeval_now);
    t_now = timeval_now.tv_sec * 1000LL + timeval_now.tv_usec / 1000LL;
#endif /*__USE_CLOCK_GETTIME*/

    return t_now;
}

/*
 * The get_time_since_epoch() function returns the time passed since the Epoch
 * as a time_val_struct structure.
 *
 * @param time_val reference to a time_val_struct (not NULL) structure
 *        to store the time value.
 */
extern void
get_time_since_epoch(time_val_struct * time_val) {
    if (time_val) {
#ifdef __USE_CLOCK_GETTIME
        struct timespec timespec_now;

        clock_gettime(CLOCK_REALTIME, &timespec_now);
        time_val->tv_sec = timespec_now.tv_sec;
        time_val->tv_usec = timespec_now.tv_nsec;
#else
        struct timeval timeval_now;

        time_util_gettimeofday(&timeval_now);
        time_val->tv_sec = timeval_now.tv_sec;
        time_val->tv_usec = timeval_now.tv_usec / 1000000L;
#endif /*__USE_CLOCK_GETTIME*/
    }
}

/**
 * Get process time (in milliseconds) since the provided time reference.
 *
 * @param t_ref reference time.
 *
 * @return time passed (in milliseconds) since t_ref.
 */
extern long long
get_wall_time_ms(long long t_ref) {
	long long t_now;
#ifdef __USE_CLOCK_GETTIME
    struct timespec timespec_now;

    clock_gettime(CLOCK_REALTIME, &timespec_now);
    t_now = timespec_now.tv_sec * 1000LL + timespec_now.tv_nsec / 1000000000LL;
#else
    struct timeval timeval_now;

    time_util_gettimeofday(&timeval_now);
    t_now = timeval_now.tv_sec * 1000LL + timeval_now.tv_usec / 1000LL;
#endif /*__USE_CLOCK_GETTIME*/
    return (t_now - t_ref);
}
