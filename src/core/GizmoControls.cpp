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

glm::vec3 GizmoControls::rotate_gizmo_axis(glm::vec3 pos, glm::vec3 axis) {
    glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
    glm::vec3 v = glm::normalize(axis);
    float c = glm::dot(up, v);
    if (c > 0.999) { return pos; }
    if (c < -0.999) { return glm::vec3(pos.x, -pos.y, -pos.z); }
    glm::vec3 rot_axis = glm::normalize(glm::cross(up, v));
    float angle = glm::acos(c);
    float s = glm::sin(angle);
    float t = 1.0 - glm::cos(angle);
    float x = rot_axis.x;
    float y = rot_axis.y;
    float z = rot_axis.z;
    glm::mat3 m = glm::mat3(
        glm::vec3(t*x*x + glm::cos(angle), t*x*y - s*z, t*x*z + s*y),
        glm::vec3(t*x*y + s*z, t*y*y + glm::cos(angle), t*y*z - s*x),
        glm::vec3(t*x*z - s*y, t*y*z + s*x, t*z*z + glm::cos(angle))
    );
    return m * pos;
}

float GizmoControls::sdf_cone(glm::vec3 pos, glm::vec2 angle, float height) {
    glm::vec2 q = height * glm::vec2(angle.x / angle.y, -1.0);
    glm::vec2 w = glm::vec2(glm::length(glm::vec2(pos.x, pos.z)), pos.y);
    glm::vec2 a = w - q * glm::clamp(glm::dot(w, q) / glm::dot(q, q), 0.0f, 1.0f);
    glm::vec2 b = w - q * glm::vec2(glm::clamp(w.x / q.x, 0.0f, 1.0f), 1.0f);
    float k = glm::sign(q.y);
    float d = glm::min(glm::dot(a, a), glm::dot(b, b));
    float s = glm::max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    return glm::sqrt(d) * glm::sign(s);
}

float GizmoControls::sdf_cylinder(glm::vec3 pos, float radius, float height) {
    glm::vec2 d = glm::abs(glm::vec2(glm::length(glm::vec2(pos.x, pos.z)), pos.y)) - glm::vec2(radius, height);
    float dist = glm::min(glm::max(d.x, d.y), 0.0f) + glm::length(glm::max(d, glm::vec2(0.0f)));
    return dist;
}

float GizmoControls::axis_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 direction, float scale) {
    float shaft_length = 0.6 * scale;
        float shaft_radius = 0.02 * scale;
        float head_length = 0.4 * scale;
        float head_radius = 0.15 * scale;
        glm::vec3 local_pos = pos - center;
        glm::vec3 axis = normalize(direction);
        glm::vec3 rotated_pos = rotate_gizmo_axis(local_pos, axis);
        // Cylinder for shaft (Y axis)
        float shaft = sdf_cylinder(rotated_pos - glm::vec3(0.0, shaft_length / 2.0, 0.0), shaft_radius, shaft_length / 2.0);
        // Cone for head (Y axis)
        glm::vec2 cone_angle = glm::vec2(head_radius, head_length);
        float head = sdf_cone(rotated_pos - glm::vec3(0.0, shaft_length + head_length / 2.0, 0.0), cone_angle, head_length / 2.0);
        // Combine with union
        if (head < shaft) {
            return head;
        }
        return shaft;
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
        float x_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(1.0f, 0.0f, 0.0f), scale);
        float y_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(0.0f, 1.0f, 0.0f), scale);
        float z_dist = axis_gizmo_sdf(pos, objectCenter, glm::vec3(0.0f, 0.0f, 1.0f), scale);

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