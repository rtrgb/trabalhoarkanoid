#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <cstdlib>
#include "game.h"
#include <SFML/Graphics/Sprite.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <optional>
#include <algorithm>

using namespace std;

const float largurapddl = 80.0f;
const float alturapddl = 20.0f;
float rbola = 12.0f;
const float blocolarg = 95.0f;
const float blocoalt = 60.0f;
const float espacoHorizontal = 10.0f;
const float espacoVertical = 10.0f;
const int INITIAL_LIVES = 3;

bool acaboupt = false;
bool ganhoupt = false;
bool estadoSalvo = false;
bool autoPlay = false;

int faseAtual = 1;
const int totalFases = 3;
int score = 0;
bool proximaFase = false;
int scoreMultiplier = 1;

struct Item
{
    sf::RectangleShape shape;
    int itemType; // 0-extra lf, 1 - i paddle Size, 2- d paddle Size, 3- extra points, 4 -score multiplier
    float fallSpeed;
};

std::vector<Item> items;

struct Particle
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float lifetime;
};

sf::Texture blocoTexture;

sf::Texture extraLifeTexture;
sf::Texture increasePaddleTexture;
sf::Texture decreasePaddleTexture;
sf::Texture extraPointsTexture;
sf::Texture scoreMultiplierTexture;

std::vector<Particle> particles;

void carregarTextura(sf::Texture &textura, const std::string &caminho)
{
    if (!textura.loadFromFile(caminho))
        std::cerr << "Erro ao carregar a imagem: " << caminho << std::endl;
}

void criarItem(const sf::Vector2f &position)
{
    if (rand() % 100 < 30)
    {
        Item item;
        item.shape.setSize(sf::Vector2f(30, 30));
        item.shape.setPosition(position);
        item.fallSpeed = 150.0f;
        item.itemType = rand() % 5;

        switch (item.itemType)
        {
        case 0:
            item.shape.setTexture(&extraLifeTexture);
            break;
        case 1:
            item.shape.setTexture(&increasePaddleTexture);
            break;
        case 2:
            item.shape.setTexture(&decreasePaddleTexture);
            break;
        case 3:
            item.shape.setTexture(&extraPointsTexture);
            break;
        case 4:
            item.shape.setTexture(&scoreMultiplierTexture);
            break;
        }

        items.push_back(item);
    }
}

void atualizarItens(float deltaTime, sf::RectangleShape &pddl, int &lives)
{
    for (auto it = items.begin(); it != items.end();)
    {
        it->shape.move({0.f, it->fallSpeed * deltaTime});

        if (it->shape.getGlobalBounds().findIntersection(pddl.getGlobalBounds()))
        {

            switch (it->itemType)
            {
            case 0:
                lives++;
                break;
            case 1:
                pddl.setSize(sf::Vector2f(pddl.getSize().x + 20, alturapddl));
                break;
            case 2:
                pddl.setSize(sf::Vector2f(std::max(20.0f, pddl.getSize().x - 20), alturapddl));
                break;
            case 3:
                score += 200 * scoreMultiplier;
                break;
            case 4:
                scoreMultiplier++;
                break;
            }
            it = items.erase(it);
        }
        else if (it->shape.getPosition().y > pddl.getPosition().y + 50)
        {
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void drawItems(sf::RenderWindow &window)
{
    for (const auto &item : items)
    {
        window.draw(item.shape);
    }
}

void exibirProximaFase(sf::RenderWindow &window, int faseAtual)
{
    sf::Font font;
    if (!font.openFromFile(CAMINHO_FONTE))
    {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return;
    }

    sf::Text proximaFaseText(font);
    proximaFaseText.setCharacterSize(35);
    std::string mensagem = "Fase " + std::to_string(faseAtual) + " Completa!\nAperte Enter para a próxima fase.";
    proximaFaseText.setString(sf::String::fromUtf8(mensagem.begin(), mensagem.end()));
    proximaFaseText.setFillColor(sf::Color::White);
    proximaFaseText.setPosition({static_cast<float>(window.getSize().x / 2 - 300), static_cast<float>(window.getSize().y / 2 - 50)});

    window.clear();
    window.draw(proximaFaseText);
    window.display();

    while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                return;
            }
        }
    }
}

