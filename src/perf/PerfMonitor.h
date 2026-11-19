#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class QQuickWindow;

/**
 * Métricas para comparar versões e estimar o comportamento na central:
 * tempo até o primeiro quadro, quadros por segundo e memória (RAM) do processo.
 */
class PerfMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int fps READ fps NOTIFY updated)
    Q_PROPERTY(double memoryMb READ memoryMb NOTIFY updated)
    Q_PROPERTY(qint64 startupMs READ startupMs NOTIFY updated)

public:
    explicit PerfMonitor(const QElapsedTimer &sinceStart, QObject *parent = nullptr);

    void attach(QQuickWindow *window);

    int fps() const { return m_fps; }
    double memoryMb() const { return m_memoryMb; }
    qint64 startupMs() const { return m_startupMs; }

    /** Memória residente (RSS) do processo em MB, ou -1 se indisponível. */
    static double currentMemoryMb();

signals:
    void updated();

private:
    QElapsedTimer m_sinceStart;
    QTimer m_tick;
    int m_frames = 0;
    int m_fps = 0;
    double m_memoryMb = -1;
    qint64 m_startupMs = -1;
};
