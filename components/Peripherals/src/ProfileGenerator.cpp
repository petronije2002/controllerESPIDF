
#include "ProfileGenerator.h"
#include <cmath>
// #include <iostream>

// float a = 1;
float c = 30;
float c2 = 0.15;

// float angleToGo = 1;

// float max_velocity = 4;

float acceleration_time = c2 * 2;
float deceleration_time = acceleration_time;
float constVelocity_time = 0;
float constAngle = 0;

float ProfileGenerator::calculateDesiredPositionAtT(float current_time, float prevVel, float step_size_ad)
{

    float currentAngle = step_size_ad * (prevVel + sigmoid(current_time, c, c2)) / 2;

    return currentAngle;
};

// Constructor
ProfileGenerator::ProfileGenerator(float maxVelocity, float maxAcceleration, int numSegments)
    : maxVelocity(maxVelocity), maxAcceleration(maxAcceleration), numSegments(numSegments), totalDistance(0.0f) {}

// Sigmoid function to create an S-curve
float ProfileGenerator::sigmoid(float x, float c, float c2) const
{
    return 1 / (1 + exp(-c * (x - c2)));
}

// Calculate the commanded velocity for the current time
float ProfileGenerator::calculateCommandedVelocity(float t, float commanded_velocity, float constVelocity_time)
{
    float v_cmd = commanded_velocity;

    if (t <= acceleration_time)
    {
        v_cmd = commanded_velocity * sigmoid(t, c, c2);
    }
    if (t > acceleration_time && t <= constVelocity_time + acceleration_time)
    {
        v_cmd = commanded_velocity;
    }
    if (t > (constVelocity_time + acceleration_time))
    {
        v_cmd = commanded_velocity - commanded_velocity * sigmoid(t - acceleration_time - constVelocity_time, c, c2);
    }

    return v_cmd;
}

// Generate the angle and velocity values
void ProfileGenerator::generateAngleAndVelocityValues(float angleToGo, float commanded_velocity)
{
    // Compute total time based on commanded velocity
    float total_time = angleToGo / commanded_velocity;

    // Calculate time spent at constant velocity
    float constVelocity_time = total_time - 2 * acceleration_time;

    if (constVelocity_time <= 0)
    {
        // If no constant velocity phase, recalculate commanded velocity to fit the acceleration phase
        commanded_velocity = 2 * (acceleration_time) / angleToGo;
        total_time = angleToGo / commanded_velocity;
        constVelocity_time = 0;
    }

    // Initialize vectors to store angle and velocity values
    positionProfile.clear();
    velocityProfile.clear();
    currentTime.clear();

    angles.clear();

    // Calculate step size for each phase
    int num_accel_segments = 10; // Example value for acceleration segments
    int num_const_segments = 3;  // Example value for constant velocity segments
    float step_size_ad = acceleration_time / (float)(num_accel_segments);
    float step_size_const = constVelocity_time / (float)(num_const_segments);

    positionProfile.resize(num_accel_segments * 2 + num_const_segments);
    velocityProfile.resize(num_accel_segments * 2 + num_const_segments);
    currentTime.resize(num_accel_segments * 2 + num_const_segments);

    // Acceleration Phase
    float current_time = 0;

    for (int i = 1; i <= num_accel_segments * 2 + num_const_segments; ++i)
    {

        if (i <= num_accel_segments)
        {

            current_time = i * step_size_ad;

            currentTime[i] = current_time;

            // float angle = positionProfile.empty() ? 0 : positionProfile.back() + current_time * calculateCommandedVelocity(current_time, commanded_velocity, acceleration_time, constVelocity_time, c, c2);

            if (i == 1)
            {

                positionProfile[i] = calculateDesiredPositionAtT(current_time, 0, step_size_ad);
                // velocityProfile.push_back( calculateCommandedVelocity(current_time, commanded_velocity, acceleration_time, constVelocity_time, c, c2));
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

            current_time = acceleration_time + (i - num_accel_segments) * step_size_const;

            currentTime[i] = current_time;

            positionProfile[i] = positionProfile[i - 1] + commanded_velocity * step_size_const;

            velocityProfile[i] = calculateCommandedVelocity(current_time, commanded_velocity, constVelocity_time);
        }

        if (i > num_accel_segments + num_const_segments && i <= 2 * num_accel_segments + num_const_segments)
        {

            current_time = acceleration_time + constVelocity_time + (i - num_accel_segments - num_const_segments) * step_size_ad;
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
    float acceleration_time = 0.3f; // Example acceleration time
    // float c = 5.0f;  // Sigmoid parameter for shaping the curve
    // float c2 = 0.5f; // Sigmoid center (midpoint of transition)
    // float commanded_velocity = maxVelocity;  // Example commanded velocity

    // Call the function to generate the angle and velocity values
    generateAngleAndVelocityValues(totalDistance, commandedVelocity);
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
