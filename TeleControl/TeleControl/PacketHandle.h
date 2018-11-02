#pragma once
#ifndef PACKETHANDLE_H_
#define PACKETHANDLE_H_


BOOL OnHandlePacket(PACKETTYPE ePacketType,
                    SOCKET sctTargetSocket,
                    char *szBuffer,
                    PACKETFORMAT &ref_stHeader,
                    PCLIENTINFO pstClientInfo,
                    CCommunicationIOCP &IOCP);

#endif // !PACKETHANDLE_H_

