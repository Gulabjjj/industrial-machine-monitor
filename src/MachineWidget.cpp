#include "MachineWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
QString barStyle(const QString &color)
{
    return QString("QProgressBar{border:1px solid #3a4150;border-radius:4px;background:#171b24;"
                   "color:#e6e9ef;text-align:center;height:18px;}"
                   "QProgressBar::chunk{background:%1;border-radius:3px;}")
        .arg(color);
}
QString formatUptime(double s)
{
    const int t = static_cast<int>(s);
    return QString("%1:%2:%3")
        .arg(t / 3600, 2, 10, QChar('0'))
        .arg((t / 60) % 60, 2, 10, QChar('0'))
        .arg(t % 60, 2, 10, QChar('0'));
}
} // namespace

MachineWidget::MachineWidget(Machine *machine, QWidget *parent)
    : QFrame(parent), m_machine(machine)
{
    setObjectName("machineCard");
    setStyleSheet("#machineCard{background:#232834;border:1px solid #3a4150;border-radius:8px;}"
                  "QLabel{color:#e6e9ef;}");

    m_title = new QLabel(machine->name());
    m_title->setStyleSheet("font-size:15px;font-weight:bold;");
    m_status = new QLabel;
    m_status->setAlignment(Qt::AlignCenter);
    m_status->setMinimumWidth(90);

    m_tempBar = new QProgressBar;
    m_tempBar->setRange(0, static_cast<int>(Machine::kMaxTempScale));
    m_tempBar->setFormat("%v °C");
    m_speedBar = new QProgressBar;
    m_speedBar->setRange(0, static_cast<int>(machine->maxSpeed() * 1.1));
    m_speedBar->setFormat("%v RPM");

    m_info = new QLabel;
    m_info->setStyleSheet("color:#9aa3b5;font-size:11px;");
    m_info->setWordWrap(true);

    m_toggleBtn = new QPushButton("Start");
    m_resetBtn = new QPushButton("Reset");
    m_faultBtn = new QPushButton("Inject Fault");

    auto *head = new QHBoxLayout;
    head->addWidget(m_title);
    head->addStretch();
    head->addWidget(m_status);

    auto *btns = new QHBoxLayout;
    btns->addWidget(m_toggleBtn);
    btns->addWidget(m_resetBtn);
    btns->addWidget(m_faultBtn);

    auto *lay = new QVBoxLayout(this);
    lay->addLayout(head);
    lay->addWidget(new QLabel("Temperature"));
    lay->addWidget(m_tempBar);
    lay->addWidget(new QLabel("Speed"));
    lay->addWidget(m_speedBar);
    lay->addWidget(m_info);
    lay->addLayout(btns);

    connect(m_toggleBtn, &QPushButton::clicked, this, &MachineWidget::onToggleClicked);
    connect(m_resetBtn, &QPushButton::clicked, m_machine, &Machine::reset);
    connect(m_faultBtn, &QPushButton::clicked, m_machine, [m = m_machine] { m->triggerFault(); });
    connect(m_machine, &Machine::readingsChanged, this, &MachineWidget::refresh);
    connect(m_machine, &Machine::stateChanged, this, &MachineWidget::refresh);

    refresh();
}

void MachineWidget::onToggleClicked()
{
    if (m_machine->state() == Machine::State::Running)
        m_machine->stop();
    else
        m_machine->start();
}

void MachineWidget::refresh()
{
    const auto st = m_machine->state();
    QString color;
    switch (st) {
    case Machine::State::Running: color = "#2ea043"; break;
    case Machine::State::Stopped: color = "#6e7681"; break;
    case Machine::State::Fault:   color = "#da3633"; break;
    }
    m_status->setText(Machine::stateName(st));
    m_status->setStyleSheet(QString("background:%1;color:white;font-weight:bold;"
                                    "border-radius:4px;padding:3px 8px;")
                                .arg(color));

    const double t = m_machine->temperature();
    m_tempBar->setValue(static_cast<int>(t));
    m_tempBar->setStyleSheet(barStyle(t >= Machine::kFaultTemp ? "#da3633"
                                      : t >= Machine::kWarnTemp ? "#d29922" : "#2f81f7"));
    m_speedBar->setValue(static_cast<int>(m_machine->speed()));
    m_speedBar->setStyleSheet(
        barStyle(m_machine->speed() > m_machine->maxSpeed() ? "#da3633" : "#8957e5"));

    QString info = QString("Uptime %1  |  Starts %2  |  Faults %3  |  Peak %4 °C")
                       .arg(formatUptime(m_machine->uptimeSeconds()))
                       .arg(m_machine->startCount())
                       .arg(m_machine->faultCount())
                       .arg(m_machine->peakTemperature(), 0, 'f', 1);
    if (st == Machine::State::Fault)
        info += "\nLast fault: " + m_machine->lastFault();
    m_info->setText(info);

    m_toggleBtn->setText(st == Machine::State::Running ? "Stop" : "Start");
    m_toggleBtn->setEnabled(st != Machine::State::Fault);
    m_resetBtn->setEnabled(st == Machine::State::Fault);
    m_faultBtn->setEnabled(st != Machine::State::Fault);
}
