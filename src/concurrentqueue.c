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
 * Implementation of a blocking queue.
 */

#include "concurrentqueue.h"
#include "concurrent.h"
#include "stdlib_util.h"

/**
 * Test whether the provided concurrentqueue structure is initialized or not.
 *
 * @param queue concurrentqueue structure.
 *
 * @return Non-zero if concurrentqueue structure is initialized, zero otherwise.
 */
static int
concurrentqueue_initialized(concurrentqueue *queue) {
    int result = 0;

    if (queue != NULL
    && queue->capacity > 0
    && queue->items != NULL
    && queue->condvar != NULL
    && queue->mutex != NULL) {
        result++;
    }

    return result;
}

/**
 * Initialize the concurrent queue.
 *
 * @param queue concurrentqueue structure.
 * @param capacity maximum size of the concurrent queue.
 */
extern void
concurrentqueue_init(concurrentqueue *queue, size_t capacity) {
    if (queue != NULL && capacity > 0) {
        queue->capacity = capacity;
        queue->size = 0;
        queue->head_index = 0;
        queue->tail_index = 0;
        queue->mutex = concurrent_create_mutex();
        queue->condvar = concurrent_create_condvar();
        queue->items = (void **)SAFE_MALLOC(sizeof(void *) * capacity);
    }
}

/**
 * Get an item from the queue. It will wait (blocking the calling thread),
 * until at least one item is available within the queue. As soon as an
 * item from the queue is returned, it's storage location will be
 * used to store a new item.
 *
 * @param queue concurrentqueue structure.
 *
 * @return reference to item from the queue.
 */
extern void *
concurrentqueue_get_item(concurrentqueue *queue) {
    void * item = NULL;

    if (!concurrentqueue_initialized(queue)) {
        return (item);
    }

    concurrent_lock_mutex(queue->mutex); /* lock queue mutex */

    while(!queue->size) {
        concurrent_wait_condvar(queue->condvar, queue->mutex); /* wait until there are items */
    }

    /* calling thread has mutex lock and
       there is at least one item in the queue */
    item = queue->items[queue->head_index];
    queue->items[queue->head_index] = NULL;
    if (queue->head_index == (queue->capacity - 1)) {
        queue->head_index = 0;
    } else {
        queue->head_index = queue->head_index + 1;
    }

    queue->size = queue->size - 1;
    if (queue->size == queue->capacity - 1) {
        concurrent_condvar_signal(queue->condvar); /* send signal that there is space to add items */
    }

    concurrent_unlock_mutex(queue->mutex); /* un-lock queue mutex */

    return item;
}

/**
 * Add an item in the queue. It will wait (blocking the calling thread),
 * until there is space for at least one item within the queue.
 *
 * @param queue concurrentqueue structure.
 * @param item item to add in the queue.
 *
 * @return Non-zero upon successful completion, zero otherwise.
 */
extern int
concurrentqueue_put_item(concurrentqueue *queue, void * item) {
    if (!concurrentqueue_initialized(queue)) {
        return (0);
    }
    concurrent_lock_mutex(queue->mutex); /* lock queue mutex */

    while(queue->size == queue->capacity) {
        concurrent_wait_condvar(queue->condvar, queue->mutex); /* wait until there is space */
    }

    queue->items[queue->tail_index] = item;
    if (queue->tail_index == (queue->capacity - 1)) {
        queue->tail_index = 0;
    } else {
        queue->tail_index = queue->tail_index + 1;
    }

    queue->size = queue->size + 1;
    if (queue->size == 1) { /* only true if previously was 0 */
        concurrent_condvar_signal(queue->condvar); /* send signal that there are items */
    }

    concurrent_unlock_mutex(queue->mutex); /* un-lock queue mutex */

    return (1);
}

/**
 * Tests whether the provided queue is empty.
 *
 * @param queue concurrentqueue structure.
 *
 * @return Non-zero if queue is empty, zero otherwise.
 */
extern int
concurrentqueue_empty(concurrentqueue *queue) {
    int size = 0;

    if (concurrentqueue_initialized(queue)) {
        concurrent_lock_mutex(queue->mutex); /* lock queue mutex */
        size = queue->size;
        concurrent_unlock_mutex(queue->mutex); /* un-lock queue mutex */
    }

    return (size == 0);
}

/**
 * Destroy the concurrent queue.
 *
 * @param queue concurrentqueue structure.
 */
extern void
concurrentqueue_destroy(concurrentqueue *queue) {
    if (concurrentqueue_initialized(queue)) {
        queue->capacity = 0;
        queue->size = 0;
        queue->head_index = 0;
        queue->tail_index = 0;
        concurrent_destroy_mutex(queue->mutex);
        concurrent_destroy_condvar(queue->condvar);
        SAFE_FREE(queue->items);
    }
}
