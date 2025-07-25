#ifndef CODER_H
#define CODER_H

#include <vector>
#include <string>
#include "./renderer.h"

struct Object {
    float x, y, z;
    float r, g, b; // Color
    std::vector<float> size; // For spheres, this is a single float; for boxes, it can be a vector of 3 floats
    std::string type; // "sphere", "box", etc.
    std::string operation; // "union", "intersection", or "subtract"
    bool selected = false; // For GUI selection
};


class Coder {
  private:
    std::string base;
    Renderer* renderer; // Pointer to the Renderer instance
    std::string shaderCode;
    std::vector<Object> objects;
  public:
    Coder(Renderer* renderer);
    void generateShaderCode(); // helper to rebuild WGSL
    void addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation);
    void addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation);
    void clearObjects();
    std::string getShaderCode() { return shaderCode+base; }
    std::vector<Object>& getObjects() {  return objects; }
};

#endif // CODER_H
