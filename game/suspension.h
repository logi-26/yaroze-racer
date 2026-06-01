#ifndef SUSPENSION_H
#define SUSPENSION_H


// Suspension and weight/inertia properties - separate from core handling attributes
typedef struct {
    int mass;           // Relative mass
    int suspensionRate; // Pitch/roll travel
    int pitchFactor;    // Maximum nose pitch angle
    int rollFactor;     // Maximum body roll angle
} SuspensionConfig;


// Suspension configs for each of the 3 vehicles
extern SuspensionConfig car3Suspension;
extern SuspensionConfig car2Suspension;
extern SuspensionConfig car5Suspension;
extern SuspensionConfig *activeSuspension;


#endif // SUSPENSION_H