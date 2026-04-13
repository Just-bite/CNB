#include "mainwindow.h"
#include "ui_mainwindow.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    MainWindow* mainWindow = nullptr;
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (widget->inherits("MainWindow")) {
            mainWindow = qobject_cast<MainWindow*>(widget);
            break;
        }
    }

    if (mainWindow) {
        QString message;
        switch (type) {
        case QtDebugMsg:
            message = msg;
            break;
        case QtWarningMsg:
            message = "WARNING: " + msg;
            break;
        case QtCriticalMsg:
            message = "CRITICAL: " + msg;
            break;
        case QtFatalMsg:
            message = "FATAL: " + msg;
            break;
        }
        mainWindow->addToLog(message);
    }
    QByteArray localMsg = msg.toLocal8Bit();
    fprintf(stderr, "%s\n", localMsg.constData());
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Token ring simulator");
    setWindowIcon(QIcon(":/icon/token-ring.png"));
    qInstallMessageHandler(myMessageHandler);

    QString full_style = "QMainWindow { background-color: black; }"
                       "QLabel { color: white; }"
                       "QComboBox, QLineEdit, QSpinBox {"
                       "    background-color: #1A1A1A;"
                       "    color: #00FF00;"
                       "    border: 1px solid #00CC00;"
                       "    padding: 5px;"
                       "}"
                       "QPushButton {"
                       "    background-color: #0A0A0A;"
                       "    color: #00FF00;"
                       "    border: 1px solid #00CC00;"
                       "    padding: 8px;"
                       "}"
                       "QPushButton:hover {"
                       "    background-color: #00CC00;"
                       "    color: black;"
                       "}"
                       "QGroupBox {"
                       "    color: white;"
                       "    border: 1px solid #00CC00;"
                       "    margin-top: 10px;"
                       "    padding-top: 10px;"
                       "}"
                       "QGroupBox::title {"
                       "    subcontrol-origin: margin;"
                       "    subcontrol-position: top center;"
                       "    padding: 0 5px 0 5px;"
                       "    color: #00FF00;"
                       "}"
                       "QTextEdit {"
                       "    background-color: #1A1A1A;"
                       "    color: white;"
                       "    border: 1px solid #00CC00;"
                       "    selection-background-color: #00CC00;"
                       "    selection-color: black;"
                       "}"
                       "QScrollBar:vertical {"
                       "    background-color: #0A0A0A;"
                       "    width: 15px;"
                       "    margin: 0px;"
                       "}"
                       "QScrollBar::handle:vertical {"
                       "    background-color: #00CC00;"
                       "    min-height: 20px;"
                       "    border-radius: 7px;"
                       "}"
                       "QScrollBar::handle:vertical:hover {"
                       "    background-color: #00FF00;"
                       "}"
                       "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
                       "    background: none;"
                       "}"
                       "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
                       "    background: none;"
                       "}";

    QString extended_style = full_style +
                       "QToolTip {"
                       "    background-color: #1A1A1A;"
                       "    color: white;"
                       "    border: 1px solid #00CC00;"
                       "    padding: 5px;"
                       "}"
                       "QGraphicsView {"
                       "    background-color: #0A0A0A;"
                       "    border: 1px solid #00CC00;"
                       "}"
                       "QTabWidget::pane {"
                       "    border: 1px solid #00CC00;"
                       "    background-color: #1A1A1A;"
                       "}"
                       "QTabBar::tab {"
                       "    background-color: #0A0A0A;"
                       "    color: #00FF00;"
                       "    border: 1px solid #00CC00;"
                       "    padding: 8px;"
                       "}"
                       "QTabBar::tab:selected {"
                       "    background-color: #00CC00;"
                       "    color: black;"
                       "}";

    this->setStyleSheet(extended_style);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout* mainSceneLayout = new QHBoxLayout(centralWidget);

    QWidget* buttonsWidget = new QWidget(this);
    QVBoxLayout* buttonsWidgetLayout = new QVBoxLayout(buttonsWidget);

    QHBoxLayout* startStopLayout = new QHBoxLayout();
    QPushButton* startSimulationButton = new QPushButton("Start simulation");
    QPushButton* stopSimulationButton = new QPushButton("Stop simulation");

    startStopLayout->addWidget(startSimulationButton);
    startStopLayout->addWidget(stopSimulationButton);

    QPushButton* modulateErrorButton = new QPushButton("Modulate error");

    QLabel* stationsCountLabel = new QLabel("Number of stations (2-12):");
    stationsCountInput = new QLineEdit();
    stationsCountInput->setValidator(new QIntValidator(2, 12, this));
    stationsCountInput->setText("8");

    QHBoxLayout* addressSelectionLayout = new QHBoxLayout();
    QLabel* senderLabel = new QLabel("Sender:");
    senderComboBox = new QComboBox();
    QLabel* receiverLabel = new QLabel("Receiver:");
    receiverComboBox = new QComboBox();

    addressSelectionLayout->addWidget(senderLabel);
    addressSelectionLayout->addWidget(senderComboBox);
    addressSelectionLayout->addWidget(receiverLabel);
    addressSelectionLayout->addWidget(receiverComboBox);

    QHBoxLayout* messageLayout = new QHBoxLayout();
    QLabel* messageLabel = new QLabel("Message:");
    QLineEdit* messageInput = new QLineEdit();
    QPushButton* sendButton = new QPushButton("Send");

    messageLayout->addWidget(messageLabel);
    messageLayout->addWidget(messageInput);
    messageLayout->addWidget(sendButton);

    QLabel* logsLabel = new QLabel("Logs:");
    logsTextEdit = new QTextEdit();
    logsTextEdit->setReadOnly(true);

    updateComboBoxes(stationsCount);

    connect(stationsCountInput, &QLineEdit::textChanged, this, &MainWindow::updateStationsCount);
    connect(sendButton, &QPushButton::clicked, this, [this, messageInput]() {
        QString message = messageInput->text();
        int sender = senderComboBox->currentText().toInt();
        int receiver = receiverComboBox->currentText().toInt();
        qDebug() << "Send message from" << sender << "to" << receiver << ":" << message;
        if(sender != receiver)
            tokenRingDrawer->setData(message, sender, receiver);
        else
            qDebug() << "Data cannot be send because source and dest are equal";
    });

    errorDialog = new QDialog(this);
    errorDialog->setMinimumSize(150, 100);
    QHBoxLayout* errorDialogLayout = new QHBoxLayout(errorDialog);

    QPushButton* stationError = new QPushButton("Station Error");
    QPushButton* markerError = new QPushButton("Marker Error");
    QPushButton* dataError = new QPushButton("Data error");

    errorDialogLayout->addWidget(stationError);
    errorDialogLayout->addWidget(markerError);
    errorDialogLayout->addWidget(dataError);


    connect(modulateErrorButton, &QPushButton::clicked, this, [this]() {
        if (tokenRingDrawer)
            errorDialog->show();
    });

    connect(stationError, &QPushButton::clicked, this, [this](){
        stationDialog = new QDialog(errorDialog);
        stationDialog->setMinimumSize(200, 150);
        QComboBox* stationsCombobx = new QComboBox(stationDialog);
        for(int i = 0; i < stationsCount; i++)
            stationsCombobx->addItem(QString::number(i));
        QPushButton* emulateStationError = new QPushButton("Emulate station error",stationDialog);

        QVBoxLayout* stationDialogLayout = new QVBoxLayout(stationDialog);
        stationDialogLayout->addWidget(stationsCombobx);
        stationDialogLayout->addWidget(emulateStationError);

        connect(emulateStationError, &QPushButton::clicked,this, [this, stationsCombobx]() {
            tokenRingDrawer->simulateErrorAtStation(stationsCombobx->currentText().toInt());
            qDebug() << "Modulated error at station" << stationsCombobx->currentText().toInt();
        });
        stationDialog->show();
    });

    connect(dataError, &QPushButton::clicked, this, [this](){
        tokenRingDrawer->simulateTokenError();
    });

    connect(markerError, &QPushButton::clicked, this, [this](){
        markerDialog = new QDialog(errorDialog);
        markerDialog->setWindowTitle("Marker Errors");

        markerDialog->setMinimumSize(200, 150);

        QPushButton* killMarker = new QPushButton("Kill marker", markerDialog);
        QPushButton* damageDestAddress = new QPushButton("Damage destination", markerDialog);

        QVBoxLayout* markerDialogLayout = new QVBoxLayout(markerDialog);
        markerDialogLayout->addWidget(killMarker);
        markerDialogLayout->addWidget(damageDestAddress);

        markerDialogLayout->addStretch();
        markerDialog->show();

        QPoint center = errorDialog->geometry().center();
        markerDialog->move(center - markerDialog->rect().center());

        connect(killMarker, &QPushButton::clicked, this, [this](){
            tokenRingDrawer->simulateKillToken();
            markerDialog->close();
        });

        connect(damageDestAddress, &QPushButton::clicked, this, [this](){
            tokenRingDrawer->simulateTokenError(true, stationsCount);
            markerDialog->close();
        });

    });

    buttonsWidgetLayout->addLayout(startStopLayout);
    buttonsWidgetLayout->addWidget(modulateErrorButton);
    buttonsWidgetLayout->addWidget(stationsCountLabel);
    buttonsWidgetLayout->addWidget(stationsCountInput);
    buttonsWidgetLayout->addLayout(addressSelectionLayout);
    buttonsWidgetLayout->addLayout(messageLayout);
    buttonsWidgetLayout->addWidget(logsLabel);
    buttonsWidgetLayout->addWidget(logsTextEdit);

    tokenRingScene = new QGraphicsScene(this);
    tokenRingSceneView = new QGraphicsView(tokenRingScene);

    tokenRingSceneView->setRenderHint(QPainter::Antialiasing);
    tokenRingSceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tokenRingSceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QRectF viewRect = tokenRingSceneView->viewport()->rect();
    tokenRingScene->setSceneRect(viewRect);

    connect(startSimulationButton, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(stopSimulationButton, &QPushButton::clicked, this, &MainWindow::stopSimulation);

    mainSceneLayout->addWidget(buttonsWidget, 1);
    mainSceneLayout->addWidget(tokenRingSceneView, 2);
}

