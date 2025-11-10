#include <numbers>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <windows.h>

#include "Simulation.h"
#include "Planet.h"
#include "Renderer.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui-SFML.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Simulation::Simulation(float gravity)
    : Simulation(sf::VideoMode::getDesktopMode(), gravity)
{
}

Simulation::Simulation(const sf::Vector2u& res, float gravity)
    : Simulation(sf::VideoMode(res), gravity)
{
}

Simulation::Simulation(const sf::VideoMode& mode, float gravity)
    : window(mode, "Solar System Simulation", sf::Style::None)
    , renderer(window)
{
    bool init = ImGui::SFML::Init(window);
    if (!init)
    {
        std::cerr << "Failed to initialize ImGui-SFML" << std::endl;
        stop();
    }

    bool loaded = font.openFromFile("assets/SpaceMono.ttf");
    if (!loaded)
        std::cerr << "Failed to load the font!" << std::endl;

    std::filesystem::create_directories("scenes");

    camera_speed = 500.0f;
    G = gravity;

    show_ui = true;
    show_name = false;
    show_trail = false;
    show_trajectory = false;
    trajectory_steps = 100;

    follow_planet = false;
	follow_rocket = false;
    draw_vel_arrow = false;
    draw_acc_arrow = false;

    total_energy = 0.0f;
    delta_time = 0.0f;
    time_scale = 1.0f;
	time_passed = 0.0f;
    selectedPlanet = 0;
    selectedScene = 0;
	rocket_turn_rate = std::numbers::pi_v<float>;

    state = State::Running;

    refreshScenesList();
}

void Simulation::run()
{
	delta_time = clock.restart().asSeconds();
	if (delta_time > 0.1f) delta_time = 0.1f;

    //Verlet Integration
    if (state == State::Running)
    {
        for (int i = 0; i < planets.size(); i++)
        {
            planets[i].updatePos(delta_time * time_scale);
            planets[i].updateTrail(delta_time * time_scale);
        }

        for (int i = 0; i < planets.size(); i++)
        {
            planets[i].updateAcc(planets, G);
            planets[i].updateVel(delta_time * time_scale);
        }

        if (rocket.has_value())
        {
			rocket->updateRocket(delta_time * time_scale, planets, G);
        }
        
        time_passed += delta_time * time_scale;
    }

    total_energy = totalEnergy();
}

void Simulation::pause()
{
    state = State::Paused;
}

void Simulation::resume()
{
    state = State::Running;
}

void Simulation::stop()
{
    state = State::Stopped;
}

void Simulation::reset()
{
	time_passed = 0.0f;
    for (int i = 0; i < planets.size(); i++)
    {
        planets[i].reset();
    }
    if (rocket.has_value())
    {
		rocket->reset();
    }
}

void Simulation::changeTimescale(float scale)
{
    time_scale = scale;
}

void Simulation::render()
{
    renderer.clearWindow();
    
    if (show_trajectory || show_rocket_trajectory)
    {
        calculateTrajectories(trajectory_steps);
        drawTrajectory(trajectory_steps);
    }

    renderer.renderPlanets(planets);
    
    if (selectedPlanet < planets.size())
    {
        planets[selectedPlanet].drawVelArrow(window, renderer.getCamera(), draw_vel_arrow);
        planets[selectedPlanet].drawAccArrow(window, renderer.getCamera(), draw_acc_arrow);
    }

    if(rocket.has_value())
    {
		renderer.renderRocket(*rocket);
	}
    
    if (show_ui)
    {
        ImGui::SFML::Render(window);
    }
    else
    {
        ImGui::EndFrame();
    }

    renderer.displayWindow();
}

