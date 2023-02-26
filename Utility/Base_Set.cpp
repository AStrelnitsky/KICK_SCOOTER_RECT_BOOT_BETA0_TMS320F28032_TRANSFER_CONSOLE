#define OPEN_WATCOM_DIRTY_HACK
#include "Base_Set.rc"
#endif
//#include <windows.h> уже включено в грязном хаке
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <math.h>

#include <commctrl.h>   //Трэкбары (слайдеры)

#include "DiaWrap.h"
#include "Protocol.h"

//HINSTANCE hinst;

FWAPI *DialogEngine=NULL, *NetEngine=NULL;
int MyDialogID=0;
HGLOBAL DiaTemplate;
int* IncomingBuffer=NULL;

fstream Log;
int DoLog=0, DoWarning=0;
unsigned long TrakDeadTime;     //после любой отправки настоек система должна быть какую-то долю секунды глуха к ползункам.

char GraphArea[768*256*3]={0};

enum    //Конкретно регистры. U32 указываются как младший-старший, U8 указываются как поле1_поле2.
{
TIME_STAMP,
FREQUANCY,
WAVEFORM,
PWM,
I_U,
MEAN_OUT_ILIM,
PULSE_OUT_ILIM,
VOUT_LIM,
VIN_OV_LIM,
VIN_UV_LIM,
IIN_OC_LIM,
IOUT_OC_LIM,
OT1_LIM,
OT2_LIM,
PFC_T_INDUCTOR_LIM,
PFC_T_BRIDGE_LIM,
SS_PWM,
SS_FREQ,
SS_CURR,
T_PID_LSB,
T_PID_MSB,
KI_PID,
KP_PID,
KD_PID,
TI_LSB,
TI_MSB,
ThH1,
ThH2,
Kerr,

TOTAL_REGS
} ModbusMainList;

enum    //Конкретно регистры. U32 указываются как младший-старший, U8 указываются как поле1_поле2.
{
ADC_TIMESTAMP,
I_IN         ,
V_IN         ,
I_OUT        ,
V_OUT        ,
W_IN         ,
W_OUT         ,
P_IN         ,
Tb1_Tb2          ,
Tpfc_Tpfc_b       ,
Light        ,
F_m          ,
PWM_m        ,
I_U_m        ,
Ph_I         ,
ei           ,
acc_i        ,
PID_out      ,
reserve1     ,
reserve2     ,
reserve3     ,
reserve4     ,
reserve5     ,
reserve6     ,
reserve7     ,
reserve8     ,
reserve9     ,
reservea     ,
reserveb     ,
reservec     ,
reserved     ,
reservee     ,
reservef     ,
reserveg     ,
reserveh     ,
reservei     ,
reservej     ,
reservek     ,
STATUS       ,

TOTAL_ADC
} ModbusADCList;

#pragma pack (push)
#pragma pack (0)

struct tModbusMain      //Тоже конкретно регистры с их умолчальными значениями.
{
    int Addr, Count;
    unsigned short Regs [TOTAL_REGS];
} ModbusMain = {40176+39, TOTAL_REGS,
    5,
    0xC,
    0,
    0x64,
    0,
    0x0B,
    0x0D,
    0,
    0xDC,
    0x96, 
    0xA0, 
    0x78, 
    0x4B,
    0x4B,
    0x4B,
    0x4B,
    0xA,
    0xA,
    0xA,
    0,  
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0x1388};

#define TOTAL_SETUP_FLAG_BYTES 2
struct
{
    int Addr, Count;
    unsigned char Bytes [TOTAL_SETUP_FLAG_BYTES];
} ModbusSETFlags = {33, TOTAL_SETUP_FLAG_BYTES*8};

#pragma pack (pop)

