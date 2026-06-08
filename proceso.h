#ifndef PROCESO_H
#define PROCESO_H

#include <QString>

// estados en los que puede encontrarse un proceso durante la simulacion
enum class EstadoProceso {
    Listo,
    Ejecutando,
    Bloqueado,
    Terminado
};

struct Proceso {
    int id;
    int tiempo_rafaga;       // burst time total inicial
    int tiempo_io;           // timepo requerido en E/S
    int prioridad;           // nivel de prioridad
    int tiempo_llegada;      // Tick en el que el proceso entra al sistema


    int tiempo_restante;
    int io_restante;         // loque le falta de E/S si esta bloqueado
    EstadoProceso estado;

    // metricas para estadistica de rendimiento
    int tiempo_espera;       // tiempo pasado en la cola de listos
    int tiempo_bloqueado;    // timepo total acumulado en bloqueo
    int tiempo_retorno;      // Turnaround time
    int tiempo_finalizacion; // tick exacto en el que termino

    //Constructor
    Proceso(int _id, int _rafaga, int _io, int _prioridad, int _llegada)
        : id(_id),
        tiempo_rafaga(_rafaga),
        tiempo_io(_io),
        prioridad(_prioridad),
        tiempo_llegada(_llegada),
        tiempo_restante(_rafaga),
        io_restante(_io),
        estado(EstadoProceso::Listo),
        tiempo_espera(0),
        tiempo_bloqueado(0),
        tiempo_retorno(0),
        tiempo_finalizacion(0) {}
};


#endif // PROCESO_H
