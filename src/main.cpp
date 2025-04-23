#include <webgpu/webgpu_glfw.h>
#include <webgpu/webgpu_cpp.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <core/core.h>




int main() {
    Core core;

    if(!core.Initialize())
        return 0;
    

    while(core.IsRunning())
        core.MainLoop();
    
    core.Terminate();

    return 0;
}