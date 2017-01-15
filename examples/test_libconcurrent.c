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

#include <stdio.h>
#include <stdlib.h>

#include <concurrent.h>
#include <concurrentqueue.h>
#include <concurrentpool.h>
#include <concurrentworker.h>
#include <concurrent_util.h>

#define STR_ITEM "STR_ITEM"

/**
 * Function to be submitted to a concurrent pool.
 *
 * @param job_data argument to be passed to this function.
 */
static void
concurent_job_n(void *job_data) {
    char * job_name = (char *)job_data;

    fprintf(stdout, "doing job %s\n", job_name);
    concurrent_util_suspend(1);
}

/**
 * Test concurrent pool functionality.
 */
static void
test_concurrentpool() {
    concurrentpool *cc_pool = concurrentpool_create();

    fprintf(stdout, "TEST CONCURRENT POOL started >>>>>>>>>>\n");
    concurrent_util_suspend(1);

    concurrentpool_init(cc_pool, 5, "simple pool");
    concurrentpool_submit_job(cc_pool, concurent_job_n, "job for pool");

    concurrent_util_suspend(5);

    concurrentpool_destroy(cc_pool);
    fprintf(stdout, "TEST CONCURRENT POOL finished >>>>>>>>>>\n");
}

/**
 * Function to be submitted to a concurrent worker.
 *
 * @param work_data argument to be passed to this function.
 */
static void
concurent_work_n(void *work_data) {
    char * work_name = (char *)work_data;

    fprintf(stdout, "doing work %s\n", work_name);
    concurrent_util_suspend(1);
}

/**
 * Test concurrent worker functionality.
 */
static void
test_concurrentworker() {
    concurrentworker *cc_worker = concurrentworker_create();

    fprintf(stdout, "TEST CONCURRENT WORKER started >>>>>>>>>>\n");
    concurrentworker_init(cc_worker, "simple worker");
    concurrentworker_submit_work(cc_worker, concurent_work_n, "1");
    concurrentworker_submit_work(cc_worker, concurent_work_n, "2");
    concurrentworker_submit_work(cc_worker, concurent_work_n, "3");
    concurrent_util_suspend(3);
    concurrentworker_destroy(cc_worker);
    fprintf(stdout, "TEST CONCURRENT WORKER finished >>>>>>>>>>\n");
}

/**
 * Function that is called by a concurrent thread to add an item in
 * a concurrent queue.
 *
 * @param cb_data argument to be passed to this function.
 * @param thread_id thread identification which calls this function.
 */
static void *
put_queue_cb(void *cb_data, size_t * thread_id) {
    concurrentqueue * cc_queue = (concurrentqueue *)cb_data;
    char * s = STR_ITEM;

    fprintf(stdout, "Put queue thread %lu started!\n", *thread_id);

    concurrentqueue_put_item(cc_queue, s);

    return NULL;
}

/**
 * Test concurrent queue functionality.
 */
static void
test_concurrentqueue() {
    char *char_p = NULL;
    concurrentqueue cc_queue;
    size_t * thread1;

    fprintf(stdout, "TEST CONCURRENT QUEUE started >>>>>>>>>>\n");

    concurrentqueue_init(&cc_queue, 31);

    thread1 = concurrent_install_cb(1000, put_queue_cb, &cc_queue);

    char_p = (char *)concurrentqueue_get_item(&cc_queue);

    concurrentqueue_destroy(&cc_queue);

    concurrent_uninstall_cb(thread1);

    fprintf(stdout, "char_p %s, p %p\n", char_p, char_p);

    fprintf(stdout, "TEST CONCURRENT QUEUE finished >>>>>>>>>>\n");
}

/**
 * Function that is called by a concurrent thread.
 *
 * @param cb_data argument to be passed to this function.
 * @param thread_id thread identification which calls this function.
 */
static void *
simple_cb(void *cb_data, size_t * thread_id) {
    fprintf(stdout, "Hello from simple_cb of thread %lu!\n", *thread_id);

    return NULL;
}

/**
 * Test concurrent thread functionality.
 */
static void
test_thread() {
    size_t * thread;

    fprintf(stdout, "TEST THREAD started >>>>>>>>>>\n");
    /* test simple thread creation */
    thread = concurrent_install_cb(1000, simple_cb, NULL);
    concurrent_util_suspend(5);
    concurrent_uninstall_cb(thread);
    thread = NULL;

    /* test periodic call-backs */
    thread = concurrent_install_periodic_cb(1000, simple_cb, NULL);
    concurrent_util_suspend(10);
    concurrent_uninstall_cb(thread);
    fprintf(stdout, "TEST THREAD finished >>>>>>>>>>\n");
}

/**
 * Function that is called by a concurrent thread which
 * locks and unlocks a concurrent mutex.
 *
 * @param cb_data argument to be passed to this function.
 * @param thread_id thread identification which calls this function.
 */
static void *
mutex_cb(void *cb_data, size_t * thread_id) {
    size_t * mutex = (size_t *)cb_data;

    if (concurrent_lock_mutex(mutex)) {
        fprintf(stdout, "thread %lu has locked mutex %lu!\n", *thread_id, *mutex);

        concurrent_util_suspend(5);

        if(concurrent_unlock_mutex(mutex)) {
            fprintf(stdout, "thread %lu released mutex %lu\n", *thread_id, *mutex);
        }
    }

    return NULL;
}

