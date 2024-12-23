#ifndef QUEUE_HANDLER_H
#define QUEUE_HANDLER_H

#include "freertos/FreeRTOS.h" 
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "MessageDefinitions.h"
#include "esp_mac.h"
#include "MessageDefinitions.h"


// Forward declaration of Message struct (you can modify it as needed)


// Queue handler class
class QueueHandler {
public:
    QueueHandler();
    ~QueueHandler();
    
    // Create the queues (can be dynamic or static)
    void createQueues();
    
    // Send message to a queue
    bool sendMessageToQueue(QueueHandle_t queue, Message  &msg);
    
    // Receive message from a queue
    bool receiveMessageFromQueue(QueueHandle_t queue, Message* msg, TickType_t waitTime);
    QueueHandle_t commandQueue;
    QueueHandle_t feedbackQueue;
    QueueHandle_t toSerialQueue;
    QueueHandle_t errorQueue;
    SemaphoreHandle_t queueMutex;
    
private:
    // Declare queue handles
   
    
    // Static Queue buffers (if using static queues)
    static StaticQueue_t commandQueueBuffer;
    static StaticQueue_t feedbackQueueBuffer;
    static StaticQueue_t errorQueueBuffer;
    static StaticQueue_t toSerialQueueBuffer;

    static uint8_t commandQueueStorage[10 * sizeof(Message)];
    static uint8_t feedbackQueueStorage[10 * sizeof(Message)];
    static uint8_t errorQueueStorage[10 * sizeof(Message)];
    static uint8_t toSerialQueueStorage[10 * sizeof(Message)];
};

#endif // QUEUE_HANDLER_H
