#pragma once
#include "Machine.h"

#include <QFrame>

class QLabel;
class QProgressBar;
class QPushButton;

// Dashboard card for a single machine. Reacts purely to Machine signals.
class MachineWidget : public QFrame {
    Q_OBJECT
public:
    explicit MachineWidget(Machine *machine, QWidget *parent = nullptr);

private slots:
    void refresh();
    void onToggleClicked();

private:
    Machine *m_machine;
    QLabel *m_title;
    QLabel *m_status;
    QLabel *m_info;
    QProgressBar *m_tempBar;
    QProgressBar *m_speedBar;
    QPushButton *m_toggleBtn;
    QPushButton *m_resetBtn;
    QPushButton *m_faultBtn;
};
