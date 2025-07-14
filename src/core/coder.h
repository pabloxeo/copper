#ifndef CODER_H
#define CODER_H

#include <vector>

struct Sphere {
    float x, y, z, radius;
    float r, g, b; // Color
    std::string operation; // "union" or "intersection"
};

class Coder {
  private:
    std::string shaderCode;
    std::vector<Sphere> spheres;
  public:
    bool useSmoothUnion = true; // Default to smooth union
    Coder();
    void generateShaderCode(); // helper to rebuild WGSL
    void addSphere(float x, float y, float z, float radius, float r, float g, float b, const std::string& operation);
    void clearSpheres();
    std::string getShaderCode() { return shaderCode; }
    // Optionally: expose spheres for UI editing
    std::vector<Sphere>& getSpheres() { return spheres; }
};

#endif // CODER_H
