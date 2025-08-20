
#ifndef COPPER_GIZMOCONTROLS_H
#define COPPER_GIZMOCONTROLS_H

#include "Camera.h"
#include <glm/glm.hpp>


float axis_gizmo_sdf(glm::vec3 pos, glm::vec3 center, glm::vec3 direction, glm::vec3 color);

void check_axis_pick(Camera *camera, glm::vec2 mouse_pos, float aspect_ratio, glm::vec3 object_center);


#endif //COPPER_GIZMOCONTROLS_H