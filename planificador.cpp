#include "planificador.h"
#include <algorithm>
#include <cstdlib> // Necesario para std::rand() en el algoritmo Aleatorio
#include <ctime>

Planificador::Planificador() {
    // Inicializar la semilla de aleatoriedad
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    reiniciar();
}

void Planificador::agregarProceso(const Proceso& p) {
    tablaProcesos.push_back(p);
}

void Planificador::configurarSimulacion(Algoritmo algo, int quantum) {
    algoritmoActual = algo;
    quantumMaximo = quantum;
    reiniciar();
}


void Planificador::reiniciar() {
    relojActual = 0;
    idProcesoCPU = -1;
    ticksCPU = 0; // cuenta los ticks que la CPU esta trabajando
    quantumRestante = quantumMaximo;

    colaListos.clear();
    colaBloqueados.clear();

    for (auto& p : tablaProcesos) {
        p.tiempo_restante = p.tiempo_rafaga;
        p.io_restante = p.tiempo_io;
        p.estado = EstadoProceso::Listo;
        p.tiempo_espera = 0;
        p.tiempo_bloqueado = 0;
        p.tiempo_retorno = 0;
        p.tiempo_finalizacion = 0;
    }
}

bool Planificador::simuracionTerminada() const {
    for (const auto& p : tablaProcesos) {
        if (p.estado != EstadoProceso::Terminado) {
            return false;
        }
    }
    return true;
}

void Planificador::limpiarProcesos() {
    tablaProcesos.clear();
    reiniciar();
}



void Planificador::ejecutarPaso() {
    if (simuracionTerminada()) return;

    // llegadas
    verificarNuevosArribos();

    // si la CPU está libre, asignamos un proceso
    if (idProcesoCPU == -1 && !colaListos.empty()) {
        auto it = colaListos.begin();  // por defecto FCFS

        if (algoritmoActual == Algoritmo::SJF) {
            for (auto iter = colaListos.begin(); iter != colaListos.end(); ++iter) {
                if (tablaProcesos[*iter].tiempo_rafaga < tablaProcesos[*it].tiempo_rafaga)
                    it = iter;
            }
        }
        else if (algoritmoActual == Algoritmo::SRTF) {
            for (auto iter = colaListos.begin(); iter != colaListos.end(); ++iter) {
                if (tablaProcesos[*iter].tiempo_restante < tablaProcesos[*it].tiempo_restante)
                    it = iter;
            }
        }
        else if (algoritmoActual == Algoritmo::PrioridadNoExpulsivo ||
                 algoritmoActual == Algoritmo::PrioridadExpulsivo) {
            for (auto iter = colaListos.begin(); iter != colaListos.end(); ++iter) {
                if (tablaProcesos[*iter].prioridad < tablaProcesos[*it].prioridad)
                    it = iter;
            }
        }
        else if (algoritmoActual == Algoritmo::Aleatorio) {
            int indice_random = std::rand() % colaListos.size();
            it = colaListos.begin() + indice_random;
        }

        // asignar a la CPU
        idProcesoCPU = *it;
        colaListos.erase(it);
        tablaProcesos[idProcesoCPU].estado = EstadoProceso::Ejecutando;

        if (algoritmoActual == Algoritmo::RoundRobin)
            quantumRestante = quantumMaximo;
    }

    //incrementar tiempo de espera de los que siguen en cola de listos
    for (int id : colaListos)
        tablaProcesos[id].tiempo_espera++;

    // ejecutar un tick en la CPU (puede expulsar)
    if (idProcesoCPU != -1) {
        Proceso& p = tablaProcesos[idProcesoCPU];
        p.tiempo_restante--;
        ticksCPU++;

        if (algoritmoActual == Algoritmo::RoundRobin)
            quantumRestante--;

        if (p.tiempo_restante == 0) {
            if (p.io_restante > 0) {
                p.estado = EstadoProceso::Bloqueado;
                colaBloqueados.push_back(idProcesoCPU);
            } else {
                p.estado = EstadoProceso::Terminado;
                p.tiempo_finalizacion = relojActual + 1;
                p.tiempo_retorno = p.tiempo_finalizacion - p.tiempo_llegada;
            }
            idProcesoCPU = -1;
        } else {
            bool expulsar = false;
            if (algoritmoActual == Algoritmo::RoundRobin && quantumRestante <= 0) {
                expulsar = true;
            } else if (algoritmoActual == Algoritmo::SRTF) {
                for (int id : colaListos) {
                    if (tablaProcesos[id].tiempo_restante < p.tiempo_restante) {
                        expulsar = true;
                        break;
                    }
                }
            } else if (algoritmoActual == Algoritmo::PrioridadExpulsivo) {
                for (int id : colaListos) {
                    if (tablaProcesos[id].prioridad < p.prioridad) {
                        expulsar = true;
                        break;
                    }
                }
            }

            if (expulsar) {
                p.estado = EstadoProceso::Listo;
                colaListos.push_back(idProcesoCPU);  // se añade DESPUÉS del incremento de espera
                idProcesoCPU = -1;
            }
        }
    }

    // actualizar procesos bloqueados
    actualizarProcesosBloqueados();

    // avanzar reloj
    relojActual++;
}



void Planificador::verificarNuevosArribos() {
    for (size_t i = 0; i < tablaProcesos.size(); ++i) {
        if (tablaProcesos[i].tiempo_llegada == relojActual) {
            colaListos.push_back(i);
        }
    }
}

void Planificador::actualizarProcesosBloqueados() {
    for (auto it = colaBloqueados.begin(); it != colaBloqueados.end(); ) {
        int id = *it;
        Proceso& p = tablaProcesos[id];
        p.io_restante--;
        p.tiempo_bloqueado++;

        if (p.io_restante <= 0) {
            p.estado = EstadoProceso::Terminado;
            p.tiempo_finalizacion = relojActual + 1;  // termino en este tick
            p.tiempo_retorno = p.tiempo_finalizacion - p.tiempo_llegada;
            it = colaBloqueados.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Metricas globales
double Planificador::calcularPorcentajeUsoCPU() const {
    if (relojActual == 0) return 0.0;
    return (static_cast<double>(ticksCPU) / relojActual) * 100.0;
}

double Planificador::calcularPromedioEspera() const {
    if (tablaProcesos.empty()) return 0.0;
    double suma = 0;
    for (const auto& p : tablaProcesos) suma += p.tiempo_espera;
    return suma / tablaProcesos.size();
}

double Planificador::calcularPromedioBloqueo() const {
    if (tablaProcesos.empty()) return 0.0;
    double suma = 0;
    for (const auto& p : tablaProcesos) suma += p.tiempo_bloqueado;
    return suma / tablaProcesos.size();
}

double Planificador::calcularPromedioEjecucion() const {
    if (tablaProcesos.empty()) return 0.0;
    double suma = 0;
    for (const auto& p : tablaProcesos) suma += p.tiempo_retorno;
    return suma / tablaProcesos.size();
}