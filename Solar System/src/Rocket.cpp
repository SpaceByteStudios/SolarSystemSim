#include "Rocket.h"

#include <SFML/System.hpp>
#include <iostream>
#include <numbers>

Rocket::Rocket()
{
	mass = 10.0;
	thrust = 500.0;
	rotation = 0.0f;

	if (!rocketTexture.loadFromFile("assets/rocket.png"))
	{
		std::cerr << "Failed to load rocket texture!" << std::endl;
	};

	rocketSprite.emplace(rocketTexture);
	rocketSprite->setOrigin(sf::Vector2f(rocketTexture.getSize().x / 2.0f, rocketTexture.getSize().y / 2.0f));
	rocketSprite->setScale(sf::Vector2f(0.25f, 0.25f));

	exhaustEmitter = ParticleEmitter(500.0f, 1.0f, 2.0f);
	exhaustEmitter.setParticleColor(sf::Color(255, 128, 0, 150), sf::Color(255, 255, 0, 0));
	exhaustEmitter.setVelocityRange(100.0f, 150.0f);
}

void Rocket::applyForce(const sf::Vector2f& force)
{
	acceleration += force / mass;
}

void Rocket::applyGravity(std::vector<Planet>& planets, float G)
{
	for (int i = 0; i < planets.size(); i++)
	{
		sf::Vector2f pos_dif = planets[i].getPosition() - getPosition();
		float dist = pos_dif.length();
		sf::Vector2f force;
		if (dist > 0.0f)
		{
			sf::Vector2f direction = pos_dif.normalized();
			force = direction * (G * planets[i].getMass() * getMass()) / (dist * dist);
			applyForce(force);
		}
		else
		{
			applyForce(sf::Vector2f());
		}
	}
}

void Rocket::applyThrust(float deltaTime)
{
	sf::Vector2f direction(sin(rotation), -cos(rotation));
	applyForce(direction * thrust);
}

void Rocket::rotate(float angle)
{
	rotation += angle;
	rocketSprite->setRotation(sf::radians(rotation));
	exhaustEmitter.setDirection(sf::Vector2f(sin(rotation + std::numbers::pi_v<float>), -cos(rotation + std::numbers::pi_v<float>)), 45.0f);
}

void Rocket::updateRocket(float deltaTime, std::vector<Planet>& planets, float G)
{
	//Verlet Integration
	position += velocity * deltaTime + 0.5f * acceleration_before * deltaTime * deltaTime;
	rocketSprite->setPosition(position);

	applyGravity(planets, G);

	velocity += 0.5f * (acceleration_before + acceleration) * deltaTime;
	acceleration_before = acceleration;
	acceleration *= 0.0f;

	exhaustEmitter.update(deltaTime);
	float offsetScale = rocketTexture.getSize().y / 4.0f * rocketSprite->getScale().y + 2.0f;
	sf::Vector2f exhaustOffset = sf::Vector2f(sin(rotation + std::numbers::pi_v<float>), -cos(rotation + std::numbers::pi_v<float>)) * offsetScale;
	exhaustEmitter.setPosition(position + exhaustOffset);
	exhaustEmitter.setVelocity(velocity);
}

void Rocket::draw(sf::RenderWindow& window)
{
	exhaustEmitter.draw(window);
	window.draw(*rocketSprite);
}

void Rocket::reset()
{
	position = sf::Vector2f(0.0f, 0.0f);
	velocity = sf::Vector2f(0.0f, 0.0f);
	acceleration_before = sf::Vector2f(0.0f, 0.0f);
	rocketSprite->setPosition(position);
	rotation = 0.0f;
	rocketSprite->setRotation(sf::radians(rotation));
}

void Rocket::emitExhaust(bool emit)
{
	exhaustEmitter.isEmitting(emit);
}

void Rocket::setPosition(const sf::Vector2f& pos)
{
	position = pos;
	rocketSprite->setPosition(position);
}

void Rocket::setVelocity(const sf::Vector2f& vel)
{
	velocity = vel;
}

void Rocket::setAcceleration(const sf::Vector2f& acc)
{
	acceleration_before = acc;
	acceleration = acc;
}

void Rocket::setRotation(float rot)
{
	rotation = rot;
	exhaustEmitter.setDirection(sf::Vector2f(sin(rotation + std::numbers::pi_v<float>), -cos(rotation + std::numbers::pi_v<float>)), 45.0f);
}

void Rocket::setMass(float m)
{
	mass = m;
}

void Rocket::setThrust(float t)
{
	thrust = t;
	float rate = t / 5000.0f;
	exhaustEmitter.setEmissionRate(rate * 1000.0f);
}

sf::Vector2f Rocket::getPosition() const
{
	return position;
}

sf::Vector2f Rocket::getVelocity() const
{
	return velocity;
}

float Rocket::getMass() const
{
	return mass;
}

float Rocket::getThrust() const
{
	return thrust;
}
