#include "Renderer.h"

Renderer::Renderer(sf::RenderWindow& renderWindow) : 
	window(renderWindow)
{
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	camera = sf::View(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(window.getSize().x, window.getSize().y)));
}

void Renderer::clearWindow()
{
	window.clear(sf::Color::Black);
}

void Renderer::displayWindow()
{
	window.display();
}

bool Renderer::isWindowOpen()
{
	return window.isOpen();
}

void Renderer::setCameraPos(const sf::Vector2f& pos)
{
	camera.setCenter(pos);
}

void Renderer::setCameraRot(float rot)
{
	camera.setRotation(sf::radians(rot));
}

void Renderer::setCameraZoom(float zoom)
{
	float zoom_factor = camera.getSize().x / window.getSize().x;
	float factor = zoom / zoom_factor;
	camera.zoom(factor);
}

void Renderer::moveCamera(const sf::Vector2f& movement)
{
	camera.move(movement);
}

void Renderer::rotateCamera(float rotation)
{
	camera.rotate(sf::radians(rotation));
}

void Renderer::zoomCamera(float zooming)
{
	camera.zoom(zooming);
}

sf::View Renderer::getCamera()
{
	return camera;
}

void Renderer::renderPlanets(std::vector<Planet>& planets)
{
	for (int i = 0; i < planets.size(); i++)
	{
		window.setView(camera);
		planets[i].drawTrail(window);
		planets[i].draw(window);
		window.setView(window.getDefaultView());
		planets[i].drawLabel(window, camera);
	}
}

void Renderer::renderRocket(Rocket& rocket)
{
	window.setView(camera);
	rocket.draw(window);
}
