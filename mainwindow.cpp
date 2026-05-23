#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QtMath>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <cmath>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_elapsed.start();
    m_timer = new QTimer(this);
    m_timer->setInterval(INTERVAL_MS);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateData);
    m_timer->start();
}

MainWindow::~MainWindow() {}

static double qFunc(double x)
{
    return 0.5 * std::erfc(x / std::sqrt(2.0));
}

void MainWindow::updateData()
{
    double elapsedSec = m_elapsed.elapsed() / 1000.0;

    // SNR: 10–30 dB with sinusoidal fading + noise
    double baseSNR = 20.0;
    double fade = 8.0 * std::sin(2.0 * M_PI * elapsedSec / 12.0)
                + 3.0 * std::sin(2.0 * M_PI * elapsedSec / 3.7)
                + 2.0 * std::sin(2.0 * M_PI * elapsedSec / 1.8);
    double noise = 1.5 * (QRandomGenerator::global()->generateDouble() - 0.5);
    double snr = qBound(10.0, baseSNR + fade + noise, 30.0);

    // BER from SNR: Q(sqrt(2 * SNR_linear)) for BPSK
    double snrLinear = std::pow(10.0, snr / 10.0);
    double ber = qFunc(std::sqrt(2.0 * snrLinear));
    ber = qBound(1e-12, ber, 0.5);

    // Packet loss rate (1500-byte packet): PER = 1 - (1-BER)^(1500*8)
    double per = 1.0 - std::pow(1.0 - ber, 1500.0 * 8.0);
    per = qBound(0.0, per, 1.0);

    qint64 now = QDateTime::currentMSecsSinceEpoch();

    m_snrSeries->append(now, snr);
    m_berSeries->append(now, ber * 100.0);

    // Scroll
    while (m_snrSeries->count() > MAX_POINTS) {
        m_snrSeries->removePoints(0, m_snrSeries->count() - MAX_POINTS);
    }
    while (m_berSeries->count() > MAX_POINTS) {
        m_berSeries->removePoints(0, m_berSeries->count() - MAX_POINTS);
    }

    // Adjust axis range
    if (m_snrSeries->count() > 1) {
        qreal tMin = m_snrSeries->at(0).x();
        qreal tMax = m_snrSeries->at(m_snrSeries->count() - 1).x();
        m_timeAxis->setRange(QDateTime::fromMSecsSinceEpoch(tMin),
                             QDateTime::fromMSecsSinceEpoch(tMax));
    }

    // Dashboard
    QString band = (elapsedSec < 30) ? "Band 41 (2.5 GHz)" : "Band 78 (3.5 GHz)";
    m_freqLabel->setText(band);
    m_snrLabel->setText(QString::number(snr, 'f', 2) + " dB");
    m_lossLabel->setText(QString::number(per * 100.0, 'f', 4) + " %");
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(14);

    // ===== Chart area =====
    setupChart();
    QFrame *chartFrame = new QFrame;
    chartFrame->setObjectName("chartFrame");
    chartFrame->setStyleSheet(R"(
        #chartFrame {
            background-color: #101432;
            border: 1px solid #1e2a5a;
            border-radius: 10px;
        }
    )");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    chartLayout->setContentsMargins(8, 8, 8, 8);
    chartLayout->addWidget(m_chartView);

    mainLayout->addWidget(chartFrame, 4);

    // ===== Dashboard area =====
    QVBoxLayout *dashLayout = new QVBoxLayout;
    dashLayout->setSpacing(12);
    setupDashboard(dashLayout);
    mainLayout->addLayout(dashLayout, 1);
}

