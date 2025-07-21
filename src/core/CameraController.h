#ifndef COPPER_CAMERACONTROLLER_H
#define COPPER_CAMERACONTROLLER_H

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct MousePosition {
    double x;
    double y;
};

struct MouseDragState {
    int prev_left_button_state;
    int prev_right_button_state;
    bool left_btn_dragging;
    bool right_btn_dragging;
    MousePosition left_btn_drag_start_pos;
    MousePosition right_btn_drag_start_pos;
};

class CameraController {

    private:
        MouseDragState mouse_drag_state{};
        Camera *camera;
        glm::mat4 initial_view_matrix;
        glm::quat total_rotation;
        glm::vec3 total_translation;
        bool upside_down;
        float radius;

    public:
        explicit CameraController(Camera *camera, GLFWwindow *window);
        void check_drag(GLFWwindow *window);
        void update_camera(GLFWwindow *window);
        // glm::mat4 getMVP();

    private:
        void check_upside_down();
        void update_view_matrix();
        glm::vec3 get_updated_eye();

        float vert_angle = 0;
        float horiz_angle = 0;
};


#endif //COPPER_CAMERACONTROLLER_H