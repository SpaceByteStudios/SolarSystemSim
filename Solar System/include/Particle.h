#pragma once

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class Particle
{
	public:
		Particle();
		Particle(float lifespan, float size, const sf::Vector2f& position, const sf::Vector2f& velocity);
		Particle(float lifespan, float size, const sf::Vector2f& position, const sf::Vector2f& velocity, const sf::Color& color);
		void update(float deltaTime);
		void draw(sf::RenderWindow& window);

		bool isAlive() const;

		void setColor(const sf::Color& start, const sf::Color& end);
	private:
		float lifespan;
		float age;
		float size;
		sf::Vector2f position;
		sf::Vector2f velocity;
		sf::Color start_color;
		sf::Color end_color;
		sf::CircleShape shape;
};