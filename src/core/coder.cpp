#include <iostream>
#include "coder.h"
#include "fstream"

//Write cube code in ../shaders/shader.wgsl
void AddCube(int a, int b) {
}

//Write sphere code in ../shaders/shader.wgsl
void AddSphere(int a, int b) {
    /*
    Spehere code:
    fn sdf_sphere(pos: vec3<f32>) -> DistanceColor {
        let t = get_time() * 0.5; // Simulated time for animation
        let animated_position = vec3(sin(t) * 2.0, sin(t) * 2.0, cos(t) * 2.0 + 4.0);
        let distance = length(pos - animated_position) - 0.5; // Sphere radius of 0.5
        return DistanceColor(distance, vec3(1.0, 0.0, 0.0)); // Red color for the first sphere
    }
    */
    std::ofstream shaderFile("../shaders/shader.wgsl", std::ios::app);
    if (shaderFile.is_open()) {
        shaderFile << "fn sdf_sphere(pos: vec3<f32>) -> DistanceColor {\n";
        shaderFile << "    let t = get_time() * 0.5; // Simulated time for animation\n";
        shaderFile << "    let animated_position = vec3(sin(t) * 2.0, sin(t) * 2.0, cos(t) * 2.0 + 4.0);\n";
        shaderFile << "    let distance = length(pos - animated_position) - 0.5; // Sphere radius of 0.5\n";
        shaderFile << "    return DistanceColor(distance, vec3(1.0, 0.0, 0.0)); // Red color for the first sphere\n";
        shaderFile << "}\n";
        shaderFile.close();
    } else {
        std::cerr << "Unable to open shader file for writing." << std::endl;
    }
}