#include "Camera.h"

Camera::Camera():
        eye(glm::vec3(0.0f, 1.0f, -1.0f)),
        center(glm::vec3(0, 0, 0)),
        up(glm::vec3(0, 1, 0)),
        view_matrix(glm::mat4(1.0f))
{
}


glm::vec3 Camera::get_eye() {
    auto mvp_matrix = this->get_view_matrix();
    mvp_matrix = glm::transpose(mvp_matrix);

    auto right = glm::vec3(mvp_matrix[0]);
    auto up = glm::vec3(mvp_matrix[1]);
    auto forward = glm::vec3(mvp_matrix[2]);

    auto eye = mvp_matrix[0][3] * right +
               mvp_matrix[1][3] * up +
               mvp_matrix[2][3] * forward;

    return eye;
}



glm::vec3 Camera::get_center() {
    return this->center;
}

glm::vec3 Camera::get_right() {
    return glm::vec3(glm::transpose(this->view_matrix)[0]);
}

glm::vec3 Camera::get_up() {
    return glm::vec3(glm::transpose(this->view_matrix)[1]);
}


glm::vec3 Camera::get_forward() {
    return glm::vec3(glm::transpose(this->view_matrix)[2]);
}

glm::mat4 Camera::get_view_matrix() {
    return this->view_matrix;
}

glm::mat3 Camera::get_view_matrix_3() {
    return glm::mat3(
            glm::transpose(this->view_matrix)[0],
            glm::transpose(this->view_matrix)[1],
            glm::transpose(this->view_matrix)[2]
        );
}

void Camera::update_view_matrix() {
    this->view_matrix = glm::lookAt(
            this->eye,
            this->center,
            this->up
    );
}

void Camera::set_view_matrix(glm::mat4 _view_matrix) {
    this->view_matrix = _view_matrix;
}