struct
{
    double Scale;
    int ControlId;
    char ToolTip[128];
}  ModbusMainGUI[TOTAL_REGS] =
{
1,  SET_TIME_STAMP,         "╪ру юЄьхЄъш тЁхьхэш ЄхыхьхЄЁшш, mS",
10, SET_FREQUANCY,          "╟рфрээр  ўрёЄюЄр ухэхЁрЎшш, kHz",
1,  SET_WAVEFORM,           "╟рфрээр  ЇюЁьр ёшуэрыр (ЁхчхЁт)",
1,  SET_PWM,                "╟рфрээ√щ ╩ю¤ЇЇшЎшхэЄ чряюыэхэш , %",
10, SET_I_U,                "╟рфрээюх чэрўхэшх ёЄрсшышчрЎшш яю Єюъє, A",
10, SET_MEAN_OUT_ILIM,      "╧Ёхфхы юуЁрэшўхэш  яю ёЁхфэхьє т√їюфэюьє Єюъє (яЁюуЁрььэ√щ, A)",
10, SET_PULSE_OUT_ILIM,     "╧Ёхфхы юуЁрэшўхэш  яю шьяєы№cэюьє т√їюфэюьє Єюъє (яЁюуЁрььэ√щ, A)",
2,  SET_VOUT_LIM,           "╧Ёхфхы юуЁрэшўхэш  яю т√їюфэюьє эряЁ цхэш■ (яЁюуЁрььэ√щ, ╨хчхЁт, V)",
2,  SET_VIN_OV_LIM,         "╧Ёхфхы чр∙шЄ√ яю тїюфэюьє яхЁхэряЁ цхэш■, V",
2,  SET_VIN_UV_LIM,         "╧Ёхфхы чр∙шЄ√ яю тїюфэюьє эхфюэряЁ цхэш■, V",
10, SET_IIN_OC_LIM,         "╧Ёхфхы чр∙шЄ√, A",
10, SET_IOUT_OC_LIM,        "╨хчхЁт, A",
1,  SET_OT1_LIM,            "╧Ёхфхы Єхяыютющ чр∙шЄ√ яхЁтюую яюыєьюёЄр ухэхЁрЄюЁр (ЁхчхЁт, C)",
1,  SET_OT2_LIM,            "╧Ёхфхы Єхяыютющ чр∙шЄ√ тЄюЁюую яюыєьюёЄр ухэхЁрЄюЁр (ЁхчхЁт, C)",
1,  SET_PFC_T_INDUCTOR_LIM, "╧Ёхфхы Єхяыютющ чр∙шЄ√ ъы■ўхщ PFC (ЁхчхЁт, C)",
1,  SET_PFC_T_BRIDGE_LIM,   "╧Ёхфхы Єхяыютющ чр∙шЄ√ ьюёЄр PFC (ЁхчхЁт, C)",
1,  SET_SS_PWM,             "╤ъюЁюёЄ№ эрЁрёЄрэш  PWM, %/s",
10, SET_SS_FREQ,            "╤ъюЁюёЄ№ эрЁрёЄрэш  ўрёЄюЄ√, kHz/s",
10, SET_SS_CURR,            "╤ъюЁюёЄ№ эрЁрёЄрэш  Єюър, A/s",
1,  SET_T_PID_LSB,          "╠ырф°шщ яюыєсрщЄ яюёЄю ээющ тЁхьхэш PID Ёхуєы ЄюЁр, 1 us",
1,  SET_T_PID_MSB,          "╤ЄрЁ°шщ яюыєсрщЄ яюёЄю ээющ тЁхьхэш PID Ёхуєы ЄюЁр, 1 us",
10, SET_KI_PID,             "╩ю¤ЇЇшЎшхэЄ шэЄхуЁшЁютрэш  ╧╚─ Ёхуєы ЄюЁр, promille",
10, SET_KP_PID,             "╩ю¤ЇЇшЎшхэЄ яЁ ьющ тхЄтш ╧╚─ Ёхуєы ЄюЁр, promille",
10, SET_KD_PID,             "╩ю¤ЇЇшЎшхэЄ фшЇЇхЁхэЎшшЁє■∙хщ тхЄтш ╧╚─ Ёхуєы ЄюЁр, promille",
1,  SET_TI_LSB,             "╠ырф°шщ яюыєсрщЄ яюёЄю ээющ тЁхьхэш шэЄхуЁрЄюЁр, 1 us",
1,  SET_TI_MSB,             "╤ЄрЁ°шщ яюыєсрщЄ яюёЄю ээющ тЁхьхэш шэЄхуЁрЄюЁр, 1 us",
10, SET_ThH1,               "┬хЁїэшщ яЁхфхы эрё√∙хэш  шэЄхуЁрЄюЁр PID Ёхуєы ЄюЁр, A",
10, SET_ThH2,               "═шцэшщ яЁхфхы эрё√∙хэш  шэЄхуЁрЄюЁр PID Ёхуєы ЄюЁр, A",
5,  SET_Kerr,               "╩ю¤ЇЇшЎшхэЄ єёшыхэш  ю°шсъш, promille"
};

