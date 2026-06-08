#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <vector>
#include <queue>
#include "proceso.h"

enum class Algoritmo {
    FCFS,
    SJF,
    Aleatorio,
    PrioridadNoExpulsivo,
    RoundRobin,
    SRTF,
    PrioridadExpulsivo
};

class Planificador {
public:
    Planificador();

    // metodos de configuración inicial
    void agregarProceso(const Proceso& p);
    void configurarSimulacion(Algoritmo algo, int quantum = 1);
    void reiniciar();

    //avanza la simulación exactamente 1 tick de reloj
    void ejecutarPaso();

    // verificacion de estado
    bool simuracionTerminada() const;
    int getRelojActual() const { return relojActual; }

    // Acceso a datos
    const std::vector<Proceso>& getTodosLosProcesos() const { return tablaProcesos; }
    const std::vector<int>& getColaListos() const { return colaListos; }
    const std::vector<int>& getColaBloqueados() const { return colaBloqueados; }
    int getIdProcesoEnCPU() const { return idProcesoCPU; }

    //calculo de metricas
    double calcularPorcentajeUsoCPU() const;
    double calcularPromedioEspera() const;
    double calcularPromedioBloqueo() const;
    double calcularPromedioEjecucion() const; // Turnaround promedio

    void limpiarProcesos();

private:
    Algoritmo algoritmoActual;
    int quantumMaximo;
    int quantumRestante; // Para controlar la expulsión en Round Robin
    int relojActual;     // Contador de pasos


    std::vector<Proceso> tablaProcesos;

    // Colas de simulacin
    std::vector<int> colaListos;
    std::vector<int> colaBloqueados;
    int idProcesoCPU; // -1 si la CPU esta en ocio

    int ticksCPU; // metrica auxiliar para calcular el % de uso de la CPU

    void verificarNuevosArribos();
    void actualizarProcesosBloqueados();
};



#endif // PLANIFICADOR_H