void Simulation::processInput()
{
    while (const auto event = window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(window, *event);

        if (event->is<sf::Event::Closed>())
            window.close();

        if (event->is<sf::Event::FocusLost>())
        {
            HWND hwnd = window.getNativeHandle();
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        if (event->is<sf::Event::FocusGained>())
        {
            HWND hwnd = window.getNativeHandle();
            ShowWindow(hwnd, SW_RESTORE);
        }

        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Space)
            {
                switch (state)
                {
                    case State::Paused:
                        state = State::Running;
                        break;
                    case State::Running:
                        state = State::Paused;
                        break;
                }
            }

            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
            {
				show_ui = !show_ui;
            }
        }
    }

    sf::View camera = renderer.getCamera();
    sf::Angle cam_rotation = camera.getRotation();
    sf::Vector2f camera_move;
    float camera_rotation = 0.0f;
    float camera_zoom = 1.0f;
    float zoom_factor = camera.getSize().x / window.getSize().x;
    float move_factor = zoom_factor * camera_speed * delta_time;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) camera_move += sf::Vector2f( 0.0f, -1.0f).rotatedBy(cam_rotation) * move_factor;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) camera_move += sf::Vector2f( 0.0f,  1.0f).rotatedBy(cam_rotation) * move_factor;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) camera_move += sf::Vector2f(-1.0f,  0.0f).rotatedBy(cam_rotation) * move_factor;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) camera_move += sf::Vector2f( 1.0f,  0.0f).rotatedBy(cam_rotation) * move_factor;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)) camera_rotation += std::numbers::pi_v<float> * 0.5 * delta_time;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) camera_rotation -= std::numbers::pi_v<float> * 0.5 * delta_time;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)) camera_zoom += 1.0 * delta_time;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)) camera_zoom -= 1.0 * delta_time;

    if (rocket.has_value() && state == State::Running)
    {
        float rocket_rotation = 0.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) rocket->applyThrust(delta_time * time_scale);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) rocket->emitExhaust(true);
		else rocket->emitExhaust(false);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) rocket_rotation -= rocket_turn_rate * delta_time * time_scale;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) rocket_rotation += rocket_turn_rate * delta_time * time_scale;

		rocket->rotate(rocket_rotation);
    }

    renderer.moveCamera(camera_move);
    renderer.rotateCamera(camera_rotation);
    renderer.zoomCamera(camera_zoom);
}

