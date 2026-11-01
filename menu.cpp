#include <SFML/Graphics.hpp>
#include <iostream>
#include "game.h"
#include <locale.h>
#include <SFML/Graphics/Sprite.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>

using namespace std;

string trim(const string &str)
{
    size_t first = str.find_first_not_of(" \t\r\n\v\f");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n\v\f");
    return str.substr(first, (last - first + 1));
}

enum GameState
{
    MENU,
    PLAYING,
    ENTER_NAME,
    INSTRUCTIONS,
    RANKING,
    ABOUT,
    PONTINFO,
    QUIT
};

struct PlayerScore
{
    string jogador;
    int pontuacao;
    string resultado;
    string dataHora;
};

bool compareScores(const PlayerScore &a, const PlayerScore &b)
{
    return a.pontuacao > b.pontuacao;
}

vector<PlayerScore> lerRanking()
{
    vector<PlayerScore> ranking;
    ifstream arquivo(CAMINHO_RANKING);

    if (arquivo.is_open())
    {
        string linha;
        PlayerScore scoreAtual;

        while (getline(arquivo, linha))
        {
            if (linha.find("Jogador: ") != string::npos)
            {
                scoreAtual.jogador = linha.substr(9);
            }
            else if (linha.find("Pontuação: ") != string::npos)
            {
                size_t pos = linha.find(":");
                if (pos != string::npos)
                {
                    string pontuacaoStr = linha.substr(pos + 1);
                    pontuacaoStr = trim(pontuacaoStr);
                    bool isValidNumber = !pontuacaoStr.empty() && all_of(pontuacaoStr.begin(), pontuacaoStr.end(), ::isdigit);
                    if (isValidNumber)
                    {
                        try
                        {
                            scoreAtual.pontuacao = stoi(pontuacaoStr);
                        }
                        catch (const invalid_argument &)
                        {
                            scoreAtual.pontuacao = 0;
                        }
                        catch (const out_of_range &)
                        {
                            scoreAtual.pontuacao = 0;
                        }
                    }
                    else
                    {
                        scoreAtual.pontuacao = 0;
                    }
                }
            }
            else if (linha.find("Resultado: ") != string::npos)
            {
                scoreAtual.resultado = linha.substr(10);
            }
            else if (linha.find("Data/Hora: ") != string::npos)
            {
                scoreAtual.dataHora = linha.substr(11);
            }
            else if (linha.find("-----------------------------") != string::npos)
            {
                ranking.push_back(scoreAtual);
                scoreAtual = PlayerScore();
            }
        }

        if (!scoreAtual.jogador.empty())
        {
            ranking.push_back(scoreAtual);
        }

        arquivo.close();
    }

    sort(ranking.begin(), ranking.end(), compareScores);

    if (ranking.size() > 5)
    {
        ranking.resize(5);
    }

    return ranking;
}

