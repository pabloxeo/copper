#include "Camera.h"

Camera::Camera():
        eye(glm::vec3(0.0f, 1.0f, -1.0f)),
        center(glm::vec3(0, 0, 0)),
        up(glm::vec3(0, 1, 0)),
        view_matrix(glm::mat4(1.0f))
{
}


glm::vec3 Camera::get_eye() {
    // calculate from angles
    // vert_angle is theta and horiz_angle is phi
//    return glm::vec3(glm::cos(this->vert_angle)*glm::sin(this->horiz_angle), glm::sin(this->vert_angle), glm::cos(this->vert_angle)*glm::cos(this->horiz_angle)) * this->radius + this->center;
    return this->eye;
}



glm::vec3 Camera::get_center() {
    return this->center;
}

glm::vec3 Camera::get_up() {
    return this->up;
}

glm::vec3 Camera::get_right_vector() {
    return glm::transpose(this->view_matrix)[0];
}

glm::vec3 Camera::get_view_dir() {
    return glm::transpose(this->view_matrix)[2];
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

void Camera::set_eye(glm::vec3 _eye) {
    this->eye = glm::vec3(_eye.x, _eye.y, _eye.z);
}

void Camera::set_center(glm::vec3 _center) {
    this->center = glm::vec3(_center.x, _center.y, _center.z);
}

void Camera::set_up(glm::vec3 _up) {
    this->up = glm::vec3(_up.x, _up.y, _up.z);
}