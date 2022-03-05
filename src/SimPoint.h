#pragma once
// this strct must be repeated exacly inside kernel source code.
typedef struct SimPoint
{
    float position[3];
    float velocity[3];
    float mass = 1.f;
    float radius = 1.0f;
    bool collided = false;
} SimPoint;