/**
 * Test concurrent mutex functionality.
 */
static void
test_mutex() {
    size_t * mutex;
    size_t * thread1;
    size_t * thread2;

    fprintf(stdout, "TEST MUTEX started >>>>>>>>>>\n");
    mutex = concurrent_create_mutex();
    thread1 = concurrent_install_cb(1000, mutex_cb, mutex);
    thread2 = concurrent_install_cb(1000, mutex_cb, mutex);

    concurrent_util_suspend(15);

    concurrent_uninstall_cb(thread1);
    concurrent_uninstall_cb(thread2);
    concurrent_destroy_mutex(mutex);
    fprintf(stdout, "TEST MUTEX finished >>>>>>>>>>\n");
}

/**
 * Function that is called by a concurrent thread which waits for a concurrent signal.
 *
 * @param client_data argument to be passed to this function.
 * @param thread_id thread identification which calls this function.
 */
static void *
broadcast_signal_cb(void *cb_data, size_t * thread_id) {
    concurrent_signal * signal = (concurrent_signal *)cb_data;

    fprintf(stdout, "thread %u trying to wait for signal %u ...\n", *thread_id, *(signal->condvar));

    if (concurrent_util_wait_signal(signal)) {
    	fprintf(stdout, "thread %u got signal %u!\n", *thread_id, *(signal->condvar));
    } else {
    	fprintf(stderr, "thread %u failed to wait for signal %u!\n", *thread_id, *(signal->condvar));
    }

    return NULL;
}

/**
 * Test concurrent signal broadcast functionality.
 */
static void
test_broadcast_signal() {
    concurrent_signal signal;
	size_t * thread1;
    size_t * thread2;
    size_t * thread3;
    size_t * thread4;
    size_t * thread5;

    fprintf(stdout, "TEST BROADCAST SIGNAL started >>>>>>>>>>\n");

    concurrent_util_init_signal(&signal);
    thread1 = concurrent_install_cb(1000, broadcast_signal_cb, &signal);
    thread2 = concurrent_install_cb(1000, broadcast_signal_cb, &signal);
    thread3 = concurrent_install_cb(1000, broadcast_signal_cb, &signal);
    thread4 = concurrent_install_cb(1000, broadcast_signal_cb, &signal);
    thread5 = concurrent_install_cb(1000, broadcast_signal_cb, &signal);

    concurrent_util_suspend(2);
    concurrent_util_send_signal(&signal);
    concurrent_util_suspend(2);
    concurrent_util_send_signal(&signal);
    concurrent_util_suspend(2);
    concurrent_util_broadcast_signal(&signal);
    concurrent_util_suspend(2);

    concurrent_uninstall_cb(thread1);
    concurrent_uninstall_cb(thread2);
    concurrent_uninstall_cb(thread3);
    concurrent_uninstall_cb(thread4);
    concurrent_uninstall_cb(thread5);
    concurrent_util_destroy_signal(&signal);
    fprintf(stdout, "TEST BROADCAST SIGNAL finished >>>>>>>>>>\n");
}

/**
 * Function that is called by a concurrent thread which waits for a concurrent signal
 * for 5 seconds.
 *
 * @param client_data argument to be passed to this function.
 * @param thread_id thread identification which calls this function.
 */
static void *
timedwait_signal_cb(void *cb_data, size_t * thread_id) {
    int result;
    concurrent_signal * signal = (concurrent_signal *)cb_data;

    fprintf(stdout, "thread %u trying to wait for signal %u for 5000 miliseconds ...\n",
	    *thread_id, *(signal->condvar));

    result = concurrent_util_timedwait_signal(signal, 5000L);
    if (result > 0) {
        fprintf(stdout, "thread %u got signal %u!\n", *thread_id, *(signal->condvar));
    } else if (result == 0) {
    	fprintf(stdout, "thread %u signal %u timed out\n", *thread_id, *(signal->condvar));
    } else {
    	fprintf(stderr, "thread %u failed to wait for signal %u!\n", *thread_id, *(signal->condvar));
    }

    return NULL;
}

/**
 * Test concurrent timed signal functionality.
 */
static void
test_timedwait_signal() {
    concurrent_signal signal;
    size_t * thread1;
    size_t * thread2;

    fprintf(stdout, "TEST TIMED WAIT SIGNAL started >>>>>>>>>>\n");

    concurrent_util_init_signal(&signal);
    thread1 = concurrent_install_cb(1000, timedwait_signal_cb, &signal);
    concurrent_util_suspend(7);
    thread2 = concurrent_install_cb(1000, timedwait_signal_cb, &signal);
    concurrent_util_suspend(3);
    concurrent_util_send_signal(&signal);
    concurrent_util_suspend(1);
    concurrent_uninstall_cb(thread1);
    concurrent_uninstall_cb(thread2);

    concurrent_util_destroy_signal(&signal);
    fprintf(stdout, "TEST TIMED WAIT SIGNAL finished >>>>>>>>>>\n");
}

/**
 * Performa all the tests.
 */
int main(int argc, char ** argv) {
    concurrent_init();

    test_mutex();
    test_thread();
    test_concurrentqueue();
    test_concurrentworker();
    test_concurrentpool();
    test_broadcast_signal();
    test_timedwait_signal();

    concurrent_shutdown();

    return 0;
}