struct
{
	double Min, Max;
	char INIKeyName[64];
} ModbusMainINI[TOTAL_REGS] =
{
	NAN, NAN, "TIME_STAMP",
	NAN, NAN, "FREQUENCY",
	NAN, NAN, "WAVEFORM",
	NAN, NAN, "PWM",
	NAN, NAN, "I_U",
	NAN, NAN, "MEAN_OUT_ILIM",
	NAN, NAN, "PULSE_OUT_ILIM",
	NAN, NAN, "VOUT_LIM",
	NAN, NAN, "VIN_OV_LIM",
	NAN, NAN, "VIN_UV_LIM",
	NAN, NAN, "IIN_OC_LIM",
	NAN, NAN, "IOUT_OC_LIM",
	NAN, NAN, "OT1_LIM",
	NAN, NAN, "OT2_LIM",
	NAN, NAN, "PFC_T_INDUCTOR_LIM",
	NAN, NAN, "PFC_T_BRIDGE_LIM",
	NAN, NAN, "SS_PWM",
	NAN, NAN, "SS_FREQ",
	NAN, NAN, "SS_CURR",
	NAN, NAN, "T_PID_LSB",
	NAN, NAN, "T_PID_MSB",
	NAN, NAN, "KI_PID",
	NAN, NAN, "KP_PID",
	NAN, NAN, "KD_PID",
	NAN, NAN, "TI_LSB",
	NAN, NAN, "TI_MSB",
	NAN, NAN, "ThH1",
	NAN, NAN, "ThH2",
	NAN, NAN, "Kerr"
};

struct
{
	int CheckboxId;
	char ToolTip[128];
	char INIKeyName[64];
} ModbusSETFlagsGUI[TOTAL_SETUP_FLAG_BYTES*8] = {
SET_CB_I_LOOP			,"┬ъы■ўхэшх/т√ъы■ўхэшх ёЄрсшышчрЎшш яю т√їюфэюьє Єюъє",           "CB_I_LOOP",
SET_CB_I_LOOP_B			,"┬ъы■ўхэшх/т√ъы■ўхэшх ёЄрсшышчрЎшш яю Єюъє яюыєьюёЄют (╨хчхЁт)", "CB_I_LOOP_B",
SET_CB_F_LOOP			,"┬ъы■ўхэшх ртЄюяюфёЄЁющъш ўрёЄюЄ√",                              "CB_F_LOOP",
SET_CB_I_PROFILE		,"╟ряєёъ ёърэшЁютрэш  яЁюЇшы  Єюър",                              "CB_I_PROFILE",
SET_CB_I_PROFILE_WRITE		,"╨рчЁх°хэшх чряшёш яЁюЇшы  Єюър",                                "CB_I_PROFILE_WRITE",
SET_CB_I_PROFILE_DEFAULT	,"╨рчЁх°хэшх чруЁєчъш яЁюЇшы  Єюър яю єьюыўрэш■",                 "CB_I_PROFILE_DEFAULT",
SET_CB_F_SCAN			,"╨хчхЁт",                                                        "CB_F_SCAN",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт",                                                        "",
MEA_CB_reserve			,"╨хчхЁт"                                                         "",
};

char ParamHeaderString[] = "MixedPlugin	Minimum	Current Value	Maximum	#Graph. Color";

