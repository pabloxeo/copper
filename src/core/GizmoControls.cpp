#include "GizmoControls.h"

#include "math.h"
#include <algorithm>
#include <initializer_list>
#include <tuple>
#include "Coder.h"

GizmoControls::GizmoControls(Camera *camera){
    this->camera = camera;
    this->objectCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    this->currentGizmo = std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), false); // Default to X axis
    this->isMoving = false;
}

bool GizmoControls::getIsMoving() {
    return this->isMoving;
}

glm::vec3 GizmoControls::getObjectCenter() {
    return this->objectCenter;
}

float GizmoControls::axis_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 direction, glm::vec3 color, float scale) {
    float shaft_length = 0.6f * scale;
    float shaft_radius = 0.02f * scale;
    float head_length = 0.2f * scale; 
    float head_radius = 0.06f * scale;

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
float GizmoControls::sdf_box(glm::vec3 pos, glm::vec3 center, glm::vec3 size) {
    glm::vec3 d = abs(pos - center) - size;
    // std::cout << d.x << ", " << d.y << ", " << d.z << std::endl;
    return glm::length(glm::max(d, glm::vec3(0.0f))) + std::min(std::max(d[0], std::max(d[1], d[2])), 0.0f);

}

float GizmoControls::plane_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 normal1, glm::vec3 normal2, float scale) {
    float size = 0.2 * scale;
    float thickness = 0.01 * scale;

    auto normal_sum = normal1 + normal2;
    auto dir_v = glm::vec3(1.0, 1.0, 1.0) - normal_sum;
    auto size_v = normal_sum * size + dir_v * thickness;

    return sdf_box(pos, center, size_v);
}

void GizmoControls::initDrag(glm::vec2 mouseUv, float aspectRatio, glm::vec3 objectCenter, float scale) {
    this->objectCenter = objectCenter;
    this->isMoving = true;

    // Initialize the current gizmo based on the mouse position
    this->checkAxisPick(mouseUv, aspectRatio, objectCenter, scale);

    if (!this->isMoving) {
        return;
    }

    this->initialObjectCenter = glm::vec3(objectCenter.x, objectCenter.y, objectCenter.z);

    auto [ro, rd] = ro_rd_from_uv(this->camera, mouseUv, aspectRatio);
    // Calculate the intersection point with the current gizmo
    auto intersection = gizmoIntersection(this->currentGizmo, this->objectCenter, ro, rd);
    this->firstIntersectionPoint = intersection;
}