void criarParticulas(const sf::Vector2f &position, int quantidade)
{
    for (int i = 0; i < quantidade; ++i)
    {
        Particle particle;
        particle.shape.setRadius(5.0f);
        particle.shape.setFillColor(sf::Color::Yellow);
        particle.shape.setPosition(position);

        float angle = static_cast<float>((rand() % 360) * 3.14 / 180.0f);
        float speed = static_cast<float>((rand() % 100) + 50);
        particle.velocity = sf::Vector2f(speed * cos(angle), speed * sin(angle));
        particle.lifetime = 1.0f;

        particles.push_back(particle);
    }
}

void atualizarParticulas(float deltaTime)
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->lifetime -= deltaTime;
        if (it->lifetime <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            it->shape.move(it->velocity * deltaTime);
            sf::Color color = it->shape.getFillColor();
            color.a = static_cast<std::uint8_t>(255 * (it->lifetime / 1.0f));
            it->shape.setFillColor(color);
            ++it;
        }
    }
}

void drawParticles(sf::RenderWindow &window)
{
    for (const auto &particle : particles)
    {
        window.draw(particle.shape);
    }
}

void salvarEstadoDoJogo(const string &jogador, int score, const string &dificuldade, bool ganhou)
{
    string caminhoArquivo = CAMINHO_RANKING;
    ofstream arquivo(caminhoArquivo, ios::app);

    if (arquivo.is_open())
    {
        time_t now = time(nullptr);
        char *dataHora = ctime(&now);
        dataHora[strlen(dataHora) - 1] = '\0';

        arquivo << "Jogador: " << jogador << "\n";
        arquivo << "Pontuação: " << score << "\n";
        arquivo << "Dificuldade: " << dificuldade << "\n";
        arquivo << "Resultado: " << (ganhou ? "Won" : "Lost") << "\n";
        arquivo << "Data/Hora: " << dataHora << "\n";
        arquivo << "-----------------------------\n";

        arquivo.close();
    }
}

void resetarJogo()
{
    faseAtual = 1;
    score = 0;
    acaboupt = false;
    ganhoupt = false;
    estadoSalvo = false;
    proximaFase = false;
    scoreMultiplier = 1;

    items.clear();

    particles.clear();
}

void resetarbola(sf::CircleShape &bola, sf::RectangleShape &pddl, sf::Vector2f &bolaDir)
{
    bola.setPosition({static_cast<float>(pddl.getPosition().x + largurapddl / 2 - rbola), static_cast<float>(pddl.getPosition().y - 2 * rbola)});
    bolaDir = {(rand() % 2 == 0) ? -1.0f : 1.0f, -1.0f};
}

string getDifficultyAsString(Difficulty difficulty)
{
    switch (difficulty)
    {
    case EASY:
        return "Fácil";
    case MEDIUM:
        return "Médio";
    case HARD:
        return "Difícil";
    default:
        return "Desconhecida";
    }
}

int configurarFase(int fase, Bloco bricks[4][6], sf::RenderWindow &window)
{
    const int rows = 4;
    const int cols = 6;
    int totalBlocos = 0;
    srand(static_cast<unsigned int>(time(0)));

    float margemX = (window.getSize().x - (cols * (blocolarg + espacoHorizontal) - espacoHorizontal)) / 2.0f;
    float margemY = 100.0f;

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            bool shouldPlaceBlock = (rand() % 2 == 0);

            if (shouldPlaceBlock)
            {
                bricks[i][j].shape.setTexture(&blocoTexture);
                bricks[i][j].shape.setSize(sf::Vector2f(blocolarg, blocoalt));
                bricks[i][j].durabilidade = rand() % 3 + 1;

                if (bricks[i][j].durabilidade == 3)
                {
                    bricks[i][j].shape.setFillColor(sf::Color::Red);
                }
                else if (bricks[i][j].durabilidade == 2)
                {
                    bricks[i][j].shape.setFillColor(sf::Color::Yellow);
                }
                else if (bricks[i][j].durabilidade == 1)
                {
                    bricks[i][j].shape.setFillColor(sf::Color::Green);
                }

                float posX = margemX + j * (blocolarg + espacoHorizontal);
                float posY = margemY + i * (blocoalt + espacoVertical);

                bricks[i][j].shape.setPosition({posX, posY});
                bricks[i][j].quebrado = false;
                bricks[i][j].tempoQuebra = 0.0f;

                totalBlocos++;
            }
            else
            {
                bricks[i][j].durabilidade = 0;
                bricks[i][j].shape.setSize(sf::Vector2f(0, 0));
            }
        }
    }
    return totalBlocos;
}

