#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void queue_init(struct queue* queue, size_t element_size, size_t max_elements)
{
    queue->max_elements = max_elements;
    queue->elements_in_use = 0;
    queue->element_size = element_size;
    queue->data = malloc(element_size * max_elements);
    queue->start = queue->data;
    queue->end = queue->start;
}

void queue_enqueue(struct queue* queue, void* value)
{
    assert(queue->elements_in_use < queue->max_elements);
    queue->elements_in_use++;

    memcpy(queue->end, value, queue->element_size);
    queue->end += queue->element_size;

    if (queue->end >
            (queue->data + (queue->element_size * queue->max_elements))) {
        queue->end = queue->data;
    }
}

void queue_dequeue(struct queue* queue, void* value)
{
    assert(queue->elements_in_use > 0);
    queue->elements_in_use--;

    if (queue->start > queue->data + queue->max_elements * queue->element_size)
        queue->start = queue->data;

    queue->start += queue->element_size;

    assert(value);
    memcpy(value, queue->start - queue->element_size, queue->element_size);
}

void queue_destroy(struct queue* queue)
{
    free(queue->data);
}

bool queue_full(struct queue* queue)
{
    return queue->elements_in_use == queue->max_elements;
}

bool queue_empty(struct queue* queue)
{
    return queue->elements_in_use == 0;
}
