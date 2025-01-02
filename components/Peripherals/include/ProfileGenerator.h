


#ifndef PROFILE_GENERATOR_H
#define PROFILE_GENERATOR_H

#include <vector>

#include "USBcom.h"

extern USBDevice usb_;




class ProfileGenerator {
public:
    ProfileGenerator(float maxVelocity, float maxAcceleration,float currentPosition, int lookupTableSize = 30);

    void generateScurveProfile(float totalDistance, float commandedVelocity);

    const std::vector<float>  &getPositionProfile() const;
    const std::vector<float>  &getVelocityProfile() const;
    
    const std::vector<float>& getTimeProfile() const;

    float getInterpolatedValue(float currentAngle, const std::vector<float>& profile, const std::vector<float>& angles) const;

    float getPositionForAngle(float currentAngle) const;
    float getVelocityForAngle(float currentAngle) const;

    float getPositionForTime(float currentTime) const;
    float getVelocitynForTime(float moment_) const ;
    
    void setAccelerationTime(float accelerationTime);  // New method to update acceleration time


private:
    std::vector<float> positionProfile;
    std::vector<float> velocityProfile;
    std::vector<float> currentTime;
    std::vector<float> angles; // Define the angles vector here

    float currentPosition; // something I need to use as starting value
    float maxVelocity;
    float maxAcceleration;

    int numSegments;

     
    float totalDistance;


    float accelerationTime=0.4; // this is default value for 'acceleration time'. 
    float decelerationTime= accelerationTime; 
    float sigmoidMin = 0.0f;
    float sigmoidMax = accelerationTime;

    float c = 30;
    float c2 = accelerationTime /2.0 ;
    float constVelocity_time = 0;
    float constAngle = 0;

    std::vector<float> sigmoidLookupTable;
    float lookupTableSize= 50;
    float sigmoidStep = (sigmoidMax - sigmoidMin) / lookupTableSize;
    
    int num_accel_segments = 15; // Example value for acceleration segments
    int num_const_segments = 3; 


    void initializeSigmoidLookupTable(); 

    float sigmoid(float x) const;
    float sigmoidLUp(float x); 
    float calculateCommandedVelocity(float t, float commanded_velocity, float constVelocity_time);  
    void generateAngleAndVelocityValues(float angleToGo, float commanded_velocity);
    // float calculateSegmentPosition(int segment, float t) const;
    // float calculateSegmentVelocity(int segment, float t) const;
    float calculateDesiredPositionAtT(float current_time, float prevVel, float step_size_ad);

     void generateAngleAndVelocityValues1(float angleToGo, float commanded_velocity);
     void generateAngleAndVelocityValues2(float angleToGo, float commanded_velocity);

};

#endif 