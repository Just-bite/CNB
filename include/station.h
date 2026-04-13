#ifndef STATION_H
#define STATION_H

#include <QDateTime>
#include <QString>
#include <QDebug>

#include "status.h"
class TokenManager;

class Station : public QObject
{
    Q_OBJECT
public:
    Station(quint8 address, TokenManager* tokenManager, bool isMonitor = false);
    void processToken(QVector<Station*> stations);
    void setData(QString data);
    void setReceiverAdresses(int s_destinationAddress);
    void sendData();
    QString getStationInfo() const;
    QStringList getMessageLog() const { return messageLog; }
    void addToMessageLog(const QString& message);
    StationState getState() const { return stationState; }
    void setState(StationState newState);
    void simulateError(ErrorType);
    bool hasError() const { return stationState == ERROR_STATE || stationState == DAMAGED; }
    void setOnline(bool online);
    void setMonitorRole(bool isMonitorRole);
    QByteArray getToken();
private:
    bool hasDataToSend() const;
    int destinationAddress = 0;
    quint8 address;
    TokenManager* tokenManager;
    QString dataToSend = "";
    bool isMonitor = false;
    StationState stationState = WAITING;
    int errorTimeout = 0;
    QStringList messagesRecieved;
    QStringList messageLog;
    bool isOnline = true;
    bool corruptMarker = false;
    bool corruptMessage = false;
};

#endif // STATION_H
