#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QByteArray>
#include <QWidget>
#include <QDebug>

#include "status.h"
#include "station.h"

class TokenManager : public QObject
{
    Q_OBJECT
public:
    TokenManager();

    bool isTokenBusy();
    bool isFrameCopied();
    bool isAddressRecognized();
    bool checkReciever(qint8 address);

    void setFrameCopiedFlag(bool copied);
    void setAddressRecognizedFlag(bool recognized);
    int monitorRingHealth(QVector<Station*> stations,int currentMonitorIndex);
    int  findNewMonitor(QVector<Station*> stations);
    void addDataToToken(QString dataToSend, int sourceAddress, int destinationAddress);
    QString getTokenData();
    int getSourceAddress();

    void initToken();
    void cleanToken();
    void clearToken();

    bool isTokenLost() const { return tokenState == TOKEN_LOST; }
    void regenerateToken();
    void simulateTokenError(bool isDest = false, int stationsCout = 0);
    void setStationCount(int count);
    int currentMonitorIndex = 0;
    QByteArray getToken(){ return token;}
private:
    QByteArray token;
    TokenState tokenState = TOKEN_FREE;
    int tokenTimeoutCounter = 0;
    int MAX_TOKEN_TIMEOUT;
    int stationHealer = 0;

};

#endif // TOKENMANAGER_H
