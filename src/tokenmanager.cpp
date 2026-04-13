#include "tokenmanager.h"

TokenManager::TokenManager()
{
}

void TokenManager::initToken()
{
    token.append(char(0x7E)); // SD (Starting Delimiter)
    token.append(char(0x00)); // AC (Access Control)
    token.append(char(0x00)); // FC (Frame Control)
    token.append(char(0x00)); // DA (Destination Address)
    token.append(char(0x00)); // SA (Source Address)
    token.append(char(0x00)); // FCS (Frame Check Sequence)
    token.append(char(0x7E)); // ED (Ending Delimiter)
    token.append(char(0x00)); // FS (Frame Status)
}

void TokenManager::addDataToToken(QString dataToSend,int sourceAddress, int destinationAddress)
{
    if (sourceAddress < 0 || sourceAddress > 255 ||
            destinationAddress < 0 || destinationAddress > 255) {
            qDebug() << "Invalid addresses";
            return;
        }

        token.clear();

        token.append(char(0x7E));                    // SD
        token.append(char(0x10));                    // AC
        token.append(char(0x00));                    // FC
        token.append(char(destinationAddress));      // DA
        token.append(char(sourceAddress));           // SA

        QByteArray dataBytes = dataToSend.toUtf8();
        token.append(dataBytes);

        char fcs = 0;
        for (int i = 0; i < token.size(); i++)
            fcs ^= token[i];

        token.append(fcs);

        token.append(char(0x7E));                    // ED
        token.append(char(0x00));                    // FS

}

bool TokenManager::checkReciever(qint8 address)
{
    if (token.size() > 3) {
        qint8 destAddress = token[3];
        return address == destAddress;
    }
    return false;
}

bool TokenManager::isTokenBusy()
{
    return (token[1] & 0x10) != 0;
}

int TokenManager::getSourceAddress()
{
    if (token.size() > 3) {
        qint8 sourceAddress = token[4];
        return sourceAddress;
    }
    return -1;
}


int TokenManager::monitorRingHealth(QVector<Station*> stations, int currentMonitorIndex)
{
    qDebug() << "Current Monitor index " << currentMonitorIndex;
    stationHealer++;
    int newMonitorIndex = currentMonitorIndex;
    if(stationHealer == 2) {
        stationHealer = 0;

        if (currentMonitorIndex >= 0 && currentMonitorIndex < stations.size() &&
            stations[currentMonitorIndex]->getState() == ERROR_STATE) {

            newMonitorIndex = findNewMonitor(stations);
            if (newMonitorIndex != -1) {
                qDebug() << "Monitor station" << currentMonitorIndex << "is offline. Transferring monitor role to station" << newMonitorIndex;
                stations[currentMonitorIndex]->setMonitorRole(false);
                stations[newMonitorIndex]->setMonitorRole(true);

            }
        }


        for(Station *station : stations) {
            if(station->getState() == ERROR_STATE){
                station->setState(WAITING);
            }
        }
    }
    return newMonitorIndex;
}

void TokenManager::cleanToken()
{
    if (token.size() >= 8) {
        token[1] = char(0x00);  // AC
        token[3] = char(0x00);  // DA
        token[4] = char(0x00);  // SA
        int dataStart = 5;
        int dataEnd = token.size() - 3;
        if (dataEnd > dataStart)
            token.remove(dataStart, dataEnd - dataStart);

        token[token.size() - 3] = char(0x00);  // FCS
        token[token.size() - 1] = char(0x00);  // FS
    }
}

void TokenManager::clearToken()
{
    token.clear();
    tokenState = TOKEN_LOST;
    tokenTimeoutCounter = 0;
    qDebug() << "Token cleared";
}

QString TokenManager::getTokenData()
{
    if (token.size() < 8)
        return "";

    if (!isTokenBusy())
        return "";

    int dataStart = 5;
    int dataEnd = token.size() - 3;


    if (dataStart >= dataEnd)
        return "";

    QByteArray dataBytes = token.mid(dataStart, dataEnd - dataStart);

    return QString::fromUtf8(dataBytes);
}

void TokenManager::setAddressRecognizedFlag(bool recognized)
{
    if (token.size() > 0) {
        int fsPosition = token.size() - 1;
        if (recognized)
            token[fsPosition] =  token[fsPosition] | 0x88;
        else
            token[fsPosition] = token[fsPosition] & ~0x88;
    }
}

void TokenManager::setFrameCopiedFlag(bool copied)
{
    if (token.size() > 0) {
        int fsPosition = token.size() - 1;
        if (copied)
            token[fsPosition] = token[fsPosition] | 0x44;
        else
            token[fsPosition] = token[fsPosition] & ~0x44;
    }
}

bool TokenManager::isAddressRecognized()
{
    if (token.size() > 0)
        return (token[token.size() - 1] & 0x88) != 0;

    return false;
}

bool TokenManager::isFrameCopied()
{
    if (token.size() > 0)
        return (token[token.size() - 1] & 0x44) != 0;

    return false;
}

void TokenManager::setStationCount(int count)
{
    MAX_TOKEN_TIMEOUT = count * 3;
}

void TokenManager::regenerateToken()
{
    token.clear();
    initToken();
    tokenState = TOKEN_FREE;
    tokenTimeoutCounter = 0;
    qDebug() << "Token regenerated by monitor";
}

void TokenManager::simulateTokenError(bool isDest, int stationsCount)
{
    if (!token.isEmpty()) {
        if(!isDest){
        char damagedByte = static_cast<char>(rand() % 256);
        token[6] = damagedByte;
        qDebug() << "Token damaged at position" << 6;
        } else{
            char damagedByte = static_cast<char>(rand() % stationsCount);
            token[3] = damagedByte;
            qDebug() << "Token damaged at dest adress now it equals" << static_cast<int>(damagedByte);
        }
        tokenState = TOKEN_DAMAGED;
    }
}

int TokenManager::findNewMonitor(QVector<Station*> stations)
{
    for (int i = 0; i < stations.size(); i++) {
        if (stations[i]->getState() != ERROR_STATE && stations[i]->getState() != DAMAGED) {
            return i;
        }
    }
    return -1;
}