void MainWindow::updateStationsCount(const QString& text)
{
    if (!text.isEmpty()) {
        int count = text.toInt();
        if (count >= 2 && count <= 12) {
            stationsCount = count;
            senderComboBox->clear();
            receiverComboBox->clear();
            for (int i = 0; i < stationsCount; ++i) {
                senderComboBox->addItem(QString::number(i));
                receiverComboBox->addItem(QString::number(i));
            }
        }
    }
}

void  MainWindow::updateComboBoxes(int stationCount) {
        senderComboBox->clear();
        receiverComboBox->clear();
        for (int i = 0; i < stationCount; ++i) {
            senderComboBox->addItem(QString::number(i));
            receiverComboBox->addItem(QString::number(i));
        }
    }

void MainWindow::startSimulation()
{
    if (tokenRingDrawer) {
          delete tokenRingDrawer;
          tokenRingDrawer = nullptr;
    }
    tokenRingScene->clear();
    tokenRingDrawer = new TokenRingDrawer(tokenRingScene, tokenRingSceneView, this);
    tokenRingDrawer->drawSkeleton(stationsCount);
    tokenRingDrawer->startTokenCirculation();
}

void MainWindow::stopSimulation()
{
    if (tokenRingDrawer) {

        tokenRingDrawer->stopTokenCirculation();

        qDebug() << "Simulation stopped";
    }
}

void MainWindow::addToLog(const QString& message)
{
    if (logsTextEdit) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        logsTextEdit->append("[" + timestamp + "] " + message);

        QTextCursor cursor = logsTextEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        logsTextEdit->setTextCursor(cursor);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event) {
       QMainWindow::resizeEvent(event);
       if (tokenRingSceneView) {
           tokenRingSceneView->fitInView(tokenRingScene->itemsBoundingRect(), Qt::KeepAspectRatio);
       }
   }

