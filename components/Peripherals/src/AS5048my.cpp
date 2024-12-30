#include "AS5048my.h"
#include "math.h"
#include "USBcom.h"


// esp_timer_handle_t timer_handle;



extern USBDevice usb_ ;
// #define MISO_PIN 12  // Replace with your chosen MISO pin
// #define MOSI_PIN 11  // Replace with your chosen MOSI pin
// #define SCK_PIN 13   // Replace with your chosen SCK pin
// #define CS_PIN 10   // Replace with your chosen CS pin

//  SPI.begin(SCK_PIN, MISO_PIN,MOSI_PIN, CS_PIN); // these pins are differ

AS5048::AS5048(SPI& spi , int csPin,uint32_t clock_speed_hz, int spi_mode ) :_csPin(csPin), angle(0.0), velocity(0.0),prevVelocity(0.0) ,smoothVelocity(0) ,smmothingVelocityFactor(0.5), spi_(spi)
{
   
    uint16_t command = 0x3FFF; // = 0x3FFF; ///this is for testing purposes only, to send 'read command' to encoder

    this->tx_data_[0] = (command >> 8) & 0xFF; // High byte
    this->tx_data_[1] = command & 0xFF;        // Low byte

    this->rx_data_[0]=0;
    this->rx_data_[1]=0;

    this->spi_.attachDevice(clock_speed_hz, &this->spi_encoder_handle, csPin, 1);
        // ESP_LOGI(TAG, "Encoder initialized on CS pin %d.", csPin);
       
   
   
    // Create a mutex for thread-safe access to shared variables
    angleMutex = xSemaphoreCreateMutex();



    this->previousAngle = this->getAngle();

    this->startTask();
}




// Creates and starts a FreeRTOS task for background angle updates
void AS5048::startTask()
{
    // Creates a task pinned to Core 0
//    if( xTaskCreatePinnedToCore(angleTask, "AngleTask", 4048, this, 3, NULL, 1) == pdTRUE){

//     ESP_LOGI("AngleTask created", "");
//    }


    if( xTaskCreatePinnedToCore(angleTaskTest, "angleTaskTest", 8096,this, 4,  &this->encoderTaskHandle, 0) == pdTRUE){

    ESP_LOGI("AngleTask created", "");
   }
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
        // int64_t start_time =esp_timer_get_time();

        // Read the current angle and update velocity
        sensor->readAngle();

        //  ESP_LOGI("Angle read ","OK");

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

            // sensor->multiTurnAngle = sensor->angle + (sensor->turnCount * M_TWOPI);
             // I will call updateMultiTurn instead:

            sensor->updateMultiTurnAngle(  sensor->angle + (sensor->turnCount * M_TWOPI) );

            // ESP_LOGI("Updated multiturn ","OK");


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

                sensor->velocity = 1000000 * cumulativeAngle / cumulativeTime ; // [rad/s]  Convert µs to seconds
                
                sensor->smoothVelocity = sensor->smmothingVelocityFactor * sensor->velocity + (1-sensor->smmothingVelocityFactor)*sensor->prevVelocity;
                
                sensor->prevVelocity = sensor->smoothVelocity;
                xSemaphoreGive(sensor->angleMutex);
            }

            // Reset accumulators
            cumulativeAngle = 0.0;
            cumulativeTime = 0;
            sampleCounter = 0;
            }

           
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


// Reads the raw angle from the encoder and updates the `angle` variable
void AS5048::readAngle()
{
    
    spi_.transfer(this->tx_data_, this->rx_data_, 2, &this->spi_encoder_handle);
    uint16_t received_value = (this->rx_data_[0] << 8) | this->rx_data_[1]; // combaine to get 16 bits value
    received_value =received_value & 0x3FFF;  // Get a 14-bit value

    float newAngle1 = ((float)received_value / 16384.0) * M_TWOPI; 

   
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    angle = newAngle1;
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

    ESP_LOGI("AS5048", "SUM");

   
    float sum = 0.0;
   
    for (int i = 0; i < bufferCount; i++) {

        


        sum += this->multiTurnBuffer[i];

        
    }
    float mean = (bufferCount > 0) ? (sum / bufferCount) : 0.0;

    xSemaphoreGive(angleMutex);
  
    return mean;
}





void AS5048::updateMultiTurnAngle(float newAngle) {

    // xSemaphoreTake(angleMutex, portMAX_DELAY);


    multiTurnBuffer[bufferIndex] = newAngle;
    bufferIndex = (bufferIndex + 1) % 5; // Move to the next index
    if (bufferCount < 5) {
        bufferCount++; // Increase count until buffer is full
    }
    //  xSemaphoreGive(angleMutex);
}



void AS5048::resetMultiTurnAngle()
{
    xSemaphoreTake(angleMutex, portMAX_DELAY);
    multiTurnAngle = this->getAngle();
    this->turnCount=0;
    xSemaphoreGive(angleMutex);
    
}




void AS5048::angleTaskTest(void *pvParameters)
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

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        //  ESP_LOGI("Angle task notified","d");
        // int64_t start_time =esp_timer_get_time();

        // Read the current angle and update velocity
        sensor->readAngle();

        //  ESP_LOGI("Angle read ","OK");

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

            // sensor->multiTurnAngle = sensor->angle + (sensor->turnCount * M_TWOPI);
             // I will call updateMultiTurn instead:

            sensor->updateMultiTurnAngle(  sensor->angle + (sensor->turnCount * M_TWOPI) );

            // ESP_LOGI("Updated multiturn ","OK");


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

                sensor->velocity = 1000000 * cumulativeAngle / cumulativeTime ; // [rad/s]  Convert µs to seconds
                
                sensor->smoothVelocity = sensor->smmothingVelocityFactor * sensor->velocity + (1-sensor->smmothingVelocityFactor)*sensor->prevVelocity;
                
                sensor->prevVelocity = sensor->smoothVelocity;
                xSemaphoreGive(sensor->angleMutex);
            }

            // Reset accumulators
            cumulativeAngle = 0.0;
            cumulativeTime = 0;
            sampleCounter = 0;
            }

           
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
