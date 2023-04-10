#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize timer counter
    currentTimerCount = -1;
    ui->CoherenceText->setVisible(false);
    ui->LengthText->setVisible(false);
    ui->Achievement->setVisible(false);
    // Create menu tree
    masterMenu = new Menu("MAIN MENU", {"START A NEW SESSION","SETTINGS","HISTORY"}, nullptr);
    mainMenuOG = masterMenu;
    initializeMainMenu(masterMenu);

    // Initialize the main menu view
    activeQListWidget = ui->mainMenuListView;
    activeQListWidget->addItems(masterMenu->getMenuItems());
    activeQListWidget->setCurrentRow(0);
    ui->menuLabel->setText(masterMenu->getName());

    ui->challengeLabel->setText("Challenge Level: " + QString::number(challengelevel));
    ui->breathintLabel->setText("Breath Interval: " + QString::number(breathint));

    // Account for device being "off" on sim start
    powerStatus = false;
    changePowerStatus();
    connect(ui->powerButton, &QPushButton::released, this, &MainWindow::powerChange);

    // charge button connections
    connect(ui->chargeAdminButton, &QPushButton::released, this, &MainWindow::rechargeBattery);

    // Battery level spin box connections
    connect(ui->batteryLevelAdminSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::changeBatteryLevel);

    // device interface button connections
    connect(ui->upButton, &QPushButton::pressed, this, &MainWindow::navigateUpMenu);
    connect(ui->downButton, &QPushButton::pressed, this, &MainWindow::navigateDownMenu);
    connect(ui->okButton, &QPushButton::pressed, this, &MainWindow::navigateSubMenu);
    connect(ui->backButton, &QPushButton::pressed, this, &MainWindow::navigateBack);
    connect(ui->menuButton, &QPushButton::pressed, this, &MainWindow::navigateToMainMenu);

    // Initialize battery levels
    ui->batteryLevelAdminSpinBox->setValue(batteryLvl);
    currentTimerCount = 0;
    // initialize the timer
    timer = new QTimer(this);
    batteryLvl = 100;
    ui->electrodeLabel_2->setVisible(true);
    ui->electrodeLabel->setVisible(false);
    initializeTimer(timer);

    pacetimer = new QTimer(this);//needed to init here otherwise wouldnt be able to check isActive()

    coherencetimer = new QTimer(this);//needed to init here otherwise wouldnt be able to check isActive()
}

MainWindow::~MainWindow()
{
    xValues.clear();
    yValues.clear();
    delete ui;
}
void MainWindow::sessionTexts(bool a){
    ui->electrodeLabel->setVisible(a);
    ui->electrodeLabel_2->setVisible(!a);
    ui->CoherenceText->setVisible(a);
    ui->LengthText->setVisible(a);
    ui->Achievement->setVisible(a);
    ui->CoherenceTextValue->setVisible(a);
    ui->LengthTextValue->setVisible(a);
    ui->AchievementValue->setVisible(a);
    ui->coherenceLevel->setVisible(a);
    ui->coherenceLevel->setReadOnly(true);
}

void MainWindow::makePlot(double elapsedTime)
{
    numData = 0;
    //xData[0] = 0;
    //yData[0] = 60;
    // create graph and assign data to it:
    ui->customPlot->addGraph();
    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("Time");
    ui->customPlot->yAxis->setLabel("HeartRate");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(0, elapsedTime + 30);
    ui->customPlot->yAxis->setRange(50, 100);
    ui->customPlot->replot();
}

void MainWindow::initializeMainMenu(Menu* m) {

    Menu* session = new Menu("START A NEW SESSION", {"YES","NO"}, m);
    Menu* settings = new Menu("SETTINGS", {"CHALLENGE LEVEL","BREATH INTERVAL"}, m);
    Menu* history = new Menu("HISTORY", {"VIEW","CLEAR"}, m);

    m->addChildMenu(session);
    m->addChildMenu(settings);
    m->addChildMenu(history);


    Menu* challengeSetting = new Menu("CHALLENGE LEVEL",{"1","2","3","4"}, settings);
    Menu* breathSetting = new Menu("BREATH INTERVAL",{"5","10","15","20","25","30"}, settings);
    settings->addChildMenu(challengeSetting);
    settings->addChildMenu(breathSetting);



    Menu* viewHistory = new Menu("VIEW",{}, history);
    Menu* clearHistory = new Menu("CLEAR", {"YES","NO"}, history);
    history->addChildMenu(viewHistory);
    history->addChildMenu(clearHistory);
}

