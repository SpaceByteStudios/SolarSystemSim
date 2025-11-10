#include "ParticleEmitter.h"
#include "Particle.h"

ParticleEmitter::ParticleEmitter() : ParticleEmitter(10.0f, 1.0f, 5.0f, sf::Color::White)
{}

ParticleEmitter::ParticleEmitter(float emissionRate, float particleLifespan, float particleSize) : ParticleEmitter(emissionRate, particleLifespan, particleSize, sf::Color::White)
{}

ParticleEmitter::ParticleEmitter(float emissionRate, float particleLifespan, float particleSize, const sf::Color& particleColor)
{
	this->emissionRate = emissionRate;
	this->timePerEmission = 1.0f / emissionRate;
	this->particleSize = particleSize;
	this->particleLifespan = particleLifespan;
	this->particleStartColor = particleColor;
	this->particleEndColor = particleColor;

	initialVelocityMin = 1.0f;
	initialVelocityMax = 2.0f;
	spreadAngle = 0.0f;
	
	emitting = false;

	timeSinceLastEmission = 0.0f;
}

void ParticleEmitter::update(float deltaTime)
{
	if (emitting)
	{
		timeSinceLastEmission += deltaTime;

		while(timeSinceLastEmission >= timePerEmission)
		{
			timeSinceLastEmission -= timePerEmission;

			float angle = atan2(direction.y, direction.x);
			float spread = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spreadAngle;
			angle += spread * (3.14159265f / 180.0f);
			float speed = initialVelocityMin + static_cast<float>(rand()) / RAND_MAX * (initialVelocityMax - initialVelocityMin);
			sf::Vector2f p_velocity(cos(angle) * speed, sin(angle) * speed);
			particles.emplace_back(particleLifespan, particleSize, position, velocity + p_velocity);
			particles.back().setColor(particleStartColor, particleEndColor);
		}
	}

	for(int i = 0; i < particles.size(); i++)
	{
		particles[i].update(deltaTime);
		if(!particles[i].isAlive())
		{
			particles.erase(particles.begin() + i);
			i--;
		}
	}
}

void ParticleEmitter::draw(sf::RenderWindow& window)
{
	for(auto& particle : particles)
	{
		particle.draw(window);
	}
}

void ParticleEmitter::isEmitting(bool emit)
{
	emitting = emit;
}

void ParticleEmitter::setEmissionRate(float rate)
{
	emissionRate = rate;
	timePerEmission = 1.0f / emissionRate;
}

void ParticleEmitter::setPosition(const sf::Vector2f& pos)
{
	position = pos;
}

void ParticleEmitter::setVelocity(const sf::Vector2f& vel)
{
	velocity = vel;
}

void ParticleEmitter::setVelocityRange(float min, float max)
{
	initialVelocityMin = min;
	initialVelocityMax = max;
}

void ParticleEmitter::setDirection(const sf::Vector2f& dir, float spread)
{
	direction = dir;
	spreadAngle = spread;
}

void ParticleEmitter::setParticleColor(const sf::Color& color)
{
	particleStartColor = color;
	particleEndColor = color;
}

void ParticleEmitter::setParticleColor(const sf::Color& startColor, const sf::Color& endColor)
{
	particleStartColor = startColor;
	particleEndColor = endColor;
}

void ParticleEmitter::setParticleSize(float size)
{
	particleSize = size;
}

void ParticleEmitter::setParticleLifespan(float lifespan)
{
	particleLifespan = lifespan;
}


