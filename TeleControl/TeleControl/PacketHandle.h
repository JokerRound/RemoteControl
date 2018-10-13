#pragma once
#ifndef PACKETHANDLE_H_
#define PACKETHANDLE_H_


BOOL OnHandlePacket(PACKETTYPE ePacketType,
                    SOCKET sctTargetSocket,
                    char *szBuffer,
                    size_t uiLen,
                    PCLIENTINFO pstClientInfo,
                    CCommunicationIOCP &IOCP);

#endif // !PACKETHANDLE_H_

