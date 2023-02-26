

/**
 * main.c
 */
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#ifdef TMS32028

#include "f2803x_common/include/clk.h"
#include "f2803x_common/include/flash.h"
#include "f2803x_common/include/gpio.h"
#include "f2803x_common/include/pie.h"
#include "f2803x_common/include/pll.h"
#include "f2803x_common/include/timer.h"
#include "f2803x_common/include/wdog.h"
//#include "f2803x_common/include/sci.h"
#include "f2803x_common/include/adc.h"
#include "f2803x_common/include/osc.h"

#endif
#include "f2803x_common/include/DSP2803x_GlobalPrototypes.h"
#include "DSP2803x_Examples.h"   // Examples Include File
#include "f2803x_common/include/esp8266.h"
#include "bootloader.h"
#include "DSP28xx_SciUtil.h"
#include "stdint.h"
#include "stdio.h"
#include <string.h>
#include "f2803x_common/include/at_commands.h"
#include "Flash2803x_API_Config.h"
#include "Flash2803x_API_Library.h"


//#include "F2803x_Gpio.h"

//#define _DEBUG 0
//#define USER_DEBUG 1
ESP8266 esp8266;


__interrupt void cpu_scia_rx_isr(void);
__interrupt void cpu_scia_tx_isr(void);
void initPerirherals(void);
void initWiFi(void);
void atCommandManager(uint16_t command);
void atCommandSend(char * str, uint16_t len);
void Example_MemCopy(Uint16 * SourceAddr, Uint16 * SourceEndAddr, Uint16 * DestAddr);
inline void start_wdg(void);
inline uint16_t findAP(void);
inline uint16_t connectAP(void);
inline void debug_console(void);
inline const char * calculate_password (uint16_t * key);
inline uint16_t char_to_number(char * ch, uint16_t l);
Uint32 command_manager(void);
void sciReadBuff(void);
__TI_pprof_out_hndl = ENTRY_POINT;
#ifdef TMS320F2802x
FLASH_Handle myFlash;
GPIO_Handle myGpio;
PIE_Handle myPie;
OSC_Obj myOsc;
TIMER_Handle myTimer0;
CPU_Handle myCpu;
ADC_Handle myAdc;
WDOG_Handle myWDog;
PLL_Handle myPll;
CLK_Handle myClk;
#endif
extern volatile struct GPIO_DATA_REGS GpioDataRegs;
extern volatile struct GPIO_CTRL_REGS GpioCtrlRegs;
extern int scia_rcv(unsigned int *rcvBuff, int buffLen, int loopMode, int typeLen);

extern Uint16 Flash28_API_LoadStart;
extern Uint16 Flash28_API_LoadEnd;
extern Uint16 Flash28_API_RunStart;

uint16_t stringSeeker(const char * str_src, const char * str_tg, uint16_t len_1, uint16_t len_2, uint16_t * num);

AT_COMMAND atCommand_tx, atCommand_rx;
uint16_t at_state = 0;
STRUCT_AP boot_ap;
char connect_at[53];
inline uint16_t authorisation(void);
void (*ApplicationPtr) (void);
extern uint16_t RamfuncsRunStart;
extern uint16_t RamfuncsLoadStart;
extern uint16_t RamfuncsLoadSize;

Uint32 main(void)
{
    Uint32 entry;
    Uint32 EntryAddr;
#ifdef TMS320F2802x
    DisableDog();
    IntOsc1Sel();
    InitPll(DSP28_PLLCR,DSP28_DIVSEL);
    //Example_MemCopy(&Flash28_API_LoadStart, &Flash28_API_LoadStart, &Flash28_API_RunStart);
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
    InitFlash();
    DELAY_US(100);

    myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
    myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
    myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
    myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
    myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
    myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
    myTimer0 = TIMER_init((void *)TIMER0_BASE_ADDR, sizeof(TIMER_Obj));
    myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));
    myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Handle));

    PIE_disable(myPie);
    PIE_disableAllInts(myPie);
    CPU_disableGlobalInts(myCpu);
    CPU_clearIntFlags(myCpu);

    CLK_enableSciaClock(myClk);
    initPerirherals();

    PIE_enable(myPie);
    CPU_enableGlobalInts(myCpu);
    CPU_enableDebugInt(myCpu);
    //CPU_enableInt(myCpu, CPU_IntNumber_9);
    EINT;                                /* Enable Global interrupt INTM*/
    ERTM;
