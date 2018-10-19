#pragma once
#ifndef MACROSHARE_H_
#define MACROSHARE_H_

// IOCP等待时间(毫秒)
#define IOCP_WAIT_TIME 100000

// Connect重连等待时间(毫秒)
#define CONNECT_WAIT_TIME 10000

// The max size of packet is 10M
#define PACKET_CONTENT_MAXSIZE (1024 * 1024 * 10)

// 数据包包头大小
#define PACKET_HEADER_SIZE (sizeof(PACKETFORMAT) - 1)

// 心跳定时器ID
#define TIMER_HEATBEAT 1

// 心跳包间隔
#define HEATBEAT_ELAPSE 10000

#ifdef _UNICODE
#define __FILET__ __FILEW__
#else
#define __FILET__ __FILE__ 
#endif // _UNICODE

#endif // !MACROSHARE_H_
