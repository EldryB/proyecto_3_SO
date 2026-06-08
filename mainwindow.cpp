#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QWidget>
#include <cstdlib>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    configurarInterfaz();

    // Configurar el reloj del sistema
    timerReloj = new QTimer(this);
    // Conectar la señal timeout del reloj a nuestra funcion tickReloj
    connect(timerReloj, &QTimer::timeout, this, &MainWindow::tickReloj);

    // Conectar los botones a sus funciones
    connect(btnGenerar, &QPushButton::clicked, this, &MainWindow::generarProcesos);
    connect(btnIniciar, &QPushButton::clicked, this, &MainWindow::iniciarSimulacion);
    connect(btnPausar, &QPushButton::clicked, this, &MainWindow::pausarSimulacion);
}

MainWindow::~MainWindow() {

}

void MainWindow::configurarInterfaz() {
    // widget ccentral y layout
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // controles
    QHBoxLayout* controlesLayout = new QHBoxLayout();

    btnGenerar = new QPushButton("1. Generar Procesos", this);

    comboAlgoritmos = new QComboBox(this);
    comboAlgoritmos->addItem("FCFS", static_cast<int>(Algoritmo::FCFS));
    comboAlgoritmos->addItem("SJF", static_cast<int>(Algoritmo::SJF));
    comboAlgoritmos->addItem("Round Robin", static_cast<int>(Algoritmo::RoundRobin));
    comboAlgoritmos->addItem("SRTF", static_cast<int>(Algoritmo::SRTF));
    comboAlgoritmos->addItem("Aleatorio", static_cast<int>(Algoritmo::Aleatorio));
    comboAlgoritmos->addItem("Prioridad no Expulsiva", static_cast<int>(Algoritmo::PrioridadNoExpulsivo));
    comboAlgoritmos->addItem("Prioridad Expulsiva", static_cast<int>(Algoritmo::PrioridadExpulsivo));

    spinQuantum = new QSpinBox(this);
    spinQuantum->setPrefix("Quantum: ");
    spinQuantum->setMinimum(1);
    spinQuantum->setValue(3);

    btnIniciar = new QPushButton("2. Iniciar", this);
    btnPausar = new QPushButton("Pausar", this);
    btnPausar->setEnabled(false);

    controlesLayout->addWidget(btnGenerar);
    controlesLayout->addWidget(comboAlgoritmos);
    controlesLayout->addWidget(spinQuantum);
    controlesLayout->addWidget(btnIniciar);
    controlesLayout->addWidget(btnPausar);

    // tabla de procesos
    tablaProcesos = new QTableWidget(0, 6, this);
    tablaProcesos->setHorizontalHeaderLabels({"ID", "Estado", "Llegada", "Ráfaga", "Restante", "E/S"});
    tablaProcesos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaProcesos->setEditTriggers(QAbstractItemView::NoEditTriggers); // Solo lectura

    // estadisticas
    QHBoxLayout* statsLayout = new QHBoxLayout();
    lblReloj = new QLabel("Reloj: 0 ticks", this);
    lblEstadoCPU = new QLabel("CPU: Inactiva", this);
    lblReloj->setStyleSheet("font-weight: bold; font-size: 14px;");

    statsLayout->addWidget(lblReloj);
    statsLayout->addWidget(lblEstadoCPU);

    // reportes
    txtEstadisticas = new QTextEdit(this);
    txtEstadisticas->setReadOnly(true);
    txtEstadisticas->setMaximumHeight(150); // Que no ocupe toda la pantalla
    txtEstadisticas->setPlaceholderText("Las estadísticas aparecerán aquí al finalizar la simulación...");

    // ensamblar todo
    mainLayout->addLayout(controlesLayout);
    mainLayout->addWidget(tablaProcesos);
    mainLayout->addLayout(statsLayout);
    mainLayout->addWidget(txtEstadisticas);

    setCentralWidget(centralWidget);
    resize(750, 550);
    setWindowTitle("Simulador de Planificación de Procesos");
}

void MainWindow::generarProcesos() {
    // vaciamos la lista por si el usuario le da varias veces al botón
    planificador.limpiarProcesos();

    // generar 5 procesos aleatorios para la prueba
    for (int i = 0; i < 5; ++i) {
        int rafaga = (std::rand() % 7) + 2;    // rafaga entre 2 y 8
        int io = (std::rand() % 4);             // E/S entre 0 y 3
        int prioridad = (std::rand() % 5) + 1;  // prioridad 1 a 5
        int llegada = (std::rand() % 5);        // llegada entre tick 0 y 4

        planificador.agregarProceso(Proceso(i, rafaga, io, prioridad, llegada));
    }

    actualizarTabla();

    // restaurar la interfaz
    lblReloj->setText("Reloj: 0 ticks");
    lblEstadoCPU->setText("CPU: Inactiva");
    lblEstadoCPU->setStyleSheet("color: black; font-weight: bold;");
    txtEstadisticas->clear();

    // asegurar que se pueda volver a iniciar
    btnIniciar->setEnabled(true);
    comboAlgoritmos->setEnabled(true);
}

