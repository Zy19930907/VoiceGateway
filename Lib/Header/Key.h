#ifndef KEY_H
#define KEY_H

#include <stm32f10x_lib.h>


#define KEY_IS_UNPRESS    0
#define KEY_IS_PRESS      1
#define KEY_IS_HOLD       2
#define KEY_IS_SPEED      4
#define KEY_IS_WAKEUP     5

typedef struct
{
     u8 Status;
     u8 HoldTimes;
     u16 Tick;
}CKey;

#define AUTO_BACK_TICK    50

// PA0 A1 A2 °´¼ü --- func  add  down

#define FUNC_KEY_PRESS  ((GPIOA ->IDR & 0x01) == 0x01)
#define ADD_KEY_PRESS   ((GPIOA ->IDR & 0x02) == 0x02)
#define DEL_KEY_PRESS   ((GPIOA ->IDR & 0x04) == 0x04)



u8   CheckDelKey(void);
u8   CheckAddKey(void);
u8   CheckFuncKey(void);
void KeyPro(void);

#endif

