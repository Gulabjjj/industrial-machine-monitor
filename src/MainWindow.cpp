#include "MainWindow.h"

#include "Machine.h"
#include "MachineWidget.h"

#include <QBrush>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_log(new EventLog(this)), m_timer(new QTimer(this))
{
    setWindowTitle("Industrial Machine Monitoring System");
    resize(1200, 800);

    // Machine fleet: name, rated RPM, heat factor (>1.25 => prone to overheating)
    m_machines.push_back(new Machine("CNC Mill #1", 1500, 1.00, this));
    m_machines.push_back(new Machine("Hydraulic Press #2", 1200, 1.10, this));
    m_machines.push_back(new Machine("Conveyor Motor #3", 1800, 0.85, this));
    m_machines.push_back(new Machine("Turbine Pump #4", 3000, 1.45, this));

    for (Machine *m : m_machines)
        connect(m, &Machine::logEvent, m_log, &EventLog::add);

    buildUi();

    connect(m_log, &EventLog::entryAdded, this, &MainWindow::appendLogRow);
    connect(m_log, &EventLog::cleared, this, [this] { m_logTable->setRowCount(0); });
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTick);

    m_log->add(EventLog::Level::Info, "System", "Monitoring system initialised");
    m_timer->start(kTickMs);
    updateStats();
}

QLabel *MainWindow::makeStatLabel(const QString &caption, QWidget *parent, QHBoxLayout *layout)
{
    auto *box = new QFrame(parent);
    box->setStyleSheet("QFrame{background:#232834;border:1px solid #3a4150;border-radius:8px;}");
    auto *v = new QVBoxLayout(box);
    auto *cap = new QLabel(caption);
    cap->setStyleSheet("color:#9aa3b5;font-size:11px;border:none;");
    auto *val = new QLabel("0");
    val->setStyleSheet("color:#e6e9ef;font-size:22px;font-weight:bold;border:none;");
    v->addWidget(cap);
    v->addWidget(val);
    layout->addWidget(box);
    return val;
}

void MainWindow::buildUi()
{
    auto *central = new QWidget;
    central->setStyleSheet("QWidget{background:#171b24;}");
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);

    // --- Statistics bar ---
    auto *stats = new QHBoxLayout;
    m_totalLbl = makeStatLabel("MACHINES", central, stats);
    m_runningLbl = makeStatLabel("RUNNING", central, stats);
    m_faultLbl = makeStatLabel("IN FAULT", central, stats);
    m_avgTempLbl = makeStatLabel("AVG TEMP (RUNNING)", central, stats);
    m_totalFaultsLbl = makeStatLabel("TOTAL FAULTS", central, stats);
    root->addLayout(stats);

    // --- Machine grid ---
    auto *grid = new QGridLayout;
    int i = 0;
    for (Machine *m : m_machines) {
        grid->addWidget(new MachineWidget(m, central), i / 2, i % 2);
        ++i;
    }
    root->addLayout(grid, 3);

    // --- Controls ---
    auto *ctl = new QHBoxLayout;
    auto *startAllBtn = new QPushButton("Start All");
    auto *stopAllBtn = new QPushButton("Stop All");
    auto *exportBtn = new QPushButton("Export Log (CSV)");
    auto *clearBtn = new QPushButton("Clear Log");
    for (auto *b : {startAllBtn, stopAllBtn, exportBtn, clearBtn})
        ctl->addWidget(b);
    ctl->addStretch();
    root->addLayout(ctl);
    connect(startAllBtn, &QPushButton::clicked, this, &MainWindow::startAll);
    connect(stopAllBtn, &QPushButton::clicked, this, &MainWindow::stopAll);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportLog);
    connect(clearBtn, &QPushButton::clicked, m_log, &EventLog::clear);

    // --- Event log ---
    m_logTable = new QTableWidget(0, 4);
    m_logTable->setHorizontalHeaderLabels({"Timestamp", "Level", "Machine", "Message"});
    m_logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_logTable->setColumnWidth(0, 170);
    m_logTable->setColumnWidth(1, 90);
    m_logTable->setColumnWidth(2, 170);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_logTable->verticalHeader()->setVisible(false);
    m_logTable->setStyleSheet("QTableWidget{background:#1c212c;color:#e6e9ef;gridline-color:#2d3340;}"
                              "QHeaderView::section{background:#232834;color:#9aa3b5;border:none;padding:4px;}");
    root->addWidget(m_logTable, 2);

    statusBar()->showMessage("Simulation running");
    statusBar()->setStyleSheet("color:#9aa3b5;");
}

void MainWindow::onTick()
{
    const double dt = kTickMs / 1000.0;
    for (Machine *m : m_machines)
        m->tick(dt);
    updateStats();
}

void MainWindow::updateStats()
{
    int running = 0, faulted = 0, totalFaults = 0;
    double tempSum = 0;
    for (const Machine *m : m_machines) {
        if (m->state() == Machine::State::Running) {
            ++running;
            tempSum += m->temperature();
        }
        if (m->state() == Machine::State::Fault)
            ++faulted;
        totalFaults += m->faultCount();
    }
    m_totalLbl->setText(QString::number(m_machines.size()));
    m_runningLbl->setText(QString::number(running));
    m_faultLbl->setText(QString::number(faulted));
    m_faultLbl->setStyleSheet(QString("color:%1;font-size:22px;font-weight:bold;border:none;")
                                  .arg(faulted ? "#da3633" : "#e6e9ef"));
    m_avgTempLbl->setText(running ? QString("%1 °C").arg(tempSum / running, 0, 'f', 1)
                                  : QString("—"));
    m_totalFaultsLbl->setText(QString::number(totalFaults));
}

void MainWindow::appendLogRow(const EventLog::Entry &e)
{
    m_logTable->insertRow(0);
    const QStringList cells{e.time.toString("yyyy-MM-dd HH:mm:ss"), EventLog::levelName(e.level),
                            e.source, e.message};
    QColor fg("#e6e9ef");
    if (e.level == EventLog::Level::Warning) fg = QColor("#d29922");
    if (e.level == EventLog::Level::Fault)   fg = QColor("#ff7b72");
    for (int c = 0; c < cells.size(); ++c) {
        auto *item = new QTableWidgetItem(cells[c]);
        item->setForeground(QBrush(fg));
        m_logTable->setItem(0, c, item);
    }
    while (m_logTable->rowCount() > kMaxLogRows)
        m_logTable->removeRow(m_logTable->rowCount() - 1);
}

void MainWindow::startAll()
{
    for (Machine *m : m_machines)
        m->start();
}

void MainWindow::stopAll()
{
    for (Machine *m : m_machines)
        m->stop();
}

void MainWindow::exportLog()
{
    const QString path = QFileDialog::getSaveFileName(this, "Export event log", "event_log.csv",
                                                      "CSV files (*.csv)");
    if (path.isEmpty())
        return;
    if (!m_log->exportCsv(path))
        QMessageBox::warning(this, "Export failed", "Could not write " + path);
    else
        statusBar()->showMessage("Log exported to " + path, 5000);
}
