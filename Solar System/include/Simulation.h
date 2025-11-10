#pragma once
#include <SFML/System.hpp>
#include "Planet.h"
#include "Rocket.h"
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
	Simulation(const sf::VideoMode& mode, float gravity);

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

	void addPlanet(const Planet& planet);
	void addPlanet(const sf::Vector2f& pos, float mass);
	void addPlanet(const sf::Vector2f& pos, float mass, const sf::Color& color);
	void addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass);
	void addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass, const sf::Color& color);
	void removePlanet(int index);
	void followPlanet(bool follow);

	void calculateTrajectories(int steps_amount);
	void drawTrajectory(int steps_amount);

	void refreshScenesList();

	std::vector<Planet>& getPlanets();

	float totalEnergy();
private:
	float camera_speed;
	float G;
	float total_energy;
	float delta_time;
	float time_scale;
	float time_passed;

	int selectedPlanet;
	int selectedScene;

	bool show_ui;
	bool show_name;
	bool show_trail;
	bool show_trajectory;
	bool show_rocket_trajectory;
	std::vector<sf::Vector2f> trajectory_pos;
	std::vector<sf::Vector2f> rocket_trajectory_pos;
	float rocket_turn_rate;

	int trajectory_steps;
	bool follow_planet;
	bool follow_rocket;
	bool draw_vel_arrow;
	bool draw_acc_arrow;
	sf::Font font;

	State state;
	sf::RenderWindow window;
	Renderer renderer;

	std::vector<Planet> planets;
	std::optional<Rocket> rocket;
	std::vector<std::string> scenes;
	std::string scene_name;
	sf::Clock clock;
};