int GrabUserInput (tModbusMain *Dest)      //Заполняет все поля, размер не проставляет (он определяется назначением пакета, т. е. нажатой кнопкой).
{
        unsigned long Msg[4]={0, BM_GETCHECK};

        double T;
//      Dest->AutoSend = 0;
        memset ((char*)Dest+8, 0, sizeof(tModbusMain)-8);

        for (int i=0; i<Dest->Count; i++)
                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, ModbusMainGUI[i].ControlId     ))) ))
                {
                	if (!isnan(ModbusMainINI[i].Max) && T > ModbusMainINI[i].Max)	//Never TRUE if Max is NAN
                	{
                		Msg[0]=ModbusMainGUI[i].ControlId;
                                *((double*)(Msg+1)) = ModbusMainINI[i].Max;
                                (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                                (*DialogEngine) (SETFOCUS    , MyDialogID, ModbusMainGUI[i].ControlId);
                                MessageBeep(MB_ICONHAND);
                                return 1;
                	}
                	if (!isnan(ModbusMainINI[i].Min) && T < ModbusMainINI[i].Min)	//Never TRUE if Min is NAN
                	{
                		Msg[0]=ModbusMainGUI[i].ControlId;
                                *((double*)(Msg+1)) = ModbusMainINI[i].Min;
                                (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                                (*DialogEngine) (SETFOCUS    , MyDialogID, ModbusMainGUI[i].ControlId);
                                MessageBeep(MB_ICONHAND);
                                return 1;
                	}
                	Dest->Regs[i]         = T * ModbusMainGUI[i].Scale;
                }
                else	return MessageBeep(MB_ICONHAND) | 1;

		//Код превратился в вырвиглазный индийский пиздец, но с разделением плагинов этот момент исправится сам по себе.

        memset ((char*)&ModbusSETFlags+8, 0, sizeof(ModbusSETFlags)-8);
        for (int i=0; i<TOTAL_SETUP_FLAG_BYTES*8; i++)
        	if (ModbusSETFlagsGUI[i].CheckboxId != MEA_CB_reserve) 
		        ModbusSETFlags.Bytes[i/8] |= ( 
		          (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)&(Msg[0]=ModbusSETFlagsGUI[i].CheckboxId) ) != 0
		        )<<(i%8); //Don't set unknown flags!


        Msg[0]=SLIDERFREQ, Msg[1]=TBM_SETPOS, Msg[2]=(WPARAM)(BOOL)TRUE, Msg[3]=*((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_FREQUANCY ))) * ModbusMainGUI[FREQUANCY].Scale;
        (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=SLIDERAMPL,                                               Msg[3]=*((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_I_U       ))) * ModbusMainGUI[I_U      ].Scale;
        (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);

        return 0;
}


int DoButton (int Button)
{
        unsigned long Msg[4]={/*ACK_PERM*/-1, BM_SETCHECK, BST_CHECKED};
        char* NameSet[5]={0};
        char Name[MAX_PATH]="Log.xls";
        int i;
        int pU32;

        switch (Button)
        {
                case ASK_CURR:
//                        Message.Hdr=MSG_DEBUG | CAT_MODE;
                        pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusMain));
cout<<"Ptr returned:"<<pU32<<endl;
                        if (pU32)
                        {
                            memcpy (&ModbusMain, (int*)pU32, sizeof (ModbusMain));
cout<<"Regs read: "<<ModbusMain.Count<<endl;
                            for (i=0; i<ModbusMain.Count; i++)
                            {
                                Msg[0]=ModbusMainGUI[i].ControlId;
                                *((double*)(Msg+1))=((double)(ModbusMain.Regs[i])) / ModbusMainGUI[i].Scale;
                                (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                            }
                        } else cout<<"Error! No data received"<<endl;	//ToDo: Log line!!!

		        pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusSETFlags));
		        Msg[1]=BM_SETCHECK;
		        Msg[3]=0;
		        if (pU32)
		        {
		                memcpy (&ModbusSETFlags, (int*)pU32, sizeof (ModbusSETFlags));
		                for (int i=0; i<TOTAL_SETUP_FLAG_BYTES*8; i++)
		                {
				        Msg[0]=ModbusSETFlagsGUI[i].CheckboxId;
				        Msg[2]=(ModbusSETFlags.Bytes[i/8]>>(i%8)) & 1;
				        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
				}
			} else cout<<"Error! No Setup Flags received"<<endl;	//ToDo: Log line!!!

                return 1;