#else
       DisableDog();
       InitSysCtrl(); //PLL activates
       InitGpio();
       DINT;
       IER = 0x0000;
       IFR = 0x0000;
       memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
       InitFlash();
       initPerirherals();
#endif

    initWiFi(); //Base initialisation ESP8266

    debug_console();
#ifdef TMS320F2802x
    if(((SysCtrlRegs.WDCR) && 0x0040) == 1) //CHECK WATCHDOG RESET
#else if TMS320F2803x
    if(SysCtrlRegs.WDCR == 0x0C0) //elbow
#endif
    {
        uint16_t flag = 0;
        DisableDog();
        flag = check_flash();
        if(flag == 1)//WAS !flag
        {
            flag = check_hardware_boot_request();
            if(!flag)
            {
                flag = check_boot_tools();
                if(!flag)
                {
                    start_wdg(); // test
                    start_programm();
                }
            }
        }
        else ///////////////////TEST
        {
            DisableDog(); //test
            EntryAddr = ENTRY_POINT;
            asm(" LC 0x3F3FF0");
            return ENTRY_POINT;
        }/////////////////////////////
    }
    if(findAP())
    {
        DisableDog();
        debug_console();
        if(connectAP())
        {
            DisableDog();
            entry =  command_manager();
        }
    }
    asm(" LC 0x3F3FF0");
   return ENTRY_POINT;
}
__interrupt void cpu_scia_rx_isr(void)
{
#ifdef TMS320F2802x
    CPU_disableInt(myCpu, CPU_IntNumber_9);
#endif
    sciReadBuff();
#ifdef TMS320F2802x
    CPU_enableInt(myCpu, CPU_IntNumber_9);
#endif
    EALLOW;
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;
    EDIS;
#ifdef TMS320F2802x
    PIE_clearInt(myPie, PIE_GroupNumber_9);
#endif
    //return;
}

