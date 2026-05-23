#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QFrame>
#include <QVector>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateData();

private:
    void setupUI();
    void setupChart();
    void setupDashboard(QVBoxLayout *dashLayout);
    QFrame *createDashboardCard(const QString &title, QLabel *&valueLabel,
                                const QString &unit, const QString &icon);

    QChartView *m_chartView = nullptr;
    QChart *m_chart = nullptr;
    QLineSeries *m_snrSeries = nullptr;
    QLineSeries *m_berSeries = nullptr;
    QValueAxis *m_snrAxis = nullptr;
    QValueAxis *m_berAxis = nullptr;
    QDateTimeAxis *m_timeAxis = nullptr;

    QTimer *m_timer = nullptr;
    QElapsedTimer m_elapsed;
    qint64 m_startTime = 0;

    QLabel *m_freqLabel = nullptr;
    QLabel *m_snrLabel = nullptr;
    QLabel *m_lossLabel = nullptr;

    static constexpr int WINDOW_SECS = 60;
    static constexpr int INTERVAL_MS = 100;
    static constexpr int MAX_POINTS = (WINDOW_SECS * 1000) / INTERVAL_MS;
};

#endif // MAINWINDOW_H
