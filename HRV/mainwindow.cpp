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
    ui->historyView->setVisible(false);
    ui->lowBatWarningList->setVisible(false);
    ui->lowBatWarningTitle->setVisible(false);
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
    //tester for applying to skin
    //connect(ui->ApplySkin,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::applySkin);
    // Initialize battery levels
    ui->batteryLevelAdminSpinBox->setValue(batteryLvl);
    currentTimerCount = 0;
    // initialize the timer
    timer = new QTimer(this);
    batteryLvl = 100;
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
//    ui->electrodeLabel->setVisible(a);
//    ui->electrodeLabel_2->setVisible(!a);
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

    //currentTherapy = t;
    ui->programViewWidget->setVisible(true);
    sessionTexts(true);
    ui->CoherentLevel->setDisabled(true);
    ui->CoherenceTextValue->setNum(0.0);//reset score values
    ui->AchievementValue->setNum(0.0);
    ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(255,0,0)}");//reset LED light
    //Timer
    currentTimerCount = -1;
    coherencetimer->stop();
    coherencetimer->disconnect();
    if(pacetimer->isActive()){
        pacetimer->stop();//fixes crashing when changing breath int without a session started previously.
        pacetimer->disconnect();
    }

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
    inSession = true;
    updatesInRed = 0;
    updatesInBlue = 0;
    updatesInGreen = 0;
    initializecoherenceTimer();
    makePlot(0);
}

