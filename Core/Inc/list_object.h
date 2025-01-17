

#ifndef __LIST___OBJECT____H___
#define __LIST___OBJECT____H___

typedef struct
{
    char* data; // Pointer to hold dynamic-sized data array
    int front;
    int rear;
    size_t dataSize;
    size_t maxSize;
    void (*printFunc)(void*);
} wts_QueueElement;

// Macro to simplify queue creation for a specific type
#define DEFINE_WTS_QUEUEOBJECT(name, type, size, printFunction)  \
    char name##_databuffer_wts_queue[size][sizeof(type)];        \
    wts_QueueElement name = {                                    \
        .data = (char *)name##_databuffer_wts_queue,             \
        .front = 0,                                              \
        .rear = 0,                                               \
        .dataSize = sizeof(type),                                \
        .maxSize = size,                                         \
        .printFunc = printFunction                               \
    }

// Returns 1 at Successed operation, fail -> 0(Zero)

uint8_t wts_Queue_Reset(wts_QueueElement* queue);
uint8_t wts_Queue_isEmpty(wts_QueueElement* queue);
uint8_t wts_Queue_isFull(wts_QueueElement* queue);
uint8_t wts_Queue_accessByIndex(wts_QueueElement *queue, int index, void *output);
void* wts_Queue_getPointerByIndex(wts_QueueElement* queue, int index);
uint8_t wts_Queue_dequeueByIndex(wts_QueueElement* queue, int index, void* output);
size_t wts_Queue_count(wts_QueueElement *queue);
uint8_t wts_Queue_enqueue(wts_QueueElement* queue, void* data);
uint8_t wts_Queue_dequeue(wts_QueueElement* queue, void* data);
void wts_Queue_printall(wts_QueueElement* queue);



#endif
