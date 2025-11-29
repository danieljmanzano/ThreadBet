#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <random>
#include <vector>  
#include <cstdlib> 
#include "corredor.hpp"

using namespace std;

int threadsFinalizadas = 0; // Contador global de threads finalizadas
mutex mutexThreads; // Protege o contador acima
mutex mutexVencedor; // Mutex para proteger o acesso ao pódio
mutex mutexRNG; // Mutex específico para proteger o gerador de números aleatórios

vector<int> podio; 

// Gerador global (protegido pelo mutexRNG)
default_random_engine gerador(chrono::system_clock::now().time_since_epoch().count());

/*  
    Função que simula a corrida de um corredor. Cada corredor é uma thread separada que executa esta função

    @param meuCorredor: Corredor que está executando esta thread
    @param tamPista: Tamanho total da pista
    @param posicoesCorredores: Referência ao vetor compartilhado que mantém as posições atuais dos corredores
    @param estadosCorredores: Referência ao vetor compartilhado que mantém os estados visuais dos corredores
    @return: void
*/
void correr(Corredor& meuCorredor, int tamPista, vector<int> &posicoesCorredores, vector<char>& estadosCorredores) {
    int posicaoAtual = 0;

    while(posicaoAtual < tamPista && !meuCorredor.eliminado) {
        int passos;

        // Protege o gerador de números aleatórios com um mutex.
        // Se várias threads tentarem gerar números ao mesmo tempo, ele pode quebrar ou gerar números ruins, por isso o mutex é necessário.
        {
            lock_guard<mutex> lock(mutexRNG);
            passos = meuCorredor.darPasso(gerador);
        }

        // Atribui o estado visual do corredor com base no resultado do passo
        if (meuCorredor.eliminado) {
            estadosCorredores[meuCorredor.id] = '#'; // Eliminado
        } else if (passos == 0) {
            estadosCorredores[meuCorredor.id] = 'X'; // Tropeçou/Azar
        } else {
            estadosCorredores[meuCorredor.id] = '>'; // Correndo normal
        }

        posicaoAtual += passos;

        if(posicaoAtual > tamPista) {
            posicaoAtual = tamPista;
        }

        this_thread::sleep_for(chrono::milliseconds(50));

        // Usa o ID do objeto corredor para atualizar a posição correta no vetor visual
        posicoesCorredores[meuCorredor.id] = posicaoAtual;

        if(posicaoAtual >= tamPista) {
            lock_guard<mutex>lock(mutexVencedor);
            podio.push_back(meuCorredor.id);
        }
    }

    // Atualiza o contador de threads finalizadas de forma segura usando o Mutex
    {
        lock_guard<mutex> lock(mutexThreads);
        threadsFinalizadas++;
    }

    return;
}


/*  
    Função para imprimir o pódio final

    @return: void
*/
void impressao_podio() {
    cout << "\n--- PODIO FINAL ---" << endl;
    cout << "1° Lugar: Corredor " << podio[0] << endl;

    // Checa se há corredores suficientes para 2º e 3º lugar
    if (podio.size() > 1) {
        cout << "2° Lugar: Corredor " << podio[1] << endl;
    }
    if (podio.size() > 2) {
        cout << "3° Lugar: Corredor " << podio[2] << endl;
    }

    return;
}


/*  
    Função para mostrar o resultado da aposta do usuário

    @param aposta: Número do corredor em que o usuário apostou
    @return: void
*/
void impressao_resultado(int aposta) {
    cout << "\n--- Resultado da Aposta ---" << endl;
    if(aposta == podio[0]) { // Checa se a aposta foi no 1º lugar
        cout << "Parabens! Sua aposta no corredor " << aposta << " venceu!" << endl;
    } else if (aposta == podio[1]) { // Checa se a aposta foi no 2º lugar
        cout << "Sua aposta no corredor " << aposta << " ficou em segundo lugar!" << endl;
    } else if (aposta == podio[2]) { // Checa se a aposta foi no 3º lugar
        cout << "Sua aposta no corredor " << aposta << " ficou em terceiro lugar!" << endl;
    } else {
        cout << "Sua aposta no corredor " << aposta << " perdeu. Boa sorte na próxima!" << endl;
    }

    return;
}