void MainWindow::navigateSubMenu() {

    int index = activeQListWidget->currentRow();
    if (index < 0) return;
    if (activeQListWidget == ui->lowBatWarningList){
        batWarningStatus = 2;
        if (activeQListWidget->currentItem()->text() == "END SESSION"){
            ui->lowBatWarningList->setVisible(false);
            ui->lowBatWarningTitle->setVisible(false);
            activeQListWidget = ui->mainMenuListView;
            navigateBack();
        }else{
            ui->lowBatWarningList->setVisible(false);
            ui->lowBatWarningTitle->setVisible(false);
            activeQListWidget = ui->mainMenuListView;
        }
        return;
    }
    // Prevent crash if ok button is selected in view
    if (masterMenu->getName() == "HISTORY") {
//        printf("in if %d", history.length());
        if (masterMenu->getMenuItems()[index] == "VIEW") {
            ui->historyView->setVisible(true);
            historyOut = "";
            if (history.length() == 0){
                historyOut = "No History";
            }else{
                for (int i = 0; i < history.length(); i++){
                    formatHistoryOut(history[i]);
                }
            }

            ui->historyView->setText(historyOut);
            return;
        }
    }

    //Logic for when the menu is the delete menu.
    if (masterMenu->getName() == "CLEAR") {
        if (masterMenu->getMenuItems()[index] == "YES") {
            //delete recordings
            history.clear();
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
    if(ui->historyView->isVisible()){
        ui->historyView->setVisible(false);
        return;
    }
    ui->rightButton->blockSignals(true);
    ui->leftButton->blockSignals(true);
    ui->mainMenuListView->setVisible(true);
    ui->CoherentLevel->setDisabled(false);
    if (inSession){
        //store data
        storedData newSession;
        newSession.achScore = ui->AchievementValue->text().toDouble();
        newSession.cLevel = challengelevel;
        newSession.sTime = currentTimerCount;
        newSession.inRed = updatesInRed;
        newSession.inBlue = updatesInBlue;
        newSession.inGreen = updatesInGreen;
        QDate date = QDateTime::currentDateTime().date();
        QTime time = QDateTime::currentDateTime().time();
        newSession.year = date.year();
        newSession.month = date.month();
        newSession.day = date.day();
        newSession.time = time.toString();
        history.push_back(newSession);
        historyOut = "";
        formatHistoryOut(newSession);
        ui->historyView->setText(historyOut);
        ui->historyView->setVisible(true);
        inSession = false;
    }
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
}

void MainWindow::navigateToMainMenu() {
    ui->mainMenuListView->setVisible(true);
    ui->CoherentLevel->setDisabled(false);
    ui->historyView->setVisible(false);
    if (inSession){
        //store data
        storedData newSession;
        newSession.achScore = ui->AchievementValue->text().toDouble();
        newSession.cLevel = challengelevel;
        newSession.sTime = currentTimerCount;
        newSession.inRed = updatesInRed;
        newSession.inBlue = updatesInBlue;
        newSession.inGreen = updatesInGreen;
        QDate date = QDateTime::currentDateTime().date();
        QTime time = QDateTime::currentDateTime().time();
        newSession.year = date.year();
        newSession.month = date.month();
        newSession.day = date.day();
        newSession.time = time.toString();
        history.push_back(newSession);
        historyOut = "";
        formatHistoryOut(newSession);
        ui->historyView->setText(historyOut);
        ui->historyView->setVisible(true);
        inSession = false;
    }
    while (masterMenu->getName() != "MAIN MENU") {
        masterMenu = masterMenu->getParent();
    }
    sessionTexts(false);
    updateMenu(masterMenu->getName(), masterMenu->getMenuItems());
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
    ui->CoherentLevel->setDisabled(false);
    ui->historyView->setVisible(false);
    if (inSession){
        //store data
        storedData newSession;
        newSession.achScore = ui->AchievementValue->text().toDouble();
        newSession.cLevel = challengelevel;
        newSession.inRed = updatesInRed;
        newSession.inBlue = updatesInBlue;
        newSession.inGreen = updatesInGreen;
        newSession.sTime = currentTimerCount;
        QDate date = QDateTime::currentDateTime().date();
        QTime time = QDateTime::currentDateTime().time();
        newSession.year = date.year();
        newSession.month = date.month();
        newSession.day = date.day();
        newSession.time = time.toString();
        history.push_back(newSession);
        historyOut = "";
        formatHistoryOut(newSession);
        ui->historyView->setText(historyOut);
        ui->historyView->setVisible(true);
        inSession = false;
    }
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
    batWarningStatus = 0;
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
            //fixes bug of mainmenu appearing in low battery alert, if alert continue wasnt selected
            ui->lowBatWarningList->setVisible(false);
            ui->lowBatWarningTitle->setVisible(false);
            activeQListWidget = ui->mainMenuListView;
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
    if (batteryLevel < 20 && batWarningStatus == 0){
        batWarningStatus = 1;
        ui->lowBatWarningList->setVisible(true);
        ui->lowBatWarningTitle->setVisible(true);
        ui->lowBatWarningList->clear();
        activeQListWidget = ui->lowBatWarningList;
        if(inSession){
            activeQListWidget->addItem("CONTINUE SESSION");
            activeQListWidget->addItem("END SESSION");
        }
        else{
            activeQListWidget->addItem("CONTINUE");
        }
        activeQListWidget->setCurrentRow(0);

    }
    changeBatteryLevel(batteryLevel);
    currentTimerCount++;
}

void MainWindow::initializepaceTimer() {

    connect(pacetimer, &QTimer::timeout, this, &MainWindow::breathpacer);

}

void MainWindow::initializecoherenceTimer() {
    if(ui->CoherentLevel->currentText()!="Incoherent"){
        xValues.clear();
        yValues.clear();

    }
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

void MainWindow::coherenceUpdate(){
    if(ui->ApplySkin->currentText()=="TRUE"){
        if(!coherencetimer->isActive()){
            coherencetimer->start();
        }
    if((currentTimerCount % 30) == 0){
        ui->customPlot->xAxis->setRange(0, currentTimerCount + 30);
    }
    if(ui->CoherentLevel->currentText()=="Incoherent"){
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
    }else{
        if(heartpos==heartBeat.size()){
            heartpos=0;
        }
        if((currentTimerCount>64)){
            xValues.pop_front();
            yValues.pop_front();
            ui->customPlot->xAxis->setRange(++numData, currentTimerCount + 20);
        }
        xValues.append(xValues.isEmpty() ? 0 : xValues.last() + xStep);
        if(heartBeat[heartpos]>70){
            yValues.append(heartBeat[heartpos]+QRandomGenerator::global()->bounded(1,5));
        }
        else{
            yValues.append(heartBeat[heartpos]-QRandomGenerator::global()->bounded(1,5));
        }
        heartpos++;
        ui->customPlot->graph(0)->setData(xValues, yValues);
        //ui->customPlot->rescaleAxes();
        ui->customPlot->replot();
    }
    updateCoherenceLabels();//updates score, length of session, achievment score
    }
    else{
        navigateBack();
    }
}

void MainWindow::updateCoherenceLabels(){
    if(currentTimerCount % 5 == 0 && currentTimerCount!=0){//checks if its every 5 seconds, to update the scores
        srand(time(0));
        double coscore = std::round(((static_cast<double>(rand()) / RAND_MAX) * 7.0) * 10) / 10;//random coherence score as data
        double currachscore = ui->AchievementValue->text().toDouble();

//        coscoredata.append(coscore);//keeps track of coherence score data
//        if (currentTimerCount > 64){//always removes the first coherence score past 64 seconds when it gets updated. to keep track properly of last 64 seconds for achievement score
//            currachscore -= coscoredata.first();
//            coscoredata.pop_front();
//        }

        ui->CoherenceTextValue->setNum(coscore);
        ui->AchievementValue->setNum(currachscore + coscore);

        //changes LED depending on challenge level and coherence score
        if(challengelevel == 1){
            if (coscore < 0.5){//red
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(255,0,0)}");
                updatesInRed++;
            }else if(coscore > 0.9){//green
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,255,0)}");
                updatesInGreen++;
            }else{//blue
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,0,255)}");
                updatesInBlue++;
            }
        }else if(challengelevel == 2){
            if (coscore < 0.6){//red
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(255,0,0)}");
                updatesInRed++;
            }else if(coscore > 2.1){//green
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,255,0)}");
                updatesInGreen++;
            }else{//blue
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,0,255)}");
                updatesInBlue++;
            }
        }else if(challengelevel == 3){
            if (coscore < 1.8){//red
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(255,0,0)}");
                updatesInRed++;
            }else if(coscore > 4.0){//green
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,255,0)}");
                updatesInGreen++;
            }else{//blue
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,0,255)}");
                updatesInBlue++;
            }
        }else if(challengelevel == 4){
            if (coscore < 4.0){//red
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(255,0,0)}");
                updatesInRed++;
            }else if(coscore > 6.0){//green
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,255,0)}");
                updatesInGreen++;
            }else{//blue
                ui->coherenceLevel->setStyleSheet("QLineEdit {background-color:rgb(0,0,255)}");
                updatesInBlue++;
            }
        }
    }


    QString timeString = QString::number(currentTimerCount/60) + ((currentTimerCount%60 < 10) ? + ":0" + QString::number(currentTimerCount%60) : + ":" + QString::number(currentTimerCount%60));
    ui->LengthTextValue->setText(timeString);
}

