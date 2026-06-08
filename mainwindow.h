#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QHBoxLayout>
#include "planificador.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Funciones que reaccionan a los botones y al reloj
    void generarProcesos();
    void iniciarSimulacion();
    void pausarSimulacion();
    void tickReloj(); // funcion que qtimer llama una vez por segundo

private:

    Planificador planificador;
    QTimer* timerReloj;

    // Elementos de la interfaz grafica
    QTableWidget* tablaProcesos;
    QComboBox* comboAlgoritmos;
    QSpinBox* spinQuantum;
    QPushButton* btnGenerar;
    QPushButton* btnIniciar;
    QPushButton* btnPausar;
    QLabel* lblReloj;
    QLabel* lblEstadoCPU;
    QTextEdit* txtEstadisticas;


    void configurarInterfaz();
    void actualizarTabla();
    QString estadoAString(EstadoProceso estado);
    void mostrarEstadisticas();
};

#endif // MAINWINDOW_H