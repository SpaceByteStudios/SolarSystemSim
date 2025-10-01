#include <numbers>
#include <iostream>
#include <random>
#include "Planet.h"

static const std::vector<std::string> planetNames = {
    "Aegir", "Boreas", "Ceres", "Draconis", "Erebus",
    "Fenrir", "Gaia", "Helios", "Icarus", "Juno",
    "Kraken", "Luna", "Morpheus", "Nyx", "Orion",
    "Pegasus", "Quillon", "Rhea", "Selene", "Titan",
    "Umbra", "Vega", "Wodan", "Xanadu", "Ymir",
    "Zephyr", "Altair", "Bellatrix", "Calypso", "Daedalus",
    "Elysium", "Fornax", "Ganymede", "Hyperion", "Io",
    "Janus", "Kael", "Lyra", "Maia", "Nemesis",
    "Oberon", "Phobos", "Quasar", "Ragnar", "Sirius",
    "Tethys", "Umbriel", "Vulcan", "Wyvern", "Xerxes",
    "Yttrium", "Zephyrus", "Artemis", "Balthor", "Calyx",
    "Dione", "Eretria", "Freyja", "Gorgon", "Hestia",
    "Ishtar", "Jotun", "Kronos", "Loki", "Minerva",
    "Neptune", "Orpheus", "Prometheus", "Quintus", "RheaSilva",
    "Saturn", "Thalassa", "Uranus", "Vulpecula", "Wraith",
    "Xanthus", "Yavanna", "Zealot", "Astraea", "Brontes",
    "Callisto", "Deimos", "Elara", "Faunus", "Griffin",
    "Hippolyta", "Iapetus", "Jormungand", "Klytia", "Luminara",
    "Mimas", "Nereus", "Odin", "Pandora", "Quirinus",
    "Ragnarok", "Selwyn", "Thorne", "Umbrielia", "Vespera",
    "Wysteria", "Xyphos", "Yggdrasil", "Zephira", "Aquila",
    "Borealis", "Cerberus", "Dagon", "Erevan", "Fafnir",
    "Ganyra", "Hades", "Isolde", "Javelin", "Kaelith"
};

std::string getRandomName()
{
	static std::mt19937 gen{ std::random_device{}() };
	std::uniform_int_distribution<> dist(0, planetNames.size() - 1);
	return planetNames[dist(gen)];
}

Planet::Planet(const sf::Vector2f& pos, float mass) : Planet(pos, sf::Vector2f(0.0f, 0.0f), mass)
{}

Planet::Planet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass) : 
	mass(mass), radius(sqrt(mass / std::numbers::pi_v<float>))
{
	initial_position = pos;
	initial_velocity = vel;
	position = pos;
	velocity = vel;

	name = getRandomName();
	showing_trail = false;

	shape.setRadius(radius);
	shape.setPosition(position);
	shape.setFillColor(sf::Color::Red);
	shape.setOrigin(sf::Vector2f(radius, radius));
}

void Planet::applyForce(const sf::Vector2f& force)
{
	if (mass <= 0.0f)
	{
		acceleration += force;
	}
	else
	{
		acceleration += force / mass;
	}
}

