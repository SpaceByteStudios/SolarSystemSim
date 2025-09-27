#pragma once
#include <SFML/System.hpp>
#include "Planet.h"
#include "Renderer.h"

class Simulation
{
public:
	enum class State
	{
		Running,
		Paused,
		Stopped
	};

	Simulation(float gravity);
	Simulation(const sf::Vector2u& res, float gravity);

	void run();
	void render();

	void pause();
	void resume();
	void stop();
	void reset();

	void changeTimescale(float scale);

	void processInput();
	void updateUI();

	bool isStopped();

	void addPlanet(const sf::Vector2f& pos, float mass);
	void addPlanet(const sf::Vector2f& pos, float mass, const sf::Color& color);
	void addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass);
	void addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass, const sf::Color& color);
	void removePlanet(int index);

	void calculateTrajectories(int steps_amount);
	void drawTrajectory(int steps_amount);

	float totalEnergy();
private:
	float camera_speed;
	float G;
	float total_energy;
	float delta_time;
	float time_scale;

	int selectedPlanet;

	bool show_ui;
	bool show_name;
	bool show_trail;
	bool show_trajectory;
	std::vector<sf::Vector2f> trajectory_pos;

	int trajectory_steps;
	bool follow_planet;
	bool draw_vel_arrow;
	bool draw_acc_arrow;
	sf::Font font;

	State state;
	sf::RenderWindow window;
	Renderer renderer;

	std::vector<Planet> planets;
	sf::Clock clock;
};