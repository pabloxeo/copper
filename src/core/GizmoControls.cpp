#include "GizmoControls.h"

#include "math.h"
#include <algorithm>
#include <initializer_list>


float axis_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 direction, glm::vec3 color) {
    float shaft_length = 0.8f;
    float shaft_radius = 0.02f;
    float head_length = 0.2f;
    float head_radius = 0.06f;

    glm::vec3 local_pos = pos - center;
    float axis_dist = glm::dot(local_pos, direction);
    glm::vec3 radial_pos = local_pos - axis_dist * direction;
    float radial_dist = glm::length(radial_pos);

    float dist;
    if (axis_dist < 0.0f) {
        dist = glm::length(local_pos);
    } else if (axis_dist < shaft_length) {
        dist = radial_dist - shaft_radius;
    } else if (axis_dist < shaft_length + head_length) {
        float head_progress = (axis_dist - shaft_length) / head_length;
        float head_r = head_radius * (1.0f - head_progress);
        dist = radial_dist - head_r;
    } else {
        glm::vec3 tip_pos = local_pos - direction * (shaft_length + head_length);
        dist = glm::length(tip_pos);
    }

    return dist; // Return the distance value
}

float sdf_box(glm::vec3 pos, glm::vec3 center, glm::vec3 size) {
    glm::vec3 d = abs(pos - center) - size;
    // std::cout << d.x << ", " << d.y << ", " << d.z << std::endl;
    return glm::length(glm::max(d, glm::vec3(0.0f))) + std::min(std::max(d[0], std::max(d[1], d[2])), 0.0f);

}

float plane_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 normal1, glm::vec3 normal2) {
    float size = 0.2;
    float thickness = 0.01;

    auto normal_sum = normal1 + normal2;
    auto dir_v = glm::vec3(1.0, 1.0, 1.0) - normal_sum;
    auto size_v = normal_sum * size + dir_v * thickness;

    return sdf_box(pos, center, size_v);
}

void check_axis_pick(Camera *camera, glm::vec2 mouse_uv, float aspect_ratio, glm::vec3 object_center) {
    auto mvp_matrix = camera->get_view_matrix();
    mvp_matrix = glm::transpose(mvp_matrix);

    auto right = glm::vec3(mvp_matrix[0]);
    auto up = glm::vec3(mvp_matrix[1]);
    auto forward = glm::vec3(mvp_matrix[2]);

    auto eye = mvp_matrix[0][3] * right +
               mvp_matrix[1][3] * up +
               mvp_matrix[2][3] * forward;


    auto uv = mouse_uv * 2.0f - glm::vec2(1.0f, 1.0f);
    uv.y = -uv.y / aspect_ratio;

    auto ro = eye;
    auto rd = glm::normalize(uv.x * right + uv.y * up + forward);
    
    // ray marching for axis gizmos
    float total_distance = 0.0f;
    float min_dist = 0.01f; // Minimum distance to consider for picking
    int max_steps = 1000;
    float max_distance = 100.0f;

    auto plane_offset = -0.5f;

    // std::cout << "Checking axis gizmo pick at mouse UV: " << mouse_uv.x << ", " << mouse_uv.y << std::endl;
    glm::vec3 pos = ro;
    for (int i = 0; i < max_steps && total_distance < max_distance; ++i) {
        

        // Check each axis
        float x_dist = axis_gizmo_sdf(pos, object_center, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        float y_dist = axis_gizmo_sdf(pos, object_center, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        float z_dist = axis_gizmo_sdf(pos, object_center, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        float xy_dist = plane_gizmo_sdf(pos, object_center + glm::vec3(0.0, 0.0, plane_offset), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        float xz_dist = plane_gizmo_sdf(pos, object_center + glm::vec3(0.0, plane_offset, 0.0), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        float yz_dist = plane_gizmo_sdf(pos, object_center + glm::vec3(plane_offset, 0.0, 0.0), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        

        if (x_dist < min_dist) {
            std::cout << "Picked X axis gizmo at distance: " << total_distance << std::endl;
            return;
        }
        if (y_dist < min_dist) {
            std::cout << "Picked Y axis gizmo at distance: " << total_distance << std::endl;
            return;
        }
        if (z_dist < min_dist) {
            std::cout << "Picked Z axis gizmo at distance: " << total_distance << std::endl;
            return;
        }

        if (xy_dist < min_dist) {
            std::cout << "Picked XY plane gizmo at distance: " << total_distance << std::endl;
            return;
        }

        if (xz_dist < min_dist) {
            std::cout << "Picked XZ plane gizmo at distance: " << total_distance << std::endl;
            return;
        }

        if (yz_dist < min_dist) {
            std::cout << "Picked YZ plane gizmo at distance: " << total_distance << std::endl;
            return;
        }

        auto distances = {
            x_dist, y_dist, z_dist, xy_dist, xz_dist, yz_dist
        };

        auto min_d = std::min(distances) * 0.9f;
        std::cout << "Distances: X: " << x_dist << ", Y: " << y_dist << ", Z: " << z_dist
                  << ", XY: " << xy_dist << ", XZ: " << xz_dist << ", YZ: " << yz_dist
                  << ", Total Distance: " << total_distance << ", Min Distance: " << min_d << std::endl;
        pos = pos + rd * min_d;
        total_distance += min_d;

        
    }

}