void GizmoControls::checkAxisPick(glm::vec2 mouseUv, float aspectRatio, glm::vec3 objectCenter, float scale) {
    // auto mvp_matrix = camera->get_view_matrix();
    // mvp_matrix = glm::transpose(mvp_matrix);

    // auto right = glm::vec3(mvp_matrix[0]);
    // auto up = glm::vec3(mvp_matrix[1]);
    // auto forward = glm::vec3(mvp_matrix[2]);

    // auto eye = mvp_matrix[0][3] * right +
    //            mvp_matrix[1][3] * up +
    //            mvp_matrix[2][3] * forward;

    // auto eye = this->camera->get_eye();
    // auto right = this->camera->get_right();
    // auto up = this->camera->get_up();
    // auto forward = this->camera->get_forward();


    // auto uv = mouse_uv * 2.0f - glm::vec2(1.0f, 1.0f);
    // uv.y = -uv.y / aspect_ratio;

    // auto ro = eye;
    // auto rd = glm::normalize(uv.x * right + uv.y * up + forward);
    
    auto [ro, rd] = ro_rd_from_uv(this->camera, mouseUv, aspectRatio);

    // ray marching for axis gizmos
    float total_distance = 0.0f;
    float min_dist = 0.01f; // Minimum distance to consider for picking
    int max_steps = 1000;
    float max_distance = 100.0f;

    auto plane_offset = -0.5f * scale;

    // std::cout << "Checking axis gizmo pick at mouse UV: " << mouse_uv.x << ", " << mouse_uv.y << std::endl;
    glm::vec3 pos = ro;
    for (int i = 0; i < max_steps && total_distance < max_distance; ++i) {
        

        // Check each axis
        float x_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), scale);
        float y_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), scale);
        float z_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), scale);

        float xy_dist = plane_gizmo_sdf(pos, objectCenter + glm::vec3(0.0, 0.0, plane_offset), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), scale);
        float xz_dist = plane_gizmo_sdf(pos, objectCenter + glm::vec3(0.0, plane_offset, 0.0), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), scale);
        float yz_dist = plane_gizmo_sdf(pos, objectCenter + glm::vec3(plane_offset, 0.0, 0.0), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), scale);

        

        if (x_dist < min_dist) {
            // std::cout << "Picked X axis gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(1.0, 0.0, 0.0), false};
            this->isMoving = true;
            return;
        }

        if (y_dist < min_dist) {
            // std::cout << "Picked Y axis gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(0.0, 1.0, 0.0), false};
            this->isMoving = true;
            return;
        }
        
        if (z_dist < min_dist) {
            // std::cout << "Picked Z axis gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(0.0, 0.0, 1.0), false};
            this->isMoving = true;
            return;
        }

        if (xy_dist < min_dist) {
            // std::cout << "Picked XY plane gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(0.0, 0.0, 1.0), true};
            this->isMoving = true;
            return;
        }

        if (xz_dist < min_dist) {
            // std::cout << "Picked XZ plane gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(0.0, 1.0, 0.0), true};
            this->isMoving = true;
            return;
        }

        if (yz_dist < min_dist) {
            // std::cout << "Picked YZ plane gizmo at distance: " << total_distance << std::endl;
            this->currentGizmo = {glm::vec3(1.0, 0.0, 0.0), true};
            this->isMoving = true;
            return;
        }

        auto distances = {
            x_dist, y_dist, z_dist, xy_dist, xz_dist, yz_dist
        };

        auto min_d = std::min(distances) * 0.9f;
        // std::cout << "Distances: X: " << x_dist << ", Y: " << y_dist << ", Z: " << z_dist
        //           << ", XY: " << xy_dist << ", XZ: " << xz_dist << ", YZ: " << yz_dist
        //           << ", Total Distance: " << total_distance << ", Min Distance: " << min_d << std::endl;
        pos = pos + rd * min_d;
        total_distance += min_d;
        
    }

    this->isMoving = false;
}


void GizmoControls::setCamera(Camera *camera) {
    this->camera = camera;
}

void GizmoControls::release() {
    this->isMoving = false;
    this->currentGizmo = std::make_tuple(glm::vec3(0.0f, 0.0f, 0.0f), false); // Reset to default
    this->objectCenter = glm::vec3(0.0f, 0.0f, 0.0f);
}

void GizmoControls::update(glm::vec2 mouseUv, float aspectRatio) {
    if (!this->isMoving) {
        return; // No movement if not active
    }
    
    auto [ro, rd] = ro_rd_from_uv(this->camera, mouseUv, aspectRatio);
    
    auto gizmoNormal = std::get<0>(this->currentGizmo);
    auto isPlane = std::get<1>(this->currentGizmo);

    auto intersection = gizmoIntersection(this->currentGizmo, this->objectCenter, ro, rd);

    // std::cout << "Intersection point: " << intersection.x << ", " << intersection.y << ", " << intersection.z << std::endl;

    auto diffV = intersection - this->firstIntersectionPoint;
    auto newPosition = this->initialObjectCenter + diffV;

    auto subsVector = isPlane ? glm::vec3(1.0) - gizmoNormal : gizmoNormal;

    auto newObjectCenter = this->objectCenter - subsVector * this->objectCenter + subsVector * newPosition;

    this->objectCenter = newObjectCenter;

    // std::cout << "Updated object center: " << this->objectCenter.x << ", " << this->objectCenter.y << ", " << this->objectCenter.z << std::endl;
}

