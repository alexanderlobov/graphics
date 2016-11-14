#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct ViewParameters {
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
};

struct ProjectionParameters {
    float fovy;
    float aspect;
    float zNear;
    float zFar;
};

glm::mat4 calculateViewProjectionMatrix(
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters);
