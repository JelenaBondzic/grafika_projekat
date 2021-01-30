#ifndef SPHERE_CAMERA_H
#define SPHERE_CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
// Defined in Camera.h
//enum Camera_Movement {
//    FORWARD,
//    BACKWARD,
//    LEFT,
//    RIGHT
//};

// Default camera values
// Defined in camera.h
//const float YAW         = -90.0f;
//const float PITCH       =  -45.0f;
//const float SPEED       =  2.5f;
//const float SENSITIVITY =  0.1f;
//const float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class SphereCamera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    glm::vec2 anglePosition;
    float  r;

    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    SphereCamera(float radius, glm::vec2 position = glm::vec2(0.35f, 1.56f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        r = radius;
        anglePosition = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;

//        Position = glm::vec3 (0, 0, r);
        Position = glm::vec3 (r*sin(anglePosition.x)*cos(anglePosition.y),
                              r*cos(anglePosition.x),
                              r*sin(anglePosition.x)*sin(anglePosition.y));
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Front, WorldUp);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {

        float velocity =  deltaTime; // * MovementSpeed je prebrzo
        if (direction == FORWARD)
            anglePosition.x -= velocity;
        if (direction == BACKWARD)
            anglePosition.x += velocity;
        if (direction == LEFT)
            anglePosition.y += velocity;
        if (direction == RIGHT)
            anglePosition.y -= velocity;

        if (anglePosition.x > 1.5) // ne treba da ide preko oko 70 stepeni, otisao i ispod table
            anglePosition.x = 1.5;
        if (anglePosition.x < 0.16) // ne teba da ide manje od oko 10 stepeni, prevrnula bi se kamera iznad table
            anglePosition.x = 0.16;


        Position = glm::vec3 (r*sin(anglePosition.x)*cos(anglePosition.y),
                              r*cos(anglePosition.x),
                              r*sin(anglePosition.x)*sin(anglePosition.y));
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 90.0f)
                Pitch = 90.0f;
            if (Pitch < -90.0f)
                Pitch = -90.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 10.0f)
            Zoom = 10.0f;
        if (Zoom > 60.0f)
            Zoom = 60.0f;
    }

    glm::vec3 getPosition(){
        return Position;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};
#endif