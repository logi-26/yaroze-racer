#ifndef VEHICLE_ATTRIBS_H
#define VEHICLE_ATTRIBS_H

// Struct for per vehicle attributes
typedef struct {
    int maxSpeed;           // Top forward speed
    int acceleration;       // Speed gain per frame under throttle
    int deceleration;       // Speed loss per frame coasting
    int brakeDeceleration;  // Speed loss per frame braking
    int maxReverseSpeed;    // Top reverse speed (negative)
    int steeringResponse;   // Steering sensitivity at low speed
    int minSteeringResponse;// Steering sensitivity at max speed
    int maxGrip;            // Lateral slip limit at low speed
    int minGrip;            // Lateral slip limit at max speed
    int slidingFriction;    // Lateral damping while sliding (/1000)
    int lateralFriction;    // Lateral damping while gripped (/1000)
    int turnRadiusFactor;   // Cornering momentum scale (lower = more oversteer)
} VehicleAttributes;

extern VehicleAttributes car3Attribs;
extern VehicleAttributes car2Attribs;
extern VehicleAttributes car5Attribs;
extern VehicleAttributes *activeVehicle;

#endif // VEHICLE_ATTRIBS_H