//                case ASK_PERM:
//return 1 | RequestADC();
/*                        Msg[2]=BST_UNCHECKED;
                        Msg[0]=ACK_CURR    ,            (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=ACK_PERM    ,            (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                        Msg[1]=WM_SETTEXT;
                        Msg[2]=0;
                        Msg[3]=(int)"?";
                        Msg[0]=SET_RSVD  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_FREQ  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_WFORM ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_AMPL  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_MILIM ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_LOOPS ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_RAMP  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_VILIM ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_VOLIM ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_PLIM  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_POUT  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_BRT1  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_BRT2  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_PDELAY,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_FAULTS,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_PIN   ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_VOUT  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_VIN   ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_IOUT  ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_IIN   ,      (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=MEA_TIMESTAMP,   (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                        Msg[1]=BM_SETCHECK;
                        Msg[2]=BST_INDETERMINATE;
                        Msg[3]=0;
                        Msg[0]=SET_ILOOPON   ,          (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_VLOOPON   ,          (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_FREQLOOPON,          (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_FREQSCANON,          (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        Msg[0]=SET_PLOOPON   ,          (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        (*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(&Message));
*/

                case SET_CURR:
//                        Message.Hdr=MSG_DEBUG | CAT_MODE;       //Меняем с умолчального на "текущие настройки"
//                        Msg[0]=ACK_CURR;        //Меняем с умолчального на "текущие настройки"
                case SET_PERM:
                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);    //Ставим галку, какая уж там есть (по умолчанию "зашивочные", по верхнему кейсу "текущие")
                        TrakDeadTime = timeGetTime();
                        DoWarning = 1;
                        if (GrabUserInput(&ModbusMain)) return 1;  //Ошибки ввода полей, нечисловые символы и т. д.
//                        RearrangeGraphs (Message.AutoSend);     //Мог измениться шаг шкалы времени? Проверим!
//                        Message.Len=52;         //На всякий случай
                        Msg[0]=SLIDERSEND, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg)) {Msg[1]=BM_SETCHECK, Msg[2]=BST_CHECKED, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);}  //Ставим галку авто-отправки слайдеров, если до этого была "серая"
//                        Msg[0]=SENDBYREQ , Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
//                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg)) {Msg[1]=BM_SETCHECK, Msg[2]=BST_CHECKED, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);}  //Ставим галку авто-отправки по запросу, если до этого была "серая"
                        (*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(&ModbusMain));
                        (*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(&ModbusSETFlags));
//                        if (DoLog) LogToFile (&Message, "Outgoing: ");
                return 1;
        }

        return 0;
}


