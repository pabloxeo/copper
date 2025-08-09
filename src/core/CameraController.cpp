#include "CameraController.h"
#include <atomic>
#include <GLFW/glfw3.h>
#include <imgui.h>

std::atomic<int> scroll_direction(0);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    scroll_direction += yoffset > 0 ? 1 : -1;
}

CameraController::CameraController(Camera *camera, GLFWwindow *window) :
        mouse_drag_state({
                                 .prev_left_button_state = GLFW_RELEASE,
                                 .prev_right_button_state = GLFW_RELEASE,
                                 .left_btn_dragging = false,
                                 .right_btn_dragging = false,
                                 .left_btn_drag_start_pos = {.x = 0, .y = 0},
                                 .right_btn_drag_start_pos = {.x = 0, .y = 0}
                         }),
        total_rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
        upside_down(false),
        radius(10.0f) {
    this->camera = camera;

    this->camera->set_eye(glm::vec3(10.0f, 0.0f, 0.0f));
    this->camera->set_center(glm::vec3(0.0f, 0.0f, 0.0f));
    this->camera->set_up(glm::vec3(0.0f, 1.0f, 0.0f));

    this->update_view_matrix();
    // this->projection_matrix = glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) window_->width() / (float)window_->height(), 0.1f, 100.0f);
    // this->model_matrix = glm::mat4(1.0f);

    glfwSetScrollCallback(window, scroll_callback);

}

void CameraController::check_upside_down() {
    if (this->vert_angle > M_PI_2 || this->vert_angle < -M_PI_2) {
        this->upside_down = true;
    } else {
        this->upside_down = false;
    }
}

void CameraController::check_drag(GLFWwindow *window) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        if (this->mouse_drag_state.prev_left_button_state == GLFW_RELEASE) {
            this->mouse_drag_state.left_btn_dragging = true;
            {
                double x_pos, y_pos;
                glfwGetCursorPos(window, &x_pos, &y_pos);

                this->mouse_drag_state.left_btn_drag_start_pos.x = x_pos;
                this->mouse_drag_state.left_btn_drag_start_pos.y = y_pos;
            }

//                if (sgn((this->camera->get_eye() - this->camera->get_center())[1]) < 0) {
            this->check_upside_down();
        }
        this->mouse_drag_state.prev_left_button_state = GLFW_PRESS;
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
        if (this->mouse_drag_state.prev_left_button_state == GLFW_PRESS) {
            this->mouse_drag_state.left_btn_dragging = false;
//                if (sgn((this->camera->get_eye() - this->camera->get_center())[1]) < 0) {
            this->check_upside_down();
        }
        this->mouse_drag_state.prev_left_button_state = GLFW_RELEASE;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        if (this->mouse_drag_state.prev_right_button_state == GLFW_RELEASE) {
            this->mouse_drag_state.right_btn_dragging = true;
            {
                double x_pos, y_pos;
                glfwGetCursorPos(window, &x_pos, &y_pos);

                this->mouse_drag_state.right_btn_drag_start_pos.x = x_pos;
                this->mouse_drag_state.right_btn_drag_start_pos.y = y_pos;
            }
        }
        this->mouse_drag_state.prev_right_button_state = GLFW_PRESS;
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        if (this->mouse_drag_state.prev_right_button_state == GLFW_PRESS) {
            this->mouse_drag_state.right_btn_dragging = false;
        }
        this->mouse_drag_state.prev_right_button_state = GLFW_RELEASE;
    }
}

