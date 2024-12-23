#include "AS5048my.h"
#include "math.h"


// #define MISO_PIN 12  // Replace with your chosen MISO pin
// #define MOSI_PIN 11  // Replace with your chosen MOSI pin
// #define SCK_PIN 13   // Replace with your chosen SCK pin
// #define CS_PIN 10   // Replace with your chosen CS pin

//  SPI.begin(SCK_PIN, MISO_PIN,MOSI_PIN, CS_PIN); // these pins are differ

AS5048::AS5048(SPI& spi , int csPin,uint32_t clock_speed_hz, int spi_mode ) :_csPin(csPin), angle(0.0), velocity(0.0),prevVelocity(0.0) ,smoothVelocity(0) ,smmothingVelocityFactor(0.5), spi_(spi)
{
   
    

    spi_.attachDevice(clock_speed_hz, &spi_encoder_handle, csPin, 1);
        // ESP_LOGI(TAG, "Encoder initialized on CS pin %d.", csPin);
    
   
   
    // Create a mutex for thread-safe access to shared variables
    angleMutex = xSemaphoreCreateMutex();
}

// Initializes the encoder
void AS5048::begin()
{
    // Configure the CS pin as output
   
    // digitalWrite(_csPin, HIGH); // Deselect the encoder initially
    
    // Initialize SPI communication
    // SPI.begin(SCK_PIN, MISO_PIN,MOSI_PIN, CS_PIN); // these pins are differences
    
    // Record the starting time
    // lastTime = micros();

    // Start the background task to read angle and velocity
    // startTask();

    // Initialize the previous angle
    // previousAngle = getAngle();
}

// Creates and starts a FreeRTOS task for background angle updates
void AS5048::startTask()
{
    // Creates a task pinned to Core 0
    xTaskCreatePinnedToCore(angleTask, "AngleTask", 2048, this, 4, NULL, 0);
}


void AS5048::angleTask(void *pvParameters)
{
    // Cast the parameter to the AS5048 object
    AS5048 *sensor = (AS5048 *)pvParameters;
    sensor->lastTime = esp_timer_get_time();

    const int speedCalcRate = 8;  // Calculate speed every 8 samples (1 kHz if sampling at 8 kHz)
    int sampleCounter = 0;       // Counter for angle samples
    float cumulativeAngle = 0.0; // Accumulated angle difference
    unsigned long cumulativeTime = 0; // Accumulated time

    while (true)
    {
        // Read the current angle and update velocity
        sensor->readAngle();

        // Calculate the elapsed time since the last reading
        unsigned long timeDiff = sensor->safeMicros(sensor->lastTime);

        if (timeDiff > 0)
        {
            // Ensure thread-safe access to shared variables
            xSemaphoreTake(sensor->angleMutex, portMAX_DELAY);

            // Calculate the difference in angle
            float angleDifference = sensor->angle - sensor->previousAngle;

            // Handle angle wrapping (e.g., crossing 0 or 360 degrees)
            if (angleDifference > M_PI){
                angleDifference -= M_TWOPI; // Wrap down 
                sensor->turnCount--;     
            }
                
                
            else if (angleDifference < -M_PI){

                angleDifference += M_TWOPI; // Wrap up
                sensor->turnCount++;  
            }

            sensor->previousAngle = sensor->angle;
            sensor->lastTime = esp_timer_get_time();
            sensor->multiTurnAngle = sensor->angle + (sensor->turnCount * M_TWOPI);

            cumulativeAngle += angleDifference;
            cumulativeTime += timeDiff;
            
            xSemaphoreGive(sensor->angleMutex);
              

            // Calculate velocity in degrees per second (x10^6 to get in sec)
            // since timeDiff is in microseconds. 
            // sensor->velocity = 1000000 * angleDifference / timeDiff;

            sampleCounter++;


            if (sampleCounter >= speedCalcRate) {
            if (cumulativeTime > 0) {
                xSemaphoreTake(sensor->angleMutex, portMAX_DELAY);
                // Calculate speed in degrees/second
                sensor->velocity = 1000000 * cumulativeAngle / cumulativeTime ; // [deg/s]  Convert µs to seconds
                
                sensor->smoothVelocity = sensor->smmothingVelocityFactor * sensor->velocity + (1-sensor->smmothingVelocityFactor)*sensor->prevVelocity;
                
                sensor->prevVelocity = sensor->smoothVelocity;
                xSemaphoreGive(sensor->angleMutex);
            }

            // Reset accumulators
            cumulativeAngle = 0.0;
            cumulativeTime = 0;
            sampleCounter = 0;
            }

            // Update the previous angle and time
            // sensor->previousAngle = sensor->angle;
            // sensor->lastTime = micros();
            // sensor->multiTurnAngle = sensor->angle + (sensor->turnCount * 360.0f);

            // Release the mutex
            // xSemaphoreGive(sensor->angleMutex);
        }

        
        // I set 8000 Hz for configTICK_RATE_HZ in FreeRTOSconfig.h, that means, it reads at freq of 8kHz
        vTaskDelay(1);
    }
}
// Static task function for updating the angle in the background
// void AS5048::angleTask(void *pvParameters)
// {
//     // Cast the parameter to the AS5048 object
//     AS5048 *sensor = (AS5048 *)pvParameters;
//     sensor->lastTime = micros();

