#pragma once

#include "Particle.h"

class ParticleEmitter
{
	public:
		ParticleEmitter();
		ParticleEmitter(float emissionRate, float particleLifespan, float particleSize);
		ParticleEmitter(float emissionRate, float particleLifespan, float particleSize, const sf::Color& particleColor);
		void update(float deltaTime);
		void draw(sf::RenderWindow& window);
		void isEmitting(bool emit);

		void setEmissionRate(float rate);

		void setPosition(const sf::Vector2f& pos);
		void setVelocity(const sf::Vector2f& vel);
		void setVelocityRange(float min, float max);
		void setDirection(const sf::Vector2f& dir, float spread);

		void setParticleColor(const sf::Color& color);
		void setParticleColor(const sf::Color& startColor, const sf::Color& endColor);
		void setParticleSize(float size);
		void setParticleLifespan(float lifespan);
	private:
		sf::Vector2f position;
		sf::Vector2f velocity;

		bool emitting;
		float emissionRate;
		float timePerEmission;
		float timeSinceLastEmission;

		float particleSize;
		float particleLifespan;
		sf::Color particleStartColor;
		sf::Color particleEndColor;

		sf::Vector2f direction;
		float spreadAngle;

		float initialVelocityMin;
		float initialVelocityMax;

		std::vector<Particle> particles;
};