#pragma once
#ifndef PACKETHANDLE_H_
#define PACKETHANDLE_H_
#include "StructShare.h"
BOOL OnHandlePacket(PACKETTYPE ePacketType,
                    SOCKET sctTargetSocket,
                    char *szBuffer,
                    size_t uiLen,
                    PCLIENTINFO  pstClientInfo,
                    CCommunicationIOCP &IOCP);

#endif // !PACKETHANDLE_H_
