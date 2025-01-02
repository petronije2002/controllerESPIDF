
#include "ProfileGenerator.h"
#include <cmath>
// #include <iostream>

// float a = 1;
// float c = 30;
// float c2 = 0.15;

// float angleToGo = 1;

// float max_velocity = 4;

// float acceleration_time = c2 * 2;
// float deceleration_time = acceleration_time;
// float constVelocity_time = 0;
// float constAngle = 0;



float ProfileGenerator::calculateDesiredPositionAtT(float current_time, float prevVel, float step_size_ad)
{

   // float currentAngle = step_size_ad * (prevVel + sigmoid(current_time)) / 2;
        float currentAngle = step_size_ad * (prevVel + sigmoidLUp(current_time)) / 2;


    return currentAngle;
};

// Constructor
ProfileGenerator::ProfileGenerator(float maxVelocity, float maxAcceleration,float currentPosition, int lookupTableSize )
    : maxVelocity(maxVelocity), maxAcceleration(maxAcceleration), totalDistance(0.0f), lookupTableSize(lookupTableSize) {


        sigmoidMin = 0.0;
        sigmoidMax = accelerationTime;

        sigmoidStep = (sigmoidMax - sigmoidMin) / this->lookupTableSize;


        positionProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f); // All positions start at initialPosition
        velocityProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);           // All velocities are zero
        currentTime = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);  


        initializeSigmoidLookupTable();
    }




// Sigmoid function to create an S-curve
float ProfileGenerator::sigmoid(float x) const
{
    return 1 / (1 + exp(-c * (x - c2)));
}

// Calculate the commanded velocity for the current time
float ProfileGenerator::calculateCommandedVelocity(float t, float commanded_velocity, float constVelocity_time)
{
    float v_cmd = commanded_velocity;

    if (t <= accelerationTime)
    {
        v_cmd = commanded_velocity * sigmoidLUp(t);
    }
    if (t > accelerationTime && t <= constVelocity_time + accelerationTime)
    {
        v_cmd = commanded_velocity;
    }
    if (t > (constVelocity_time + accelerationTime))
    {
        v_cmd = commanded_velocity - commanded_velocity * sigmoidLUp(t - accelerationTime - constVelocity_time);
    }

    return v_cmd;
}

// Generate the angle and velocity values
void ProfileGenerator::generateAngleAndVelocityValues(float angleToGo, float commanded_velocity)
{
    // Compute total time based on commanded velocity
    float total_time = angleToGo / commanded_velocity;

    // Calculate time spent at constant velocity
    float constVelocity_time = total_time - 2 * accelerationTime;

    if (constVelocity_time <= 0)
    {
        // If no constant velocity phase, recalculate commanded velocity to fit the acceleration phase
        commanded_velocity = 2 * (accelerationTime) / angleToGo;
        total_time = angleToGo / commanded_velocity;
        constVelocity_time = 0;
    }

    // Initialize vectors to store angle and velocity values
    // positionProfile.clear();
    // velocityProfile.clear();
    // currentTime.clear();

    angles.clear();

    // Calculate step size for each phase
    // num_accel_segments = 15; // Example value for acceleration segments
    // int num_const_segments = 3;  // Example value for constant velocity segments
    float step_size_ad = accelerationTime / (float)(num_accel_segments);
    float step_size_const = constVelocity_time / (float)(num_const_segments);

    // positionProfile.resize(num_accel_segments * 2 + num_const_segments + 1 );
    // velocityProfile.resize(num_accel_segments * 2 + num_const_segments +1 );
    // currentTime.resize(num_accel_segments * 2 + num_const_segments +1  );

    // Acceleration Phase
    float current_time = 0;

    for (int i = 1; i <= num_accel_segments * 2 + num_const_segments ; ++i)
    {

        if (i <= num_accel_segments)
        {

            current_time = i * step_size_ad;

            currentTime[i] = current_time;


            // float angle = positionProfile.empty() ? 0 : positionProfile.back() + current_time * calculateCommandedVelocity(current_time, commanded_velocity, acceleration_time, constVelocity_time, c, c2);

            if (i == 1)
            {

                positionProfile[i] = calculateDesiredPositionAtT(current_time, 0, step_size_ad);
                velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);

                 continue;
            }
            else
            {

                positionProfile[i] = positionProfile[i - 1] + calculateDesiredPositionAtT(current_time, float(velocityProfile[i - 1]), step_size_ad);

                if (i == num_accel_segments)
                {
                    constAngle = angleToGo - 2 * positionProfile[i];

                    step_size_const = (constAngle / commanded_velocity) / num_const_segments;

                    if (constVelocity_time == 0)
                    {

                        constVelocity_time = 0;
                    }
                    else
                    {
                        constVelocity_time = 0;
                        constVelocity_time = constAngle / commanded_velocity;
                    }
                }

                velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
            }
        }

        if (i > num_accel_segments && i <= num_accel_segments + num_const_segments)
        {

            current_time = accelerationTime + (i - num_accel_segments) * step_size_const;

            currentTime[i] = current_time;

            positionProfile[i] = positionProfile[i - 1] + commanded_velocity * step_size_const;

            velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
        }

        if (i > num_accel_segments + num_const_segments && i <= 2 * num_accel_segments + num_const_segments)
        {

            current_time = accelerationTime + constVelocity_time + (i - num_accel_segments - num_const_segments) * step_size_ad;
            currentTime[i] = current_time;

            int rel_i = i - num_accel_segments - num_const_segments;

            positionProfile[i] = positionProfile[i - 1] + (positionProfile[num_accel_segments + 1 - rel_i] - positionProfile[num_accel_segments - rel_i]);

            velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
        }
    }
}