void MainWindow::formatHistoryOut(storedData d)
{
    double tempAve;
    double tempRed;
    double tempBlue;
    double tempGreen;
    double tempTotal;
    if (d.sTime < 5){
        tempAve = 0;
        tempRed = 0;
        tempBlue = 0;
        tempGreen =0;
    }else {
        tempAve = d.achScore/((d.sTime - (d.sTime % 5))/5);
        tempTotal = d.inRed + d.inBlue + d.inGreen;
        tempRed = std::round(((d.inRed / tempTotal) * 100) * 100) / 100;
        tempBlue = std::round(((d.inBlue / tempTotal) * 100) * 100) / 100;
        tempGreen = std::round(((d.inGreen / tempTotal) * 100) * 100) / 100;
    }
    historyOut = historyOut + QString::number(d.year) + "-" + QString::number(d.month) + "-" + QString::number(d.day) + "  " +
              d.time + "\n  Achievement Score: " + QString::number(d.achScore) + "\n  Session Time: " + QString::number(d.sTime)
            + "\n  Challenge Level: " + QString::number(d.cLevel) + "\n  Red Zone: " + QString::number(tempRed) + "%\n  Blue Zone: "
            + QString::number(tempBlue) + "%\n  Green Zone: " + QString::number(tempGreen) + "%\n  Average Coherence: " + QString::number(tempAve) + "\n";
}

void MainWindow::on_ApplySkin_currentTextChanged(const QString &onskin)//updates the HR sensor image
{
    if (onskin == "TRUE"){//enable HR sensor
        ui->electrodeLabel_2->setVisible(false);//off pic is disabled
        ui->electrodeLabel->setVisible(true);//on pic is enabled
    }
    if (onskin == "FALSE"){//disable HR sensor
        ui->electrodeLabel_2->setVisible(true);//off pic is enabled
        ui->electrodeLabel->setVisible(false);//on pic is disabled
    }
}
