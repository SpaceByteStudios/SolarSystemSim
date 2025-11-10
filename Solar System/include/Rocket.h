#pragma once

#include <SFML/System.hpp>
#include "Planet.h"
#include "ParticleEmitter.h"

class Rocket
{
	public:
		Rocket();

		void applyForce(const sf::Vector2f& force);
		void applyGravity(std::vector<Planet>& planets, float G);
		void applyThrust(float deltaTime);
		void rotate(float angle);
		void updateRocket(float deltaTime, std::vector<Planet>& planets, float G);
		void draw(sf::RenderWindow& window);
		void reset();

		void emitExhaust(bool emit);

		void setPosition(const sf::Vector2f& pos);
		void setVelocity(const sf::Vector2f& vel);
		void setAcceleration(const sf::Vector2f& acc);
		void setRotation(float rot);
		void setMass(float m);
		void setThrust(float t);

		sf::Vector2f getPosition() const;
		sf::Vector2f getVelocity() const;
		float getMass() const;
		float getThrust() const;
	private:
		sf::Vector2f position;
		sf::Vector2f velocity;
		sf::Vector2f acceleration;
		sf::Vector2f acceleration_before;
		float rotation;
		float mass;
		float thrust;

		sf::Texture rocketTexture;
		std::optional<sf::Sprite> rocketSprite;

		ParticleEmitter exhaustEmitter;
};