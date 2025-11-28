#ifndef CORREDOR_HPP
#define CORREDOR_HPP

#include <string>
#include <random>
#include <iostream>

using namespace std;

struct Corredor {
    int id;
    int velMin;
    int velMax;
    // Resistencia: chance de não cansar. ex: 0.8 significa 80% de chance de correr normal, 20% de chance de cansar e andar o mínimo.
    double resistencia; 
    // Turnos parados: contador interno para casos de azar (não andar nada)
    int turnosParado;
    // Eliminado: flag para indicar se o corredor ficou incapacitado durante a corrida (não consegue correr mais)
    bool eliminado;

    // Construtor
    Corredor(int id, int vMin, int vMax, double resist) 
        : id(id), velMin(vMin), velMax(vMax), resistencia(resist), turnosParado(0), eliminado(false) {}

    // Método para calcular o próximo passo
    int darPasso(default_random_engine& gerador) {
        // Caso ainda esteja atordoado, não anda nada e decrementa o contador
        if (turnosParado > 0) {
            turnosParado--;
            return 0;
        }

        // Distribuições locais para decidir este passo específico
        uniform_real_distribution<double> distFadiga(0.0, 1.0);
        uniform_int_distribution<int> distVelocidade(velMin, velMax);
        uniform_int_distribution<int> distTropeco(0, 10000);
        uniform_int_distribution<int> distEliminacao(0, 10000);

        // Pequena chance de eliminação do corredor (não consegue mais correr)
        if (distEliminacao(gerador) < 2) { // 0.02% de chance
            eliminado = true;
            return 0;
        }

        // Pequena chance de azar que força o corredor a não andar nada por 15 turnos
        if (distTropeco(gerador) < 5) { // 0.05% de chance
            turnosParado = 14; // Fica parado por 14 turnos adicionais (total 15)
            return 0;
        }

        // Verifica se o corredor cansou neste turno
        if (distFadiga(gerador) > resistencia) {
            // Cansou! Anda apenas a velocidade mínima
            return velMin;
        }
        
        // Corre normalmente
        return distVelocidade(gerador);
    }

    void mostrarAtributos() const {
        cout << "Corredor " << id << " | Vel: [" << velMin << "-" << velMax << "] | Resistencia: " << (int)(resistencia * 100) << "%" << endl;
    }
};

#endif