__interrupt void cpu_scia_tx_isr(void)
{
  return;
}
void initPerirherals(void)
{
#ifdef TMS320F2802x
       GPIO_setPullUp(myGpio, GPIO_Number_0, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_0, GPIO_0_Mode_GeneralPurpose); //GREEN_LED
       GPIO_setDirection(myGpio, GPIO_Number_0, GPIO_Direction_Output);
       GPIO_setPullUp(myGpio, GPIO_Number_1, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_1, GPIO_1_Mode_GeneralPurpose); //GREEN_RED
       GPIO_setDirection(myGpio, GPIO_Number_1, GPIO_Direction_Output);

       GPIO_setPullUp(myGpio, GPIO_Number_2, GPIO_PullUp_Enable);
       GPIO_setMode(myGpio, GPIO_Number_2, GPIO_2_Mode_GeneralPurpose); //log_DCDC_On
       GPIO_setDirection(myGpio, GPIO_Number_2, GPIO_Direction_Output);

       GPIO_setPullUp(myGpio, GPIO_Number_3, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_3, GPIO_3_Mode_GeneralPurpose); //log_DQ
       GPIO_setDirection(myGpio, GPIO_Number_3, GPIO_Direction_Output);

       GPIO_setPullUp(myGpio, GPIO_Number_4, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_4, GPIO_4_Mode_GeneralPurpose); //log_GPIO4
       GPIO_setDirection(myGpio, GPIO_Number_4, GPIO_Direction_Input);

       GPIO_setPullUp(myGpio, GPIO_Number_5, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_5, GPIO_5_Mode_GeneralPurpose); //log_GPIO5
       GPIO_setDirection(myGpio, GPIO_Number_5, GPIO_Direction_Input);

       GPIO_setPullUp(myGpio, GPIO_Number_6, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_6, GPIO_6_Mode_GeneralPurpose); //log_OC_R
       GPIO_setDirection(myGpio, GPIO_Number_6, GPIO_Direction_Input);

       GPIO_setMode(myGpio, GPIO_Number_7, GPIO_7_Mode_SCIRXDA); //WiFi_RX
       GPIO_setMode(myGpio, GPIO_Number_12, GPIO_12_Mode_SCITXDA); //WiFi_TX
       GPIO_setPullUp(myGpio, GPIO_Number_7, GPIO_PullUp_Enable); //WiFi_RX
       GPIO_setPullUp(myGpio, GPIO_Number_12, GPIO_PullUp_Enable); //WiFi_TX

       GPIO_setPullUp(myGpio, GPIO_Number_16, GPIO_PullUp_Enable);
       GPIO_setMode(myGpio, GPIO_Number_16, GPIO_16_Mode_GeneralPurpose); //WiFi_GPIO2
       GPIO_setDirection(myGpio, GPIO_Number_16, GPIO_Direction_Output);
       GPIO_setHigh(myGpio, GPIO_Number_16);

       GPIO_setPullUp(myGpio, GPIO_Number_17, GPIO_PullUp_Enable);
       GPIO_setMode(myGpio, GPIO_Number_17, GPIO_16_Mode_GeneralPurpose); //WiFi_RST
       GPIO_setDirection(myGpio, GPIO_Number_17, GPIO_Direction_Output);
       GPIO_setPullUp(myGpio, GPIO_Number_18, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_18, GPIO_18_Mode_GeneralPurpose); //Log_0T_Int_R
       GPIO_setDirection(myGpio, GPIO_Number_18, GPIO_Direction_Output);
       GPIO_setLow(myGpio, GPIO_Number_18);
       GPIO_setPullUp(myGpio, GPIO_Number_19, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_19, GPIO_19_Mode_GeneralPurpose); //Log_0T_Ext_R
       GPIO_setDirection(myGpio, GPIO_Number_19, GPIO_Direction_Output);
       GPIO_setLow(myGpio, GPIO_Number_19);
       GPIO_setDirection(myGpio, GPIO_Number_19, GPIO_Direction_Input);
       GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_GeneralPurpose); //Log_GPIO28
       GPIO_setDirection(myGpio, GPIO_Number_28, GPIO_Direction_Input);
       GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_GeneralPurpose); //Log_GPIO29
       GPIO_setDirection(myGpio, GPIO_Number_29, GPIO_Direction_Output);
       GPIO_setLow(myGpio, GPIO_Number_29);
       GPIO_setDirection(myGpio, GPIO_Number_29, GPIO_Direction_Input);
       GPIO_setPullUp(myGpio, GPIO_Number_34, GPIO_PullUp_Disable);
       GPIO_setMode(myGpio, GPIO_Number_34, GPIO_34_Mode_GeneralPurpose); //Log_0V_R
       GPIO_setDirection(myGpio, GPIO_Number_34, GPIO_Direction_Output);
       GPIO_setLow(myGpio, GPIO_Number_34);
       GPIO_setDirection(myGpio, GPIO_Number_34, GPIO_Direction_Input);
     //  PIE_setDebugIntVectorTable(myPie);
       PIE_enable(myPie);

       EALLOW;            // This is needed to write to EALLOW protected registers
          // PIE_registerPieIntHandler(myPie, PIE_GroupNumber_1, PIE_SubGroupNumber_7, (intVec_t)&cpu_timer0_isr);
             PIE_registerPieIntHandler(myPie, PIE_GroupNumber_9, PIE_SubGroupNumber_1, (intVec_t)&cpu_scia_rx_isr);
         //  PIE_registerPieIntHandler(myPie, PIE_GroupNumber_10, PIE_SubGroupNumber_1, (intVec_t)&adc_isr);
             PIE_registerPieIntHandler(myPie, PIE_GroupNumber_9, PIE_SubGroupNumber_2, (intVec_t)&cpu_scia_tx_isr);
            // PIE_enableInt(myPie, PIE_GroupNumber_9,  PIE_InterruptSource_SCIARX);
            // PIE_enableInt(myPie, PIE_GroupNumber_9,  PIE_InterruptSource_SCIATX);
       EDIS;    // This is needed to disable write to EALLOW protected registers
#else if TMS320F2803x
      EALLOW;
      GpioCtrlRegs.GPADIR.bit.GPIO0 |= 1;//  Log_0V_Vin_R - output
      GpioCtrlRegs.GPAPUD.bit.GPIO0 &= 0;//  Log_0V_Vin_R - enable pull up
      GpioCtrlRegs.GPADIR.bit.GPIO1 |= 1;//  Log_OC_Iboost_R - output
      GpioCtrlRegs.GPAPUD.bit.GPIO1 &= 0;//  Log_0V_Vin_R - enable pull up

      GpioCtrlRegs.GPADIR.bit.GPIO10 |= 1; //Log_LED_Red_On - output
      GpioCtrlRegs.GPAPUD.bit.GPIO10 &= 0; //Log_LED_Red_On - enable pull up
      GpioCtrlRegs.GPADIR.bit.GPIO11 = 1; //Log_LED_Green_On - output
      GpioCtrlRegs.GPAPUD.bit.GPIO11 &= 0; //Log_LED_Green_On - enable pull up

      GpioCtrlRegs.GPADIR.bit.GPIO7 &= 0;//   Wifi_RX - input
      GpioCtrlRegs.GPAPUD.bit.GPIO7 &= 0;//   Wifi_RX - enable pull up
      GpioCtrlRegs.GPADIR.bit.GPIO12 |= 1;//  Wifi_RX - output
      GpioCtrlRegs.GPAPUD.bit.GPIO12 &= 0;//  Wifi_RX - enable pull up


      GpioCtrlRegs.GPAMUX1.bit.GPIO7 |= 2;// ASSIGN TO SCIA
      GpioCtrlRegs.GPAMUX1.bit.GPIO12 |= 2;// ASSIGN TO SCIA
      EDIS;