//extern "C" int __export __syscall PluginEngine (unsigned long uMsg, unsigned long wParam, unsigned long lParam)
extern "C" int __export WINAPI PluginEngine (long DiaId, unsigned long uMsg, unsigned long wParam, unsigned long lParam)
{
        char Text[256];
        unsigned long Msg[4]={LASTTYPE, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
if (!uMsg) cout<<"WM_NULL "<<wParam<<" "<<lParam<<endl;

        if (!uMsg && (!DialogEngine && !NetEngine || (unsigned long)DialogEngine==wParam && (unsigned long)NetEngine==lParam) )
        {
                DialogEngine=(FWAPI*)(wParam);
                NetEngine=(FWAPI*)(lParam);
                cout<<"Opening a dialog by a template "<<DialogEngine<<endl;
                WCB* PluginEnginePtr[2] = {PluginEngine, NULL};

//DiaTemplate=LoadResource(hinst, FindResource (hinst, "BASE_SET", RT_DIALOG));

		if ((*DialogEngine) (GETDIALOGDATA_HEADER, -1, (unsigned long)(ParamHeaderString)))
		{	//Data prepared by GETDIALOGDATA_HEADER and GETDIALOGDATA_LINE are guaranteed to be valid (as well as their pointers) until next call, so both functions are not thread-safe.
			double *Params;
			for (int i=0; i<TOTAL_REGS; i++)
				if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)(ModbusMainINI[i].INIKeyName)) ) )
				{
					ModbusMain.Regs[i] = Params[1] * ModbusMainGUI[i].Scale;
					ModbusMainINI[i].Min = Params[0];
					ModbusMainINI[i].Max = Params[2];
				}// else ModbusMain.Regs[i] = ModbusMainINI[i].Min = ModbusMainINI[i].Max = NAN;	//Regs is zero because not double

		        memset ((char*)&ModbusSETFlags+8, 0, sizeof(ModbusSETFlags)-8);
			for (int i=0; i<TOTAL_SETUP_FLAG_BYTES*8; i++)
				if (ModbusSETFlagsGUI[i].INIKeyName[0])
				 if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)(ModbusSETFlagsGUI[i].INIKeyName)) ) )
					ModbusSETFlags.Bytes[i/8] |= (Params[1]!=0) << (i%8);

//cout<<(int)(ModbusSETFlags.Bytes[0])<<" "<<(int)(ModbusSETFlags.Bytes[1])<<endl;

//			double *Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("TestParam")) );
//			if (!Params) cout<<"TestParam is not present in .INI!"<<endl;
//			else for (int i=0; i<4; i++) cout<<Params[i]<<" "; cout<<endl;
		}

                /*MyDialogID = */(*DialogEngine) (OPENDIALOG, (unsigned long)(DiaTemplate), (unsigned long)(&PluginEnginePtr));
//                                 (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&Message)) );    //ага, именно что передаём адрес первого элемента, там тип сообщения, которые мы будем принимать; во втором элементе -- число однотипных модбас-полей, чтобы знать, сколько запрашивать.
//                Message.Hdr=MSG_DEBUG|CAT_MODE; //Второй "наш" тип
//                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusMain)) );    //буфер на все зарегистрированные сообщения один, можно из любого вызова взять.
                                                //Третий тип, потом вынесем в другой плуг
//                                 (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ResetLogs)) );

                return 0;//(int)("Test point 1");
        }

// TODO!!! SEND IT ON DEVICE RECONNECT!!!
// if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam ) ModbusMain.Count = TOTALREGS;    //Return to Max Supported

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED) return DoButton(wParam&0xFFFF);

        if (uMsg==WM_TIMER)     //Обработка "отложенных" дёрганий ползунков
        {                
                Msg[0]=0;       //номер таймера
                Msg[1]=0;       //время таймера = 0 (отключить таймер)
                (*DialogEngine) (TIMER, MyDialogID, (int)Msg);

                Msg[0]=SLIDERFREQ, Msg[1]=TBM_GETPOS, Msg[2]=Msg[3]=0;
                int j=(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_FREQUANCY, *((double*)(Msg+1))=j / ModbusMainGUI[FREQUANCY].Scale, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

                Msg[0]=SLIDERAMPL, Msg[1]=TBM_GETPOS, Msg[2]=Msg[3]=0;
                j=(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_I_U, *((double*)(Msg+1))=j / ModbusMainGUI[I_U].Scale, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

                Msg[0]=SLIDERSEND, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg) == BST_CHECKED) //В этот раз строго не серая.
                        return DoButton (SET_CURR);     //Аналогично нажатию "отправки".
        }

        if (uMsg==WM_HSCROLL)
        {
                int j=wParam>>16;       //Враппер позаботился

                Msg[0] = timeGetTime() - TrakDeadTime;//Благо он у нас беззнаковый
                if (Msg[0]<100) //Смена значений полей не ранее 1/10 секунды после прошлой отправки!
                {
                        Msg[0]=0;       //номер таймера
                        Msg[1]=100;     //время таймера
//cout<<"Timer set! "<<
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);
//cout<<endl;
                        return 1;
                }

                if (lParam == SLIDERFREQ)       //Тоже враппер позаботился
                {
                        Msg[0]=SET_FREQUANCY, *((double*)(Msg+1))=j / ModbusMainGUI[FREQUANCY].Scale, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                }
                if (lParam == SLIDERAMPL)       //Тоже враппер позаботился
                {
                        Msg[0]=SET_I_U, *((double*)(Msg+1))=j / ModbusMainGUI[I_U].Scale, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                }

                Msg[0]=SLIDERSEND, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg) == BST_CHECKED) //В этот раз строго не серая.
                        return DoButton (SET_CURR);     //Аналогично нажатию "отправки".

        }