//     const int speedCalcRate = 8;  // Calculate speed every 8 samples (1 kHz if sampling at 8 kHz)
//     int sampleCounter = 0;       // Counter for angle samples
//     float cumulativeAngle = 0.0; // Accumulated angle difference
//     unsigned long cumulativeTime = 0; // Accumulated time

//     while (true)
//     {
//         // Read the current angle and update velocity
//         sensor->readAngle();

//         // Calculate the elapsed time since the last reading
//         unsigned long timeDiff = sensor->safeMicros(sensor->lastTime);

//         if (timeDiff > 0)
//         {
//             // Ensure thread-safe access to shared variables
//             xSemaphoreTake(sensor->angleMutex, portMAX_DELAY);

//             // Calculate the difference in angle
//             float angleDifference = sensor->angle - sensor->previousAngle;

//             // Handle angle wrapping (e.g., crossing 0 or 360 degrees)
//             if (angleDifference > 180.0f){
//                 angleDifference -= 360.0f; // Wrap down 
//                 sensor->turnCount--;     
//             }
                
                
//             else if (angleDifference < -180.0f){

//                 angleDifference += 360.0f; // Wrap up
//                 sensor->turnCount++;  
//             }
            
              

//             // Calculate velocity in degrees per second (x10^6 to get in sec)
//             // since timeDiff is in microseconds. 
//             sensor->velocity = 1000000 * angleDifference / timeDiff;

//             // Update the previous angle and time
//             sensor->previousAngle = sensor->angle;
//             sensor->lastTime = micros();
//             sensor->multiTurnAngle = sensor->angle + (sensor->turnCount * 360.0f);

//             // Release the mutex
//             xSemaphoreGive(sensor->angleMutex);
//         }

        
//         // I set 8000 Hz for configTICK_RATE_HZ in FreeRTOSconfig.h, that means, it reads at freq of 8kHz
//         vTaskDelay(1);
//     }
// }

// Reads the raw angle from the encoder and updates the `angle` variable
void AS5048::readAngle()
{
    uint16_t command = 0x3FFF; // Command to read the angle

    // Begin SPI communication with the encoder
    // SPI.begin(SCK_PIN, MISO_PIN,MOSI_PIN, CS_PIN); // these pins are differences
    // SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE1));

    // digitalWrite(_csPin, LOW);       // Select the encoder
    // SPI.transfer16(command);        // Send the read command
    // digitalWrite(_csPin, HIGH);     // Deselect the encoder

    // digitalWrite(_csPin, LOW);      // Select again to receive data
    // rawAngle = SPI.transfer16(0x0000); // Read the response
    // digitalWrite(_csPin, HIGH);     // Deselect the encoder
    // SPI.endTransaction();
    uint8_t tx_data[2] = {0x3F, 0xFF};  // Command to read position
    uint8_t rx_data[2] = {0};           // Buffer for received data

    uint16_t rawAngle = spi_.transfer(tx_data, rx_data, 2, spi_encoder_handle);

    // Mask the top 2 bits (PAR and EF) and convert to degrees
    rawAngle &= 0x3FFF;

    // float newAngle = (float)rawAngle / 16384.0 * 360.0;
    //instead, I lower the resolution to 12 bits, because of noise
    float newAngle = ((float)rawAngle / 16384.0) * M_TWOPI; 

    // Update the shared `angle` variable safely
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    angle = newAngle;
    xSemaphoreGive(angleMutex);
}

// Returns the current angle (thread-safe)
float AS5048::getAngle()
{
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    float currentAngle = angle;
    xSemaphoreGive(angleMutex);
    return currentAngle;
}

// Returns the current velocity (thread-safe)
float AS5048::getVelocity()
{
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    float currentVelocity = smoothVelocity;
    xSemaphoreGive(angleMutex);
    return currentVelocity;
}

// Handles micros() overflow safely
unsigned long AS5048::safeMicros(unsigned long lastTime)
{
    unsigned long currentTime = esp_timer_get_time();
    if (currentTime < lastTime)
    {
        // Handle overflow: current time wrapped around
        return (0xFFFFFFFF - lastTime) + currentTime;
    }
    else
    {
        // No overflow: regular difference
        return currentTime - lastTime;
    }
}

float AS5048::getMultiTurnAngle()
{
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    float currentMultiTurnAngle = multiTurnAngle;
    xSemaphoreGive(angleMutex);
    return currentMultiTurnAngle;
}



void AS5048::resetMultiTurnAngle()
{
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    float currentMultiTurnAngle = this->getAngle();
    this->turnCount=0;
    xSemaphoreGive(angleMutex);
    
}