void MainWindow::initializeTimer(QTimer* t) {

    connect(t, &QTimer::timeout, this, &MainWindow::updateTimer);

}

void MainWindow::navigateUpMenu() {

    int nextIndex = activeQListWidget->currentRow() - 1;

    if (nextIndex < 0) {
        nextIndex = activeQListWidget->count() - 1;
    }

    activeQListWidget->setCurrentRow(nextIndex);
}

void MainWindow::navigateDownMenu() {

    int nextIndex = activeQListWidget->currentRow() + 1;

    if (nextIndex > activeQListWidget->count() - 1) {
        nextIndex = 0;
    }

    activeQListWidget->setCurrentRow(nextIndex);
}

void MainWindow::beginSession() {

    //ui->electrodeLabel->setVisible(true);
    //ui->electrodeLabel_2->setVisible(false);
    //currentTherapy = t;
    ui->programViewWidget->setVisible(true);
    sessionTexts(true);
    //Timer
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->SessionView->setScene(scene);
    //start a new timer
    //currentTimerCount = t->getTime();
    //timeString = QString::number(currentTimerCount/60) + ":00";
    scene->addText(QString::fromStdString("Session IP"));
    //new timer
    //initializeTimer(t->getTimer());

    //Set up recording object for this therapy
    //Note: The name and start time of the recording is set properly here, the power level and duration will be saved at the end.
    //Record* newR = new Record(t->getName(), QDateTime::currentDateTime(), 0, 0);
    //recordings.append(newR);

    //Power Buttons Enabled
    ui->rightButton->blockSignals(false);
    ui->leftButton->blockSignals(false);




    //starts breath pacer animation, stop the timer
    int breathcalc = breathint * 10;
    int paceint = (breathcalc)/ 2;//this is 10s interval is 1 breath
    pacetimer->setInterval(paceint);
    pacetimer->start();

    initializepaceTimer();
    ui->breathpacer->setValue(0);

    //starts coherence updates
    coherencetimer->setInterval(1000);
    coherencetimer->start();

    initializecoherenceTimer();
    makePlot(0);
}

void MainWindow::navigateSubMenu() {

    int index = activeQListWidget->currentRow();
    if (index < 0) return;

    // Prevent crash if ok button is selected in view
    if (masterMenu->getName() == "VIEW") {
        return;
    }

    //Logic for when the menu is the delete menu.
    if (masterMenu->getName() == "CLEAR") {
        if (masterMenu->getMenuItems()[index] == "YES") {
            //delete recordings
            navigateBack();
            return;
        }
        else {
            navigateBack();
            return;
        }
    }

    if (masterMenu->getName() == "START A NEW SESSION") {
        if (masterMenu->getMenuItems()[index] == "YES") {
            //start a session here
            MainWindow::updateMenu("Session", {});
            ui->mainMenuListView->setVisible(false);
            MainWindow::beginSession();
            return;
        }
        else {//NO selected, and go back
            navigateBack();
            return;
        }
    }
    if (masterMenu->getName() == "CHALLENGE LEVEL") {

        if (masterMenu->getMenuItems()[index] == "YES") {
            //edit challenge level here
            navigateBack();
            return;
        }
        else {//NO selected, and go back, a challenge level is selected
            //make changes to challenge level member var
            challengelevel = masterMenu->getMenuItems()[index].toInt();
            QString challengeleveltext = masterMenu->getMenuItems()[index];

            ui->challengeLabel->setText("Challenge Level: " + challengeleveltext);
            navigateBack();
            return;
        }
    }

    if (masterMenu->getName() == "BREATH INTERVAL") {
        if (masterMenu->getMenuItems()[index] == "YES") {
            //edit challenge level here
            navigateBack();
            return;
        }
        else {//NO selected, and go back,a breath interval is selected
            breathint = masterMenu->getMenuItems()[index].toInt();
            QString breathinttext = masterMenu->getMenuItems()[index];

            ui->breathintLabel->setText("Breath Interval: " + breathinttext);
            navigateBack();
            return;
        }
    }


    //If the menu is a parent and clicking on it should display more menus.//prob not needed
    if (masterMenu->get(index)->getMenuItems().length() > 0) {
        masterMenu = masterMenu->get(index);
        MainWindow::updateMenu(masterMenu->getName(), masterMenu->getMenuItems());
    }

}


void MainWindow::updateMenu(const QString selectedMenuItem, const QStringList menuItems) {

    activeQListWidget->clear();
    activeQListWidget->addItems(menuItems);
    activeQListWidget->setCurrentRow(0);

    ui->menuLabel->setText(selectedMenuItem);
}