/*        if (uMsg==EN_KILLFOCUS)
        {
cout<<wParam<<" "<<lParam<<endl;
                unsigned long Msg[4]={0, TBM_SETPOS, (WPARAM)(BOOL)TRUE};
                double T;

                if (wParam>>16 == SET_FREQ)
                {
                        Msg[0]=SLIDERFREQ;
                        if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_FREQ     ))) ))
                        {
                                Msg[3]=T;
                                (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                        }                       
                }
                if (wParam>>16 == SET_AMPL)
                {
                        Msg[0]=SLIDERAMPL;
                        if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_AMPL     ))) ))
                        {
                                Msg[3]=T;
                                (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                        }                       
                }
        }*/

        if (uMsg==WM_INITDIALOG)
        {
		MyDialogID = lParam;
cout<<"Newly created Dialog ID = "<<MyDialogID<<endl;

                unsigned long Msg[4]={0, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, (LPARAM)0};
		for (int i=0; i<TOTAL_SETUP_FLAG_BYTES*8; i++)
		{
                        Msg[0]=ModbusSETFlagsGUI[i].CheckboxId;
                        Msg[1]=(int)(ModbusSETFlagsGUI[i].ToolTip);
                        (*DialogEngine) (ATTACHTOOLTIP, MyDialogID, (int)Msg);
		}

                Msg[0]=SLIDERFREQ, Msg[1]=TBM_SETRANGE, Msg[2]=1, Msg[3]=(LPARAM) MAKELONG( ModbusMainINI[FREQUANCY].Min*ModbusMainGUI[FREQUANCY].Scale, ModbusMainINI[FREQUANCY].Max*ModbusMainGUI[FREQUANCY].Scale );	//0x00, 0x3E8);
                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SLIDERAMPL,                                Msg[3]=(LPARAM) MAKELONG( ModbusMainINI[I_U].Min*ModbusMainGUI[I_U].Scale, ModbusMainINI[I_U].Max*ModbusMainGUI[I_U].Scale );	//0x00, 0x50);
                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                for (int i=0; i<ModbusMain.Count; i++)
                {
                        Msg[0]=ModbusMainGUI[i].ControlId;
                        Msg[1]=(int)(ModbusMainGUI[i].ToolTip);
                        (*DialogEngine) (ATTACHTOOLTIP, MyDialogID, (int)Msg);

                        *((double*)(Msg+1))=((double)(ModbusMain.Regs[i])) / ModbusMainGUI[i].Scale;
                        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                }

	        Msg[1]=BM_SETCHECK;
	        Msg[3]=0;
                for (int i=0; i<TOTAL_SETUP_FLAG_BYTES*8; i++)
                {
		        Msg[0]=ModbusSETFlagsGUI[i].CheckboxId;
		        Msg[2]=(ModbusSETFlags.Bytes[i/8]>>(i%8)) & 1;
		        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
		}
        }
        
//      return 0xDEADC0DE;
        return 0;
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        if (fdwReason==DLL_PROCESS_ATTACH)
        {
//              hinst=hinstDLL;
                DiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "BASE_SET", RT_DIALOG));
                cout<<"Plug Init "<<DiaTemplate<<" "<<fdwReason<<endl;
//cout<<"Supported Size: "<<sizeof(MsgDebugLogs)<<endl;
        }

        return 1;
}
