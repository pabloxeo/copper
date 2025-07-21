#ifndef OMP_RM_CAMERA_H
#define OMP_RM_CAMERA_H

#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera {
private:
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    glm::mat4 view_matrix;

public:
    Camera();

    glm::vec3 get_eye();
    glm::vec3 get_center();
    glm::vec3 get_up();
    glm::vec3 get_right_vector();
    glm::vec3 get_view_dir();
    glm::mat4 get_view_matrix();
    glm::mat3 get_view_matrix_3();

    void update_view_matrix();
    void set_view_matrix(glm::mat4 view_matrix);
    void set_eye(glm::vec3 _eye);
    void set_center(glm::vec3 _center);
    void set_up(glm::vec3 _up);
};

#endif //OMP_RM_CAMERA_H
