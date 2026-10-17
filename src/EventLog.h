#pragma once
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

// Central, timestamped event store. Observers subscribe to entryAdded().
class EventLog : public QObject {
    Q_OBJECT
public:
    enum class Level { Info, Warning, Fault };

    struct Entry {
        QDateTime time;
        Level level;
        QString source;
        QString message;
    };

    explicit EventLog(QObject *parent = nullptr);

    static QString levelName(Level level);

    const QList<Entry> &entries() const { return m_entries; }
    bool exportCsv(const QString &path) const;

public slots:
    void add(EventLog::Level level, const QString &source, const QString &message);
    void clear();

signals:
    void entryAdded(const EventLog::Entry &entry);
    void cleared();

private:
    QList<Entry> m_entries;
};
