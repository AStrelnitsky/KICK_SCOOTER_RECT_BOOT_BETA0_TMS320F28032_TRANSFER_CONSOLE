#include "bootloader.h"
#include "DSP28x_Project.h"
#ifdef TMS320F2802x
#include "f2803x_common/include/sci.h"
#include "f2803x_common/include/clk.h"
#include "f2803x_common/include/flash.h"
#include "f2803x_common/include/gpio.h"
#include "f2803x_common/include/pie.h"
#include "f2803x_common/include/pll.h"
#include "f2803x_common/include/timer.h"
#include "f2803x_common/include/wdog.h"
#include "f2803x_common/include/osc.h"
#include "f2803x_common/include/sci.h"
#endif
#include "f2803x_common/include/DSP2803x_GlobalPrototypes.h"

extern volatile struct SCI_REGS SciaRegs;
#include "math.h"
#include "Flash2803x_API_Config.h"
#include "Flash2803x_API_Library.h"
#include "DSP28xx_SciUtil.h"
#include "f2803x_common/include/esp8266.h"

#pragma CODE_SECTION(start_programm, "ramfuncs");

extern ESP8266 esp8266;
HEADER BlockHeader;
uint16_t sciReadWord(uint16_t * err_flag)
{
    uint16_t word = 0;
    int errFlg = 0;
    uint16_t buff[2];
    errFlg = scia_rcv(buff, 2, LONGLOOP, 1);
    if(errFlg != TIMEOUT)
    {
        word = buff[0] + (buff[1] << 8);
        *err_flag = 0;
    }
    else
    {
       // word = 0xDEAD;
        *err_flag = 1;
    }
    return word;
}
long sciReadLongWord(uint16_t * err_flag)
{
    uint16_t word = 0;
    long word_long = 0;
    int errFlg = 0;
    uint16_t buff[2];
    errFlg = scia_rcv(buff, 2, LONGLOOP, 1);
    if(errFlg != TIMEOUT)
    {
        word = buff[0] + (buff[1] << 8);
    }
    word_long = ((long)word) << 16;
    errFlg = scia_rcv(buff, 2, LONGLOOP, 1);
    if(errFlg != TIMEOUT)
    {
        word = buff[0] + (buff[1] << 8);
    }
    word_long += ((long)word);

    return word_long;
}
char * erase(char * status)
{
    uint16_t flag;
    FLASH_ST EraseStatus;
    flag = Flash_Erase(SECTORH, &EraseStatus);
    if(flag == STATUS_SUCCESS)
    {
        flag = Flash_Erase(SECTORG, &EraseStatus);
    }
    if(flag == STATUS_SUCCESS)
    {
        flag = Flash_Erase(SECTORF, &EraseStatus);
    }
    if(flag == STATUS_SUCCESS)
    {
         flag = Flash_Erase(SECTORE, &EraseStatus);
    }
    if(flag == STATUS_SUCCESS)
    {
         flag = Flash_Erase(SECTORD, &EraseStatus);
    }
    if(flag == STATUS_SUCCESS)
    {
        status[0] = 1;
        status[1] = 0;
    }
    else
    {
        status[0] = 0;
        status[1] = 0;
    }
    return status;
}
char * verify(void)
{
    char status[2];
    return status;
}
void bootloader(char * snd, uint16_t len)
{
    char status[2];
    char buff[2];
    long key;
    int ap = 1;
    uint16_t err = 0;
    while(ap)
    {
        uint16_t word = 0;
        int errFlg = 0;
        uint16_t buff[2];
        errFlg = scia_rcv(buff, 2, LONGLOOP, 1);
        if(errFlg != TIMEOUT)
        {
            word = buff[0] + (buff[1] << 8);
            ap = 0;
        }
        if(word == BOOT_REQUEST)
        {
            int i = 0;
            send_ack(0x00C0);//send_ack(0x87);
            for(i = 0; i < 8; ++i)
            {
                key = wait_key();
                    if( key == 0)
                    {
                        send_ack(0x00C1);//0x88
                    }
                    else
                    {
                        ap = 1;
                        send_ack(0x80C1);//0x88
                        break;
                    }
             }
            if(!ap)
            {
                if(sciReadLongWord(&err) == BEGIN)
                {
                    send_ack(0x00C2);//send_ack(0x8B);
                    start_programm();
                }
                else
                {
                    send_ack(0x80C2);
                    ap = 1;
                }
            }
        }
        else
        {
            status[1] = 80;
            status[0] = 0xC0;
        }
    }
    memcpy(snd,&status[0],len);
}
uint16_t wait_key(void)
{
    uint16_t key = 1;
    uint16_t buff[2];
    int errFlg = 0;
    errFlg = scia_rcv(buff, 2, LONGLOOP, 1);
    if(errFlg != TIMEOUT)
    {
       key = buff[0] + buff[1] << 8;
    }
    else
    {
       key = 0xFF;
    }
    return key;
}
void start_programm(void)
{
    FLASH_ST FlashStatus;
    Uint16 progBuf[PROG_BUFFER_LENGTH];
    uint16_t tmt = 1;
        Uint16 wordData;
        Uint16 status;
        Uint16 i,j;
        uint16_t err = 0;
       //  Make sure code security is disabled

        while(tmt)
        {
           BlockHeader.BlockSize = sciReadWord(&err);
           tmt = timeoutCheck(0x00C2, err);
           if(tmt > 1 )
           {
               return;
           }
        }
        tmt = 1;
        CsmUnlock(); // FOR TEST
        EALLOW;
        Flash_CPUScaleFactor = SCALE_FACTOR;
        Flash_CallbackPtr = NULL;
        EDIS;
        status = Flash_Erase(SECTORG, &FlashStatus); // FOR TEST
        status = Flash_Erase(SECTORF, &FlashStatus); // FOR TEST

        if(status != STATUS_SUCCESS)
        {
            return;
        }
        while(BlockHeader.BlockSize != (Uint16)0x0000)
        {
            send_ack(0x00C3);
            toggle_LEDS();
          //  {
               BlockHeader.DestAddr = sciReadLongWord(&err); //old
                if((BlockHeader.DestAddr != 0) && (err != 1))
                {
                    send_ack(0x00C5);
                }
                else
                {
                    send_ack(0x80C5);
                }
                BlockHeader.ProgBuffAddr = (Uint32)progBuf;
                for(i = 1; i <= BlockHeader.BlockSize; i++)
                {
                    while(tmt)
                    {
                        wordData = sciReadWord(&err); // inaccurate
                        tmt = timeoutCheck(0x00C5,  err);
                        if(tmt > 1 )
                        {
                            DELAY_US(SEND_TIMEOUT);
                           return;
                        }
                    }
                    tmt = 1;
                    *(Uint16 *)BlockHeader.ProgBuffAddr++ = wordData;
                }
                status = Flash_Program((Uint16 *) BlockHeader.DestAddr, (Uint16 *)progBuf, BlockHeader.BlockSize, &FlashStatus);
                if(status != STATUS_SUCCESS)
                {
                    status = Flash_Program((Uint16 *) BlockHeader.DestAddr, (Uint16 *)progBuf, BlockHeader.BlockSize, &FlashStatus);
                                   if(status != STATUS_SUCCESS)
                                   {
                                       send_ack(0x80C4);
                                       DELAY_US(SEND_TIMEOUT);
                                       return;
                                   }
                   // send_ack(0x80C4);
                   // return;
                }
                else
                {
                    send_ack(0x00C6);
                    DELAY_US(SEND_TIMEOUT);
                }
         //   }
            BlockHeader.BlockSize = sciReadWord(&err); //old
            toggle_LEDS();
        }

        if(status == STATUS_SUCCESS)
        {
            progBuf[0] = 0xA;
            progBuf[1] = 0xB;
            progBuf[2] = 0xC;
            progBuf[3] = 0xD;
            progBuf[4] = 0xE;
            progBuf[5] = 0xF;
            progBuf[6] = 0x0;
            progBuf[7] = 0x1;
            status = Flash_Program((Uint16 *) 0x3F4FE0, (Uint16 *)progBuf, 8, &FlashStatus);
        }
        send_ack(0x00C4);
        return ;
}
void toggle_LEDS(void)
{
#ifdef DUAL
#ifdef CPU1
    GpioDataRegs.GPATOGGLE.bit.GPIO18 = 1;
#else if CPU2
    GpioDataRegs.GPATOGGLE.bit.GPIO19 = 1;
#endif
#endif
#ifdef TMS320F2803x
    EALLOW;
    GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
    GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;
    EDIS;
#endif
}
uint16_t timeoutCheck(uint16_t state, uint16_t t)
{
    uint16_t st = 0;
    uint16_t ack = 0x8100;
    ack += state;
    static uint16_t r_cnt;
    if(t == 1)
    {
        send_ack(ack);
        if(r_cnt > REPEAT_TIMEOUT)
        {
            r_cnt = 0;
            ack = 0x8200;
            ack += state;
            send_ack(ack);
            return ack;
        }
        else
        {
            ++r_cnt;
            st = 1;
        }

    }
    return st;
}