void ajustarVelocidadeBola(sf::Vector2f &bolaDir)
{
    float magnitude = sqrt(bolaDir.x * bolaDir.x + bolaDir.y * bolaDir.y);
    bolaDir /= magnitude;
}

void playGame(sf::RenderWindow &window, Difficulty gameDifficulty, const string &nomeJogador, bool autoPlay)
{
    sf::Texture galaxytexture;
    carregarTextura(galaxytexture, "images/cascais.jpg");

    sf::Sprite galaxy(galaxytexture);
    galaxy.setScale({static_cast<float>(window.getSize().x) / galaxytexture.getSize().x,
                     static_cast<float>(window.getSize().y) / galaxytexture.getSize().y});

    carregarTextura(blocoTexture, "images/hmbg.png");

    carregarTextura(extraLifeTexture, "images/heart.png");

    carregarTextura(increasePaddleTexture, "images/plus.png");

    carregarTextura(decreasePaddleTexture, "images/x.png");

    carregarTextura(extraPointsTexture, "images/coin.png");

    carregarTextura(scoreMultiplierTexture, "images/star.png");

    const float MIN_VELOCITY_X = 200.0f;
    const float MIN_VELOCITY_Y = 200.0f;
    const float MAX_VELOCITY_X = 350.0f;
    const float MAX_VELOCITY_Y = 350.0f;
    float bolaSpeed = 350.0f;
    float pddlSpeed = 350.0f;

    switch (gameDifficulty)
    {
    case EASY:
        bolaSpeed *= 0.90f;
        pddlSpeed *= 0.90f;
        rbola = 15.0f;
        break;
    case MEDIUM:
        break;
    case HARD:
        bolaSpeed *= 1.50f;
        pddlSpeed *= 1.13f;
        rbola = 9.5f;
        break;
    }

    if (autoPlay)
    {
        pddlSpeed *= 2.0f;
    }

    sf::Texture paddltx;
    if (!paddltx.loadFromFile("images/trampo.png"))
    {
        return;
    }

    paddltx.setSmooth(true);
    sf::RectangleShape pddl(sf::Vector2f(largurapddl, alturapddl));
    pddl.setPosition({static_cast<float>(window.getSize().x / 2 - largurapddl / 2), static_cast<float>(window.getSize().y - alturapddl - 10)});
    pddl.setTexture(&paddltx);

    sf::Texture texture;
    if (!texture.loadFromFile("images/bola8.png"))
    {
        return;
    }

    sf::CircleShape bola(rbola);
    texture.setSmooth(true);
    bola.setTexture(&texture);
    bola.setPointCount(100);
    sf::Vector2f bolaDir(0.0f, -1.0f);
    resetarbola(bola, pddl, bolaDir);

    const int rows = 4;
    const int cols = 6;

    Bloco bricks[rows][cols];
    int blokkino = configurarFase(faseAtual, bricks, window);

    int lives = INITIAL_LIVES;
    sf::Font font;
    if (!font.openFromFile(CAMINHO_FONTE))
    {
        std::cerr << "Erro ao carregar a fonte!" << std::endl;
        return;
    }

    sf::Text livesText(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition({10.f, 10.f});

    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({650.f, 10.f});

    sf::Text timerText(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition({350.f, 10.f});

    sf::Clock gameClock;

    sf::Clock clock;

    while (window.isOpen())
    {
        if (proximaFase)
        {
            exibirProximaFase(window, faseAtual);
            proximaFase = false;
        }

        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float deltaTime = clock.restart().asSeconds();

        if (!acaboupt)
        {
            atualizarParticulas(deltaTime);

            if (autoPlay)
            {
                float paddleCenter = pddl.getPosition().x + largurapddl / 2;
                float ballCenter = bola.getPosition().x + rbola;

                float difference = ballCenter - paddleCenter;
                float moveSpeed = std::min(fabs(difference), pddlSpeed * deltaTime);

                if (difference > 15.0f)
                {
                    pddl.move({moveSpeed, 0.f});
                }
                else if (difference < -15.0f)
                {
                    pddl.move({-moveSpeed, 0.f});
                }

                if (pddl.getPosition().x < 0)
                {
                    pddl.setPosition({0.f, pddl.getPosition().y});
                }
                else if (pddl.getPosition().x + largurapddl > window.getSize().x)
                {
                    pddl.setPosition({static_cast<float>(window.getSize().x - largurapddl), pddl.getPosition().y});
                }
            }

            else
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) && pddl.getPosition().x > 0)
                {
                    pddl.move({static_cast<float>(-pddlSpeed * deltaTime), 0.f});
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) && pddl.getPosition().x + largurapddl < window.getSize().x)
                {
                    pddl.move({static_cast<float>(pddlSpeed * deltaTime), 0.f});
                }
            }

            const float MIN_BALL_SPEED = 0.8f;
            float currentBallSpeed = sqrt(bolaDir.x * bolaDir.x + bolaDir.y * bolaDir.y);
            if (currentBallSpeed < MIN_BALL_SPEED)
            {
                bolaDir *= (MIN_BALL_SPEED / currentBallSpeed);
            }

            bola.move(bolaDir * bolaSpeed * deltaTime);

            atualizarItens(deltaTime, pddl, lives);

            if (bola.getPosition().x < 0)
            {
                bola.setPosition({0.f, bola.getPosition().y});
                bolaDir.x = -bolaDir.x;
            }
            else if (bola.getPosition().x + 2 * rbola > window.getSize().x)
            {
                bola.setPosition({static_cast<float>(window.getSize().x - 2 * rbola), bola.getPosition().y});
                bolaDir.x = -bolaDir.x;
            }

            if (bola.getPosition().y < 0)
            {
                bola.setPosition({bola.getPosition().x, 0.f});
                bolaDir.y = -bolaDir.y;
            }

            if (bola.getGlobalBounds().findIntersection(pddl.getGlobalBounds()))
            {
                bola.setPosition({bola.getPosition().x, static_cast<float>(pddl.getPosition().y - 2 * rbola)});
                bolaDir.y = -fabs(bolaDir.y);
                bolaDir.x = (rand() % 2 == 0) ? -1.0f : 1.0f;
                ajustarVelocidadeBola(bolaDir);
            }

            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    if (bricks[i][j].durabilidade > 0 && bola.getGlobalBounds().findIntersection(bricks[i][j].shape.getGlobalBounds()))
                    {
                        bricks[i][j].durabilidade--;

                        if (bricks[i][j].durabilidade == 0)
                        {
                            bricks[i][j].quebrado = true;
                            bricks[i][j].tempoQuebra = 0.5f;
                            criarParticulas(bricks[i][j].shape.getPosition(), 20);
                            criarItem(bricks[i][j].shape.getPosition());
                            blokkino--;

                            if (faseAtual == 1)
                                score += 100;
                            else if (faseAtual == 2)
                                score += 125;
                            else if (faseAtual == 3)
                                score += 150;
                        }
                        else if (bricks[i][j].durabilidade == 2)
                        {
                            bricks[i][j].shape.setFillColor(sf::Color::Yellow);
                        }
                        else if (bricks[i][j].durabilidade == 1)
                        {
                            bricks[i][j].shape.setFillColor(sf::Color::Green);
                        }

                        sf::FloatRect bolaBounds = bola.getGlobalBounds();
                        sf::FloatRect brickBounds = bricks[i][j].shape.getGlobalBounds();

                        float overTop = bolaBounds.position.y + bolaBounds.size.y - brickBounds.position.y;
                        float overBottom = brickBounds.position.y + brickBounds.size.y - bolaBounds.position.y;
                        float overLeft = bolaBounds.position.x + bolaBounds.size.x - brickBounds.position.x;
                        float overRight = brickBounds.position.x + brickBounds.size.x - bolaBounds.position.x;

                        float menorSobreposicao = std::min({overTop, overBottom, overLeft, overRight});

                        if (menorSobreposicao == overTop)
                        {
                            bolaDir.y = -bolaDir.y;
                            bola.setPosition({bola.getPosition().x, static_cast<float>(brickBounds.position.y - bolaBounds.size.y)});
                        }
                        else if (menorSobreposicao == overBottom)
                        {
                            bolaDir.y = -bolaDir.y;
                            bola.setPosition({bola.getPosition().x, static_cast<float>(brickBounds.position.y + brickBounds.size.y)});
                        }
                        else if (menorSobreposicao == overLeft)
                        {
                            bolaDir.x = -bolaDir.x;
                            bola.setPosition({static_cast<float>(brickBounds.position.x - bolaBounds.size.x), bola.getPosition().y});
                        }
                        else if (menorSobreposicao == overRight)
                        {
                            bolaDir.x = -bolaDir.x;
                            bola.setPosition({static_cast<float>(brickBounds.position.x + brickBounds.size.x), bola.getPosition().y});
                        }
                        ajustarVelocidadeBola(bolaDir);
                    }

                    if (bricks[i][j].quebrado)
                    {
                        bricks[i][j].tempoQuebra -= deltaTime;
                        if (bricks[i][j].tempoQuebra > 0)
                        {
                            sf::Color corAtual = bricks[i][j].shape.getFillColor();
                            corAtual.a = static_cast<std::uint8_t>(255 * (bricks[i][j].tempoQuebra / 0.5f));
                            bricks[i][j].shape.setFillColor(corAtual);
                        }
                        else
                        {
                            bricks[i][j].shape.setSize(sf::Vector2f(0, 0));
                        }
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
                    acaboupt = true;
                    ganhoupt = false;
                }
            }

            if (blokkino == 0)
            {
                if (faseAtual < totalFases)
                {
                    faseAtual++;
                    blokkino = configurarFase(faseAtual, bricks, window);
                    resetarbola(bola, pddl, bolaDir);
                    proximaFase = true;
                }
                else
                {
                    acaboupt = true;
                    ganhoupt = true;
                }
            }

            livesText.setString("Lives: " + std::to_string(lives));
            scoreText.setString("Score: " + std::to_string(score));

            sf::Time elapsed = gameClock.getElapsedTime();
            timerText.setString("Time: " + std::to_string(static_cast<int>(elapsed.asSeconds())) + "s");

            window.clear();
            window.draw(galaxy);
            window.draw(pddl);
            window.draw(bola);
            for (int i = 0; i < rows; ++i)
            {
                for (int j = 0; j < cols; ++j)
                {
                    window.draw(bricks[i][j].shape);
                }
            }

            drawParticles(window);
            drawItems(window);

            window.draw(livesText);
            window.draw(scoreText);
            window.draw(timerText);
            window.display();
        }
        else
        {
            window.clear();
            sf::Text endText(font);
            endText.setCharacterSize(35);

            if (acaboupt && !estadoSalvo)
            {
                string gameDifficultyAsString = getDifficultyAsString(gameDifficulty);
                salvarEstadoDoJogo(nomeJogador, score, gameDifficultyAsString, ganhoupt);
                estadoSalvo = true;
            }

            if (ganhoupt)
            {
                window.draw(galaxy);
                endText.setString("Bela partida!\nPressione M para voltar ao Menu ou Q para sair.");
                endText.setFillColor(sf::Color::Green);
            }
            else
            {
                endText.setString("Perdeu como sempre!\nPressione M para voltar ao Menu ou Q para sair.");
                endText.setFillColor(sf::Color::Red);
            }

            endText.setPosition({200.f, 200.f});
            window.draw(endText);
            window.display();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M))
            {
                resetarJogo();
                return;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            {
                window.close();
            }
        }
    }
}
