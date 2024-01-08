#include "camera.h"

Camera CreateNewCamera() {
    Camera Result = { 0 };
    vec3 Position = { 0.0f, 0.0f, 2.0f };
    memcpy(Result.Position, Position, sizeof(vec3));
    vec3 WorldUp = { 0.0f, 1.0f, 0.0f };
    memcpy(Result._WorldUp, WorldUp, sizeof(vec3));
    vec3 Right = { 1.0f, 0.0f, 0.0f };
    memcpy(Result._Right, Right, sizeof(vec3));
    vec3 Velocity = { 0.0f, 0.0f, 0.0f };
    memcpy(Result._Velocity, Velocity, sizeof(vec3));
    vec3 Front = { 0.0f, 0.0f, 0.0f };
    memcpy(Result._Front, Front, sizeof(vec3));
    vec3 Target = { 0.0f, 0.0f, 0.0f };
    memcpy(Result.Target, Target, sizeof(vec3));
    Result._Pitch = 0.0f;
    Result._Yaw = -90.0f;
    Result._MoveSpeed = 8.0f;
    Result._LookSpeed = 64.0f;
    Result._PlayerHeight = 2.0f;
    _UpdateVectorsCamera(&Result);
    return Result;
}

void MoveCamera(Camera* camera, float dx, float dy, float dt) {
    vec3 RightMovement;
    glm_vec3_scale(camera->_Right, dx * camera->_MoveSpeed * dt, RightMovement);
    vec3 FrontMovement;
    glm_vec3_scale(camera->_Front, dy * camera->_MoveSpeed * dt, FrontMovement);
    glm_vec3_addadd(FrontMovement, RightMovement, camera->Position);
    _UpdateVectorsCamera(camera);
}

void RotateCamera(Camera* camera, float dx, float dy, float dt) {
    float RotateVelocity = camera->_LookSpeed * dt;
    camera->_Yaw += dx * RotateVelocity;
    camera->_Pitch += dy * RotateVelocity;

    if (camera->_Pitch > 89.0f) {
        camera->_Pitch = 89.0f;
    }
    if (camera->_Pitch < -89.0f) {
        camera->_Pitch = -89.0f;
    }

    _UpdateVectorsCamera(camera);
}

void _UpdateVectorsCamera(Camera* camera) {
    camera->_Front[0] = cos(glm_rad(camera->_Yaw)) * cos(glm_rad(camera->_Pitch));
    camera->_Front[1] = sin(glm_rad(camera->_Pitch));
    camera->_Front[2] = sin(glm_rad(camera->_Yaw)) * cos(glm_rad(camera->_Pitch));
    glm_normalize(&camera->_Front);
    glm_cross(&camera->_Front, &camera->_WorldUp, &camera->_Right);
    glm_normalize(&camera->_Right);

    glm_cross(camera->_Right, camera->_Front, camera->Up);
    glm_normalize(camera->Up);
    camera->Position[1] = camera->Position[1] < camera->_PlayerHeight
        ? camera->_PlayerHeight
        : camera->Position[1] > camera->_PlayerHeight
        ? camera->_PlayerHeight
        : camera->Position[1];
    
    glm_vec3_add(camera->Position, camera->_Front, camera->Target);
}