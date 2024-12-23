


#ifndef PROFILE_GENERATOR_H
#define PROFILE_GENERATOR_H

#include <vector>

class ProfileGenerator {
public:
    ProfileGenerator(float maxVelocity, float maxAcceleration, int numSegments);

    void generateScurveProfile(float totalDistance, float commandedVelocity);

    const std::vector<float>& getPositionProfile() const;
    const std::vector<float>& getVelocityProfile() const;
    
    const std::vector<float>& getTimeProfile() const;

    float getInterpolatedValue(float currentAngle, const std::vector<float>& profile, const std::vector<float>& angles) const;
    float getPositionForAngle(float currentAngle) const;
    float getVelocityForAngle(float currentAngle) const;
    float getPositionForTime(float currentTime) const;
    float getVelocitynForTime(float moment_) const ;

private:
    std::vector<float> positionProfile;
    std::vector<float> velocityProfile;
    std::vector<float> currentTime;
    std::vector<float> angles; // Define the angles vector here
    float maxVelocity;
    float maxAcceleration;

    int numSegments;

     
    float totalDistance;
    float constVelocity_time = 0;



    float sigmoid(float x, float c, float c2) const;
    float calculateCommandedVelocity(float t, float commanded_velocity, float constVelocity_time);  
    void generateAngleAndVelocityValues(float angleToGo, float commanded_velocity);
    // float calculateSegmentPosition(int segment, float t) const;
    // float calculateSegmentVelocity(int segment, float t) const;
    float calculateDesiredPositionAtT(float current_time, float prevVel, float step_size_ad);

};

#endif 