void CameraController::update_camera(GLFWwindow *window) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;  // GUI is using the mouse, skip camera interaction
    }
    if (scroll_direction != 0) {
        float speed = this->radius * 0.1f;
        this->radius += speed * (float) (-scroll_direction);

        this->update_view_matrix();
        scroll_direction = 0;
    }

    this->check_drag(window);

    if (this->mouse_drag_state.right_btn_dragging) {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);

        if ((int) x_pos != (int) this->mouse_drag_state.right_btn_drag_start_pos.x ||
            (int) y_pos != (int) this->mouse_drag_state.right_btn_drag_start_pos.y) {
            float speed =  this->radius / 1000.0f;
            glm::vec2 translate_screen_space = glm::vec2(x_pos - this->mouse_drag_state.right_btn_drag_start_pos.x,
                                                         y_pos - this->mouse_drag_state.right_btn_drag_start_pos.y)
                                               * speed;

            glm::vec3 camera_x = this->camera->get_right_vector();
            glm::vec3 camera_z = this->camera->get_view_dir();
            auto norm_v = glm::normalize(translate_screen_space);
            float angle = glm::atan(-norm_v.y, norm_v.x);

            glm::vec3 translate_world_space = glm::rotate(glm::mat4(1.0f), -M_PIf + angle, camera_z) *
                                              glm::vec4(camera_x.x, camera_x.y, camera_x.z, 1.0f);

            this->camera->set_center(
                    this->camera->get_center() + translate_world_space * glm::length(translate_screen_space));
            this->update_view_matrix();

            this->mouse_drag_state.right_btn_drag_start_pos.x = x_pos;
            this->mouse_drag_state.right_btn_drag_start_pos.y = y_pos;
        }

    }

    if (!this->mouse_drag_state.left_btn_dragging) {
        return;
    }

    glm::vec4 eye(this->camera->get_eye().x, this->camera->get_eye().y, this->camera->get_eye().z, 1);
    glm::vec4 center(this->camera->get_center().x, this->camera->get_center().y, this->camera->get_center().z, 1);

    double x_pos, y_pos;
    glfwGetCursorPos(window, &x_pos, &y_pos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if ((int) x_pos == (int) this->mouse_drag_state.left_btn_drag_start_pos.x &&
        (int) y_pos == (int) this->mouse_drag_state.left_btn_drag_start_pos.y) {
        return;
    }

    float delta_y = M_PI / (float) height;
    float delta_x = M_PI / (float) width;

    float angle_y = delta_y * (float) (y_pos - this->mouse_drag_state.left_btn_drag_start_pos.y);
    float angle_x = delta_x * (float) (x_pos - this->mouse_drag_state.left_btn_drag_start_pos.x);

    this->vert_angle -= angle_y;
    this->horiz_angle += (this->upside_down ? 1.0f : -1.0f) * angle_x;

    if (this->vert_angle > M_PI) {
        this->vert_angle -= 2 * M_PI;
    } else if (this->vert_angle < -M_PI) {
        this->vert_angle += 2 * M_PI;
    }
    if (this->horiz_angle > M_PI) {
        this->horiz_angle -= 2 * M_PI;
    } else if (this->horiz_angle < -M_PI) {
        this->horiz_angle += 2 * M_PI;
    }

    // std::cout << "vert_angle: " << glm::degrees(this->vert_angle) << "   horiz_angle: " << glm::degrees(this->horiz_angle) << std::endl;
//    std::cout <<  << std::endl;
    if (floor(this->vert_angle) == this->vert_angle) {
//        std::cout << "vert_angle: " << glm::degrees(this->vert_angle) << "   horiz_angle: " << glm::degrees(this->horiz_angle) << std::endl;
    }
    // this->vert_angle = glm::clamp(this->vert_angle, -M_PI_2 + 0.01f, M_PI_2 - 0.01f);


    glm::quat rot_matrix_y = glm::angleAxis(this->vert_angle, glm::vec3(1, 0, 0));
    glm::quat rot_matrix_x = glm::angleAxis(this->horiz_angle, glm::vec3(0, 1, 0));

    glm::quat q = (rot_matrix_y * rot_matrix_x);
    this->total_rotation = q;

    this->update_view_matrix();

    this->mouse_drag_state.left_btn_drag_start_pos.x = x_pos;
    this->mouse_drag_state.left_btn_drag_start_pos.y = y_pos;
}

glm::vec3 CameraController::get_updated_eye() {
    return glm::vec3(
            glm::cos(this->vert_angle) * glm::sin(this->horiz_angle),
            glm::sin(this->vert_angle),
            glm::cos(this->vert_angle) * glm::cos(this->horiz_angle)
    ) * this->radius;
}

void CameraController::update_view_matrix() {
    auto matrix = glm::transpose(
            mat4_cast(this->total_rotation)
    );

    auto right = glm::vec3(matrix[0]);
    auto up = glm::vec3(matrix[1]);
    auto view_dir = glm::vec3(matrix[2]);

    auto camera_center = this->camera->get_center();
    auto translation = glm::vec3(
            glm::dot(right, camera_center),
            glm::dot(up, camera_center),
            glm::dot(view_dir, camera_center));

    matrix[0][3] = translation.x;
    matrix[1][3] = translation.y;
    matrix[2][3] = translation.z - this->radius;

    this->camera->set_view_matrix(glm::transpose(matrix));
}