// Generate the S-curve profile for the given distance
void ProfileGenerator::generateScurveProfile(float totalDistance, float commandedVelocity)
{
    this->totalDistance = totalDistance;

    // Set up the profile generation parameters
    // float acceleration_time = 0.3f; // Example acceleration time
    // float c = 5.0f;  // Sigmoid parameter for shaping the curve
    // float c2 = 0.5f; // Sigmoid center (midpoint of transition)
    // float commanded_velocity = maxVelocity;  // Example commanded velocity

    // Call the function to generate the angle and velocity values
    generateAngleAndVelocityValues2(totalDistance, commandedVelocity);
}

// Get the position profile
const std::vector<float> &ProfileGenerator::getPositionProfile() const
{
    return positionProfile;
}

// Get the velocity profile
const std::vector<float> &ProfileGenerator::getVelocityProfile() const
{
    return velocityProfile;
}

const std::vector<float> &ProfileGenerator::getTimeProfile() const
{
    return currentTime;
}

// Interpolate a value (used for getting position/velocity at a given angle)
float ProfileGenerator::getInterpolatedValue(float currentAngle, const std::vector<float> &profile, const std::vector<float> &angles) const
{
    
    for (size_t i = 0; i < angles.size() - 1; ++i)
    {
        if (currentAngle >= angles[i] && currentAngle < angles[i + 1])
        {
            float t = (currentAngle - angles[i]) / (angles[i + 1] - angles[i]);
            return profile[i] + t * (profile[i + 1] - profile[i]);
        }
    }
    return 0.0f; // Default return if out of bounds
}

// Get the position for a given angle
float ProfileGenerator::getPositionForAngle(float currentAngle) const
{
    return getInterpolatedValue(currentAngle, positionProfile, positionProfile);
}

// Get the velocity for a given angle
float ProfileGenerator::getVelocityForAngle(float currentAngle) const
{

    return getInterpolatedValue(currentAngle, velocityProfile, positionProfile);
}


float ProfileGenerator::getPositionForTime(float moment_) const
{
    return getInterpolatedValue(moment_, positionProfile, currentTime);
}


float ProfileGenerator::getVelocitynForTime(float moment_) const
{
    return getInterpolatedValue(moment_, velocityProfile, currentTime);
}


void ProfileGenerator::initializeSigmoidLookupTable() {
    sigmoidLookupTable.resize(lookupTableSize+1);
    for (int i = 1; i < lookupTableSize; ++i) {
        float x = sigmoidMin + i * sigmoidStep;
        sigmoidLookupTable[i] =  sigmoid(x);
        //1 / (1 + std::exp(-x));  // Sigmoid formula
    }
}


