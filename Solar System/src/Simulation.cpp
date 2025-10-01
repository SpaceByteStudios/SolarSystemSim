#include <numbers>
#include <iostream>
#include "Simulation.h"
#include "Renderer.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui-SFML.h"

Simulation::Simulation(float gravity) : Simulation(sf::Vector2u(1280, 720), gravity)
{}

Simulation::Simulation(const sf::Vector2u& res, float gravity) 
    : window(sf::VideoMode(res), "Solar System Simulation"), renderer(window)
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

    camera_speed = 500.0f;
    G = gravity;

	show_ui = true;
    show_name = false;
    show_trail = false;
    show_trajectory = false;
    trajectory_steps = 100;

    follow_planet = false;
    draw_vel_arrow = false;
    draw_acc_arrow = false;

    total_energy = 0.0f;
    delta_time = 0.0f;
    time_scale = 1.0f;
    selectedPlanet = 0;

    state = State::Running;
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
    }

    if (show_trajectory)
    {
        calculateTrajectories(trajectory_steps);
    }

    total_energy = totalEnergy();
    //std::cout << total_energy << std::endl;
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
    for (int i = 0; i < planets.size(); i++)
    {
        planets[i].reset();
    }
}

void Simulation::changeTimescale(float scale)
{
    time_scale = scale;
}

void Simulation::render()
{
    renderer.clearWindow();

    if (show_trajectory && !trajectory_pos.empty())
    {
        drawTrajectory(trajectory_steps);
    }

    renderer.renderPlanets(planets);
    
    if (selectedPlanet < planets.size())
    {
        planets[selectedPlanet].drawVelArrow(window, renderer.getCamera(), draw_vel_arrow);
        planets[selectedPlanet].drawAccArrow(window, renderer.getCamera(), draw_acc_arrow);
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

    renderer.moveCamera(camera_move);
    renderer.rotateCamera(camera_rotation);
    renderer.zoomCamera(camera_zoom);
}

void Simulation::updateUI()
{
    ImGui::SFML::Update(window, sf::seconds(delta_time));
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.7f));
	ImGui::SetNextWindowSize(ImVec2(350.0f, 700.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::Begin("Scene Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

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
    ImGui::DragInt("trajectory steps", &trajectory_steps);

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

        if( ImGui::Button("Set As Start Values"))
        {
            p.setInitialPosition(p.getPosition());
			p.setInitialVelocity(p.getVelocity());
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
        ImGui::Checkbox("Follow Planet", &follow_planet);
        if (follow_planet)
        {
            renderer.setCameraPos(p.getPosition());
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
    
    sf::View camera = renderer.getCamera();
    sf::Vector2f cam_pos = camera.getCenter();
    float cam_rot = camera.getRotation().asDegrees();
    float cam_zoom = camera.getSize().x / window.getSize().x;
    ImGui::SeparatorText("Camera");
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
    ImGui::Text("Total Energy: %.3e", total_energy);
    ImGui::Text("Frame rate: %.1f FPS", ImGui::GetIO().Framerate);
    
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

void Simulation::calculateTrajectories(int steps_amount)
{
    trajectory_pos.clear();
    std::vector<sf::Vector2f> planet_values;
    float time_step = 1.0f / 10.0f;

    std::vector<Planet> tempPlanets = planets;

    for (int step = 0; step < steps_amount; step++)
    {
        for (auto& p : tempPlanets)
        {
            trajectory_pos.push_back(p.getPosition());
        }

        for (auto& p : tempPlanets)
        {
            p.updatePos(time_step);
        }
        for (auto& p : tempPlanets)
        {
            p.updateAcc(tempPlanets, G);
            p.updateVel(time_step);
        }
    }
}

void Simulation::drawTrajectory(int steps_amount)
{
    for (int i = 0; i < planets.size(); i++)
    {
        sf::Color color = planets[i].getColor();
        sf::VertexArray line(sf::PrimitiveType::LineStrip, steps_amount);

        for (int j = 0; j < steps_amount; j++)
        {
            line[j].color = color;
            sf::Vector2i screenPos = window.mapCoordsToPixel(trajectory_pos[i + j * planets.size()], renderer.getCamera());
            sf::Vector2f pos((float)screenPos.x, (float)screenPos.y);
            line[j].position = pos;
        }

        window.draw(line);
    }
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