void MainWindow::setupChart()
{
    m_chart = new QChart;
    m_chart->setBackgroundBrush(QColor(16, 20, 50));
    m_chart->setBackgroundVisible(true);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setLabelColor(QColor(180, 200, 240));
    m_chart->legend()->setAlignment(Qt::AlignTop);
    m_chart->setMargins(QMargins(0, 0, 0, 0));
    m_chart->setBackgroundRoundness(0);

    // SNR series (left Y axis)
    m_snrSeries = new QLineSeries;
    m_snrSeries->setName("SNR (dB)");
    m_snrSeries->setColor(QColor(0, 230, 180));
    QPen snrPen(QColor(0, 230, 180), 2.0);
    m_snrSeries->setPen(snrPen);

    // BER series (right Y axis)
    m_berSeries = new QLineSeries;
    m_berSeries->setName("BER × 100");
    m_berSeries->setColor(QColor(255, 90, 120));
    QPen berPen(QColor(255, 90, 120), 1.5, Qt::DashLine);
    m_berSeries->setPen(berPen);

    m_chart->addSeries(m_snrSeries);
    m_chart->addSeries(m_berSeries);

    // Time axis (X)
    m_timeAxis = new QDateTimeAxis;
    m_timeAxis->setFormat("hh:mm:ss");
    m_timeAxis->setTitleText("Time");
    m_timeAxis->setTitleBrush(QBrush(QColor(140, 160, 200)));
    m_timeAxis->setLabelsColor(QColor(140, 160, 200));
    m_timeAxis->setGridLineColor(QColor(30, 40, 70));
    m_timeAxis->setLinePenColor(QColor(60, 80, 120));
    m_timeAxis->setRange(QDateTime::currentDateTime(),
                         QDateTime::currentDateTime().addSecs(WINDOW_SECS));
    QFont axisFont;
    axisFont.setPointSize(8);
    m_timeAxis->setLabelsFont(axisFont);

    // SNR Y axis
    m_snrAxis = new QValueAxis;
    m_snrAxis->setTitleText("SNR (dB)");
    m_snrAxis->setTitleBrush(QBrush(QColor(0, 230, 180)));
    m_snrAxis->setLabelsColor(QColor(0, 230, 180));
    m_snrAxis->setGridLineColor(QColor(30, 40, 70));
    m_snrAxis->setLinePenColor(QColor(0, 230, 180));
    m_snrAxis->setRange(5, 35);
    m_snrAxis->setTickCount(7);
    m_snrAxis->setLabelsFont(axisFont);

    // BER Y axis
    m_berAxis = new QValueAxis;
    m_berAxis->setTitleText("BER × 100");
    m_berAxis->setTitleBrush(QBrush(QColor(255, 90, 120)));
    m_berAxis->setLabelsColor(QColor(255, 90, 120));
    m_berAxis->setGridLineVisible(false);
    m_berAxis->setLinePenColor(QColor(255, 90, 120));
    m_berAxis->setRange(0, 50);
    m_berAxis->setTickCount(6);
    m_berAxis->setLabelsFont(axisFont);

    m_chart->addAxis(m_timeAxis, Qt::AlignBottom);
    m_chart->addAxis(m_snrAxis, Qt::AlignLeft);
    m_chart->addAxis(m_berAxis, Qt::AlignRight);

    m_snrSeries->attachAxis(m_timeAxis);
    m_snrSeries->attachAxis(m_snrAxis);
    m_berSeries->attachAxis(m_timeAxis);
    m_berSeries->attachAxis(m_berAxis);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setBackgroundBrush(QBrush(QColor(16, 20, 50)));
}

void MainWindow::setupDashboard(QVBoxLayout *dashLayout)
{
    // Title
    QLabel *title = new QLabel("Dashboard");
    title->setStyleSheet(R"(
        color: #e0e8ff; font-size: 22px; font-weight: bold;
        padding: 8px 0;
        border-bottom: 2px solid #2a3a6a;
        margin-bottom: 4px;
    )");
    dashLayout->addWidget(title);

    // Cards
    QLabel *freqVal, *snrVal, *lossVal;
    QFrame *freqCard = createDashboardCard("Current Band", freqVal, "", "📶");
    QFrame *snrCard  = createDashboardCard("Current SNR", snrVal, "dB", "📊");
    QFrame *lossCard = createDashboardCard("Packet Loss Rate", lossVal, "%", "📦");

    m_freqLabel = freqVal;
    m_snrLabel  = snrVal;
    m_lossLabel = lossVal;

    dashLayout->addWidget(freqCard);
    dashLayout->addWidget(snrCard);
    dashLayout->addWidget(lossCard);
    dashLayout->addStretch();

    // Status indicator
    QLabel *statusLabel = new QLabel("● Live Monitoring");
    statusLabel->setStyleSheet(R"(
        color: #00e6b4; font-size: 13px; font-weight: bold;
        padding: 6px 0;
    )");
    dashLayout->addWidget(statusLabel);
}

QFrame *MainWindow::createDashboardCard(const QString &title, QLabel *&valueLabel,
                                         const QString &unit, const QString &icon)
{
    QFrame *card = new QFrame;
    card->setObjectName("dashCard");
    card->setStyleSheet(R"(
        #dashCard {
            background-color: #161c42;
            border: 1px solid #263060;
            border-radius: 8px;
            padding: 14px;
        }
    )");
    card->setMinimumHeight(100);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(6);

    QLabel *titleLbl = new QLabel(icon + "  " + title);
    titleLbl->setStyleSheet("color: #8899cc; font-size: 12px; border: none; background: transparent;");

    valueLabel = new QLabel("--");
    valueLabel->setStyleSheet(R"(
        color: #e0e8ff; font-size: 26px; font-weight: bold;
        border: none; background: transparent;
    )");

    QLabel *unitLbl = new QLabel(unit);
    unitLbl->setStyleSheet("color: #6678aa; font-size: 12px; border: none; background: transparent;");

    layout->addWidget(titleLbl);
    layout->addWidget(valueLabel);
    layout->addWidget(unitLbl);

    return card;
}
