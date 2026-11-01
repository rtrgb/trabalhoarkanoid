#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

enum Difficulty
{
    EASY,
    MEDIUM,
    HARD
};

struct Bloco
{
    sf::RectangleShape shape;
    int durabilidade = 0;
    bool quebrado = false;
    float tempoQuebra = 0.0f;
};

extern bool autoPlay;

// Caminhos relativos à pasta do projeto (o main muda o diretório de trabalho para lá)
const string CAMINHO_FONTE = "fonts/arial.ttf";
const string CAMINHO_RANKING = "ranking.txt";

void playGame(sf::RenderWindow &window, Difficulty gameDifficulty, const std::string &nomeJogador, bool autoPlay);

#endif