/*
    Função para imprimir o estado atual da pista e dos corredores

    @param tamPista: Tamanho total da pista
    @param nCorredores: Número total de corredores
    @param posicoesCorredores: Vetor com as posições atuais dos corredores
    @param estadosCorredores: Vetor com os estados visuais dos corredores
    @param aposta: Número do corredor em que o usuário apostou
    @return: void
*/
void impressao_pista(int tamPista, int nCorredores, const vector<int>& posicoesCorredores, const vector<char> estadosCorredores, int aposta) {
    const int LARGURA_VISUAL = 50; // Largura fixa da pista no terminal

    cout << "--- Jogo de Corrida Multithread (SO) ---\n";
    cout << "Pista de " << tamPista << "m | Aposta no Corredor: " << aposta << "\n";
    cout << "-------------------------------------------\n";

    // Desenha cada corredor
    for(int i = 1; i <= nCorredores; i++) {
        cout << "Corredor " << i;

        if (i < 10) {
            cout << " "; // Alinhamento para IDs < 10
        }
        cout << ": [";

        // Calcula a posição do corredor proporcionalmente ao tamanho da pista
        int posVisual = (int)(((double)posicoesCorredores[i] / tamPista) * LARGURA_VISUAL);

        // Desenha o rastro
        for(int j = 0; j < posVisual; j++) {
            cout << "-";
        }

        // Desenha o "avatar"
        if (posicoesCorredores[i] < tamPista) {
            cout << estadosCorredores[i]; // Em movimento, tropeçou ou eliminado
        } else {
            cout << "F"; // Finalizou
        }

        // Desenha o espaço vazio
        for(int j = posVisual + 1; j < LARGURA_VISUAL; j++) {
            cout << " ";
        }

        cout << "] (" << posicoesCorredores[i] << "/" << tamPista << "m)" << endl;
    }

    for (int i = 1; i <= nCorredores; i++) {
        if (estadosCorredores[i] == 'X') {
            cout << "Corredor " << i << " tropeçou!\n";
        }
    }

    for (int i = 1; i <= nCorredores; i++) {
        if (estadosCorredores[i] == '#') {
            cout << "Corredor " << i << " foi eliminado da corrida!\n";
        }
    }

    return;
}


/*
    Função para limpar o terminal de acordo com o sistema operacional

    @param opcaoSistema: 1 para Windows, 2 para Linux/Mac
    @return: void
*/
void limpa_terminal(int opcaoSistema) {
    if (opcaoSistema == 1) {
        system("cls"); // Comando específico para Windows
    } else {
        system("clear"); // Comando específico para Linux/Mac
    }
}


/*
    Função para gerar corredores com atributos aleatórios

    @param nCorredores: Número de corredores a serem gerados
    @return: Vetor de corredores (objetos) gerados
*/
vector<Corredor> gera_corredores(int nCorredores) {
    vector<Corredor> corredores;
    uniform_int_distribution<int> distVelMin(1, 3);
    uniform_int_distribution<int> distBonusVelMax(2, 5); // VelMax é VelMin + Bonus
    uniform_real_distribution<double> distResistencia(0.5, 0.95); // 50% a 95% de resistência

    cout << "\n--- Análise dos Atletas ---\n";
    for(int i = 1; i <= nCorredores; i++) {
        int vMin = distVelMin(gerador);
        int vMax = vMin + distBonusVelMax(gerador);
        double resist = distResistencia(gerador);

        // Cria o corredor e coloca no vetor
        corredores.emplace_back(i, vMin, vMax, resist); // Obs.: não é preciso passar 'eliminado' ou 'turnosParado', pois o construtor já inicializa
        
        // Mostra os atributos para o usuário
        corredores.back().mostrarAtributos();
    }
    cout << "---------------------------\n";

    return corredores;
}


