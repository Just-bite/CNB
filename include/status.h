#ifndef STATUS_H
#define STATUS_H

enum StationState {
    WAITING,
    TRANSMITTING,
    RECEIVING,
    ERROR_STATE,
    DAMAGED
};

enum TokenState {
    TOKEN_FREE,
    TOKEN_BUSY,
    TOKEN_LOST,
    TOKEN_DAMAGED
};

enum class ErrorType {
    StationOffline,
    CorruptMarker,
    CorruptMessage
};


#endif // STATUS_H
