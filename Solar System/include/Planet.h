#pragma once
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

class Planet
{
public:
	Planet(const sf::Vector2f& pos, float mass);
	Planet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass);

	void applyForce(const sf::Vector2f& force);
	void applyGravity(std::vector<Planet>& planets, float G);
	void updateShapePos();
	void updateLabelPos();
	void updatePos(float deltaTime);
	void updateAcc(std::vector<Planet>& planets, float G);
	void updateVel(float deltaTime);
	void draw(sf::RenderWindow &window) const;
	void drawLabel(sf::RenderWindow& window, const sf::View& cameraView);
	void drawArrow(sf::RenderWindow& window, const sf::View& cameraView, const sf::Vector2f& vec, float length, sf::Color color);
	void drawVelArrow(sf::RenderWindow& window, const sf::View& cameraView, bool drawing);
	void drawAccArrow(sf::RenderWindow& window, const sf::View& cameraView, bool drawing);
	void updateTrail();
	void drawTrail(sf::RenderWindow& window);
	void reset();
	void initLabel(const sf::Font& font);
	void showLabel(bool showing);
	void showTrail(bool showing);

	sf::Vector2f getInitialPosition();
	sf::Vector2f getInitialVelocity();
	sf::Vector2f getPosition();
	sf::Vector2f getVelocity();
	sf::Vector2f getAcceleration();
	float getRadius();
	float getMass();
	std::string getName();
	sf::Color getColor();
	sf::CircleShape getShape();
	
	void setInitialPosition(const sf::Vector2f& init_pos);
	void setInitialVelocity(const sf::Vector2f& init_vel);
	void setPosition(const sf::Vector2f& pos);
	void setVelocity(const sf::Vector2f& vel);
	void setAcceleration(const sf::Vector2f& acc);
	void setMass(float mass);
	void setColor(const sf::Color& color);
private:
	sf::Vector2f initial_position;
	sf::Vector2f initial_velocity;
	sf::Vector2f position;
	sf::Vector2f velocity;
	sf::Vector2f acceleration_before;
	sf::Vector2f acceleration;

	std::string name;
	float mass;
	float radius;

	bool showing_trail;
	std::vector<sf::Vector2f> trail;

	std::optional<sf::Text> label;
	sf::Color color;
	sf::CircleShape shape;
};