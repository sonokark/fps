#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
    glm::vec3 position;
    float yaw;
    float pitch;

    glm::vec3 right;
    glm::vec3 forward;
    glm::vec3 up;

    glm::mat4 view;
};

inline void Camera_MoveStraight(Camera* camera, float distance);
inline void Camera_MoveVertically(Camera* camera, float distance);
inline void Camera_Strafe(Camera* camera, float distance);
inline void Camera_Rotate(Camera* camera, float delta_yaw, float delta_pitch);

inline void Camera_RecomputeDirectionVectors(Camera* camera);
inline void Camera_RecomputeViewMatrix(Camera* camera);

// Implementation of inline functions

inline void Camera_MoveStraight(Camera* camera, float distance)
{
    camera->position += distance * camera->forward;
}

inline void Camera_MoveVertically(Camera* camera, float distance)
{
    camera->position.y += distance;
}

inline void Camera_Strafe(Camera* camera, float distance)
{
    camera->position += distance * camera->right;
}

inline void Camera_Rotate(Camera* camera, float delta_yaw, float delta_pitch)
{
    constexpr float max_pitch = glm::radians(89.0f);
    constexpr float min_pitch = -glm::radians(89.0f);

    camera->yaw += delta_yaw;

    camera->pitch += delta_pitch;

    if (camera->pitch > max_pitch)
        camera->pitch = max_pitch;

    if (camera->pitch < min_pitch)
        camera->pitch = min_pitch;
}

inline void Camera_RecomputeDirectionVectors(Camera* camera)
{
    camera->forward.x = cosf(camera->yaw) * cosf(camera->pitch);
    camera->forward.y = sinf(camera->pitch);
    camera->forward.z = sinf(camera->yaw) * cosf(camera->pitch);

    camera->right.x = -sinf(camera->yaw);
    camera->right.y = 0.0f;
    camera->right.z = cosf(camera->yaw);

    camera->up = glm::cross(camera->right, camera->forward);
}

inline void Camera_RecomputeViewMatrix(Camera* camera)
{
    camera->view = glm::lookAt(camera->position, camera->position + camera->forward, camera->up);
}

#endif // !CAMERA_HPP_
