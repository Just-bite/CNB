#include "tokenringdrawer.h"

TokenRingDrawer::TokenRingDrawer(QGraphicsScene* tokenRingScene, QGraphicsView* tokenRingGraphicsView, QWidget* parrent)
    : tokenRingScene(tokenRingScene), tokenRingGraphicsView(tokenRingGraphicsView), parrent(parrent)
{
     tokenRegenerationTimer = nullptr;
}


void TokenRingDrawer::drawSkeleton(int stationsCount)
{
    QPen redPen(Qt::red);
    redPen.setWidth(4);
    qreal ringDiameter = tokenRingScene->height() * 0.8;
    tokenRingScene->addEllipse(QRect(tokenRingScene->width() * 0.1, tokenRingScene->height() * 0.1,
                                                              ringDiameter, ringDiameter), redPen);

    qreal centerX = tokenRingScene->width() * 0.1 + ringDiameter / 2;
    qreal centerY = tokenRingScene->height() * 0.1 + ringDiameter / 2;
    qreal iconSize = 0.1 * ringDiameter;

    tokenManager = new TokenManager();
    tokenManager->setStationCount(stationsCount);

    for(int i = 0 ; i < stationsCount; i++){
        qreal angle = 2 *  M_PI * i / stationsCount;
        qreal x = centerX + ringDiameter * cos(angle) / 2;
        qreal y = centerY + ringDiameter * sin(angle) / 2;
        qreal xT = centerX + ringDiameter * 0.8 * cos(angle) / 2;
        qreal yT = centerY + ringDiameter * 0.8 * sin(angle) / 2;

        if(!i){
            bool isMonitor = true;
            stations.append(new Station(i, tokenManager, isMonitor));
            addToken(xT, yT, iconSize);
        }
        else
            stations.append(new Station(i,tokenManager));
        addStationIcon(x, y, iconSize);
    }
}


void TokenRingDrawer::addStationIcon(qreal x, qreal y, qreal size)
{
    QPixmap stationPixmap(":/icon/station.png");

    if (!stationPixmap.isNull()) {
        stationPixmap = stationPixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QGraphicsPixmapItem* stationItem = tokenRingScene->addPixmap(stationPixmap);
        stationItem->setPos(x - size / 2, y - size / 2);
        stationItem->setZValue(1);
        stationItem->setToolTip("Station " + QString::number(stationIcons.size()));
        stationIcons.append(stationItem);
        stationItem->setAcceptHoverEvents(true);

        QGraphicsEllipseItem* stateCircle = tokenRingScene->addEllipse(
            x - size/2 - 5, y - size/2 - 5,
            size + 10, size + 10,
            QPen(Qt::NoPen),
            QBrush(Qt::transparent)
        );
        stateCircle->setZValue(0);
        stateCircle->setVisible(true);
        stationStateCircles.append(stateCircle);
    }
}

void TokenRingDrawer::updateStationStateVisual(int stationIndex)
{
    if (stationIndex < 0 || stationIndex >= stationStateCircles.size() ||
        stationIndex >= stations.size()) {
        return;
    }

    Station* station = stations[stationIndex];
    QGraphicsEllipseItem* stateCircle = stationStateCircles[stationIndex];

    QColor stateColor;
    bool visible = true;

    switch (station->getState()) {
    case WAITING:
        stateColor = Qt::green;
        break;
    case TRANSMITTING:
        stateColor = QColor(255, 165, 0);
        break;
    case RECEIVING:
        stateColor = Qt::blue;
        break;
    case ERROR_STATE:
        stateColor = Qt::red;
        break;
    default:
        stateColor = Qt::gray;
        break;
    }
    if (visible) {
        QPen pen(stateColor);
        pen.setWidth(3);
        stateCircle->setPen(pen);
        stateCircle->setBrush(QBrush(Qt::transparent));
        stateCircle->setVisible(true);
    } else {
        stateCircle->setVisible(false);
    }
}

void TokenRingDrawer::addToken(qreal x, qreal y, qreal size)
{
    QPixmap tokenPixmap(":/icon/letter.png");

    if (!tokenPixmap.isNull()) {
        tokenPixmap = tokenPixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        tokenItem = tokenRingScene->addPixmap(tokenPixmap);
        tokenItem->setPos(x - size / 2, y - size / 2);
        tokenItem->setZValue(1);
    }
}

void TokenRingDrawer::startTokenCirculation() {
    tokenTimer = new QTimer(this);
    connect(tokenTimer, &QTimer::timeout, this, &TokenRingDrawer::moveTokenToNextStation);
    tokenTimer->start(1000);
}



void TokenRingDrawer::moveTokenToNextStation() {
    if (!isTokenAlive)
        return;

    if (currentStationIndex >= 0 && currentStationIndex < stations.size()
            && stations[currentStationIndex]->getState() != ERROR_STATE ) {
        stations[currentStationIndex]->setState(WAITING);
        updateStationStateVisual(currentStationIndex);
    }

    currentStationIndex = (currentStationIndex + 1) % stations.size();

    qreal ringDiameter = tokenRingScene->height() * 0.8;
    qreal centerX = tokenRingScene->width() * 0.1 + ringDiameter / 2;
    qreal centerY = tokenRingScene->height() * 0.1 + ringDiameter / 2;

    qreal angle = 2 * M_PI * currentStationIndex / stations.size();
    qreal x = centerX + ringDiameter * 0.8 * cos(angle) / 2;
    qreal y = centerY + ringDiameter * 0.8 * sin(angle) / 2;
    qreal iconSize = 0.1 * ringDiameter;

    if (tokenItem && isTokenAlive) {
        QGraphicsItemAnimation* animation = new QGraphicsItemAnimation();
        animation->setItem(tokenItem);

        QTimeLine* timeline = new QTimeLine(500);
        timeline->setFrameRange(0, 100);
        activeTimelines.append(timeline);

        animation->setTimeLine(timeline);
        animation->setPosAt(1.0, QPointF(x - iconSize/2, y - iconSize/2));

        connect(timeline, &QTimeLine::finished, [this, animation, timeline]() {
            activeTimelines.removeOne(timeline);
            delete animation;
            delete timeline;
        });

        timeline->start();
    }

    if (isTokenAlive) {
        stations[currentStationIndex]->processToken(stations);
    }

    for(int i = 0; i < stations.size(); i++){
        updateStationToolTip(i);
        updateStationStateVisual(i);
    }
}

