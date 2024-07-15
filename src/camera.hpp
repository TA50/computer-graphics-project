#pragma once

#include "modules/Starter.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform2.hpp>


class Camera {

public:
    struct {
        glm::mat4 perspective;
        glm::mat4 view;
        glm::mat4 world;
    } matrices;

    void setPosition(glm::vec3 p) {
        CamPosition = p;
    }

    void rotate(float yawDelta, float pitchDelta, float rollDelta) {
        CamYaw += ROT_SPEED * yawDelta;
        CamPitch += ROT_SPEED * pitchDelta;
        CamRoll += ROT_SPEED * rollDelta;

//        Yaw = (Yaw < 0.0f ? 0.0f : (Yaw > 2 * M_PI ? 2 * M_PI : Yaw));
//        Pitch = (Pitch < 0.0f ? 0.0f : (Pitch > M_PI_2 - 0.01f ? M_PI_2 - 0.01f : Pitch));
//        Roll = (Roll < -M_PI ? -M_PI : (Roll > M_PI ? M_PI : Roll));
    }

    void print() {

        std::cout << "================Camera=================" << std::endl;
        std::cout << "Angles: " << std::endl;
        std::cout << "Yaw: " << glm::degrees(CamYaw) << std::endl;
        std::cout << "Pitch: " << glm::degrees(CamPitch) << std::endl;
        std::cout << "Roll: " << glm::degrees(CamRoll) << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "Position: " << std::endl;
        std::cout << "X: " << CamPosition.x << " Y: " << CamPosition.y << " Z: " << CamPosition.z << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << "Target: " << std::endl;
        std::cout << "X: " << CamTarget.x << " Y: " << CamTarget.y << " Z: " << CamTarget.z << std::endl;
        std::cout << "=================================" << std::endl;


    }

    void lookAt(glm::vec3 pos, float Yaw = 0) {
        CamTarget = pos + glm::vec3(
                glm::rotate(
                        glm::mat4(1), Yaw, glm::vec3(0, 1, 0))
                * glm::vec4(CamTargetDelta, 1)
        );

        CamPosition = CamTarget + glm::vec3(
                glm::rotate(
                        glm::mat4(1), Yaw + CamYaw, glm::vec3(0, 1, 0)
                ) * glm::rotate(glm::mat4(1), -CamPitch, glm::vec3(1, 0, 0))
                * glm::vec4(0, 0, CamDistance, 1));


    }

    void move(float distanceDelta) {
        CamDistance -= MOVE_SPEED * distanceDelta;
        CamDistance = (CamDistance < 7.0f ? 7.0f : (CamDistance > 15.0f ? 15.0f : CamDistance));
    }

    void translate(glm::vec3 posDelta) {
        CamPosition += MOVE_SPEED * posDelta;
    }

    void updateWorld(float yaw = 0.0f) {

        glm::vec3 dp = glm::vec3(glm::rotate(glm::mat4(1), yaw, glm::vec3(0, 1, 0))
                                 * glm::vec4(0, 0, 0, 1)
        );

        auto res = CamPosition + dp;
        glm::mat4 M = glm::translate(glm::mat4(1.0f), glm::vec3(res.x, res.y, 0))
                      * glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0, 1, 0))
                      * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(1, 0, 0))
                      * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0, 0, 1));
        matrices.world = M;

    }

    void updatePerspective() {

        matrices.perspective = glm::perspective(fov, aspect, znear, zfar);
        matrices.perspective[1][1] *= -1;
    }

    void updateViewMatrix() {
        matrices.view = glm::lookAt(CamPosition, CamTarget, up);
//        matrices.view[1][1] *= -1;
    }

    void setEuler(float yaw, float pitch, float roll) {
        CamYaw = yaw;
        CamPitch = pitch;
        CamRoll = roll;
    }

    float fov = glm::radians(20.0f);
    float aspect = 4.0f / 3.0f;
    float znear = 0.1f;
    float zfar = 500.0f;



    float CamYaw = M_PI;
    float CamPitch = glm::radians(20.0f);
    float CamRoll = 0.0f;
    float CamDistance = 4;
    glm::vec3 CamTargetDelta = glm::vec3(2, 2, 2);
    glm::vec3 CamTarget = glm::vec3(0);
    glm::vec3 CamPosition = glm::vec3(0);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
private:
    float ROT_SPEED = glm::radians(120.0f);
    float MOVE_SPEED = 10.0f;



};