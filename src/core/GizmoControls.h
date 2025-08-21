
#ifndef COPPER_GIZMOCONTROLS_H
#define COPPER_GIZMOCONTROLS_H

#include "Camera.h"

#include <tuple>
#include <glm/glm.hpp>


class GizmoControls {
    public:

        GizmoControls(Camera *camera);

        void initDrag(glm::vec2 mouseUv, float aspectRatio, glm::vec3 object_center);
        void checkAxisPick(glm::vec2 mouseUv, float aspect_ratio, glm::vec3 object_center);

        void setCamera(Camera *camera);
        void release();
        void update(glm::vec2 mouseUv, float aspectRatio);
        bool getIsMoving();
        glm::vec3 getObjectCenter();
        
    private:
    
        static float axis_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 direction, glm::vec3 color);
        static float sdf_box(glm::vec3 pos, glm::vec3 center, glm::vec3 size);
        static float plane_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 normal1, glm::vec3 normal2);
        
        // The center of the object being manipulated.
        glm::vec3 objectCenter;
        glm::vec3 initialObjectCenter;

        glm::vec3 firstIntersectionPoint;
        
        // The current gizmo being manipulated (normal / direction, isPlane).
        // If the gizmo is a plane, the first element is the normal of the plane, and the second element is true.
        // If the gizmo is an axis, the first element is the direction of the axis, and the second element is false.
        std::tuple<glm::vec3, bool> currentGizmo;

        // Indicates wether object movement with the gizmo is currently active.
        bool isMoving;

        Camera *camera;
}; 

std::tuple<glm::vec3, glm::vec3> ro_rd_from_uv(Camera *camera, glm::vec2 mouse_uv, float aspect_ratio);
glm::vec3 planeLineIntersection(glm::vec3 planePoint, glm::vec3 planeNormal, glm::vec3 linePoint, glm::vec3 lineDirection);
glm::vec3 gizmoIntersection(std::tuple<glm::vec3, bool> currentGizmo, glm::vec3 objectCenter, glm::vec3 ro, glm::vec3 rd);


#endif //COPPER_GIZMOCONTROLS_H