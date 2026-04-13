#include "station.h"
#include "tokenmanager.h"

Station::Station(quint8 address, TokenManager* tokenManager, bool isMonitor)
    : address(address), tokenManager(tokenManager), isMonitor(isMonitor)
{
    if(isMonitor)
        tokenManager->initToken();
}

bool Station::hasDataToSend() const
{
    return dataToSend != "";
}

void Station::sendData()
{
    QByteArray data = dataToSend.toUtf8();

    if(this->address != destinationAddress){
    tokenManager->addDataToToken(
        QString::fromUtf8(data),
        this->address,
        destinationAddress
    );
    }
    else
        addToMessageLog("Data cannot be send because source and dest are equal");
}

void Station::setData(QString data)
{
    dataToSend = data;
}

void Station::setReceiverAdresses(int s_destinationAddress)
{
    destinationAddress = s_destinationAddress;
}

void Station::processToken(QVector<Station*> stations) {
    if (hasError()) {
        addToMessageLog("ERROR: Station " + QString::number(address) + " offline, skipping token");
        if (isMonitor) {
            qDebug() << "Monitor station checking ring health...";
            tokenManager->currentMonitorIndex = tokenManager->monitorRingHealth(stations, tokenManager->currentMonitorIndex);
        }
        return;
    }

    qDebug() << "Station" << address << "processing token. Token busy:" << tokenManager->isTokenBusy();

    if (tokenManager->isTokenBusy()) {
        if (tokenManager->checkReciever(address)) {
            setState(RECEIVING);

            tokenManager->setAddressRecognizedFlag(true);
            QString receivedData = tokenManager->getTokenData();
            messagesRecieved.append(receivedData);

            QString logMsg = QString("Received data: %1").arg(receivedData);
            addToMessageLog(logMsg);
            char fcs = 0;
            QByteArray token = tokenManager->getToken();

            for (int i = 0; i < token.size() - 3; i++)
                fcs ^= token[i];

            if(fcs != token[token.size() - 3])
                 addToMessageLog("Data has been corrupted!");

            tokenManager->setFrameCopiedFlag(true);
        }
        else if (tokenManager->getSourceAddress() == address) {
            setState(RECEIVING);

            bool aFlag = tokenManager->isAddressRecognized();
            bool cFlag = tokenManager->isFrameCopied();

            QString logMsg = QString("Token returned. Flags A:%1 C:%2").arg(aFlag).arg(cFlag);
            addToMessageLog(logMsg);
            if (aFlag && cFlag && destinationAddress == tokenManager->getToken()[3])
                addToMessageLog("Data successfully delivered!");
            else if(aFlag && cFlag && destinationAddress != tokenManager->getToken()[3])
                addToMessageLog("Delivery was successfull but the destination was changed to" +
                                                  QString::number(tokenManager->getToken()[3]));
            else
                addToMessageLog("Delivery failed");

            qDebug() << "Cleaning token and resetting data...";
            tokenManager->cleanToken();
            dataToSend = "";
        }
    } else {
        if (hasDataToSend()) {
            setState(TRANSMITTING);
            qDebug() << "Station" << address << "sending data:" << dataToSend << "to station" << destinationAddress;
            sendData();
            qDebug() << "Data sent successfully";

        } else {
            qDebug() << "Station" << address << "has no data to send, passing free token";
            setState(WAITING);
        }
    }

    if (isMonitor) {
        qDebug() << "Monitor station checking ring health...";
        tokenManager->currentMonitorIndex = tokenManager->monitorRingHealth(stations, tokenManager->currentMonitorIndex);
    }
}

void Station::addToMessageLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString fullMessage = "[" + timestamp + "] " + message;
    messageLog.append(fullMessage);


    qDebug() << QString("Station %1: %2").arg(address).arg(message);

    if (messageLog.size() > 50)
        messageLog.removeFirst();
}

QString Station::getStationInfo() const
{
    QString stateStr;
    switch (stationState) {
    case WAITING: stateStr = "🟢 Waiting"; break;
    case TRANSMITTING: stateStr = "🟡 Transmitting"; break;
    case RECEIVING: stateStr = "🔵 Receiving"; break;
    case ERROR_STATE: stateStr = "🔴 Error"; break;
    case DAMAGED: stateStr = "⚫ Damaged"; break;
    }

    QString info = QString("Station %1\n").arg(address);
    info += QString("State: %1\n").arg(stateStr);
    info += QString("Monitor: %1\n").arg(isMonitor ? "Yes" : "No");
    info += QString("Messages in queue: %1\n").arg(hasDataToSend() ? "1" : "0");
    info += QString("Messages received: %1\n").arg(messagesRecieved.size());

    if (hasError()) {
        info += "🚨 STATION OFFLINE 🚨\n";
    }

    info += "Last messages:\n";
    int count = qMin(3, messagesRecieved.size());
    for (int i = messagesRecieved.size() - count; i < messagesRecieved.size(); i++) {
        info += QString("  • %1\n").arg(messagesRecieved[i]);
    }

    return info;
}

void Station::setState(StationState newState)
{
    if (stationState != newState) {
        stationState = newState;
        QString stateStr;
        switch (stationState) {
        case WAITING: stateStr = "Waiting"; break;
        case TRANSMITTING: stateStr = "Transmitting"; break;
        case RECEIVING: stateStr = "Receiving"; break;
        case ERROR_STATE: stateStr = "Error"; break;
        case DAMAGED: stateStr = "Damaged"; break;
        }
    }
}

void Station::setOnline(bool online)
{
    isOnline = online;

    if (online)
        stationState = WAITING;
    else
        stationState = ERROR_STATE;

    addToMessageLog(online ? "Station ONLINE" : "Station OFFLINE");
}


void Station::simulateError(ErrorType type)
{
    switch (type) {
    case ErrorType::StationOffline:
        setOnline(false);
        break;

    case ErrorType::CorruptMarker:
        corruptMarker = true;
        addToMessageLog("Marker will be corrupted during transfer");
        break;

    case ErrorType::CorruptMessage:
        corruptMessage = true;
        addToMessageLog("Message will be corrupted during transfer");
        break;
    }
}

void Station::setMonitorRole(bool isMonitorRole)
{
    if (isMonitor != isMonitorRole) {
        isMonitor = isMonitorRole;
        if (isMonitorRole) {
            addToMessageLog("🎯 Assigned as network monitor");
            qDebug() << "Station" << address << "is now monitor";
        } else {
            addToMessageLog("Monitor role removed");
        }
    }
}

QByteArray Station::getToken()
{
    return tokenManager->getToken();
}
