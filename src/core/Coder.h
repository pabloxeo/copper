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
    int id;
    float x, y, z;
    float r, g, b;
    std::vector<float> size;
    std::string type;
    std::string operation;
};

struct Gizmo {
    glm::vec3 position;
    glm::vec3 xAxis = glm::vec3(1, 0, 0);
    glm::vec3 yAxis = glm::vec3(0, 1, 0);
    glm::vec3 zAxis = glm::vec3(0, 0, 1);
    float scale = 1.0f;
    bool visible = true;
};

class Coder {

  private:

    std::string base;
    std::string shaderCode;
    Renderer* renderer;
    int objectIdCounter = 1;
    Object defaultObject;
    int selectedObjectId = -1;

  public:

    std::vector<Object> objects;

    Coder(Renderer* renderer);

    void generateShaderCode(); 
    std::string getShaderCode() { return shaderCode+base; }

    void addSphere(float x, float y, float z, float size, float r, float g, float b, const std::string& operation);
    void addBox(float x, float y, float z, std::vector<float> size, float r, float g, float b, const std::string& operation);
    void clearObjects();
    

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
    void deleteObject(int id);

    void addTest(){
        for(int i = 0; i < 100; ++i) {
            addSphere(i * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, "union");
        }
    }

    void generateGizmoShader();
    Gizmo getGizmoForSelectedObject();
    std::string getGizmoShaderCode();

    bool saveScene(const std::string& filename);
    bool loadScene(const std::string& filename);
};

#endif // CODER_H
