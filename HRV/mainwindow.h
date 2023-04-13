#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QListWidget>
#include <QGraphicsView>
#include <QTime>
#include <QStatusBar>
#include <QVector>
#include <QtGlobal>
#include <QTimer>
#include "menu.h"
#include <QDateTime>

struct storedData{
    int year = 2023;
    int month = 04;
    int day = 1;
    QString time;
    int sTime = 0;
    int cLevel = 0;
    int inRed = 0;
    int inBlue = 0;
    int inGreen = 0;
    double achScore = 0;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Menu* masterMenu;
    Menu* mainMenuOG;

    Ui::MainWindow *ui;
    QListWidget *activeQListWidget;

    void updateMenu(const QString, const QStringList);
    void initializeMainMenu(Menu*);

    double batteryLvl;
    int batWarningStatus = 0;
    int currentTimerCount;
    int maxPowerLevel;
    bool powerStatus;
    QTimer* timer;
    QTimer* pacetimer;
    QTimer* coherencetimer;
    void initializeTimer(QTimer*);
    void initializepaceTimer();
    void initializecoherenceTimer();
    void changePowerStatus();
    void drainBattery();
    void formatHistoryOut(storedData);

    int challengelevel = 1;
    int breathint = 10;
    void sessionTexts(bool);
    void setBreathInt(int);
    void setChallengelevel(int);
    void makePlot(double);
    double xData[101], yData[101];
    void updateCoherenceLabels();
    int numData;
    int progressDirection = 1;

    double yVal;
    double xStep =1;
    //for incoherent data
    QVector<double> xValues;
    QVector<double> yValues;
    //for coherent data
    QVector<double> heartBeat ={60,71,60,80,55,82,65,74,61,79,55,70,65,84,62,78,53,85,62,71,63,77,55,70,65,86};
    //for tracking coherence scores
//    QVector<double> coscoredata;
    int heartpos=0;

    QVector<storedData> history;
    QString historyOut = "";
    bool inSession = false;
    int updatesInRed = 0;
    int updatesInBlue = 0;
    int updatesInGreen = 0;

private slots:
    void updateTimer();
    void navigateDownMenu();
    void navigateUpMenu();
    void navigateSubMenu();
    void navigateToMainMenu();
    void navigateBack();
    void rechargeBattery();
    void changeBatteryLevel(double);
    void powerChange();
    void beginSession();
    void breathpacer();
    void coherenceUpdate();
    //void applySkin();
    void on_ApplySkin_currentTextChanged(const QString &arg1);
};
#endif // MAINWINDOW_H
