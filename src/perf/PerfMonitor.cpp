#include "perf/PerfMonitor.h"

#include <QFile>
#include <QQuickWindow>
#include <QTextStream>
#include <QtDebug>

#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <psapi.h>
#endif

PerfMonitor::PerfMonitor(const QElapsedTimer &sinceStart, QObject *parent)
    : QObject(parent), m_sinceStart(sinceStart)
{
    m_tick.setInterval(1000);
    connect(&m_tick, &QTimer::timeout, this, [this] {
        m_fps = m_frames;
        m_frames = 0;
        m_memoryMb = currentMemoryMb();
        emit updated();
    });
    m_tick.start();
}

void PerfMonitor::attach(QQuickWindow *window)
{
    connect(window, &QQuickWindow::frameSwapped, this, [this] {
        ++m_frames;
        if (m_startupMs < 0) {
            m_startupMs = m_sinceStart.elapsed();
            qInfo().noquote() << "Primeiro quadro em" << m_startupMs << "ms";
            emit updated();
        }
    });
}

double PerfMonitor::currentMemoryMb()
{
#if defined(Q_OS_LINUX)
    QFile f(QStringLiteral("/proc/self/status"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;
    QTextStream in(&f);
    for (QString line = in.readLine(); !line.isNull(); line = in.readLine()) {
        if (line.startsWith(QLatin1String("VmRSS:"))) // "VmRSS:   123456 kB"
            return line.section(QLatin1Char(' '), 1, -1, QString::SectionSkipEmpty).section(QLatin1Char(' '), 0, 0).toDouble() / 1024.0;
    }
    return -1;
#elif defined(Q_OS_WIN)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        return double(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    return -1;
#else
    return -1;
#endif
}