void Planet::applyGravity(std::vector<Planet>& planets, float G)
{
	for (int i = 0; i < planets.size(); i++)
	{
		if (&planets[i] == this)
		{
			continue;
		}

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

void Planet::updateShapePos()
{
	shape.setPosition(position);
}

void Planet::updateLabelPos()
{
	label->setPosition(sf::Vector2f(position.x, position.y + radius + 7));
	sf::FloatRect textRect = label->getLocalBounds();
	label->setOrigin(sf::Vector2f(textRect.size.x / 2.0f, textRect.size.y / 2.0f));
}

void Planet::updatePos(float deltaTime)
{
	position += velocity * deltaTime + 0.5f * acceleration_before * deltaTime * deltaTime;
	updateShapePos();
	updateLabelPos();
}

void Planet::updateAcc(std::vector<Planet>& planets, float G)
{
	applyGravity(planets, G);
}

void Planet::updateVel(float deltaTime)
{
	velocity += 0.5f * (acceleration_before + acceleration) * deltaTime;
	acceleration_before = acceleration;
	acceleration *= 0.0f;
}

void Planet::draw(sf::RenderWindow& window) const
{
	window.draw(shape);
}

void Planet::drawLabel(sf::RenderWindow& window, const sf::View& cameraView)
{
	sf::Vector2f worldPos = shape.getPosition();
	sf::Vector2i screenPos = window.mapCoordsToPixel(worldPos, cameraView);
	float zoom_factor = cameraView.getSize().x / window.getSize().x;
	label->setPosition(sf::Vector2f((float)screenPos.x, (float)screenPos.y + (radius + 32) / zoom_factor));
	window.draw(label.value());
}

void Planet::drawArrow(sf::RenderWindow& window, const sf::View& cameraView, 
	const sf::Vector2f& vec, float length, sf::Color color = sf::Color::White)
{
	sf::Vector2i screenPos = window.mapCoordsToPixel(position, cameraView);
	sf::Vector2f pos((float)screenPos.x, (float)screenPos.y);

	sf::Vector2f dir = vec.normalized();
	sf::Vector2f end = pos + dir * length;
	
	float headLength = 20.f;
	float headWidth = 10.f;
	sf::Vector2f perp(-dir.y, dir.x);

	sf::Vector2f thickness = perp * 2.0f;
	sf::ConvexShape shaft;
	shaft.setPointCount(4);
	shaft.setPoint(0, pos + thickness);
	shaft.setPoint(1, pos - thickness);
	shaft.setPoint(2, end - thickness);
	shaft.setPoint(3, end + thickness);
	shaft.setFillColor(color);

	sf::ConvexShape arrowHead;
	arrowHead.setPointCount(3);
	arrowHead.setPoint(0, end + dir * headLength);
	arrowHead.setPoint(1, end + perp * headWidth);
	arrowHead.setPoint(2, end - perp * headWidth);
	arrowHead.setFillColor(color);
	
	window.draw(shaft);
	window.draw(arrowHead);
}

void Planet::drawVelArrow(sf::RenderWindow& window, const sf::View& cameraView, bool drawing)
{
	if (drawing && velocity.length() > 0.0f)
	{
		drawArrow(window, cameraView, velocity, 50.0f, sf::Color(66, 135, 245));
	}
}

void Planet::drawAccArrow(sf::RenderWindow& window, const sf::View& cameraView, bool drawing)
{
	if (drawing && acceleration_before.length() > 0.0f)
	{
		drawArrow(window, cameraView, acceleration_before, 50.0f, sf::Color(245, 66, 66));
	}
}

void Planet::updateTrail(float deltaTime)
{
	trail.push_back(position);

	float amount_factor = (1.0f / 60.0f) / deltaTime;
	std::size_t maxSize = static_cast<std::size_t>(300 * amount_factor);

	if (trail.size() > maxSize)
	{
		trail.erase(trail.begin(), trail.begin() + (trail.size() - maxSize));
	}
}

void Planet::drawTrail(sf::RenderWindow& window)
{
	if (!showing_trail)
		return;

	sf::VertexArray line(sf::PrimitiveType::LineStrip, trail.size());

	for (std::size_t i = 0; i < trail.size(); i++)
	{
		line[i].position = trail[i];
		float alpha = 192.0f * (static_cast<float>(i) / trail.size());
		sf::Color color(255, 255, 255, alpha);
		line[i].color = color;
	}

	window.draw(line);
}

void Planet::reset()
{
	position = initial_position;
	velocity = initial_velocity;
	acceleration_before = sf::Vector2f(0.0f, 0.0f);
	updateShapePos();
	updateLabelPos();
	trail.clear();
}

void Planet::initLabel(const sf::Font& font)
{
	label.emplace(font, name);
	label->setCharacterSize(14);
	label->setFillColor(sf::Color::Transparent);
	sf::FloatRect textRect = label->getLocalBounds();
	label->setOrigin(sf::Vector2f(textRect.size.x / 2.0f, textRect.size.y / 2.0f));
	label->setPosition(sf::Vector2f(position.x, position.y + radius + 32));
}

void Planet::showLabel(bool showing)
{
	if(showing)
		label->setFillColor(sf::Color::White);
	else
		label->setFillColor(sf::Color::Transparent);
}

void Planet::showTrail(bool showing)
{
	showing_trail = showing;
}

sf::Vector2f Planet::getInitialPosition()
{
	return initial_position;
}

sf::Vector2f Planet::getInitialVelocity()
{
	return initial_velocity;
}

sf::Vector2f Planet::getPosition()
{
	return position;
}

sf::Vector2f Planet::getVelocity()
{
	return velocity;
}

sf::Vector2f Planet::getAcceleration()
{
	return acceleration;
}

float Planet::getRadius()
{
	return radius;
}

float Planet::getMass()
{
	return mass;
}

std::string Planet::getName()
{
	return name;
}

sf::Color Planet::getColor()
{
	return color;
}

sf::CircleShape Planet::getShape()
{
	return shape;
}

void Planet::setInitialPosition(const sf::Vector2f& init_pos)
{
	initial_position = init_pos;
}

void Planet::setInitialVelocity(const sf::Vector2f& init_vel)
{
	initial_velocity = init_vel;
}

void Planet::setPosition(const sf::Vector2f& pos)
{
	position = pos;
	updateShapePos();
	updateLabelPos();
}

void Planet::setVelocity(const sf::Vector2f& vel)
{

	velocity = vel;
}

void Planet::setAcceleration(const sf::Vector2f& acc)
{
	acceleration = acc;
}

void Planet::setRadius(float radius)
{
	this->radius = radius;
	shape.setRadius(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
}

void Planet::setMass(float mass)
{
	this->mass = mass;
	radius = sqrt(this->mass / std::numbers::pi_v<float>);
	shape.setRadius(radius);
	shape.setOrigin(sf::Vector2f(radius, radius));
}

void Planet::setName(const std::string& name)
{
	this->name = name;
	if (label)
		label->setString(name);
}

void Planet::setColor(const sf::Color& color)
{
	this->color = color;
	shape.setFillColor(color);
}