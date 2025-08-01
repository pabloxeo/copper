#ifndef CODER_H
#define CODER_H

class Renderer;

#include <vector>
#include <string>
#include <glm/glm.hpp>


struct SelectedObjectProperties {
    glm::vec3 size;
    glm::vec3 position;
    glm::vec3 color;
};

struct Object {
    int id; // Unique identifier
    float x, y, z;
    float r, g, b; // Color
    std::vector<float> size; // For spheres, this is a single float; for boxes, it can be a vector of 3 floats
    std::string type; // "sphere", "box", etc.
    std::string operation; // "union", "intersection", or "subtract"
};

class Coder {

  private:
    std::string base;
    std::string shaderCode;
    Renderer* renderer; // Pointer to the Renderer instance
    int objectIdCounter = 1; // Counter for unique object IDs
    Object defaultObject;
    int selectedObjectId = -1; // ID of the currently selected object

  public:
    std::vector<Object> objects;
    Coder(Renderer* renderer);
    void generateShaderCode(); // helper to rebuild WGSL
    void addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation);
    void addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation);
    void clearObjects();
    std::string getShaderCode() { return shaderCode+base; }

    int getSelectedObjectId() { return selectedObjectId; }
    void setSelectedObjectId(int id) { selectedObjectId = id; }

    std::vector<Object>& getObjects() { return objects; }
    Object& getSelectedObject(int id) {
        for (auto& obj : objects) {
            if (obj.id == id) {
                return obj;
            }
        }
        return defaultObject; // Return the default object if not found
    }

    SelectedObjectProperties getSelectedObjectProperties(int id) {
        SelectedObjectProperties props;
        for (const auto& obj : objects) {
            if (obj.id == id) {
                props.size = glm::vec3(obj.size[0], obj.size.size() > 1 ? obj.size[1] : obj.size[0], obj.size.size() > 2 ? obj.size[2] : obj.size[0]);
                props.position = glm::vec3(obj.x, obj.y, obj.z);
                props.color = glm::vec3(obj.r, obj.g, obj.b);

                return props;
            }
        }
        return props;
    }
    void deleteObject(int id) {
        for (auto it = objects.begin(); it != objects.end(); ++it) {
            if (it->id == id) {
                objects.erase(it);
                break;
            }
        }
    }

    void addTest(){
        for(int i = 0; i < 100; ++i) {
            addSphere(i * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, "union");
        }
    }

};

#endif // CODER_H