glm::vec3 gizmoIntersection(std::tuple<glm::vec3, bool> currentGizmo, glm::vec3 objectCenter, glm::vec3 ro, glm::vec3 rd) {
    auto gizmoNormal = std::get<0>(currentGizmo);
    auto isPlane = std::get<1>(currentGizmo);

    glm::vec3 normal = glm::vec3(gizmoNormal[0], gizmoNormal[1], gizmoNormal[2]);

    if (!isPlane) {
        // line two is the gizmo
        glm::vec3 k2 = objectCenter;
        glm::vec3 v2 = normal;

        glm::vec3 k1 = ro;
        glm::vec3 v1 = rd;

        float v1_dot_v2 = glm::dot(v1, v2);
        float v1_dot_v1 = glm::dot(v1, v1);
        float v2_dot_v2 = glm::dot(v2, v2);
        float k2_dot_v1 = glm::dot(k2, v1);
        float k2_dot_v2 = glm::dot(k2, v2);
        float k1_dot_v1 = glm::dot(k1, v1);
        float k1_dot_v2 = glm::dot(k1, v2);

        if (fabs(v1_dot_v2) < 1e-6) {
            // check if there is an intersection
            if (fabs(k2_dot_v1 - k1_dot_v1) > 1e-6) {
                // Lines are parallel and do not intersect
                return k1; // Return a point on the gizmo line
            }

            // Lines are parallel and overlap, return the closest point on the gizmo line to k2
            float t = glm::dot(k2 - k1, v1) / glm::dot(v1, v1);
            return k1 + t * v1;
        }

        // auto n = glm::cross(v1, v2);
        // auto n1 = glm::cross(v1, n);

        // return k2 + glm::dot(k1 - k2, n1) / glm::dot(v2, n1) * v2;

        float num = -(v1_dot_v1) / v1_dot_v2 * (k2_dot_v2 - k1_dot_v2) - k1_dot_v1 + k2_dot_v1;
        float den = v2_dot_v2 * v1_dot_v1 / v1_dot_v2 - v1_dot_v2;

        float l2 = num / den;

        // auto a = v1_dot_v1;
        // auto b = v1_dot_v2;
        // auto c = v2_dot_v2;
        // auto d = k1_dot_v1 - k2_dot_v1;
        
        //std::cout << "l2: " << l2 << std::endl;
        //std::cout << "num: " << num << ", den: " << den << std::endl;
        //std::cout << "v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
        //std::cout << "v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl;
        //std::cout << "v1_dot_v2: " << v1_dot_v2 << std::endl;
        //std::cout << "k1: " << k1.x << ", " << k1.y << ", " << k1.z << std::endl;

        return k2 + l2 * v2;
    }
    auto pointInPlane = isPlane ? objectCenter + glm::vec3(1.0) * -0.5f : objectCenter;

    auto p = planeLineIntersection(pointInPlane, normal, ro, rd);
    
    return p;
}

std::tuple<glm::vec3, glm::vec3> ro_rd_from_uv(Camera *camera, glm::vec2 mouse_uv, float aspect_ratio) {
    auto eye = camera->get_eye();
    auto right = camera->get_right();
    auto up = camera->get_up();
    auto forward = camera->get_forward();

    auto uv = mouse_uv * 2.0f - glm::vec2(1.0f, 1.0f);
    uv.y = -uv.y / aspect_ratio;

    auto ro = eye;
    auto rd = glm::normalize(uv.x * right + uv.y * up + forward);

    return std::make_tuple(ro, rd);
}

glm::vec3 planeLineIntersection(glm::vec3 planePoint, glm::vec3 planeNormal, glm::vec3 linePoint, glm::vec3 lineDirection) {
    float d = glm::dot(planeNormal, planePoint - linePoint) / glm::dot(planeNormal, lineDirection);
    return linePoint + d * lineDirection;
}