#include "Machine.h"

#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

namespace {
double noise(double amplitude)
{
    return (QRandomGenerator::global()->generateDouble() * 2.0 - 1.0) * amplitude;
}
// First-order approach of `value` toward `target` with time constant tau.
double approach(double value, double target, double dt, double tau)
{
    return value + (target - value) * (1.0 - std::exp(-dt / tau));
}
} // namespace

Machine::Machine(QString name, double ratedSpeedRpm, double heatFactor, QObject *parent)
    : QObject(parent), m_name(std::move(name)), m_ratedSpeed(ratedSpeedRpm), m_heatFactor(heatFactor)
{
}

QString Machine::stateName(State s)
{
    switch (s) {
    case State::Stopped: return QStringLiteral("STOPPED");
    case State::Running: return QStringLiteral("RUNNING");
    case State::Fault:   return QStringLiteral("FAULT");
    }
    return {};
}

void Machine::setState(State s)
{
    if (m_state == s)
        return;
    m_state = s;
    emit stateChanged(s);
}

void Machine::start()
{
    if (m_state == State::Running)
        return;
    if (m_state == State::Fault) {
        emit logEvent(EventLog::Level::Warning, m_name,
                      "Start refused: machine is in FAULT. Reset required.");
        return;
    }
    ++m_startCount;
    setState(State::Running);
    emit logEvent(EventLog::Level::Info, m_name, "Machine started");
}

void Machine::stop()
{
    if (m_state != State::Running)
        return;
    setState(State::Stopped);
    emit logEvent(EventLog::Level::Info, m_name, "Machine stopped");
}

void Machine::reset()
{
    if (m_state != State::Fault)
        return;
    m_lastFault.clear();
    m_warnActive = false;
    setState(State::Stopped);
    emit logEvent(EventLog::Level::Info, m_name, "Fault cleared, machine ready");
}

void Machine::triggerFault(const QString &reason)
{
    if (m_state == State::Fault)
        return;
    raiseFault(reason);
}

void Machine::raiseFault(const QString &reason)
{
    ++m_faultCount;
    m_lastFault = reason;
    setState(State::Fault);
    emit logEvent(EventLog::Level::Fault, m_name, "FAULT: " + reason);
}

void Machine::tick(double dt)
{
    if (m_state == State::Running) {
        m_uptime += dt;

        const double targetSpeed = m_ratedSpeed * (1.0 + noise(0.03));
        m_speed = approach(m_speed, targetSpeed, dt, 1.5);

        // Rare random speed spike -> may trigger an overspeed fault
        if (QRandomGenerator::global()->generateDouble() < 0.004)
            m_speed *= 1.0 + 0.10 + QRandomGenerator::global()->generateDouble() * 0.15;

        const double targetTemp =
            kAmbientTemp + m_heatFactor * 55.0 * (m_speed / m_ratedSpeed) + noise(1.2);
        m_temperature = approach(m_temperature, targetTemp, dt, 6.0);
    } else {
        m_speed = approach(m_speed, 0.0, dt, 1.2);
        if (m_speed < 1.0)
            m_speed = 0.0;
        m_temperature = approach(m_temperature, kAmbientTemp, dt, 10.0);
    }

    m_peakTemp = std::max(m_peakTemp, m_temperature);

    // ---- Automated fault detection ----
    if (m_state == State::Running) {
        if (m_temperature >= kFaultTemp) {
            raiseFault(QString("Overheat detected (%1 °C)").arg(m_temperature, 0, 'f', 1));
        } else if (m_speed > maxSpeed()) {
            raiseFault(QString("Overspeed detected (%1 RPM)").arg(m_speed, 0, 'f', 0));
        } else if (m_temperature >= kWarnTemp && !m_warnActive) {
            m_warnActive = true;
            emit logEvent(EventLog::Level::Warning, m_name,
                          QString("High temperature warning (%1 °C)").arg(m_temperature, 0, 'f', 1));
        } else if (m_temperature < kWarnTemp - 5.0) {
            m_warnActive = false;
        }
    }

    emit readingsChanged();
}
