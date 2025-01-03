#ifndef AS5048MY_H
#define AS5048MY_H



#include "esp_mac.h"

// #include "SPImaster1.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_timer.h"
#include "SPImaster1.h"

#include "esp_log.h"


/**  Encoder Class Constructor: 
 * @brief 
 * 
 * @param CS [int]  chip selector. This is the GPIO  serving to select SPI chip
 * 
 */
class AS5048
{
public:
    /** @brief  Constructor: Takes the chip-select (CS) pin as input
     *  
     */
    AS5048(SPI& spi , int csPin,uint32_t clock_speed_hz, int spi_mode );
    
    // Initializes the encoder (sets up SPI, GPIO, and starts background task)
   
    // Starts the FreeRTOS task for updating the angle
    void startTask();
    
    // Retrieves the latest angle (thread-safe)
    float getAngle();
    
    // Retrieves the calculated velocity (thread-safe)
    float getVelocity();
    float getMultiTurnAngle();
    void resetMultiTurnAngle();
    void readAngle();


    // int bufferLength = 8;

    float multiTurnBuffer[5]= {0.0,0.0,0.0,0.0,0.0};

    int bufferIndex = 0;

    int bufferCount = 0;

    void updateMultiTurnAngle(float newAngle) ;




    spi_device_handle_t spi_encoder_handle;

    uint8_t tx_data_[2];  // Command to send
    uint8_t rx_data_[2];  // Buffer for received data

    TaskHandle_t encoderTaskHandle = NULL;


    

private:
    int _csPin;                // Chip-select pin for SPI communication
    float angle;               // Current angle in degrees
    float previousAngle;       // Previous angle for velocity calculation
    float velocity; 
    float prevVelocity; 
    float smoothVelocity;
    float smmothingVelocityFactor;          // Velocity in degrees per second
    unsigned long lastTime;    // Last time in microseconds (used for velocity calculation)
    float turnCount = 0;
    float multiTurnAngle;

    SPI &spi_;
    ; 
    
    SemaphoreHandle_t angleMutex; // Mutex for thread-safe access to `angle` and `velocity`

    // Static task function for running in the FreeRTOS task
    static void angleTask(void *pvParameters);

    

    static void angleTaskTest(void *pvParameters);


    
    // Reads and updates the current angle and velocity
    float readAngleTesting();
    
    // Handles micros() overflow safely to calculate elapsed time
    unsigned long safeMicros(unsigned long lastTime);
};

#endif