float ProfileGenerator::sigmoidLUp(float x)  {
    if (x <= sigmoidMin) return 0.0;  // Clamp to min
    if (x >= accelerationTime) return 1.0;   // Clamp to max

    // Compute the index and interpolation factor
    float normalizedX = (x - sigmoidMin) / sigmoidStep;
    int index = static_cast<int>(normalizedX);
    float t = normalizedX - index;  // Fractional part for interpolation

    // Interpolate between adjacent values
    return sigmoidLookupTable[index] + t * (sigmoidLookupTable[index + 1] - sigmoidLookupTable[index]);
}




void ProfileGenerator::generateAngleAndVelocityValues1(float angleToGo, float commanded_velocity)
{
    // Compute total time based on commanded velocity
    float total_time = angleToGo / commanded_velocity;

    // Calculate time spent at constant velocity
    float constVelocity_time = total_time - 2 * accelerationTime;

    if (constVelocity_time <= 0)
    {
        // If no constant velocity phase, recalculate commanded velocity to fit the acceleration phase
        commanded_velocity = 2 * (accelerationTime) / angleToGo;
        total_time = angleToGo / commanded_velocity;
        constVelocity_time = 0;


        positionProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f); // All positions start at initialPosition
        velocityProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);           // All velocities are zero
        currentTime = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);  

    

    }

   
    float step_size_ad = accelerationTime / (float)(num_accel_segments);
    float step_size_const = constVelocity_time / (float)(num_const_segments);

    // Acceleration Phase
    float current_time = 0;

    for (int i = 1; i <= num_accel_segments * 2 + num_const_segments +1 ; ++i)
    {

        if (i <= num_accel_segments)
        {

            current_time = i * step_size_ad;

            currentTime[i] = current_time;

            if (i == 1)
            {

                positionProfile[i] = calculateDesiredPositionAtT(current_time, 0, step_size_ad);
                velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);

                 continue;
            }
            else
            {

                positionProfile[i] = positionProfile[i - 1] + calculateDesiredPositionAtT(current_time, float(velocityProfile[i - 1]), step_size_ad);
                velocityProfile[i] =calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
                
                if (i == num_accel_segments)
                {

                    // velocityProfile[i] = commanded_velocity;

                     velocityProfile[i] =commanded_velocity;

                    constAngle = angleToGo - 2 * positionProfile[i];

                    step_size_const = (constAngle / commanded_velocity) / num_const_segments;


                    if (constVelocity_time == 0)
                    {

                        constVelocity_time = 0;
                    }
                    else
                    {
                        constVelocity_time = 0;
                        constVelocity_time = constAngle / commanded_velocity;
                    }
                }

                // velocityProfile[i] =commanded_velocity;
                //  calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
            }
        }

        if (i > num_accel_segments && i <= (num_accel_segments + num_const_segments) && step_size_const != 0)
        {

            current_time = accelerationTime + (i - num_accel_segments) * step_size_const;

            currentTime[i] = current_time;

            positionProfile[i] = positionProfile[i - 1] + commanded_velocity * step_size_const;

            velocityProfile[i] = commanded_velocity;
            //  calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
        }

        if (i > num_accel_segments + num_const_segments && i <= 2 * num_accel_segments + num_const_segments+1)
        {
            current_time = accelerationTime + constVelocity_time + (i - num_accel_segments - num_const_segments) * step_size_ad;
           
            currentTime[i] = current_time;

            int rel_i = i - num_accel_segments - num_const_segments ;


             positionProfile[i] = angleToGo - positionProfile[num_accel_segments  - rel_i ] ;

             velocityProfile[i] =  velocityProfile[num_accel_segments - rel_i];





            // positionProfile[i] = positionProfile[i - 1] + (positionProfile[num_accel_segments + 1 - rel_i] - positionProfile[num_accel_segments - rel_i]);

            // velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);

            // if(i ==2 * num_accel_segments + num_const_segments +1){
            //      velocityProfile[i]  = 0;



            // }
        }
    }
}



// HERE TRY TO ADJUST CORRECT FUNCTION!!!!!