int main(void) {
    // Antes de executar, pergunta o sistema operacional para usar o comando correto de limpar terminal
    int opcaoSistema;
    cout << "Escolha o sistema operacional:\n1. Windows\n2. Linux/Mac\nOpção: ";
    while(!(cin >> opcaoSistema) || (opcaoSistema != 1 && opcaoSistema != 2)) {
        cout << "Opção inválida. Escolha 1 para Windows ou 2 para Linux/Mac: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    int tamPista = 0;
    int nCorredores = 0;
    int aposta = 0;

    limpa_terminal(opcaoSistema);
    cout << "--- Jogo de Corrida Multithread (SO) ---\n";
    cout << "Defina o tamanho da pista (ex: 500): ";

    // Lendo as entradas
    while(!(cin >> tamPista) || tamPista <= 0) {
        cout << "Valor invalido. Insira um numero inteiro positivo para o tamanho da pista: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    cout << "Defina o número de corredores (max 20): ";
    while(!(cin >> nCorredores) || nCorredores <= 0 || nCorredores > 20) {
        cout << "Valor inválido. Escolha um numero entre 1 e 20: ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    // Criação dos corredores (objetos) com atributos aleatórios
    vector<Corredor> listaCorredores = gera_corredores(nCorredores);

   
    vector<int> posicoesCorredores(nCorredores + 1, 0); // Vetor para manter controle das posições
    vector<thread> threadsCorredores; // Vetor para armazenar as threads dos corredores
    vector<char> estadosCorredores(nCorredores + 1, '>'); // Vetor para armazenar os estados visuais dos corredores. Inicializa "em movimento"

    cout << "Em qual corredor (1 a " << nCorredores << ") você aposta? ";
    while(!(cin >> aposta) || aposta < 1 || aposta > nCorredores) {
        cout << "Aposta inválida. Escolha um corredor entre 1 e " << nCorredores << ": ";
        cin.clear();
        cin.ignore(10000, '\n');
    }

    // Limpa o terminal antes da largada
    limpa_terminal(opcaoSistema);
    cout << ">>> Pista de " << tamPista << " metros. " << nCorredores << " corredores na largada. Aposta no " << aposta << ". <<<\n";
    this_thread::sleep_for(chrono::seconds(1));
    cout << "\n>>> Preparar... JÁ! <<<\n";
    this_thread::sleep_for(chrono::seconds(1));

    // Inicializar as threads
    for(int i = 0; i < nCorredores; i++) {
        threadsCorredores.emplace_back(correr, ref(listaCorredores[i]), tamPista, ref(posicoesCorredores), ref(estadosCorredores));
    }

    cout << "Corrida iniciada!\n";

    // Contador local de finalizados
    int contagemLocalFinalizados = 0;
    // Loop de monitoramento que continua até todos os corredores finalizarem
    while (contagemLocalFinalizados < nCorredores) {
        this_thread::sleep_for(chrono::milliseconds(50));

        // Limpa o terminal a cada iteração, dando a sensação de movimento do corredor
        limpa_terminal(opcaoSistema);
        // Imprime o estado atual da pista
        impressao_pista(tamPista, nCorredores, posicoesCorredores, estadosCorredores, aposta);

        { // Atualiza o contador de finalizados de forma segura usando o Mutex
            lock_guard<mutex> lock(mutexThreads);
            contagemLocalFinalizados = threadsFinalizadas;
        } // As chaves garantem que o mutex só fique bloqueado pelo tempo exato de ler o size() do pódio
    }

    // Sincronização de encerramento
    for(auto& t : threadsCorredores) {
        if(t.joinable()) {
            t.join(); // Espera a thread t terminar
        }
    }

    // Imprime o pódio final
    impressao_podio();

    // Mostra quem não terminou a prova
    cout << "\n--- Relatório Médico ---" << endl;
    bool teveEliminado = false;
    for(const auto& c : listaCorredores) {
        if(c.eliminado) {
            cout << "Corredor " << c.id << " sofreu uma lesão grave e nao completou a prova." << endl;
            teveEliminado = true;
        }
    }
    if(!teveEliminado) cout << "Todos os atletas terminaram a prova bem." << endl;

    // Mostra o resultado da aposta do usuário
    impressao_resultado(aposta);

    return 0;
}