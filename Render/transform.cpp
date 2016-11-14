#include "transform.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 calculateViewProjectionMatrix(
        const ViewParameters& viewParameters,
        const ProjectionParameters& projectionParameters)
{
    // const glm::vec3 axis_y(0, 1, 0);
    // glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle),
    //                                  axis_y);
    glm::mat4 view = glm::lookAt(
            viewParameters.eye,
            viewParameters.center,
            viewParameters.up);
    glm::mat4 projection = glm::perspective(
            projectionParameters.fovy,
            projectionParameters.aspect,
            projectionParameters.zNear,
            projectionParameters.zFar);

    // return projection * view * rotation;
    return projection * view;
}