void ProfileGenerator::generateAngleAndVelocityValues2(float angleToGo, float commanded_velocity)
{
    // Compute total time based on commanded velocity
    float total_time = angleToGo / commanded_velocity;

    // Calculate time spent at constant velocity
    float constVelocity_time = total_time - 2 * accelerationTime;


    float step_size_ad = 0;
    float step_size_const = 0;

    // positionProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f); // All positions start at initialPosition
    // velocityProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);           // All velocities are zero
    // currentTime = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);  
        

    if (constVelocity_time <= 0)
    {
        // If no constant velocity phase, recalculate commanded velocity to fit the acceleration phase
        commanded_velocity = angleToGo/ (2 * (accelerationTime)) ;
        total_time = angleToGo / commanded_velocity;
        constVelocity_time = 0;


       

        // positionProfile.resize(num_accel_segments * 2 + 1 );
        // velocityProfile.resize(num_accel_segments * 2 + 1 );
        // currentTime.resize(num_accel_segments * 2 + 1 );

        step_size_ad = accelerationTime / (float)(num_accel_segments);
    
    }else{

        // positionProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f); // All positions start at initialPosition
        // velocityProfile = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);           // All velocities are zero
        // currentTime = std::vector<float>(num_accel_segments * 2 + num_const_segments +1 , 0.0f);  
    
        // positionProfile.resize(num_accel_segments * 2 + num_const_segments + 1 );
        // velocityProfile.resize(num_accel_segments * 2 + num_const_segments +1 );
        // currentTime.resize(num_accel_segments * 2 + num_const_segments +1  );
        
        
        step_size_ad = accelerationTime / (float)(num_accel_segments);
        step_size_const = constVelocity_time / (float)(num_const_segments);

        //  ESP_LOGW("step size check:", " %f, %d",step_size_ad , positionProfile.size() );
        //  ESP_LOGW("step size check:", " %f",step_size_const );

       

    }


    // Acceleration Phase
    float current_time = 0;

    for (int i = 1; i <= num_accel_segments * 2 + num_const_segments +1 ; ++i)
    {

        if (i <= num_accel_segments )
        {

            current_time = i * step_size_ad;

            currentTime[i] = current_time;

            if (i == 1)
            {

                positionProfile[i] = calculateDesiredPositionAtT(current_time, 0, step_size_ad);
                velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);

                 continue;
            }
            else
            {

                positionProfile[i] = positionProfile[i - 1] + calculateDesiredPositionAtT(current_time, float(velocityProfile[i - 1]), step_size_ad);
                velocityProfile[i] =calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
                
                if (i == num_accel_segments && constVelocity_time != 0)
                {

                    // velocityProfile[i] = commanded_velocity;

                    velocityProfile[i] = commanded_velocity;

                    constAngle = angleToGo - 2 * positionProfile[i];

                    step_size_const = (constAngle / commanded_velocity) / num_const_segments;

                    constVelocity_time = constAngle / commanded_velocity;


                    // if (constVelocity_time == 0)
                    // {

                    //     constVelocity_time = 0;
                    // }
                    // else
                    // {
                    //     constVelocity_time = 0;
                    //     constVelocity_time = constAngle / commanded_velocity;
                    // }
                }else if(i == num_accel_segments && constVelocity_time == 0){



                    velocityProfile[i] = commanded_velocity;
                    constAngle = angleToGo - 2 * positionProfile[i];

                    constVelocity_time = constAngle / commanded_velocity;
                    step_size_const = (constAngle / commanded_velocity) / num_const_segments;
                    // constVelocity_time = 0;

                }
            }
        }

        if (i > num_accel_segments && i <= (num_accel_segments + num_const_segments)  )
        {

            current_time = accelerationTime + (i - num_accel_segments) * step_size_const;

            currentTime[i] = current_time;

            positionProfile[i] = positionProfile[i - 1] + commanded_velocity * step_size_const;

            velocityProfile[i] = commanded_velocity;
            //  calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
        }

        if (i > num_accel_segments + num_const_segments && i <= 2 * num_accel_segments + num_const_segments+1)
        {
            current_time = accelerationTime + constVelocity_time + (i - num_accel_segments - num_const_segments) * step_size_ad;
           
            currentTime[i] = current_time;

            int rel_i = i - num_accel_segments - num_const_segments ;


             positionProfile[i] = angleToGo - positionProfile[num_accel_segments  - rel_i ] ;

             velocityProfile[i] =  velocityProfile[num_accel_segments - rel_i];





            // positionProfile[i] = positionProfile[i - 1] + (positionProfile[num_accel_segments + 1 - rel_i] - positionProfile[num_accel_segments - rel_i]);

            // velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);

            // if(i ==2 * num_accel_segments + num_const_segments +1){
            //      velocityProfile[i]  = 0;



            // }
        }
    }
}
