#include "EventLog.h"

#include <QFile>
#include <QTextStream>

EventLog::EventLog(QObject *parent) : QObject(parent) {}

QString EventLog::levelName(Level level)
{
    switch (level) {
    case Level::Info:    return QStringLiteral("INFO");
    case Level::Warning: return QStringLiteral("WARNING");
    case Level::Fault:   return QStringLiteral("FAULT");
    }
    return {};
}

void EventLog::add(Level level, const QString &source, const QString &message)
{
    Entry e{QDateTime::currentDateTime(), level, source, message};
    m_entries.append(e);
    emit entryAdded(e);
}

void EventLog::clear()
{
    m_entries.clear();
    emit cleared();
}

bool EventLog::exportCsv(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out << "timestamp,level,source,message\n";
    for (const Entry &e : m_entries) {
        QString msg = e.message;
        msg.replace('"', "\"\"");
        out << e.time.toString(Qt::ISODate) << ',' << levelName(e.level) << ','
            << e.source << ",\"" << msg << "\"\n";
    }
    return true;
}
