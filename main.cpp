// Include the C++ wrapper instead of the raw header(s)
#include "App.h"

int main() {
	App app;

	if (!app.Initialize()) {
		return 1;
	}


	while (app.IsRunning()) {
		app.MainLoop();
	}

	return 0;
}

