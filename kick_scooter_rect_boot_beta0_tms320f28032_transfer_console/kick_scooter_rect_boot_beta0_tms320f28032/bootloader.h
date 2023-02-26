#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "DSP28x_Project.h"
#include "stdint.h"
#include "stddef.h"
#include "flash_programming_c28.h" // Flash API example header file
//#include "Flash2803x_API_Config.h"
//#include "Flash2803x_API_Library.h"
#endif
#ifdef TMS320F2802x
#include "f2803x_common/include/gpio.h"
#endif
#include "DSP28xx_SciUtil.h"
#define AUTH "REASONANCE_DEVICE_PVHOD_3.0_CORES_1\r\n"
#define AUTH_PASSWORD "reasonance@1REASONANCE@2"
#define AUTH_SUCCSESS "AUTHENTICATION SUCCESS"
#define BOOT_GPIO GPIO_Number_4

#define RESET 0x0F
#define ERASE 0x300
#define VERIFY 0x500
#define RUN 0x010
#define PROGRAMM 0x100

//#define TMS320F228037 1
#define TMS320F2803x 1
#define REPEAT_TIMEOUT 100//3
#ifdef TMS320F228037
    #define ENTRY_POINT 0x003F0000
    #define BEGIN 0x1F8000//x3F7000 // for TMS320f28037
#else if TMS320F2803x
#define ENTRY_POINT  0x3F3FF0//0x003F0000
#define BEGIN 0x1F8000//x3F7000 // for TMS320f28037
#endif

#define BOOT_REQUEST 0x08AA
#define PROG_BUFFER_LENGTH 0x010

#define SEND_TIMEOUT 3 //100
void bootloader(char *snd, uint16_t len);
char * erase(char * status);
char * verify(void);
uint16_t timeoutCheck(uint16_t state, uint16_t tmt);
//extern FLASH_ST FlashStatus;

uint16_t sciReadWord(uint16_t * err_flag);

long sciReadLongWord(uint16_t * err_flag);
uint16_t wait_key(void);
void start_programm(void);
inline void simpleDelay(long tmt);
uint16_t check_flash(void);
void toggle_LEDS(void);
#pragma CODE_SECTION(start_programm, "ramfuncs");
//extern Uint16 progBuf[PROG_BUFFER_LENGTH];
typedef struct
 {
     Uint16 BlockSize;
     Uint32 DestAddr;
     Uint32 ProgBuffAddr;
 } HEADER ;
typedef struct
{
    char ssid[29];
    int rssi;
    uint16_t postfix[2];
}STRUCT_AP;
#ifdef TMS320F2802x
extern GPIO_Handle myGpio;
#endif

inline uint16_t check_boot_tools(void)
{
    uint16_t flag = 0;
    return flag;
}
inline uint16_t check_hardware_boot_request(void)
{
    uint16_t flag = 0;
#ifdef TMS320F2802x
    flag = GPIO_getData(myGpio, GPIO_Number_4);
#endif
    return flag;
}

inline void send_ack(uint16_t cmd)
{
    char buff[2];
    buff[0] = cmd;
    buff[1] = cmd >> 8;
    scia_xmit(buff, 2, 1);
}
inline void simpleDelay(long tmt)
{
    long i = 0;
    for (i = 0; i < tmt; ++i);
}
