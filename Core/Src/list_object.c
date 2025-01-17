

#include "main.h"
#include "list_object.h"
#include "string.h"


uint8_t wts_Queue_Reset(wts_QueueElement* queue)
{
  queue->front = queue->rear = 0;
  return 1;
}

// Function to check if the queue is empty
uint8_t wts_Queue_isEmpty(wts_QueueElement* queue)
{
    return queue->front == queue->rear;
}

// Function to check if the queue is full
uint8_t wts_Queue_isFull(wts_QueueElement* queue)
{
    return (queue->rear + 1) % queue->maxSize == queue->front;
}

// Function to access an element by index
uint8_t wts_Queue_accessByIndex(wts_QueueElement *queue, int index, void *output)
{
    // Validate the index is within bounds of current elements in the queue
    int count = (queue->rear + queue->maxSize - queue->front) % queue->maxSize;
    if (index < 0 || index >= count)
    {
//        printf("%s: Index out of bounds\n", __func__);
        return 0; // False, invalid index
    }

    // Calculate the actual position in the circular buffer
    int position = (queue->front + index) % queue->maxSize;
    memcpy(output, queue->data + (position * queue->dataSize), queue->dataSize);
    return 1; // True, access successful
}

void* wts_Queue_getPointerByIndex(wts_QueueElement* queue, int index)
{
    // Calculate the current number of elements in the queue
    int count = (queue->rear + queue->maxSize - queue->front) % queue->maxSize;

    // Validate the index
    if (index < 0 || index >= count) {
        return NULL; // Invalid index
    }

    // Calculate the actual position in the circular buffer
    int position = (queue->front + index) % queue->maxSize;

    // Return the pointer to the element
    return queue->data + (position * queue->dataSize);
}

// Function to dequeue by index
uint8_t wts_Queue_dequeueByIndex(wts_QueueElement* queue, int index, void* output)
{
    // Calculate the current number of elements in the queue
    int count = (queue->rear + queue->maxSize - queue->front) % queue->maxSize;

    // Validate the index
    if (index < 0 || index >= count) {
        // Index out of bounds
        return 0; // False, invalid index
    }

    // Calculate the actual position in the circular buffer
    int position = (queue->front + index) % queue->maxSize;

    if(output != NULL)
    {
      // Copy the data to the output
      memcpy(output, queue->data + (position * queue->dataSize), queue->dataSize);
    }

    // Shift elements after the dequeued element to the left
    for (int i = position; i != queue->rear; i = (i + 1) % queue->maxSize) {
        int next = (i + 1) % queue->maxSize;
        memcpy(queue->data + (i * queue->dataSize), queue->data + (next * queue->dataSize), queue->dataSize);
    }

    // Adjust the rear pointer
    queue->rear = (queue->rear + queue->maxSize - 1) % queue->maxSize;

    return 1; // True, operation successful
}

size_t wts_Queue_count(wts_QueueElement *queue)
{
    return (queue->rear + queue->maxSize - queue->front) % queue->maxSize;
}

// Function to wts_Queue_enqueue data
uint8_t wts_Queue_enqueue(wts_QueueElement* queue, void* data)
{
    if (wts_Queue_isFull(queue))
    {
//        printf("wts_QueueElement is full!\n");
        return 0;
    }

    memcpy(queue->data + (queue->rear * queue->dataSize), data, queue->dataSize);
    queue->rear = (queue->rear + 1) % queue->maxSize;
    return 1;
}

// Function to wts_Queue_dequeue data
uint8_t wts_Queue_dequeue(wts_QueueElement* queue, void* data)
{
    if (wts_Queue_isEmpty(queue))
    {
//        printf("wts_QueueElement is empty!\n");
        return 0;
    }

    if(data != NULL)
      memcpy(data, queue->data + (queue->front * queue->dataSize), queue->dataSize);

    queue->front = (queue->front + 1) % queue->maxSize;
    return 1;
}

// Function to print the queue
void wts_Queue_printall(wts_QueueElement* queue)
{
    int i = queue->front;
    while (i != queue->rear)
    {
        queue->printFunc(queue->data + (i * queue->dataSize));
        i = (i + 1) % queue->maxSize;
    }
}


