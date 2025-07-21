#include <iostream>
#include <string>
#include "core/core.h"




int main() {
    Core core;

    if(!core.Initialize())
        return 0;
    

    while(core.IsRunning())
        core.MainLoop();
    
    core.Terminate();

    return 0;
}