void MainWindow::navigateBack() {

    ui->rightButton->blockSignals(true);
    ui->leftButton->blockSignals(true);
    ui->mainMenuListView->setVisible(true);
    //stop session
    sessionTexts(false);
    currentTimerCount = -1;
    coherencetimer->stop();
    coherencetimer->disconnect();
    xValues.clear();
    yValues.clear();
    numData=0;

    if(pacetimer->isActive()){
        pacetimer->stop();//fixes crashing when changing breath int without a session started previously.
        pacetimer->disconnect();
    }

    ui->CoherenceTextValue->setNum(0.0);//reset score values
    ui->AchievementValue->setNum(0.0);
    //save session

    if (masterMenu->getName() == "MAIN MENU") {
        activeQListWidget->setCurrentRow(0);
    }
    else {
        masterMenu = masterMenu->getParent();
        updateMenu(masterMenu->getName(), masterMenu->getMenuItems());
    }

//    ui->programViewWidget->setVisible(false); // dk why this is done, this shutsdown the menu screen
//    ui->electrodeLabel->setVisible(false);
}

void MainWindow::navigateToMainMenu() {
    ui->mainMenuListView->setVisible(true);
    while (masterMenu->getName() != "MAIN MENU") {
        masterMenu = masterMenu->getParent();
    }
    sessionTexts(false);
    updateMenu(masterMenu->getName(), masterMenu->getMenuItems());
//    ui->programViewWidget->setVisible(false);
//    ui->electrodeLabel->setVisible(false);
    currentTimerCount = -1;
    coherencetimer->stop();
    coherencetimer->disconnect();
    if(pacetimer->isActive()){
        pacetimer->stop();//fixes crashing when changing breath int without a session started previously.
        pacetimer->disconnect();
    }

    ui->CoherenceTextValue->setNum(0.0);//reset score values
    ui->AchievementValue->setNum(0.0);
    numData=0;
    xValues.clear();
    yValues.clear();
}

void MainWindow::changePowerStatus() {

    activeQListWidget->setVisible(powerStatus);
    ui->menuLabel->setVisible(powerStatus);
    ui->statusBarQFrame->setVisible(powerStatus);
    ui->SessionView->setVisible(powerStatus);
    ui->challengeLabel->setVisible(powerStatus);
    ui->breathintLabel->setVisible(powerStatus);
    ui->programViewWidget->setVisible(powerStatus);

    //Remove this if we want the menu to stay in the same position when the power is off
    //Remove this if we want the data to remain same after off is pressed for challengeLevel and BreathInt
    if (powerStatus) {
        challengelevel=1;
        breathint=10;
        QString challengeleveltext = "1";
        QString breathinttext = "10";
        ui->breathintLabel->setText("Breath Interval: " + breathinttext);
        ui->challengeLabel->setText("Challenge Level: " + challengeleveltext);
        MainWindow::navigateToMainMenu();
        currentTimerCount = -1;
        coherencetimer->stop();
        coherencetimer->disconnect();
        if(pacetimer->isActive()){
            pacetimer->stop();//fixes crashing when changing breath int without a session started previously.
            pacetimer->disconnect();
        }

        ui->CoherenceTextValue->setNum(0.0);//reset score values
        ui->AchievementValue->setNum(0.0);
        xValues.clear();
        yValues.clear();
        numData=0;
    }

    ui->upButton->setEnabled(powerStatus);
    ui->downButton->setEnabled(powerStatus);
    ui->leftButton->setEnabled(powerStatus);
    ui->rightButton->setEnabled(powerStatus);
    ui->menuButton->setEnabled(powerStatus);
    ui->okButton->setEnabled(powerStatus);
    ui->backButton->setEnabled(powerStatus);


}

void MainWindow::powerChange() {

    if(!powerStatus){
        if(batteryLvl!=0){
            powerStatus  = !powerStatus;
            changePowerStatus();
            timer->start(1000);
        }
    }else{
        powerStatus  = !powerStatus;
        changePowerStatus();
        timer->stop();
    }

}

void MainWindow::rechargeBattery() {

    int batteryLevel = 100.0;
    changeBatteryLevel(batteryLevel);
}

void MainWindow::updateTimer() {

    drainBattery();
}

