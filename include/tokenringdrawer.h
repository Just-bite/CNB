#ifndef TOKENRINGDRAWER_H
#define TOKENRINGDRAWER_H

#define _USE_MATH_DEFINES

#include <QGraphicsItemAnimation>
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QTimeLine>
#include <QTimer>
#include <QDebug>
#include <cmath>

#include "station.h"
#include "tokenmanager.h"

class TokenRingDrawer  : public QWidget
{
    Q_OBJECT
public:
    TokenRingDrawer(QGraphicsScene* tokenRingScene, QGraphicsView* tokenRingGraphicsView, QWidget* parrent = nullptr);
    virtual ~TokenRingDrawer();

    void drawSkeleton(int stationsCount);
    void startTokenCirculation();
    void stopTokenCirculation();
    void setData(QString data, int senderAdress, int receiverAdress);
    void simulateErrorAtStation(int stationIndex);
    void simulateTokenError(bool isDest = false, int stationsCount = 0);
    void simulateKillToken();
private slots:
    void moveTokenToNextStation();
    void regenerateToken();
private:
    void updateStationToolTip(int stationIndex);
    void stopAllAnimations();
    void addStationIcon(qreal x, qreal y, qreal size);
    void addToken(qreal x, qreal y, qreal size);
    QTimer* tokenTimer;
     QList<QTimeLine*> activeTimelines;
    QTimer* tokenRegenerationTimer;
    int currentStationIndex = 0;
    TokenManager* tokenManager;
    QGraphicsScene* tokenRingScene;
    QGraphicsView* tokenRingGraphicsView;
    QWidget* parrent;
    QVector<Station*> stations;
    QVector<QGraphicsPixmapItem*> stationIcons;
    QGraphicsPixmapItem* tokenItem;
    QVector<QGraphicsEllipseItem*> stationStateCircles;
    void updateStationStateVisual(int stationIndex);
     bool isTokenAlive = true;
};

#endif // TOKENRINGDRAWER_H
