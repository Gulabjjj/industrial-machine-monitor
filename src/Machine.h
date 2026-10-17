#pragma once
#include "EventLog.h"

#include <QObject>
#include <QString>

// Simulated industrial machine: start/stop, temperature & speed readings,
// and automatic fault detection (overheat / overspeed).
class Machine : public QObject {
    Q_OBJECT
public:
    enum class State { Stopped, Running, Fault };
    Q_ENUM(State)

    // heatFactor > ~1.25 means the machine will eventually overheat when running.
    Machine(QString name, double ratedSpeedRpm, double heatFactor, QObject *parent = nullptr);

    static constexpr double kAmbientTemp    = 25.0;
    static constexpr double kWarnTemp       = 85.0;
    static constexpr double kFaultTemp      = 95.0;
    static constexpr double kMaxTempScale   = 120.0;
    static constexpr double kOverspeedRatio = 1.15;

    const QString &name() const { return m_name; }
    State state() const { return m_state; }
    double temperature() const { return m_temperature; }
    double speed() const { return m_speed; }
    double ratedSpeed() const { return m_ratedSpeed; }
    double maxSpeed() const { return m_ratedSpeed * kOverspeedRatio; }
    QString lastFault() const { return m_lastFault; }

    double uptimeSeconds() const { return m_uptime; }
    int faultCount() const { return m_faultCount; }
    int startCount() const { return m_startCount; }
    double peakTemperature() const { return m_peakTemp; }

    static QString stateName(State s);

public slots:
    void start();
    void stop();
    void reset();                         // clear a fault -> Stopped
    void triggerFault(const QString &reason = QStringLiteral("Manual fault injection"));
    void tick(double dtSeconds);          // advance the simulation

signals:
    void stateChanged(Machine::State state);
    void readingsChanged();
    void logEvent(EventLog::Level level, const QString &source, const QString &message);

private:
    void setState(State s);
    void raiseFault(const QString &reason);

    QString m_name;
    double m_ratedSpeed;
    double m_heatFactor;

    State m_state = State::Stopped;
    double m_temperature = kAmbientTemp;
    double m_speed = 0.0;
    QString m_lastFault;
    bool m_warnActive = false;

    double m_uptime = 0.0;
    int m_faultCount = 0;
    int m_startCount = 0;
    double m_peakTemp = kAmbientTemp;
};
