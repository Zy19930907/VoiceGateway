#ifndef NET_PRO_H
#define NET_PRO_H

#include "Public.h"

#define ACK        0x10
#define SoftVerb   10


void MakeFrameHeader(void);
void NetPro(void);
void HandleCmd(u8 *buf,u16 len);
void NetIntProcess(void);
void AnswerBasePara(void);
void AnswerSetTime(void);
void AnswerAck(void);
void AnswerFactoryTime(void);
void AnswerSetIp(void);
void AnswerSetResetTimes(void);
void AnswerSetIp(void);
void AnswerFactoryTime(void);
void AnswerIpAddr(void);
void AnswerSoftVerb(void);
void AnswerBasePara(void);
void ReadFactoryTime(void);
void AnswerVoiceBufFree(void);
void AnswerSetIapIp(void);
void AnswerSetMac(void);
void AnswerCheckMac(void);

#endif
