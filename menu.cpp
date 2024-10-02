#include <SFML/Graphics.hpp>
#include <iostream>
#include "game.h"
#include <locale.h>


enum GameState
{
    MENU,
    PLAYING,
    INSTRUCTIONS,
    ABOUT,
    QUIT
};

GameState gameState = MENU;

int main()
{
    setlocale(LC_ALL,"pt_BR");

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Arca Noeoid");

    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf"))
    {
        std::cout << "Error loading font\n";
        return -1;
    }

    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(45);
    menuText.setFillColor(sf::Color::White);
    menuText.setPosition(100, 100);
    menuText.setString(
        "==========================\n"
        "  BEM-VINDO AO ARCA NOEOID\n"
        "==========================\n"
        "1. Jogar\n"
        "2. Instrucoes\n"
        "3. Sobre\n"
        "4. Sair\n"
        "Selecione uma opcao: ");

    sf::Text instructionsText;
    instructionsText.setFont(font);
    instructionsText.setCharacterSize(24);
    instructionsText.setFillColor(sf::Color::White);
    instructionsText.setPosition(50, 50);
    instructionsText.setString(
        "Detalhamento do Jogo:\n\n"
        "\n- O objetivo do jogo seria destruir todos os blocos sem deixar a bola cair!\n");

    sf::Text instructionsDetails;
    instructionsDetails.setFont(font);
    instructionsDetails.setCharacterSize(18);
    instructionsDetails.setFillColor(sf::Color::Yellow);
    instructionsDetails.setPosition(50, 230);
    instructionsDetails.setString(
        "- Use 'A' para mover o paddle para a esquerda.\n"
        "\n- Use 'D' para mover o paddle para a direita.\n"
        "\n- Evite que a bola caia no fundo da tela.\n"
        "\n- O jogador tem um numero limitado de vidas.\n\n"
        "\nPressione Enter para voltar ao menu...");

    sf::Text aboutText;
    aboutText.setFont(font);
    aboutText.setCharacterSize(24);
    aboutText.setFillColor(sf::Color::Blue);
    aboutText.setPosition(50, 50);
    aboutText.setString(
        "Equipe de Desenvolvimento:\n\n"
        "Gabriel: Desenvolvimento do codigo e pesquisas.\n"
        "ChatGPT: Ajuda na procura das bibliotecas e formatação.\n"
        "\nPressione Enter para voltar ao menu...");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (gameState == MENU)
            {
                if (event.type == sf::Event::TextEntered)
                {
                    if (event.text.unicode == '1')
                    {
                        gameState = PLAYING;
                    }
                    else if (event.text.unicode == '2')
                    {
                        gameState = INSTRUCTIONS;
                    }
                    else if (event.text.unicode == '3')
                    {
                        gameState = ABOUT;
                    }
                    else if (event.text.unicode == '4')
                    {
                        gameState = QUIT;
                        window.close();
                    }
                }
            }
            else if (gameState == ABOUT || gameState == INSTRUCTIONS)
            {

                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                {
                    gameState = MENU;
                }
            }
        }

        window.clear();

        if (gameState == MENU)
        {
            window.draw(menuText);
        }
        else if (gameState == PLAYING)
        {
            playGame(window);
            gameState = MENU;
        }
        else if (gameState == INSTRUCTIONS)
        {
            window.draw(instructionsText);
            window.draw(instructionsDetails);
        }
        else if (gameState == ABOUT)
        {
            window.draw(aboutText);
        }

        window.display();
    }

    return 0;
}
