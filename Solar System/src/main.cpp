#include <windows.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <vector>
#include <numbers>

#include "Planet.h"
#include "Simulation.h"

int main()
{
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);

	const float PI = std::numbers::pi_v<float>;
    const float gravity = 10000.0f;

    Simulation simulation(gravity);

    while (!simulation.isStopped())
    {
        simulation.processInput();
        simulation.run();
        simulation.updateUI();
        simulation.render();
    }
}