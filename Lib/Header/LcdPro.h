#ifndef LCDPRO_H
#define LCDPRO_H

#include "Public.h"

typedef enum
{
    START,
    WAIT1,
    WAIT2,
    NORMAL,
    IDLE,
    MAIN_MENU,
    ADDRESS,
    ADDRESS_SET_ACK,
    ADDRESS_SET_RETURN,

    TIME,
    TIME_SET_ACK,
    TIME_SET_RETURN,

    RECORD,  
    RESET_FACTORY,
    RESET_SUB,  
}CDisLevel;

typedef struct
{
     u8  flag;
     u16 tick;
	u16 SecCnt;
}CDo;

void LoopDis(void);
void CheckHuiCaiTimes(void);
void WarnPro(void);


#endif