#endif
       EALLOW;
             SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;
             SciaRegs.SCICCR.bit.STOPBITS = 0;  /*Number of stop bits. (0: One stop bit, 1: Two stop bits)*/
             SciaRegs.SCICCR.bit.PARITY = 0;/*Parity mode (0: Odd parity, 1: Even parity)*/
             SciaRegs.SCICCR.bit.PARITYENA = 0;   /*Enable Pary Mode */
             SciaRegs.SCICCR.bit.LOOPBKENA = 0;   /*Loop Back enable*/
             SciaRegs.SCICCR.bit.ADDRIDLE_MODE = 0;/*ADDR/IDLE Mode control*/
             SciaRegs.SCICCR.bit.SCICHAR = 7;     /*Character length*/
             SciaRegs.SCICTL1.bit.RXERRINTENA = 0;/*Disable receive error interrupt*/
             SciaRegs.SCICTL1.bit.SWRESET = 1;    /*Software reset*/
             SciaRegs.SCICTL1.bit.TXENA = 1;      /* SCI transmitter enable*/
             SciaRegs.SCICTL1.bit.RXENA = 1;      /* SCI receiver enable*/
             SciaRegs.SCIHBAUD = 0U;//0U;//10U;
             SciaRegs.SCILBAUD = 15U;//43U;

             /*Free run, continue SCI operation regardless of suspend*/
             SciaRegs.SCIPRI.bit.FREE = 3;
             SciaRegs.SCIFFCT.bit.ABDCLR = 0;
             SciaRegs.SCIFFCT.bit.CDC = 0;
             SciaRegs.SCIFFTX.bit.SCIRST = 1;     /* SCI reset rx/tx channels*/
             SciaRegs.SCIFFTX.bit.SCIFFENA = 1;   /* SCI FIFO enhancements are enabled.*/
             SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;/* Re-enable transmit FIFO operation.*/
             SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;/* Re-enable receive FIFO operation.*/
             SciaRegs.SCIFFRX.bit.RXFFIL = 1;
             SciaRegs.SCICTL1.bit.RXERRINTENA = 1;
             SciaRegs.SCIPRI.bit.SOFT = 3;
             /*Enable receive error interrupt by default if receive FIFO interrupt is enabled*/
             EDIS;
}
void initWiFi(void)
{
    uint16_t init = 1;
    uint16_t reset_flag = 1;
    EALLOW;
    GpioCtrlRegs.GPAMUX1.all &= 0xFFFFFCFF;
    GpioCtrlRegs.GPADIR.bit.GPIO5 |= 1; //WIFI_RST
    GpioCtrlRegs.GPAPUD.bit.GPIO5 &= 0; //WIFI_RST PULL - UP;
    EDIS;
    esp8266.config_status = ESP8266_IS_NOT_CONFIGURED;
    esp8266.s_mode = SERIAL_MODE_AT;
    esp8266.c_status = ESP8266_BUSY;//ESP8266_IDLE;
    esp8266.reset_counter = 0;
    memset(&(esp8266.AT_rx_buff), 0, sizeof(&esp8266.AT_rx_buff));
    memset(&(esp8266.AT_tx_buff), 0, sizeof(&esp8266.AT_tx_buff));
    esp8266.first_reqest = 0;
    esp8266.reset_status = 0;
    atCommand_tx.len = 4;
    memcpy(&(atCommand_tx.str[0]), AT(0), 4);
    atCommand_rx.len = 4;
    atCommand_tx.console = 1;

    while(reset_flag) // HARDWARE RESET ESP8266
    {
        if (reset_flag)
        {
               if((esp8266.reset_counter) < 2000)
               {
#ifdef TMS320F2802x
                   GPIO_setLow(myGpio, GPIO_Number_17);
#else if TMS320F2803x
                   GpioDataRegs.GPACLEAR.bit.GPIO5 = 1;
#endif
                   esp8266.c_status = ESP8266_BUSY;
                   ++(esp8266.reset_counter);
               }
               else
               {
                   reset_flag = 0;
#ifdef TMS320F2802x
                   GPIO_setHigh(myGpio, GPIO_Number_17);
#else if TMS320F2803x
                   GpioDataRegs.GPASET.bit.GPIO5 = 1;
#endif
                   esp8266.c_status = ESP8266_IDLE; // CHANGE TO "ESP8266_BUSY" FOR THE RECONFIG
                   esp8266.reset_counter = 0;
                   break;
               }
         }
    }
    while(init)
    {
        const char *s1;
        const char *s_at = "AT+CWMODE_CUR=1\r\n\r\nOK";
        uint16_t n = 0;
        toggle_LEDS();
        sciReadBuff();
        s1 = (char*) (esp8266.AT_rx_buff);
        if(!stringSeeker(s1,s_at,100,21, &n))
        {
            atCommandManager((int)AT_CWMODE_STA(NUM));
            atCommand_tx.tx_flag = 1;
            esp8266.c_status = ESP8266_CWMODE_STA_TX;
        }
        else
        {
            init = 0;
            esp8266.c_status = ESP8266_CWMODE_STA_OK;
        }
    }
}
void atCommandManager(uint16_t command)
{
    switch(command)
    {
        case ((int)AT(NUM)):
        {
           atCommand_tx.len = AT(LEN);
           memcpy(&(atCommand_tx.str[0]), AT(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_UART_Q(NUM)):
        {
            atCommand_tx.len = AT_UART_Q(LEN);
            memcpy(&(atCommand_tx.str[0]), AT_UART_Q(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CWMODE_STA(NUM)):
        {
            atCommand_tx.len = AT_CWMODE_STA(LEN);
            memcpy(&(atCommand_tx.str[0]), AT_CWMODE_STA(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CIPSTART_UDP(NUM)):
        {
            atCommand_tx.len = AT_CIPSTART_UDP(LEN);
            memcpy(&(atCommand_tx.str[0]), AT_CIPSTART_UDP(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CIPSTART_TCP(NUM)):
        {
            atCommand_tx.len = AT_CIPSTART_TCP(LEN);
            memcpy(&(atCommand_tx.str[0]), AT_CIPSTART_TCP(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CWLAP_SET(NUM)):
        {
             atCommand_tx.len = AT_CWLAP_SET(LEN);
             memcpy(&(atCommand_tx.str[0]), AT_CWLAP_SET(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CWLAP_EXE(NUM)):
        {
            atCommand_tx.len = AT_CWLAP_EXE(LEN);
            memcpy(&(atCommand_tx.str[0]), AT_CWLAP_EXE(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CWLAPOPT(NUM)):
        {
             atCommand_tx.len = AT_CWLAPOPT(LEN);
             memcpy(&(atCommand_tx.str[0]), AT_CWLAPOPT(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CWQAP(NUM)):
        {
             atCommand_tx.len = AT_CWQAP(LEN);
             memcpy(&(atCommand_tx.str[0]), AT_CWQAP(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CIPMODE_TR(NUM)):
        {
             atCommand_tx.len = AT_CIPMODE_TR(LEN);
             memcpy(&(atCommand_tx.str[0]), AT_CIPMODE_TR(STR), atCommand_tx.len);
        }
        break;
        case ((int) AT_CIPSEND_TR(NUM)):
        {
             atCommand_tx.len = AT_CIPSEND_TR(LEN);
             memcpy(&(atCommand_tx.str[0]), AT_CIPSEND_TR(STR), atCommand_tx.len);
        }
        break;
        default : break;
    }
    if(atCommand_tx.tx_flag)
    {
        uint16_t len = atCommand_tx.len;
        atCommand_tx.tx_flag = 0;
        atCommandSend(&(atCommand_tx.str[0]), len);
    }
}
void atCommandSend(char * str, uint16_t len)
{
    scia_xmit(str, len, 1);
}
uint16_t stringSeeker(const char * str_src, const char * str_tg, uint16_t len_1, uint16_t len_2, uint16_t * num)
{
    const char *cut_string;
    uint16_t i = 0;
    for(i = 0; i < len_1; i++ )
    {
        cut_string = &str_src[i];
        if(!strncmp(cut_string, str_tg, len_2))
        {
            *num = i;
            return 1;
        }
    }
    return 0;
}
inline void start_wdg(void)
{
#ifdef TMS320F2802x
    WDOG_setPreScaler(myWDog, WDOG_PreScaler_OscClk_by_512_by_64);
    WDOG_enableOverRide(myWDog);
    WDOG_enable(myWDog);
#else if TMS320F2803x
    uint16_t regValue = SysCtrlRegs.WDCR;
    regValue &= (~(1 << 6));
    ServiceDog();
    EALLOW;
    SysCtrlRegs.WDCR = 0x002F;
    EDIS;
#endif
}
void Example_MemCopy(Uint16 * SourceAddr, Uint16 * SourceEndAddr, Uint16 * DestAddr)
{
    while(SourceAddr < SourceEndAddr)
    {
        *DestAddr++ = *SourceAddr++;
    }
    return;
}
inline uint16_t findAP(void)
{
    uint16_t ap = 1;
    uint16_t flag = 0;

    while(ap)
    {
            const char *s1;
            const char *s_at = "AT+CWLAPOPT=1,7\r\n\r\nOK";
            uint16_t n = 0;
#ifdef TMS320F2802x
            GPIO_toggle(myGpio,GPIO_Number_0);
#else if TMS320F2803x
#endif
            sciReadBuff();
            s1 = (char*) (esp8266.AT_rx_buff);
            if(!stringSeeker(s1,s_at,100,21, &n))
            {
                atCommandManager((int)AT_CWLAPOPT(NUM));
                atCommand_tx.tx_flag = 1;
                esp8266.c_status = ESP8266_CWLAPOPT_TX;
            }
       else
       {
        ap = 0;
        esp8266.c_status = ESP8266_CWLAPOPT_OK;
       }
#ifdef TMS320F2802x
       GPIO_toggle(myGpio,GPIO_Number_1);
#else if TMS320F2803x
#endif
    }
    ap = 1;
    start_wdg(); // TEST
    //ServiceDog();
    while(ap)
    {
        const char *s1;
        const char *s_at = "ReasonanceUpdateFirmware";
        uint16_t n = 0;
        static uint16_t service_counter = 0;
        if(service_counter < 14)
        {
           ServiceDog();
           ++service_counter;
        }
        toggle_LEDS();
        sciReadBuff();
        s1 = (char*) (esp8266.AT_rx_buff);
        if(!stringSeeker(s1,s_at,100,24, &n))
        {
            atCommandManager((int)AT_CWLAP_EXE(NUM));
            atCommand_tx.tx_flag = 1;
            esp8266.c_status = ESP8266_CWLAP_EXE_TX;
        }
        else
        {
            ap = 0;
            flag = 1;
            memcpy(&(boot_ap.ssid[0]),&(esp8266.AT_rx_buff[n]), 29);
            {
                   char rssi[4];
                   memcpy(&(rssi[0]),&esp8266.AT_rx_buff[n + 28 + 3], sizeof(rssi));
                   if(rssi[0] == '-')
                   {
                       boot_ap.rssi = -1 * (int) char_to_number(&rssi[1], 2);
                   }
                   else
                   {
                       boot_ap.rssi =  (int) char_to_number(&rssi[0], 2);
                   }
                   boot_ap.postfix[0] = char_to_number(&esp8266.AT_rx_buff[n + 25], 2);
                   boot_ap.postfix[1] = char_to_number(&esp8266.AT_rx_buff[n + 28], 1);
            }
            esp8266.c_status = ESP8266_CWLAP_EXE_OK;
        }
        toggle_LEDS();
    }
    return flag;
}
inline uint16_t connectAP(void)
{
    uint16_t flag = 0;
    uint16_t ap = 1;
    const char *s1;
    const char *s_at = "GOT";//"CONNECTED\r\nWIFI GOT IP\r\n";
    const char * at_cwjap = "AT+CWJAP=\" ";
    const char * ssid;
    ssid = boot_ap.ssid;
    const char * pswd = calculate_password(boot_ap.postfix);
    const char * mid1 = "\",\" ";
    const char * mid2 = "\"\r\n ";
    while(ap)  //disconnect from any AP
    {
        const char *s1;
        const char *s_at = "AT+CWQAP\r\n\r\nOK";
        uint16_t n = 0;
        toggle_LEDS();
        sciReadBuff();
        s1 = (char*) (esp8266.AT_rx_buff);
        if(!stringSeeker(s1,s_at,144,14, &n))
        {
            atCommandManager((int)AT_CWQAP(NUM));
            atCommand_tx.tx_flag = 1;
            esp8266.c_status = ESP8266_CWQAP_TX;
        }
        else
        {
           ap = 0;
           esp8266.c_status = ESP8266_CWQAP_OK;
        }
        toggle_LEDS();
    }
    ap = 1;
    strncpy(&(connect_at[0]),at_cwjap,11);
    strncpy(&(connect_at[10]),ssid,29);
    strncpy(&(connect_at[39]),mid1,3);
    strncpy(&(connect_at[42]),pswd,8);
    strncpy(&(connect_at[50]),mid2,3);
    memset(&(esp8266.AT_rx_buff[0]),0,SCI_RX_SIZE);
    while(ap) // connect to finded AP
    {
        uint16_t n = 0;
        toggle_LEDS();
        sciReadBuff();
        s1 = (char*) (esp8266.AT_rx_buff);
        if(!stringSeeker(s1,s_at,100,3, &n))
        {
            {
               uint16_t len = 53;
               memcpy(&(atCommand_tx.str[0]), &connect_at[0], len);
               atCommandSend(&(atCommand_tx.str[0]), len);
               DELAY_US(10);
            }
            esp8266.c_status = ESP8266_CWJAP_TX;
        }
        else
        {
            ap = 0;
            esp8266.c_status = ESP8266_CWJAP_OK;
            flag = 1;
        }
    }
    ap = 1;
    while(ap) //start TCP lient
    {
        const char *s1;
        const char *s_at = "CONNECT\r\n\r\nOK";
        const char *s_at_al = "ALREADY";
        const char *s_at_busy = "busy";
        uint16_t n = 0;
        toggle_LEDS();
        sciReadBuff();
        s1 = (char*) (esp8266.AT_rx_buff);
        if(((!stringSeeker(s1,s_at,100,13, &n)) && (!stringSeeker(s1,s_at_al,100,7, &n))))
        {
                {
                    atCommandManager((int)AT_CIPSTART_TCP(NUM));
                    atCommand_tx.tx_flag = 1;
                }
                esp8266.c_status = ESP8266_CIPSTART_TCP_TX;
        }
        else
        {
           ap = 0;
           esp8266.c_status = ESP8266_CIPSTART_TCP_OK;
        }
    }
     ap = 1;
     memset(&(esp8266.AT_rx_buff[0]),0,SCI_RX_SIZE);
     while(ap) //start transprent mode
     {
           const char *s1;
           const char *s_at = "OK";
           printf( "%s", s_at);
           uint16_t n = 0;
           toggle_LEDS();
           sciReadBuff();
           s1 = (char*) (esp8266.AT_rx_buff);
           if(!stringSeeker(s1,s_at,100,2, &n))
           {
               {
                    atCommandManager((int)AT_CIPMODE_TR(NUM));
                    atCommand_tx.tx_flag = 1;
               }
               esp8266.c_status = ESP8266_CIPMODE_TX;
           }
           else
           {
               ap = 0;
               esp8266.c_status = ESP8266_CIPMODE_OK;
           }
    }
     ap = 1;
     memset(&(esp8266.AT_rx_buff[0]),0,SCI_RX_SIZE);
     while(ap)
     {
            const char *s1;
            const char *s_at = "OK\r\n\r\n>";
            uint16_t n = 0;
            toggle_LEDS();
            sciReadBuff();
            s1 = (char*) (esp8266.AT_rx_buff);
            if(!stringSeeker(s1,s_at,100,7, &n))
            {
               {
                    atCommandManager((int)AT_CIPSEND_TR(NUM));
                    atCommand_tx.tx_flag = 1;
               }
              esp8266.c_status = ESP8266_CIPSEND_TX;
            }
            else
            {
                ap = 0;
                esp8266.c_status = ESP8266_CIPSEND_OK;
            }
     }
     if(authorisation())// WAIT AUTHORISATION 3 TIMES (NOT REALESAED)
     {
         const char * s_at = AUTH_SUCCSESS;
         uint16_t len = 22;
         memcpy(&(atCommand_tx.str[0]), s_at, len);
         atCommandSend(&(atCommand_tx.str[0]), len);
         DELAY_US(10000);
         flag = 1;
     }
    return flag;
}
inline const char * calculate_password (uint16_t * key)
{
    const char * pswd = "100@101@ ";

    return pswd;
}
inline uint16_t char_to_number(char * ch, uint16_t l)
{
    uint16_t num = 0;
    switch(l)
    {
        case 1:
        {
            num = ch[0] - 48;
        }
        break;
        case 2:
        {
            num = 10*(ch[0] - 48) + ch[1] - 48;
        }
        break;
        case 3:
        {
             num = 100*(ch[0] - 48) + 10*(ch[1] - 48) + (ch[2] - 48);
        }
        break;
        default : break;
    }
    return num;
}
inline uint16_t authorisation(void)
{
    uint16_t flag = 0;
    uint16_t cnt = 0;
    uint16_t ap = 1;
    const char *s1;
    const char *s_at = AUTH;
    const char *s_psw = AUTH_PASSWORD;
    while(ap) // connect to finded AP
       {
           uint16_t n = 0;
           toggle_LEDS();
           sciReadBuff();
           s1 = (char*) (esp8266.AT_rx_buff);
           if (cnt >= 20) break;                       //test, need to be 3
           if(!stringSeeker(s1,s_psw,100,24, &n))
           {
               ++cnt;
               {
                  uint16_t len = 38;
                  memcpy(&(atCommand_tx.str[0]), s_at, len);
                  atCommandSend(&(atCommand_tx.str[0]), len);
                  //DELAY_US(100000); // for debug
               }
               esp8266.c_status = ESP8266_AUTH_TX;
           }
           else
           {
               ap = 0;
               esp8266.c_status = ESP8266_AUTH_OK;
               flag = 1;
           }
       }
    return flag;
}
void sciReadBuff(void)
{
       unsigned int recbuff[SCI_RX_SIZE];
       int errFlg = NOERROR;
       {
          int i;
          for (i = 0; i < SCI_RX_SIZE; i++)
          {
            recbuff[i] = 0;
          }
          errFlg = scia_rcv(recbuff, SCI_RX_SIZE, LONGLOOP, 1);
          if(errFlg != TIMEOUT)
          {
              memcpy( &(esp8266.AT_rx_buff[0]), recbuff, SCI_RX_SIZE);
          }
       }
       return;
}
Uint32 command_manager(void)
{
    Uint32 entry;
    uint16_t err = 0;
    while(1)
    {
        toggle_LEDS();
        switch(sciReadWord(&err))
        {
             case 0: break;
             case RESET:
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x80;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
                 start_wdg();
             }
             break;
             case ERASE :
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x81;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
                 atCommandSend(erase(&(atCommand_tx.str[0])), 2);
             }
             break;
             case VERIFY :
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x82;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
                 atCommandSend(verify(), 2);
             }
             break;
             case RUN :
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x83;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
                 entry = ENTRY_POINT;
                 return (entry);//(2*BEGIN); //TEST ELBOW
             }
             break;
             case PROGRAMM :
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x84;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
                 bootloader(&(atCommand_tx.str[0]),2);
                 DELAY_US(SEND_TIMEOUT);
                 atCommandSend(&(atCommand_tx.str[0]), 2);
             }
             break;
             default :
             {
                 atCommand_tx.str[1] = 0;
                 atCommand_tx.str[0] = 0x8085;
                 atCommandSend(&(atCommand_tx.str[0]), 2);
                 DELAY_US(SEND_TIMEOUT);
             }
             break;
        }
        toggle_LEDS();
    }
}
uint16_t check_flash(void)
{
    Uint16 flag = 0;
    FLASH_ST FlashStatus;
    uint16_t buff[8] = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x01};
    uint32_t *buff_32 = (uint32_t * )buff;
    flag = Flash_Verify((Uint16 *) 0x3F4FE0, &buff_32[0], sizeof(buff), &FlashStatus);
    return (uint16_t)flag;
}
inline void debug_console(void)
{
#ifdef USER_DEBUG
    uint16_t ap = 1;
    uint16_t len;
    const char *s1;
    const char *s_1310 = "\r\n";
    uint16_t n = 0;
    while(ap)
    {
        ap = atCommand_tx.console;
        GPIO_toggle(myGpio,GPIO_Number_0);
        sciReadBuff();
        if(atCommand_tx.tx_flag)
        {
            s1 = (char*) (&(atCommand_tx.str[0]));
            if(stringSeeker(s1,s_1310,100,2, &n))
            {
                len  = n + 2;
                atCommand_tx.tx_flag = 0;
                atCommandSend(&(atCommand_tx.str[0]), len);
            }
        }
    }
#endif
}

