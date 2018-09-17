#ifndef NET_PRO_H
#define NET_PRO_H

#include "Public.h"


void MakeFrameHeader(void);
void NetPro(void);
void HandleTcpData(u8 *buf,u16 len,u8 flag);
void NetIntProcess(void);
void AnswerCurInfo(u8 flag);
void AnswerBasePara(void);
void AnswerIpAddr(void);
void AnswerSetTime(void);
void AnswerAck(u8 flag);
void AnswerFactoryTime(void);
void AnswerSetIp(void);
void AnswerSetResetTimes(void);
void AnswerCleanRWptr(void);
void CanInfoToNet(u8 cmd,u8 *buf,u8 len);
void AnswerPowerInfo(u8 addr);
void AnswerSoftVerb(void);
void AnswerSetMac(void);
void AnswerCheckMac(void);
void AnswerSetNetFilterTime(void);
void AnswerCheckNetFilterTime(void);
void AnswerUnRegisterInfo(void);
void AnswerForBootLoader(void);
void AnswerErrorBitsInfo(u8 *buf,u16 len);
void AnswerWrongAck(void);
void AnswerCheckCrcCntInfo(void);
void AnswerCleanCrcCntInfo(void);
void AnswerCrcFail(void);
void AnswerResetCrcFail(void);
void AnswerCrcInfo(void);
void AnswerSetBiaoJiao(void);
void AnswerReseetNet(void);
void AnawerSetUdp(void);
void AnswerExcuteList(void);
u8 CheckRecBuf(u8 *buf,u16 len);
void HandleCmd(u8 *buf,u16 len,u8 flag);
void AnswerCycleSec(void);
void AnswerGetIPDInfo(u8 addr);
void AnswerGetPkPowerInfo(u8 addr);
void AnswerSetDebugSocket(u8 cmd);
void SendDebugData(u32 ID,u8* buf,u8 len,u8 type);
void AnswerSet3_0Break(void); 
void AnswerCleanRecTimes(void);
void AnswerCleanNetCrc(void);

#endif