void TokenRingDrawer::stopAllAnimations() {
    for (QTimeLine* timeline : activeTimelines) {
        timeline->stop();
        timeline->deleteLater();
    }
    activeTimelines.clear();

    QList<QTimeLine*> sceneTimelines = tokenRingScene->findChildren<QTimeLine*>();
    for (QTimeLine* timeline : sceneTimelines) {
        timeline->stop();
        timeline->deleteLater();
    }

    QList<QGraphicsItemAnimation*> animations = tokenRingScene->findChildren<QGraphicsItemAnimation*>();
    for (QGraphicsItemAnimation* animation : animations) {
        animation->deleteLater();
    }
}

void TokenRingDrawer::simulateKillToken()
{
    if (!isTokenAlive) {
        qDebug() << "Token already killed!";
        return;
    }

    qDebug() << "Killing token...";

    isTokenAlive = false;
    stopAllAnimations();
    stopTokenCirculation();

    if (tokenItem) {
        tokenItem->setEnabled(false);
        tokenRingScene->removeItem(tokenItem);
        delete tokenItem;
        tokenItem = nullptr;
    }

    if (tokenManager) {
        tokenManager->clearToken();
    }

    for (int i = 0; i < stations.size(); ++i) {
        if (stations[i]->getState() != ERROR_STATE && stations[i]->getState() != DAMAGED) {
            stations[i]->setState(WAITING);
        }
        updateStationStateVisual(i);
        updateStationToolTip(i);
    }

    if (tokenRegenerationTimer) {
        tokenRegenerationTimer->stop();
        delete tokenRegenerationTimer;
    }

    tokenRegenerationTimer = new QTimer(this);
    tokenRegenerationTimer->setSingleShot(true);
    connect(tokenRegenerationTimer, &QTimer::timeout, this, &TokenRingDrawer::regenerateToken);
    tokenRegenerationTimer->start(1000 * stations.size());

    qDebug() << "Token killed. Regeneration in" << stations.size() << " seconds...";
}

void TokenRingDrawer::regenerateToken()
{
    qDebug() << "Regenerating token...";

    isTokenAlive = true;

    if (tokenManager) {
        tokenManager->regenerateToken();
    }

    qreal ringDiameter = tokenRingScene->height() * 0.8;
    qreal centerX = tokenRingScene->width() * 0.1 + ringDiameter / 2;
    qreal centerY = tokenRingScene->height() * 0.1 + ringDiameter / 2;
    qreal iconSize = 0.1 * ringDiameter;

    qreal angle = 2 * M_PI * 0 / stations.size();
    qreal x = centerX + ringDiameter * 0.8 * cos(angle) / 2;
    qreal y = centerY + ringDiameter * 0.8 * sin(angle) / 2;

    addToken(x, y, iconSize);

    currentStationIndex = 0;

    startTokenCirculation();

    if (tokenRegenerationTimer) {
        tokenRegenerationTimer->deleteLater();
        tokenRegenerationTimer = nullptr;
    }

    qDebug() << "Token regenerated and circulation restarted";
}

void TokenRingDrawer::updateStationToolTip(int stationIndex)
{
    if (stationIndex >= 0 && stationIndex < stationIcons.size() && stationIndex < stations.size()) {
        QString tooltip = stations[stationIndex]->getStationInfo();
        stationIcons[stationIndex]->setToolTip(tooltip);
    }
}

TokenRingDrawer::~TokenRingDrawer() {
    stopTokenCirculation();

    if (tokenRegenerationTimer) {
        tokenRegenerationTimer->stop();
        delete tokenRegenerationTimer;
    }

    for (QTimeLine* timeline : activeTimelines) {
        timeline->stop();
        delete timeline;
    }
    activeTimelines.clear();

    qDeleteAll(stations);
    stations.clear();
    delete tokenManager;
}

void TokenRingDrawer::stopTokenCirculation() {
    if (tokenTimer) {
        tokenTimer->stop();
        delete tokenTimer;
        tokenTimer = nullptr;
    }
    stopAllAnimations();

    qDebug() << "Token circulation stopped";
}

void TokenRingDrawer::setData(QString data, int senderAdress, int receiverAdress)
{
    stations[senderAdress]->setData(data);
    stations[senderAdress]->setReceiverAdresses(receiverAdress);
    qDebug() << "Data set :" << data << "at sender address" << senderAdress << "and recievers address" << receiverAdress;
}

void TokenRingDrawer::simulateErrorAtStation(int stationIndex)
{
    if (stationIndex >= 0 && stationIndex < stations.size()) {
        stations[stationIndex]->simulateError(ErrorType::StationOffline);
        qDebug() << "Error simulated at station" << stationIndex;

        updateStationToolTip(stationIndex);
        updateStationStateVisual(stationIndex);
    }
}

void TokenRingDrawer::simulateTokenError(bool isDest, int stationsCount)
{
    if (tokenManager) {
        tokenManager->simulateTokenError(isDest, stationsCount);
        qDebug() << "Token error simulated";
    }
}