void MainWindow::iniciarSimulacion() {
    if (planificador.getTodosLosProcesos().empty()) return;

    Algoritmo algo = static_cast<Algoritmo>(comboAlgoritmos->currentData().toInt());
    int quantum = spinQuantum->value();

    // Si la simulacion ya habia terminado, la reseteamos para evaluar el nuevo algoritmo
    if (planificador.simuracionTerminada()) {
        planificador.configurarSimulacion(algo, quantum);
        txtEstadisticas->clear();
    } else if (planificador.getRelojActual() == 0) {
        // es una simulacion completamente nueva
        planificador.configurarSimulacion(algo, quantum);
    }

    btnGenerar->setEnabled(false);
    btnIniciar->setEnabled(false);
    btnPausar->setEnabled(true);
    comboAlgoritmos->setEnabled(false);

    // iniciar el reloj, un tick por segundo
    timerReloj->start(1000);
}

void MainWindow::pausarSimulacion() {
    timerReloj->stop();
    btnIniciar->setEnabled(true);
    btnPausar->setEnabled(false);
}

// Funcion del tiempo
void MainWindow::tickReloj() {
    // Avanzamos la simulación 1 paso
    planificador.ejecutarPaso();

    // actualizamos los textos y la tabla
    lblReloj->setText(QString("Reloj: %1 ticks").arg(planificador.getRelojActual()));

    int cpuID = planificador.getIdProcesoEnCPU();
    if (cpuID != -1) {
        lblEstadoCPU->setText(QString("CPU: Ejecutando Proceso %1").arg(cpuID));
        lblEstadoCPU->setStyleSheet("color: green; font-weight: bold;");
    } else {
        lblEstadoCPU->setText("CPU: Inactiva");
        lblEstadoCPU->setStyleSheet("color: red; font-weight: bold;");
    }

    actualizarTabla();

    // verificamos si terminamos
    if (planificador.simuracionTerminada()) {
        timerReloj->stop();
        lblEstadoCPU->setText("Simulación Completada");
        lblEstadoCPU->setStyleSheet("color: blue; font-weight: bold;");

        // reactivamos los botones
        btnGenerar->setEnabled(true);
        btnIniciar->setEnabled(true);
        btnPausar->setEnabled(false);
        comboAlgoritmos->setEnabled(true);

        mostrarEstadisticas();
    }
}

void MainWindow::actualizarTabla() {
    const auto& procesos = planificador.getTodosLosProcesos();
    tablaProcesos->setRowCount(procesos.size());

    for (size_t i = 0; i < procesos.size(); ++i) {
        const Proceso& p = procesos[i];

        tablaProcesos->setItem(i, 0, new QTableWidgetItem(QString::number(p.id)));
        tablaProcesos->setItem(i, 1, new QTableWidgetItem(estadoAString(p.estado)));
        tablaProcesos->setItem(i, 2, new QTableWidgetItem(QString::number(p.tiempo_llegada)));
        tablaProcesos->setItem(i, 3, new QTableWidgetItem(QString::number(p.tiempo_rafaga)));
        tablaProcesos->setItem(i, 4, new QTableWidgetItem(QString::number(p.tiempo_restante)));
        tablaProcesos->setItem(i, 5, new QTableWidgetItem(QString::number(p.io_restante)));

        // darle color a la fila en ejecucion
        if (p.estado == EstadoProceso::Ejecutando) {
            for(int col = 0; col < 6; col++) {
                tablaProcesos->item(i, col)->setBackground(QColor(144, 238, 144)); // Verde claro
            }
        }
    }
}

QString MainWindow::estadoAString(EstadoProceso estado) {
    switch (estado) {
    case EstadoProceso::Listo: return "Listo";
    case EstadoProceso::Ejecutando: return "Ejecutando";
    case EstadoProceso::Bloqueado: return "Bloqueado (E/S)";
    case EstadoProceso::Terminado: return "Terminado";
    default: return "Desconocido";
    }
}

void MainWindow::mostrarEstadisticas() {
    // recolectamos los datos basicos
    int totalProcesos = planificador.getTodosLosProcesos().size();
    int tiempoTotal = planificador.getRelojActual();

    // El rendimineto por paso es la cantidad de procesos dividida entre el tiempo total
    double rendimiento = (tiempoTotal > 0) ? (static_cast<double>(totalProcesos) / tiempoTotal) : 0.0;

    // armamos el texto usando html basico
    QString reporte = "<h3 style='color: #2c3e50;'>📊 Reporte de Rendimiento</h3>";
    reporte += "<ul>";

    reporte += QString("<li><b>Tiempo Total:</b> %1 ticks</li>").arg(tiempoTotal);
    reporte += QString("<li><b>Total procesos completados:</b> %1</li>").arg(totalProcesos);
    reporte += QString("<li><b>Uso del Procesador:</b> %1 %</li>").arg(planificador.calcularPorcentajeUsoCPU(), 0, 'f', 2);
    reporte += QString("<li><b>Tiempo promedio de espera:</b> %1 ticks</li>").arg(planificador.calcularPromedioEspera(), 0, 'f', 2);
    reporte += QString("<li><b>Tiempo promedio de ejecución (Retorno):</b> %1 ticks</li>").arg(planificador.calcularPromedioEjecucion(), 0, 'f', 2);
    reporte += QString("<li><b>Tiempo promedio en bloqueo (E/S):</b> %1 ticks</li>").arg(planificador.calcularPromedioBloqueo(), 0, 'f', 2);
    reporte += QString("<li><b>Arribo de procesos por paso:</b> %1 procesos/tick</li>").arg(rendimiento, 0, 'f', 3);

    reporte += "</ul>";

    // mostramos el texto en la caja
    txtEstadisticas->setHtml(reporte);
}