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

#ifndef CONCURRENTQUEUE_H_
#define CONCURRENTQUEUE_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* Structure to store concurrent queue information. */
typedef struct _concurrentqueue_struct {
    int head_index; /* queue read item index */
    int tail_index; /* queue add item index */
    int capacity; /* queue items capacity */
    int size; /* items counts in the queue */
    size_t * mutex; /* mutex for the queue */
    size_t * condvar; /* thread condition variable for the queue */
    void ** items; /* queue items */
} concurrentqueue;

/**
 * Initialize the concurrent queue.
 *
 * @param queue concurrentqueue structure.
 * @param capacity maximum size of the concurrent queue.
 */
extern void
concurrentqueue_init(concurrentqueue *queue, size_t capacity);

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
concurrentqueue_get_item(concurrentqueue *queue);

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
concurrentqueue_put_item(concurrentqueue *queue, void * item);

/**
 * Tests whether the provided queue is empty.
 *
 * @param queue concurrentqueue structure.
 *
 * @return Non-zero if queue is empty, zero otherwise.
 */
extern int
concurrentqueue_empty(concurrentqueue *queue);

/**
 * Destroy the concurrent queue.
 *
 * @param queue concurrentqueue structure.
 */
extern void
concurrentqueue_destroy(concurrentqueue *queue);

#ifdef  __cplusplus
}
#endif

#endif /* CONCURRENTQUEUE_H_ */