string nomeJogador;
GameState gameState = MENU;
Difficulty gameDifficulty = MEDIUM;

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "pt_BR.UTF-8");

    // Garante que imagens, fonte e ranking sejam encontrados mesmo rodando de outra pasta
    if (argc > 0)
    {
        std::error_code ec;
        filesystem::path pastaExe = filesystem::absolute(argv[0], ec).parent_path();
        if (!ec && !pastaExe.empty())
            filesystem::current_path(pastaExe, ec);
    }

    sf::RenderWindow window(sf::VideoMode({1920u, 1080u}), "Arca Noeoid");

    sf::Font font;
    if (!font.openFromFile(CAMINHO_FONTE))
    {
        return -1;
    }

    sf::Texture wallpaperTexture;
    if (!wallpaperTexture.loadFromFile("images/cascais.jpg"))
    {
        return -1;
    }

    sf::Sprite wallpaper(wallpaperTexture);
    wallpaper.setScale({static_cast<float>(window.getSize().x) / wallpaperTexture.getSize().x,
                        static_cast<float>(window.getSize().y) / wallpaperTexture.getSize().y});

    sf::Texture welcomeTexture;
    if (!welcomeTexture.loadFromFile("images/Welcome.png"))
    {
        return -1;
    }

    sf::Sprite welcomeSprite(welcomeTexture);
    welcomeSprite.setPosition({50.f, -100.f});

    sf::Texture mmTexture;
    if (!mmTexture.loadFromFile("images/mm.png"))
    {
        return -1;
    }

    sf::Sprite mmSprite(mmTexture);
    mmSprite.setPosition({50.f, 250.f});

    vector<PlayerScore> ranking;
    bool selectingDifficulty = false;

    while (window.isOpen())
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            const auto *textEvent = event->getIf<sf::Event::TextEntered>();
            const auto *keyEvent = event->getIf<sf::Event::KeyPressed>();
            const bool enterPressionado = keyEvent && keyEvent->code == sf::Keyboard::Key::Enter;

            if (gameState == MENU)
            {
                if (selectingDifficulty)
                {
                    if (textEvent)
                    {
                        if (textEvent->unicode == '1')
                        {
                            gameDifficulty = EASY;
                            selectingDifficulty = false;
                            gameState = ENTER_NAME;
                        }
                        else if (textEvent->unicode == '2')
                        {
                            gameDifficulty = MEDIUM;
                            selectingDifficulty = false;
                            gameState = ENTER_NAME;
                        }
                        else if (textEvent->unicode == '3')
                        {
                            gameDifficulty = HARD;
                            selectingDifficulty = false;
                            gameState = ENTER_NAME;
                        }
                    }
                }
                else
                {
                    if (textEvent)
                    {
                        if (textEvent->unicode == '1')
                        {
                            selectingDifficulty = true;
                        }
                        else if (textEvent->unicode == '2')
                        {
                            gameState = INSTRUCTIONS;
                        }
                        else if (textEvent->unicode == '3')
                        {
                            gameState = ABOUT;
                        }
                        else if (textEvent->unicode == '4')
                        {
                            ranking = lerRanking();
                            gameState = RANKING;
                        }
                        else if (textEvent->unicode == '5')
                        {
                            gameState = QUIT;
                            window.close();
                        }
                        else if (textEvent->unicode == '6')
                        {
                            gameState = PONTINFO;
                        }
                        else if (textEvent->unicode == '7')
                        {
                            autoPlay = true;
                            selectingDifficulty = true;
                        }
                    }
                }
            }
            else if (gameState == ENTER_NAME)
            {
                if (textEvent)
                {
                    if (textEvent->unicode == 8)
                    {
                        if (!nomeJogador.empty())
                            nomeJogador.pop_back();
                    }
                    else if (textEvent->unicode == '\n' || textEvent->unicode == 13)
                    {
                        if (!nomeJogador.empty())
                        {
                            gameState = PLAYING;
                            playGame(window, gameDifficulty, nomeJogador, autoPlay);
                            gameState = MENU;
                        }
                    }
                    else if (textEvent->unicode < 128)
                    {
                        nomeJogador += static_cast<char>(textEvent->unicode);
                    }
                }
            }
            else if (gameState == ABOUT || gameState == INSTRUCTIONS || gameState == PONTINFO)
            {
                if (enterPressionado)
                {
                    gameState = MENU;
                }
            }
            else if (gameState == RANKING)
            {
                if (enterPressionado)
                {
                    gameState = MENU;
                }
            }
        }

        window.clear();
        window.draw(wallpaper);

        if (gameState == MENU)
        {
            if (selectingDifficulty)
            {
                sf::Text difficultyText(font);
                difficultyText.setCharacterSize(50);
                difficultyText.setFillColor(sf::Color::Yellow);
                difficultyText.setPosition({100.f, 300.f});
                difficultyText.setString(
                    "Selecione a dificuldade:\n"
                    "1. Facil\n"
                    "2. Medio\n"
                    "3. Dificil\n");
                window.draw(difficultyText);
            }
            else
            {
                window.draw(welcomeSprite);
                window.draw(mmSprite);
            }
        }
        else if (gameState == ENTER_NAME)
        {
            sf::Text enterNameText(font);
            enterNameText.setCharacterSize(50);
            enterNameText.setFillColor(sf::Color::White);
            enterNameText.setPosition({100.f, 100.f});
            enterNameText.setString("Digite seu nome:");
            window.draw(enterNameText);

            sf::Text playerNameDisplay(font);
            playerNameDisplay.setCharacterSize(42);
            playerNameDisplay.setFillColor(sf::Color::Green);
            playerNameDisplay.setPosition({100.f, 150.f});
            playerNameDisplay.setString(nomeJogador);
            window.draw(playerNameDisplay);
        }
        else if (gameState == INSTRUCTIONS)
        {
            sf::Text instructionsText(font);
            instructionsText.setCharacterSize(32);
            instructionsText.setFillColor(sf::Color::White);
            instructionsText.setPosition({50.f, 50.f});
            instructionsText.setString(
                "Detalhamento do Jogo:\n\n"
                "- O objetivo do jogo se resume a destruir todos os blocos sem deixar a bola cair!\n");
            window.draw(instructionsText);
        }
        else if (gameState == ABOUT)
        {
            sf::Text aboutText(font);
            aboutText.setCharacterSize(50);
            aboutText.setFillColor(sf::Color::White);
            aboutText.setPosition({50.f, 50.f});
            aboutText.setString(
                "Equipe de Desenvolvimento:\n\n"
                "Gabriel: Desenvolvimento do code e pesquisas.\n"
                "ChatGPT: Ajuda no nos artigos e formatting.\n\n"
                "Pressione Enter para voltar ao menu...");
            window.draw(aboutText);
        }
        else if (gameState == PONTINFO)
        {
            sf::Text scoreExplanationText(font);
            scoreExplanationText.setCharacterSize(32);
            scoreExplanationText.setFillColor(sf::Color::Yellow);
            scoreExplanationText.setPosition({50.f, 50.f});
            scoreExplanationText.setString(
                "Sistema de Pontos:\n\n"
                "Blocos Quebrados: Ganhe 100 pontos por bloco que se destrua.\n\n"
                "Pressione Enter para voltar ao menu...");
            window.draw(scoreExplanationText);
        }
        else if (gameState == RANKING)
        {
            sf::Text rankingTitle(font);
            rankingTitle.setCharacterSize(30);
            rankingTitle.setFillColor(sf::Color::Yellow);
            rankingTitle.setPosition({100.f, 50.f});
            rankingTitle.setString("Ranking dos Jogadores");
            window.draw(rankingTitle);

            float yOffset = 100;
            int numScoresToShow = min(static_cast<int>(ranking.size()), 5);
            for (int i = 0; i < numScoresToShow; ++i)
            {
                const auto &player = ranking[i];
                sf::Text playerText(font);
                playerText.setCharacterSize(20);
                playerText.setFillColor(sf::Color::White);
                playerText.setPosition({100.f, static_cast<float>(yOffset)});

                string linhaRanking = "Jogador: " + player.jogador + " | Pontos: " + to_string(player.pontuacao) +
                                      " | Resultado: " + player.resultado + " | Data/Hora: " + player.dataHora;
                playerText.setString(sf::String::fromUtf8(linhaRanking.begin(), linhaRanking.end()));
                window.draw(playerText);
                yOffset += 30;
            }

            sf::Text backToMenu(font);
            backToMenu.setCharacterSize(20);
            backToMenu.setFillColor(sf::Color::Green);
            backToMenu.setPosition({100.f, static_cast<float>(yOffset + 50)});
            backToMenu.setString("Pressione Enter para voltar ao menu principal");
            window.draw(backToMenu);
        }

        window.display();
    }

    return 0;
}