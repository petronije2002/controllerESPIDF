#ifndef MESSAGE_DEFINITIONS_H
#define MESSAGE_DEFINITIONS_H

// Define your Message struct
typedef enum {
    MESSAGE_TYPE_COMMAND,
    MESSAGE_TYPE_FEEDBACK,
    MESSAGE_TYPE_ERROR
}  MessageType;

// Struct to define the message format
typedef struct {
    MessageType type; // Type of the message
    int id;       // ID to distinguish subtypes or instances
    union {
        struct { // Command message
            float position;
            float velocity;
        } commandData;

        struct { // Feedback message
            float currentPosition;
            float currentVelocity;
            float torque;
        } feedbackData;

        struct { // Error message
            int errorCode;
            char errorMessage[100]; // Error message, can hold up to 100 characters
        } errorData;
    } payload;
} Message;


#endif // MESSAGE_DEFINITIONS_H