void Simulation::updateUI()
{
    ImGui::SFML::Update(window, sf::seconds(delta_time));
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.7f));
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(350.0f, 100.0f), ImVec2(350.0f, 900.0f));
    ImGui::Begin("Scene Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SeparatorText("Scenes");
    if (ImGui::Button("Refresh Scenes"))
        refreshScenesList();

    if (scenes.empty())
    {
        ImGui::Text("No Scenes saved.");
        if (ImGui::Button("Save as New Scene"))
        {
            json j = planets;

            std::ofstream file(std::string("scenes/scene_New.json"));
            if (file.is_open())
            {
                file << j.dump(4);
                file.close();
            }
			refreshScenesList();
        }
    }
    else
    {
        if (selectedScene >= scenes.size())
        {
            selectedScene = 0;
        }

        if (ImGui::BeginCombo("Select Scene", scenes[selectedScene].c_str()))
        {
            for (int i = 0; i < scenes.size(); i++)
            {
                bool isSelected = (selectedScene == i);
                if (ImGui::Selectable(scenes[i].c_str(), isSelected))
                {
                    selectedScene = i;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        std::string& s = scenes[selectedScene];

        ImGui::InputText("##", &scene_name);
		ImGui::SameLine();
        if (ImGui::Button("Rename Scene"))
        {
            std::string old_name = std::string("scenes/") + scenes[selectedScene] + ".json";
			std::string new_name = std::string("scenes/") + scene_name + ".json";
            std::filesystem::rename(old_name, new_name);
			refreshScenesList();
			selectedScene = std::distance(scenes.begin(), std::find(scenes.begin(), scenes.end(), scene_name));
        }

        if (ImGui::Button("Save Scene"))
        {
            json j = planets;

            std::ofstream file(std::string("scenes/") + scenes[selectedScene] + ".json");
            if (file.is_open())
            {
                file << j.dump(4);
                file.close();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Save as New Scene"))
        {
            json j = planets;

			std::string new_name = scenes[selectedScene] + "_New";

            std::ofstream file(std::string("scenes/") + new_name + ".json");
            if (file.is_open())
            {
                file << j.dump(4);
                file.close();
            }
            refreshScenesList();
            selectedScene = std::distance(scenes.begin(), std::find(scenes.begin(), scenes.end(), new_name));
        }

        if (ImGui::Button("Load Scene"))
        {
            std::ifstream infile(std::string("scenes/") + scenes[selectedScene] + ".json");
            if (infile.is_open())
            {
                json j;
                infile >> j;
                planets = j.get<std::vector<Planet>>();

                for (int i = 0; i < planets.size(); i++)
                {
                    planets[i].initLabel(font);
                    planets[i].showLabel(show_name);
                    planets[i].showTrail(show_trail);
                }

                reset();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete Scene"))
        {
            std::string file_name = std::string("scenes/") + scenes[selectedScene] + ".json";
            std::filesystem::remove(file_name);
            refreshScenesList();
		}
    }

    ImGui::SeparatorText("Controls");
    if (ImGui::Button("Reset"))
        reset();
    
    bool paused = state == State::Paused;
    if (ImGui::Checkbox("Pause Simulation", &paused))
    {
        state = paused ? State::Paused : State::Running;
    }

    if (ImGui::Checkbox("Show planets name", &show_name))
    {
        for (int i = 0; i < planets.size(); i++)
        {
            planets[i].showLabel(show_name);
        }
    }

    if (ImGui::Checkbox("Show planets trail", &show_trail))
    {
        for (int i = 0; i < planets.size(); i++)
        {
            planets[i].showTrail(show_trail);
        }
    }

    ImGui::Checkbox("Show planets trajectory", &show_trajectory);
    ImGui::Checkbox("Show rocket trajectory", &show_rocket_trajectory);
    ImGui::DragInt("trajectory steps", &trajectory_steps, 1.0f, 1, 20000);

    ImGui::DragFloat("timescale", &time_scale, 0.01f, 0.0f, 0.0f, "%.2f");
    ImGui::DragFloat("gravity", &G, 1.0f, 0.0f, 0.0f, "%.0f");

    ImGui::SeparatorText("Planets");
    if (planets.empty())
    {
        ImGui::Text("No planets available.");
        if (ImGui::Button("Add Planet"))
        {
            addPlanet(sf::Vector2f(0.0f, 0.0f), 500.0f);
        };
    }
    else
    {
        if (selectedPlanet >= planets.size())
        {
            selectedPlanet = 0;
        }

        if (ImGui::BeginCombo("Select Planet", planets[selectedPlanet].getName().c_str()))
        {
            for (int i = 0; i < planets.size(); i++)
            {
                bool isSelected = (selectedPlanet == i);
                if (ImGui::Selectable(planets[i].getName().c_str(), isSelected))
                {
                    selectedPlanet = i;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        Planet& p = planets[selectedPlanet];
        
        bool requestAdd = false;
        bool requestRemove = false;
        if (ImGui::Button("Add Planet"))
        {
            requestAdd = true;
        };
        ImGui::SameLine();
        if (ImGui::Button("Remove Planet"))
        {
            requestRemove = true;
        };

		std::string name = p.getName();
        if (ImGui::InputText("Planet Name", &name))
        {
			p.setName(name);
        }

        float init_pos[2] = { p.getInitialPosition().x, p.getInitialPosition().y };
        if (ImGui::DragFloat2("Start Position", init_pos, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setInitialPosition(sf::Vector2f(init_pos[0], init_pos[1]));
        }

        float init_vel[2] = { p.getInitialVelocity().x, p.getInitialVelocity().y };
        if (ImGui::DragFloat2("Start Velocity", init_vel, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setInitialVelocity(sf::Vector2f(init_vel[0], init_vel[1]));
        }

        float pos[2] = { p.getPosition().x, p.getPosition().y};
        if (ImGui::DragFloat2("Position", pos, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setPosition(sf::Vector2f(pos[0], pos[1]));
        }

        float vel[2] = { p.getVelocity().x, p.getVelocity().y};
        if (ImGui::DragFloat2("Velocity", vel, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setVelocity(sf::Vector2f(vel[0], vel[1]));
        }

        if (ImGui::Button("Set As Start Values"))
        {
            p.setInitialPosition(p.getPosition());
            p.setInitialVelocity(p.getVelocity());
        }

        float mass = p.getMass();
        if (ImGui::DragFloat("Mass", &mass, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setMass(mass);
        }

        float radius = p.getRadius();
        if (ImGui::DragFloat("Radius", &radius, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            p.setRadius(radius);
        }

        float col[3] = {
            p.getColor().r / 255.0f,
            p.getColor().g / 255.0f,
            p.getColor().b / 255.0f
        };
        if (ImGui::ColorEdit3("Color", col))
        {
            sf::Color new_col;
            new_col.r = (int)(col[0] * 255);
            new_col.g = (int)(col[1] * 255);
            new_col.b = (int)(col[2] * 255);
            p.setColor(new_col);
        }
        ImGui::Checkbox("Draw Vel Arrow", &draw_vel_arrow);
        ImGui::Checkbox("Draw Acc Arrow", &draw_acc_arrow);

        if (requestAdd)
        {
            addPlanet(sf::Vector2f(0.0f, 0.0f), 500.0f);
        }

        if (requestRemove)
        {
            removePlanet(selectedPlanet);
        }
    }

	ImGui::SeparatorText("Rocket");
    if (rocket.has_value())
    {
        bool requestRemove = false;
        if (ImGui::Button("Remove Rocket"))
        {
            requestRemove = true;
        };

        float pos[2] = { rocket->getPosition().x, rocket->getPosition().y };
        if (ImGui::DragFloat2("Rocket Position", pos, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            rocket->setPosition(sf::Vector2f(pos[0], pos[1]));
        }

        float vel[2] = { rocket->getVelocity().x, rocket->getVelocity().y };
        if (ImGui::DragFloat2("Rocket Velocity", vel, 1.0f, 0.0f, 0.0f, "%.1f"))
        {
            rocket->setVelocity(sf::Vector2f(vel[0], vel[1]));
        }

        float thrust = rocket->getThrust();
        if (ImGui::SliderFloat("Thrust", &thrust, 0.0f, 5000.0f, "%.1f"))
        {
			rocket->setThrust(thrust);
        }

        float turn_speed = rocket_turn_rate * 180.0f / std::numbers::pi_v<float>;
        if (ImGui::SliderFloat("Turn rate", &turn_speed, 0.0f, 1440.0f, "%.1f"))
        {
			rocket_turn_rate = turn_speed * std::numbers::pi_v<float> / 180.0f;
        }

        if (requestRemove)
        {
            rocket.reset();
        }
    }
    else
    {
        if (ImGui::Button("Add Rocket"))
        {
            rocket.emplace();
		}
    }
    
    sf::View camera = renderer.getCamera();
    sf::Vector2f cam_pos = camera.getCenter();
    float cam_rot = camera.getRotation().asDegrees();
    float cam_zoom = camera.getSize().x / window.getSize().x;
    ImGui::SeparatorText("Camera");

    if (planets.size() <= 0)
    {
        follow_planet = false;
    }

    if(!rocket.has_value())
    {
        follow_rocket = false;
	}

	ImGui::BeginDisabled(!(planets.size() > 0));
    ImGui::Checkbox("Follow Planet", &follow_planet);
    if (follow_planet && planets.size() > 0)
    {
		follow_rocket = false;
        renderer.setCameraPos(planets[selectedPlanet].getPosition());
    }
	ImGui::EndDisabled();

    ImGui::BeginDisabled(!rocket.has_value());
    ImGui::Checkbox("Follow Rocket", &follow_rocket);
    if (follow_rocket && rocket.has_value())
    {
		follow_planet = false;
		renderer.setCameraPos(rocket->getPosition());
    }
    ImGui::EndDisabled();

    ImGui::Text("Position: (%.1f, %.1f)", cam_pos.x, cam_pos.y);
    ImGui::Text("Rotation: %.1f Degrees", cam_rot);
    ImGui::Text("Zoom: %.2f", cam_zoom);
    if (ImGui::Button("Reset Camera"))
    {
        if (follow_planet && selectedPlanet << planets.size())
        {
            renderer.setCameraPos(planets[selectedPlanet].getPosition());
        }
        else
        {
            renderer.setCameraPos(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f));
        }
        renderer.setCameraRot(0.0f);
        renderer.setCameraZoom(1.0f);
    }

    ImGui::SeparatorText("Stats");
	ImGui::Text("Time Passed: %.1f s", time_passed);
    ImGui::Text("Total Energy: %.3e", total_energy);
    ImGui::Text("Frame rate: %.1f FPS", ImGui::GetIO().Framerate);

    if (ImGui::Button("Exit"))
    {
        stop();
    }
    
    ImGui::End();
    ImGui::PopStyleColor();
}

bool Simulation::isStopped()
{   
    if (!renderer.isWindowOpen())
        state = State::Stopped;

    if (state == State::Stopped)
        ImGui::SFML::Shutdown(window);

    return state == State::Stopped;
}

void Simulation::addPlanet(const Planet& planet)
{
    Planet p = planet;
    p.initLabel(font);
    p.showLabel(show_name);
	planets.push_back(p);
}

void Simulation::addPlanet(const sf::Vector2f& pos, float mass)
{
    addPlanet(pos, sf::Vector2f(), mass, sf::Color::Red);
}

void Simulation::addPlanet(const sf::Vector2f& pos, float mass, const sf::Color& color)
{
    addPlanet(pos, sf::Vector2f(), mass, color);
}

void Simulation::addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass)
{
    addPlanet(pos, vel, mass, sf::Color::Red);
}

void Simulation::addPlanet(const sf::Vector2f& pos, const sf::Vector2f& vel, float mass, const sf::Color& color)
{
    Planet planet(pos, vel, mass);
    planet.setColor(color);
    planet.initLabel(font);
    planet.showLabel(show_name);
    planets.push_back(planet);
}

void Simulation::removePlanet(int index)
{
    if (index >= 0 && index < planets.size())
    {
        planets.erase(planets.begin() + index);
    }
}

void Simulation::followPlanet(bool follow)
{
	follow_planet = follow;
}

void Simulation::calculateTrajectories(int steps_amount)
{
    trajectory_pos.clear();
	rocket_trajectory_pos.clear();
    std::vector<sf::Vector2f> planet_values;
    float time_step = 1.0f / 10.0f;

    std::vector<Planet> tempPlanets = planets;
	std::optional<Rocket> tempRocket = rocket;

    trajectory_pos.reserve(steps_amount * std::max(1, (int)tempPlanets.size()));

    if (tempRocket.has_value())
    {
        rocket_trajectory_pos.reserve(steps_amount);
		tempRocket->setAcceleration(sf::Vector2f(0.0f, 0.0f));
		tempRocket->emitExhaust(false);
    }

    for (int step = 0; step < steps_amount; step++)
    {
        for (auto& p : tempPlanets)
        {
            trajectory_pos.push_back(p.getPosition());
            p.updatePos(time_step);
        }
        for (auto& p : tempPlanets)
        {
            p.updateAcc(tempPlanets, G);
            p.updateVel(time_step);
        }

        if (tempRocket.has_value())
        {
            rocket_trajectory_pos.push_back(tempRocket->getPosition());
			tempRocket->updateRocket(time_step, tempPlanets, G);
        }
    }
}

void Simulation::drawTrajectory(int steps_amount)
{
	window.setView(renderer.getCamera());

    if (show_trajectory && !trajectory_pos.empty())
    {
        for (int i = 0; i < planets.size(); i++)
        {
            sf::Color color = planets[i].getColor();
            sf::VertexArray line(sf::PrimitiveType::LineStrip, steps_amount);

            for (int j = 0; j < steps_amount; j++)
            {
                line[j].color = color;
                line[j].position = trajectory_pos[i + j * planets.size()];
            }

            window.draw(line);
        }
    }

    if (show_rocket_trajectory && !rocket_trajectory_pos.empty())
    {
        sf::VertexArray line(sf::PrimitiveType::LineStrip, steps_amount);

        for (int i = 0; i < steps_amount; i++)
        {
            line[i].color = sf::Color::White;
            line[i].position = rocket_trajectory_pos[i];
        }

        window.draw(line);
    }
}

void Simulation::refreshScenesList()
{
    scenes.clear();
    for (const auto& entry : std::filesystem::directory_iterator("scenes"))
    {
        if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".json")
        {
            scenes.push_back(entry.path().stem().string());
        }
	}
}

std::vector<Planet>& Simulation::getPlanets()
{
    return planets;
}

float Simulation::totalEnergy()
{
    float KE = 0.0f;
    float PE = 0.0f;
    
    for (auto& p : planets)
    {
        KE += 0.5f * p.getMass() * (p.getVelocity().x * p.getVelocity().x + p.getVelocity().y * p.getVelocity().y);
    }
    
    for (size_t i = 0; i < planets.size(); ++i)
    {
        for (size_t j = i + 1; j < planets.size(); ++j)
        {
            sf::Vector2f dr = planets[i].getPosition() - planets[j].getPosition();
            float dist = std::sqrt(dr.x * dr.x + dr.y * dr.y);
            PE -= G * planets[i].getMass() * planets[j].getMass() / dist;
        }
    }

    return KE + PE;
}