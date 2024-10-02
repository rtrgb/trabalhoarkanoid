#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include "game.h"

const float largurapddl = 100.0f;
const float alturapddl = 20.0f;
const float rbola = 10.0f;
const float blocolarg = 75.0f;
const float blocoalt = 30.0f;
const float espacamento = 50.0f;
const int INITIAL_LIVES = 3;

void resetarbola(sf::CircleShape &bola, sf::RectangleShape &pddl, sf::Vector2f &bolaDir)
{
    bola.setPosition(pddl.getPosition().x + largurapddl / 2 - rbola, pddl.getPosition().y - 2 * rbola);
    bolaDir = {(rand() % 2 == 0) ? -1.0f : 1.0f, -1.0f};
}

void playGame(sf::RenderWindow &window)
{
    sf::RectangleShape pddl(sf::Vector2f(largurapddl, alturapddl));
    pddl.setFillColor(sf::Color::Green);
    pddl.setPosition(window.getSize().x / 2 - largurapddl / 2, window.getSize().y - alturapddl - 10);

    sf::CircleShape bola(rbola);
    bola.setFillColor(sf::Color::Red);
    sf::Vector2f bolaDir(0.0f, -1.0f);
    resetarbola(bola, pddl, bolaDir);

    const int rows = 4;
    const int cols = 6;
    sf::RectangleShape bricks[rows][cols];
    
    float totalGridWidth = (cols * blocolarg) + (cols - 1) * espacamento;
    float startX = (window.getSize().x - totalGridWidth) / 2;

    
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            bricks[i][j].setSize(sf::Vector2f(blocolarg, blocoalt));
            bricks[i][j].setFillColor(sf::Color::Blue);
            bricks[i][j].setPosition(startX + j * (blocolarg + espacamento), i * (blocoalt + espacamento) + 30);
        }
    }
    int lives = INITIAL_LIVES;
    int score = 0;
    int blokkino = rows * cols;
    float bolaSpeed = 300.0f;
    float pddlSpeed = 400.0f;

    sf::Font font;
    font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(10, 10);

    livesText.setString("Lives: " + std::to_string(lives));
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(650, 10);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        
        float deltaTime = clock.restart().asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && pddl.getPosition().x > 0)
        {
            pddl.move(-pddlSpeed * deltaTime, 0); 
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && pddl.getPosition().x + largurapddl < window.getSize().x)
        {
            pddl.move(pddlSpeed * deltaTime, 0);
        }

        bola.move(bolaDir * bolaSpeed * deltaTime);

        if (bola.getPosition().x < 0 || bola.getPosition().x + 2 * rbola > window.getSize().x)
        {
            bolaDir.x = -bolaDir.x;
        }
        if (bola.getPosition().y < 0)
        {
            bolaDir.y = -bolaDir.y;
        }

        if (bola.getGlobalBounds().intersects(pddl.getGlobalBounds()))
        {
            bolaDir.y = -bolaDir.y;
        }

        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                if (bricks[i][j].getSize().x > 0 && bola.getGlobalBounds().intersects(bricks[i][j].getGlobalBounds()))
                {
                    bolaDir.y = -bolaDir.y;
                    bricks[i][j].setSize(sf::Vector2f(0, 0)); 
                    score += 100;
                    blokkino--;
                }
            }
        }

        if (bola.getPosition().y > window.getSize().y)
        {
            lives--;
            if (lives > 0)
            {
                resetarbola(bola, pddl, bolaDir);
            }
            else 
            {
                window.close();
            }
        }

        if (blokkino == 0)
        {
            window.close();
        }

        livesText.setString("Lives: " + std::to_string(lives));
        scoreText.setString("Score: " + std::to_string(score));

        window.clear();
        window.draw(pddl);
        window.draw(bola);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                window.draw(bricks[i][j]);
            }
        }
        window.draw(livesText);
        window.draw(scoreText);

        window.display();
    }
}
