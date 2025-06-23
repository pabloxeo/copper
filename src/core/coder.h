#ifndef CODER_H
#define CODER_H

#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu_glfw.h>
#include <ui/window.h>
#include "./renderer.h"


using namespace wgpu;

class Coder {
  public:
    void AddCube(int a, int b);
    void AddSphere(int a, int b);
};
#endif // !CODER_H
