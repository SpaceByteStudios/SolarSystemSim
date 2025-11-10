#include "Particle.h"

Particle::Particle() : Particle(1.0f, 5.0f, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(0.0f, 0.0f), sf::Color::White)
{
}

Particle::Particle(float lifespan, float size, const sf::Vector2f& position, const sf::Vector2f& velocity) : Particle(lifespan, size, position, velocity, sf::Color::White)
{
}

Particle::Particle(float lifespan, float size, const sf::Vector2f& position, const sf::Vector2f& velocity, const sf::Color& color)
{
	age = 0.0f;
	this->lifespan = lifespan;
	this->size = size;
	this->position = position;
	this->velocity = velocity;
	this->start_color = color;
	this->end_color = color;
	shape.setRadius(size);
	shape.setFillColor(color);
	shape.setOrigin(sf::Vector2f(size, size));
	shape.setPosition(position);
}

void Particle::update(float deltaTime)
{
	age += deltaTime;
	position += velocity * deltaTime;

	float lifeRatio = age / lifespan;
	std::uint8_t r = static_cast<std::uint8_t>(start_color.r + (end_color.r - start_color.r) * lifeRatio);
	std::uint8_t g = static_cast<std::uint8_t>(start_color.g + (end_color.g - start_color.g) * lifeRatio);
	std::uint8_t b = static_cast<std::uint8_t>(start_color.b + (end_color.b - start_color.b) * lifeRatio);
	std::uint8_t a = static_cast<std::uint8_t>(start_color.a + (end_color.a - start_color.a) * lifeRatio);
	shape.setFillColor(sf::Color(r, g, b, a));
}

void Particle::draw(sf::RenderWindow& window)
{
	shape.setPosition(position);
	window.draw(shape);
}

bool Particle::isAlive() const
{
	return age < lifespan;
}

void Particle::setColor(const sf::Color& start, const sf::Color& end)
{
	start_color = start;
	end_color = end;
}
