#pragma once
#include "EventLog.h"

#include <QMainWindow>
#include <vector>

class Machine;
class QHBoxLayout;
class QLabel;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onTick();
    void appendLogRow(const EventLog::Entry &entry);
    void updateStats();
    void startAll();
    void stopAll();
    void exportLog();

private:
    void buildUi();
    QLabel *makeStatLabel(const QString &caption, QWidget *parent, QHBoxLayout *layout);

    static constexpr int kTickMs = 500;
    static constexpr int kMaxLogRows = 1000;

    EventLog *m_log;
    std::vector<Machine *> m_machines;
    QTimer *m_timer;

    QLabel *m_totalLbl = nullptr;
    QLabel *m_runningLbl = nullptr;
    QLabel *m_faultLbl = nullptr;
    QLabel *m_avgTempLbl = nullptr;
    QLabel *m_totalFaultsLbl = nullptr;
    QTableWidget *m_logTable = nullptr;
};
