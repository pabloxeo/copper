#ifndef CORE_H
#define CORE_H

#include "../core/Renderer.h"

using namespace wgpu;


class Core {
public:
    bool Initialize();

    void Terminate();

    void MainLoop();

    bool IsRunning();

private:
    Window window;
    Renderer renderer;
};



#endif //CORE_H