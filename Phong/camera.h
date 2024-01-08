/**
 * @file camera.h
 * @author Jovan Ivosevic
 * @brief Camera related structs and functions
 *
 */

#pragma once
#include "cglm/cglm.h"
#include <string.h>

typedef struct Camera {
    vec3 _WorldUp;
    vec3 Position;
    vec3 _Front;
    vec3 Up;
    vec3 _Right;
    vec3 _Velocity;
    vec3 Target;

    float _MoveSpeed;
    float _LookSpeed;
    float _Pitch;
    float _Yaw;
    float _PlayerHeight; // Should be moved out
} Camera;

Camera CreateNewCamera();
/**
 * @brief Moves camera in specified direction
 *
 * @param dir Direction
 * @param dt Delta time
 */
void MoveCamera(Camera* camera, float dx, float dy, float dt);
/**
 * @brief Rotates camera depending on difference between previous and current cursor position
 *
 * @param dx Delta x
 * @param dy Delta y
 * @param dt Delta time
 */
void RotateCamera(Camera* camera, float dx, float dy, float dt);

void _UpdateVectorsCamera(Camera* camera);