void MainWindow::changeBatteryLevel(double newLevel) {

    if (newLevel >= 0.0 && newLevel <= 100.0) {
        if (newLevel == 0.0 && powerStatus == true) {
            powerChange();
            batteryLvl = 0;
            //ui->powerButton->setDisabled(true);
        }else{
            batteryLvl = newLevel;
        }

        ui->batteryLevelAdminSpinBox->setValue(newLevel);
        int newLevelInt = int(newLevel);
        ui->batteryLevelBar->setValue(newLevelInt);

        QString highBatteryHealth = "QProgressBar { selection-background-color: rgb(78, 154, 6); background-color: rgb(0, 0, 0); }";
        QString mediumBatteryHealth = "QProgressBar { selection-background-color: rgb(196, 160, 0); background-color: rgb(0, 0, 0); }";
        QString lowBatteryHealth = "QProgressBar { selection-background-color: rgb(164, 0, 0); background-color: rgb(0, 0, 0); }";

        if (newLevelInt >= 50) {
            ui->batteryLevelBar->setStyleSheet(highBatteryHealth);
        }
        else if (newLevelInt >= 20) {
            ui->batteryLevelBar->setStyleSheet(mediumBatteryHealth);
        }
        else {
            ui->batteryLevelBar->setStyleSheet(lowBatteryHealth);
        }
    }
}

void MainWindow::drainBattery() {

    //1000 constant because 15 minutes is the longest therapy and we feel as it should last at least 15 minutes at full power
    double batteryLevel = batteryLvl - 0.05;

    changeBatteryLevel(batteryLevel);
    currentTimerCount++;
}

void MainWindow::initializepaceTimer() {

    connect(pacetimer, &QTimer::timeout, this, &MainWindow::breathpacer);

}

void MainWindow::initializecoherenceTimer() {

    connect(coherencetimer, &QTimer::timeout, this, &MainWindow::coherenceUpdate);

}

void MainWindow::breathpacer(){
    int value = ui->breathpacer->value() + progressDirection;

    if (value >= 100){

        progressDirection = -1;
        value = 100;

    }else if (value <= 0){
        progressDirection = 1;
        value = 0;
    }
    ui->breathpacer->setValue(value);
}

//void MainWindow::coherenceUpdate()
//{
//    if((int(currentTimerCount) % 30) == 0){
//        ui->customPlot->xAxis->setRange(0, currentTimerCount + 30);
//    }
//    numData++;
//    xData[numData] = currentTimerCount;

//    double r =( (rand() % 5));
//    yData[numData] = yData[numData-1] + r;
//    if (yData[numData] < 0){
//        yData[numData] = yData[numData] + 4;
//    } else if (yData[numData] > 16){
//        yData[numData] = yData[numData] - 4;
//    }

//    // generate some data:
//    QVector<double> x(numData), y(numData); // initialize with entries 0..100
//    for (int i=0; i<numData; ++i)
//    {
//      x[i] = i/50.0; // x goes from -1 to 1
//      //printf("%0.2f %0.2f",xData[i], yData[i]);
//      //y[i] = yData[i]; // let's plot a quadratic function
//      y[i] = yData[i];
//    }

//    ui->customPlot->graph(0)->setData(x, y);
//    ui->customPlot->replot();
//}

void MainWindow::coherenceUpdate(){
    updateCoherenceLabels();//updates score, length of session, achievment score
    if((currentTimerCount % 30) == 0){
        ui->customPlot->xAxis->setRange(0, currentTimerCount + 30);
    }
    if((currentTimerCount>64)){
        xValues.pop_front();
        yValues.pop_front();
        ui->customPlot->xAxis->setRange(++numData, currentTimerCount + 20);
    }
    xValues.append(xValues.isEmpty() ? 0 : xValues.last() + xStep);
    yVal =QRandomGenerator::global()->bounded(50,100);
    yValues.append(yVal);
    ui->customPlot->graph(0)->setData(xValues, yValues);
    //ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
}

void MainWindow::updateCoherenceLabels(){
    if(currentTimerCount % 5 == 0){//checks if its every 5 seconds, to update the scores
        srand(time(0));
        double coscore = std::round(((static_cast<double>(rand()) / RAND_MAX) * 16.0) * 10) / 10;//random coherence score as data
        double currachscore = ui->AchievementValue->text().toDouble();

        ui->CoherenceTextValue->setNum(coscore);
        ui->AchievementValue->setNum(currachscore + coscore);
    }


    QString timeString = QString::number(currentTimerCount/60) + ((currentTimerCount%60 < 10) ? + ":0" + QString::number(currentTimerCount%60) : + ":" + QString::number(currentTimerCount%60));
    ui->LengthTextValue->setText(timeString);
}
