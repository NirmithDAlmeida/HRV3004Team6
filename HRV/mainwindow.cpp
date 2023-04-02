#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize timer counter
    currentTimerCount = -1;

    // Create menu tree
    masterMenu = new Menu("MAIN MENU", {"START A NEW SESSION","SETTINGS","HISTORY"}, nullptr);
    mainMenuOG = masterMenu;
    initializeMainMenu(masterMenu);

    // Initialize the main menu view
    activeQListWidget = ui->mainMenuListView;
    activeQListWidget->addItems(masterMenu->getMenuItems());
    activeQListWidget->setCurrentRow(0);
    ui->menuLabel->setText(masterMenu->getName());


    // Account for device being "off" on sim start
    powerStatus = false;
    changePowerStatus();
    connect(ui->powerButton, &QPushButton::released, this, &MainWindow::powerChange);

    //git test

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
    initializeTimer(timer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeMainMenu(Menu* m) {

    Menu* session = new Menu("START A NEW SESSION", {"YES","NO"}, m);
    Menu* settings = new Menu("SETTINGS", {"CHALLENGE LEVEL","BREATH INTERVAL"}, m);
    Menu* history = new Menu("HISTORY", {"VIEW","CLEAR"}, m);

    m->addChildMenu(session);
    m->addChildMenu(settings);
    m->addChildMenu(history);


    Menu* challengeSetting = new Menu("CHALLENGE LEVEL",{"YES","NO"}, settings);
    Menu* breathSetting = new Menu("BREATH INTERVAL",{"YES","NO"}, settings);
    settings->addChildMenu(challengeSetting);
    settings->addChildMenu(breathSetting);



    Menu* viewHistory = new Menu("VIEW",{}, history);
    Menu* clearHistory = new Menu("CLEAR", {"YES","NO"}, history);
    history->addChildMenu(viewHistory);
    history->addChildMenu(clearHistory);
}

void MainWindow::initializeTimer(QTimer* t) {

    connect(t, &QTimer::timeout, this, &MainWindow::updateTimer);

    t->start(1000);

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
            navigateBack();
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
        else {//NO selected, and go back
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
        else {//NO selected, and go back
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
    //stop session
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
    while (masterMenu->getName() != "MAIN MENU") {
        masterMenu = masterMenu->getParent();
    }

    updateMenu(masterMenu->getName(), masterMenu->getMenuItems());
//    ui->programViewWidget->setVisible(false);
//    ui->electrodeLabel->setVisible(false);
}

void MainWindow::changePowerStatus() {

    activeQListWidget->setVisible(powerStatus);
    ui->menuLabel->setVisible(powerStatus);
    ui->statusBarQFrame->setVisible(powerStatus);
    ui->treatmentView->setVisible(powerStatus);
    ui->frequencyLabel->setVisible(powerStatus);
    ui->programViewWidget->setVisible(powerStatus);

    //Remove this if we want the menu to stay in the same position when the power is off
    if (powerStatus) {
        MainWindow::navigateToMainMenu();
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
    powerStatus  = !powerStatus;
    changePowerStatus();
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
}
