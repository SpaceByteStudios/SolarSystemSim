#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "Planet.h"

class Renderer
{
public:
	Renderer(sf::RenderWindow& window);

	void clearWindow();
	void displayWindow();

	bool isWindowOpen();

	void setCameraPos(const sf::Vector2f& pos);
	void setCameraRot(float rot);
	void setCameraZoom(float zoom);

	void moveCamera(const sf::Vector2f& movement);
	void rotateCamera(float rotation);
	void zoomCamera(float zooming);
	
	sf::View getCamera();

	void renderPlanets(std::vector<Planet>& planets);
private:
	sf::RenderWindow& window;
	sf::View camera;
};