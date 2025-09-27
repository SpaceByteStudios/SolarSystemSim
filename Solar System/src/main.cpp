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

    const float gravity = 10000.0f;
    sf::Vector2f midpoint(640.0f, 360.0f);

    Simulation simulation(gravity);

    float planet_vel = sqrt((gravity * 100000.0f) / 3000.0f);
    simulation.addPlanet(midpoint, 100000.0f, sf::Color::Yellow);
    simulation.addPlanet(sf::Vector2f(-3000.0f, 0.0) + midpoint, sf::Vector2f(0.0f, -1.0f * planet_vel), 2500.0f);

    while (!simulation.isStopped())
    {
        simulation.processInput();
        simulation.updateUI();
        simulation.run();
        simulation.render();
    }
}