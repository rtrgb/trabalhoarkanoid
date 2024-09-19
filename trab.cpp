#include <iostream>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
#endif

using namespace std;

const int largura = 50;
const int altura = 25;
const int larguraBarra = 10;
int vidas = 3;
int pontuacao = 0;
bool jogoAtivo = true;

int xBola, yBola, dxBola, dyBola;
int xBarra;
int blocos[altura][largura]; 

void setColor(int color) {
    cout << "\033[" << color << "m"; 
}

#ifndef _WIN32 //chatgpt
void configurarTermios(bool ativar) {
    static struct termios oldt, newt;
    if (ativar) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
       
        fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
    }
}

char getch() { //chatgpt
    char buf = 0;
    ssize_t bytesRead = read(STDIN_FILENO, &buf, 1);
    if (bytesRead < 0) {
        perror("read()");
        return 0;
    }
    return buf;
}
#endif

void inicializar() {
    srand(time(0));
    xBola = largura / 2;
    yBola = altura - 3; 
    dxBola = (rand() % 2 == 0) ? 1 : -1;
    dyBola = -1;
    xBarra = largura / 2 - larguraBarra / 2;
    vidas = 3;
    pontuacao = 0;
    jogoAtivo = true;

    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            blocos[i][j] = 0;
        }
    }

    int margemLateral = 1;  
    int margemSuperior = 2;  
    int numLinhasBlocos = 6; 

    for (int i = 0; i < numLinhasBlocos; i++) {
        for (int j = margemLateral + 1; j < largura - margemLateral - 1; j += 4) {
            blocos[i + margemSuperior][j] = 1;
            blocos[i + margemSuperior][j + 1] = 1;
        }
    }
}


void desenhar() {
    
    cout << "\033[H\033[J";

    
    for (int i = 0; i < largura + 2; i++) {
        cout << "#";
    }
    cout << endl;

    
    for (int i = 0; i < altura; i++) {
        cout << "#"; 
        for (int j = 0; j < largura; j++) {
            if (i == yBola && j == xBola) {
                cout << "O"; 
            } else if (i == altura - 1 && j >= xBarra && j < xBarra + larguraBarra) {
                cout << "="; 
            } else if (blocos[i][j] == 1) {
                setColor(31 + (i % 6)); 
                cout << "X"; 
                setColor(0);
            } else {
                cout << " ";
            }
        }
        cout << "#"; 
        cout << endl;
    }

    for (int i = 0; i < largura + 2; i++) {
        cout << "#";
    }
    cout << endl;

    cout << "Pontuação: " << pontuacao << " | Vidas: " << vidas << endl;
}

void moverBarra() { //chatgpt
#ifdef _WIN32

    bool leftKey = (GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000);
    bool rightKey = (GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000);

    
    if (leftKey) {
        if (xBarra > 1) { 
            xBarra--;
        }
    }
    if (rightKey) {
        if (xBarra + larguraBarra < largura - 1) { 
            xBarra++;
        }
    }
#else
    char tecla;
    
    while (read(STDIN_FILENO, &tecla, 1) > 0) {
        if ((tecla == 'a' || tecla == 'A') && xBarra > 1) {
            xBarra--;
        } else if ((tecla == 'd' || tecla == 'D') && xBarra + larguraBarra < largura - 1) {
            xBarra++;
        } else if (tecla == 27) {
            jogoAtivo = false;
        }
    }
#endif
}

void verificarColisaoBarra() {
    if (yBola == altura - 2 && xBola >= xBarra && xBola < xBarra + larguraBarra) {
        dyBola = -dyBola;
    }
}

void verificarColisaoBlocos() {
    if (yBola >= 0 && yBola < altura && xBola >= 0 && xBola < largura) {
        if (blocos[yBola][xBola] == 1) {
            blocos[yBola][xBola] = 0;
            dyBola = -dyBola;
            pontuacao += 10;
        }
    }
}

void moverBola() {
    xBola += dxBola;
    yBola += dyBola;

    
    if (xBola <= 1 || xBola >= largura - 2) {
        dxBola = -dxBola;
    }

    
    if (yBola <= 1) {
        dyBola = -dyBola;
    }

    
    if (yBola >= altura - 1) {
        vidas--;
        if (vidas <= 0) {
            jogoAtivo = false;
        } else {
            xBola = largura / 2;
            yBola = altura - 3;
            dxBola = (rand() % 2 == 0) ? 1 : -1;
            dyBola = -1;
        }
    }
}

bool verificarVitoria() {
    for (int i = 0; i < altura; i++) {
        for (int j = 0; j < largura; j++) {
            if (blocos[i][j] == 1) {
                return false;
            }
        }
    }
    return true;
}

void jogo() {
    inicializar();
#ifndef _WIN32
    configurarTermios(true); 
#endif

    
    cout << "\033[?25l";

    int contadorBola = 0;
    int velocidadeBola = 2;

    while (jogoAtivo) {
        desenhar();
        moverBarra();

        if (contadorBola >= velocidadeBola) {
            moverBola();
            verificarColisaoBarra();
            verificarColisaoBlocos();
            contadorBola = 0;
        } else {
            contadorBola++;
        }

        if (verificarVitoria()) {
            desenhar();
            cout << "Parabéns! Você venceu!" << endl;
            break;
        }

#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000); 
#endif
    }

    if (vidas <= 0) {
        desenhar();
        cout << "Game Over! Você perdeu!" << endl;
    }

    // Restaura o cursor
    cout << "\033[?25h";

#ifndef _WIN32
    configurarTermios(false); 
#endif

    cout << "Pressione Enter para voltar ao menu..." << endl;
    cin.ignore();
    cin.get();
}


void menu() {
    int opcao;
    do {
#ifdef _WIN32
        system("cls");
#else
        cout << "\033[H\033[J"; //chatgpt
#endif
        cout << "======================" << endl;
        cout << "     BEM-VINDO AO     " << endl;
        cout << "        JOGO          " << endl;
        cout << "======================" << endl;
        cout << "1. Jogar" << endl;
        cout << "2. Sobre" << endl;
        cout << "3. Sair" << endl;
        cout << "Selecione uma opção: ";
        cin >> opcao;

        switch(opcao) {
            case 1:
                jogo();
                break;
            case 2:
                cout << "\nDesenvolvido por Gabriel.\n" << endl;
                cout << "Usado CHATGPT para auxiliar em bibliotecas," << endl;
                cout << "terminal, cor dos blocos, sleep," << endl;
                cout << "e reconhecimento de input sem Enter.\n" << endl;
                cout << "Pressione Enter para voltar ao menu..." << endl;
                cin.ignore();
                cin.get();
                break;
            case 3:
                cout << "Saindo do programa..." << endl;
                break;
            default:
                cout << "Opção inválida! Tente novamente." << endl;
                cout << "Pressione Enter para voltar ao menu..." << endl;
                cin.ignore();
                cin.get();
                break;
        }
    } while (opcao != 3);
}

int main() {
    // Mostra o cursor no início
    cout << "\033[?25h";
    menu();
    return 0;
}
