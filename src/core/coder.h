#ifndef CODER_H
#define CODER_H

#include <vector>
#include <string>

struct Object {
    float x, y, z;
    float r, g, b; // Color
    std::vector<float> size; // For spheres, this is a single float; for boxes, it can be a vector of 3 floats
    std::string type; // "sphere", "box", etc.
    std::string operation; // "union", "intersection", or "subtract"
};


class Coder {
  private:
    std::string shaderCode;
    std::vector<Object> objects;
  public:
    bool useSmoothUnion = true; // Default to smooth union
    Coder();
    void generateShaderCode(); // helper to rebuild WGSL
    void addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation);
    void addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation);
    void clearObjects();
    std::string getShaderCode() { return shaderCode; }
    // Optionally: expose spheres for UI editing
    std::vector<Object>& getObjects() {  return objects; }
};

#endif // CODER_H
