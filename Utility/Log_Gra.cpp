#define OPEN_WATCOM_DIRTY_HACK
#include "Log_Gra.rc"
#endif
//#include <windows.h> 㦥 ����祭� � ��吝�� 堪�
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <math.h>

#include <commctrl.h>   //������ (᫠�����)

#include "DiaWrap.h"
#include "Protocol.h"

//HINSTANCE hinst;

FWAPI *DialogEngine=NULL, *NetEngine=NULL;
int MyDialogID=0;
int CanvasID=0;
HGLOBAL DiaTemplate;
int* IncomingBuffer=NULL;

fstream Log;
int DoLog=0, DoWarning=0;
unsigned long TrakDeadTime;     //��᫥ �� ��ࠢ�� ���⮥� ��⥬� ������ ���� �����-� ���� ᥪ㭤� ���� � ����㭪��.

char GraphArea[768*256*3]={0};

enum    //�����⭮ ॣ�����. U32 㪠�뢠���� ��� ����訩-���訩, U8 㪠�뢠���� ��� ����1_����2.
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

struct tModbusADC
{
    int Addr, Count;
    unsigned short Regs [TOTAL_ADC];
} ModbusADC = {40176, TOTAL_ADC};

#define TOTAL_ADC_FLAG_BYTES 4
struct
{
    int Addr, Count;
    unsigned char Bytes [TOTAL_ADC_FLAG_BYTES];
} ModbusADCFlags = {1, TOTAL_ADC_FLAG_BYTES*8};

#pragma pack (pop)

unsigned long LastTimestamp = 0xFFFFFFFF;
unsigned long TimestampStep = 0;    //�᫨ ����, � �� �ᮢ���. ����� ����, � ����� �� ���� �����६���� � ᫥���騬.
int NoDraw = 1;                     //�� �뫮 �������� ����஥�. �� �ᮢ���.

struct
{
    double Scale;               //� ��� ��� ���� �� ॣ�����, � ����. ���⮬� ���ᮢ����� ����ﬨ� ����� (ᬥ饭�� �� ��易⥫쭮 2, ��� ������ �१ Size).
    int MinMaxColorId[3];
    int Size;
    int MinMaxColorVal[3];
    char ToolTip[128];
    char INIKeyName[64];
}  ModbusADCGUI[] =
{
1000,   MEA_reserve                ,MEA_reserve                ,MEA_reserve                  ,2,0,255,0xFFFFFF,      "Timestamp, S",                                            "",
10,              MEA_I_IN_GRAPH_MIN,         MEA_I_IN_GRAPH_MAX,         MEA_I_IN_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������������ ���, �",                                     "MEA_I_IN",  
2 ,              MEA_V_IN_GRAPH_MIN,         MEA_V_IN_GRAPH_MAX,         MEA_V_IN_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� ����������, �",                                   "MEA_V_IN",  
10,             MEA_I_OUT_GRAPH_MIN,        MEA_I_OUT_GRAPH_MAX,        MEA_I_OUT_GRAPH_COLOR,2,0,255,0xFFFFFF,      "�������� ���, �",                                         "MEA_I_OUT", 
2 ,             MEA_V_OUT_GRAPH_MIN,        MEA_V_OUT_GRAPH_MAX,        MEA_V_OUT_GRAPH_COLOR,2,0,255,0xFFFFFF,      "�������� ����������, �",                                  "MEA_V_OUT", 
1 ,              MEA_W_IN_GRAPH_MIN,         MEA_W_IN_GRAPH_MAX,         MEA_W_IN_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������������ ������� ��������, ��",                       "MEA_W_IN",  
1 ,             MEA_W_OUT_GRAPH_MIN,        MEA_W_OUT_GRAPH_MAX,        MEA_W_OUT_GRAPH_COLOR,2,0,255,0xFFFFFF,      "���������� ������� ��������, ��",                         "MEA_W_OUT", 
2 ,              MEA_P_IN_GRAPH_MIN,         MEA_P_IN_GRAPH_MAX,         MEA_P_IN_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������������ �������, ���*�",                             "MEA_P_IN",  
1 ,               MEA_Tb1_GRAPH_MIN,          MEA_Tb1_GRAPH_MAX,          MEA_Tb1_GRAPH_COLOR,1,0,255,0xFFFFFF,      "����������� 1 ��������� ����������, C",                   "MEA_Tb1",   
1 ,               MEA_Tb2_GRAPH_MIN,          MEA_Tb2_GRAPH_MAX,          MEA_Tb2_GRAPH_COLOR,1,0,255,0xFFFFFF,      "����������� 2 ��������� ����������, C",                   "MEA_Tb2",   
1 ,              MEA_Tpfc_GRAPH_MIN,         MEA_Tpfc_GRAPH_MAX,         MEA_Tpfc_GRAPH_COLOR,1,0,255,0xFFFFFF,      "����������� ������ pfc, C",                               "MEA_Tpfc",  
1 ,            MEA_Tpfc_b_GRAPH_MIN,       MEA_Tpfc_b_GRAPH_MAX,       MEA_Tpfc_b_GRAPH_COLOR,1,0,255,0xFFFFFF,      "����������� ����� pfc, C",                                "MEA_Tpfc_b",
.1,             MEA_Light_GRAPH_MIN,        MEA_Light_GRAPH_MAX,        MEA_Light_GRAPH_COLOR,2,0,255,0xFFFFFF,      "���������� ������������, ��",                             "MEA_Light", 
10,               MEA_F_m_GRAPH_MIN,          MEA_F_m_GRAPH_MAX,          MEA_F_m_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� ������� �������, ���",                            "MEA_F_m",   
1 ,             MEA_PWM_m_GRAPH_MIN,        MEA_PWM_m_GRAPH_MAX,        MEA_PWM_m_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� �������� ���, %",                                 "MEA_PWM_m", 
10,             MEA_I_U_m_GRAPH_MIN,        MEA_I_U_m_GRAPH_MAX,        MEA_I_U_m_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� ����������� �������� ������������ �� ����, �",    "MEA_I_U_m", 
1 ,              MEA_Ph_I_GRAPH_MIN,         MEA_Ph_I_GRAPH_MAX,         MEA_Ph_I_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������ �������� ���� ����, ��.",                          "MEA_Ph_I",  
10,                MEA_ei_GRAPH_MIN,           MEA_ei_GRAPH_MAX,           MEA_ei_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� �������� ������ ������������, �",                 "MEA_ei",    
10,             MEA_acc_i_GRAPH_MIN,        MEA_acc_i_GRAPH_MAX,        MEA_acc_i_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� �������� ������������ �����������, �",            "MEA_acc_i",
10,           MEA_PID_out_GRAPH_MIN,      MEA_PID_out_GRAPH_MAX,      MEA_PID_out_GRAPH_COLOR,2,0,255,0xFFFFFF,      "������� ��a����� ������ PID ����������, �",               "MEA_PID_out",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1 ,           MEA_reserve          ,      MEA_reserve          ,      MEA_reserve            ,2,0,255,0xFFFFFF,      "������",                                                  "",
1,             MEA_STATUS_GRAPH_MIN,       MEA_STATUS_GRAPH_MAX,       MEA_STATUS_GRAPH_COLOR,2,0,255,0xFFFFFF,      "��������� ����������",                                     "MEA_STATUS"
};

char ParamHeaderString[] = "MixedPlugin	Minimum	Current Value	Maximum	#Graph. Color";

static int OldHeights[256]={0};
int AddDash (char *Graph, int X, int Y, int Shift, int Length, int Height, int ColorBGR, int Id) //����� �������� �⠭�� ����� ������, �뭥�� � ᠬ �३����. ����� ����� ��� �㭪�� ��த�襬 ����᪨� ���.
{
        int From;

        if (Shift>=X) memset (Graph, 0, X*Y*3);
        else if (Shift)
        {
                memmove (Graph, Graph+Shift*3, X*Y*3-Shift);
                for (int yy=0; yy<Y; yy++) memset (Graph + X*yy*3 + (X-Shift)*3, 0, Shift*3);
        }

        if (Height<0)
        {
                if (Id>=0 && Id<256) OldHeights[Id]=0;
                return 1;
        }
        if (Height >=Y)
        {
                if (Id>=0 && Id<256) OldHeights[Id]=Y-1;
                return 1;
        }

        if (Id<0 || Id>255) From=Height;
        else
        {
                From=OldHeights[Id];    //�������� ��������� ����� -- �ਤ���� ���������
                OldHeights[Id]=Height;
        }

        if (Length > X) Length = X;

        for (int yy = min (From, Height); yy<=max (From, Height); yy++)
        {
                for (int xx=(X-Length)*3; xx<X*3; xx+=3)
                {
                        Graph[X*yy*3 + xx + 0] |= ColorBGR;
                        Graph[X*yy*3 + xx + 1] |= ColorBGR >>  8;
                        Graph[X*yy*3 + xx + 2] |= ColorBGR >> 16;
                }
        }

        return 0;
}
/*
struct GRAPH_PARAM
{
        unsigned long LastTimestamp;
        unsigned long TimestampStep;    //�᫨ ����, � �� �ᮢ���. ����� ����, � ����� �� ���� �����६���� � ᫥���騬.
        int NoDraw;                     //�� �뫮 �������� ����஥�. �� �ᮢ���.
        int MeasMeanIIn_Min  ;
        int MeasMeanIIn_Max  ;
        int MeasMeanIIn_Color;
        int MeasMeanIOut_Min  ;
        int MeasMeanIOut_Max  ;
        int MeasMeanIOut_Color;
        int MeasVIn_Min  ;
        int MeasVIn_Max  ;
        int MeasVIn_Color;
        int MeasVOut_Min  ;
        int MeasVOut_Max  ;
        int MeasVOut_Color;
        int MeasPIn_Min  ;
        int MeasPIn_Max  ;
        int MeasPIn_Color;
        int MeasPOut_Min  ;
        int MeasPOut_Max  ;
        int MeasPOut_Color;
        int MeasBr1Temper_Min  ;
        int MeasBr1Temper_Max  ;
        int MeasBr1Temper_Color;
        int MeasBr2Temper_Min  ;
        int MeasBr2Temper_Max  ;
        int MeasBr2Temper_Color;
        int MeasPhaseDelay_Min  ;
        int MeasPhaseDelay_Max  ;
        int MeasPhaseDelay_Color;
        int MeasFaults_Min  ;
        int MeasFaults_Max  ;
        int MeasFaults_Color;
} GraphParam = {0xFFFFFFFF, 0, 1};      //���襡��� ���祭�� "��� �� �뫮" (� u16 �� ����� ⠪��� ���� �� �����).
*/
int RearrangeGraphs(int TimeStep)
{
        double T;

        if (TimeStep >= 0)      //�맮� ��-�� ��������� ����஥�: �஢��塞 ⮫쪮 蠣 誠�� �६���
        {
                if (TimeStep == TimestampStep) return 0;     //��祣� �� ����������, ��祣� �� ����ᮢ뢠��
                else TimestampStep = TimeStep;
                if (!TimeStep) return 0;                        //�㫥��� 蠣 誠�� �६���: �⮯ ��設�, �� ��䨪� �� ��ࠥ�.

                memset (GraphArea, 192, 768*256*3);     //��ࠥ� ���� ��䨪�, ���� ���� � ���� 蠣�� �६���.
                (*DialogEngine) (UPDATECANVAS, CanvasID, (unsigned long)(GraphArea));
                LastTimestamp = 0xFFFFFFFF;

                return 1;       //�� �� � �� �� "�訡��", �� ���-� ���������, �� ����� ���祭�� ��ᮢ���⨬� � ����.
        }

        //�맮� �� ����䥩�: ������ ���������� �� ����� �����.
        memset (GraphArea, 192, 768*256*3);     //���砫� ��� ����ࠥ� � 䨣��
        (*DialogEngine) (UPDATECANVAS, CanvasID, (unsigned long)(GraphArea));
        LastTimestamp = 0xFFFFFFFF;
        NoDraw = 0;  

//        unsigned long Msg[4];

        for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
        {
        	for (int j=0; j<3; j++)
        	{
        		T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, ModbusADCGUI[i].MinMaxColorId[j]     )));
        		if (!isfinite(T))
        		{
        			ModbusADCGUI[i].MinMaxColorVal[0]=ModbusADCGUI[i].MinMaxColorVal[1]=ModbusADCGUI[i].MinMaxColorVal[2]=0;
        			if (ModbusADCGUI[i].MinMaxColorId[j] != MEA_reserve) MessageBeep(MB_ICONHAND);
        			break;
        		}
	                ModbusADCGUI[i].MinMaxColorVal[j] = T;
		}
        }

        return 0;
}

/*int UpdateGraphs (MsgDebug *Data)
{
        if (GraphParam.NoDraw || !GraphParam.TimestampStep) return 1;   //�訡�窮 ��諮: ��� �������� ����஥� ���� 蠣 ࠢ�� ���! (��᫥���� ������ ������� � ����砥� �⪫�祭�� ��⮮�ࠢ��).

        if (GraphParam.LastTimestamp > 0xFFFF) GraphParam.LastTimestamp = Data->Timestamp;      //�㫥��� �窠 ������ ��䨪�

        unsigned short Delta = Data->Timestamp - GraphParam.LastTimestamp;      //���� ��⠩᪨� ᥪ�� ����� ����: �᫨ ����稭� �� ����� �������� �����, 祬 �� ���-���������, ������ ��९������� ⮦� �㤥� ��ࠡ�⠭ ���४⭮.
        int Shift = Delta / GraphParam.TimestampStep;   //������ �� �६���, ��ࠦ����� � ���ᥫ�� (�. �. � 楫��� 蠣�� ��⮮�ࠢ��).
        GraphParam.LastTimestamp += Shift*GraphParam.TimestampStep;     //���� ��⠩᪨� ᥪ�� ����� ���: ������� �� ������ �訡�� ���㣫����!
        GraphParam.LastTimestamp &= 0xFFFF;             //� ��� ⠬ ���-⠪� U16
        int Length = max(Shift, 1);     //���� �᫨ ����� �������� � �� �� �������, ��� ࠢ�� ����� ������� ���祪 �� �뢠��.
        double T;

        for (int i=0,b=0,IsSh=1; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
        	if (ModbusADCGUI[i].MinMaxColorVal[0] < ModbusADCGUI[i].MinMaxColorVal[1])
        	{
                        switch (ModbusADCGUI[i].Size)
                        {
                                case 1:
                                        T=((double)(
                                                (  ((char*)(ModbusADC.Regs))[b]  )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 2:
                                        T=((double)(
                                                * ((unsigned short*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 4:
                                        T=((double)(
                                                * ((unsigned long*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                        }
		        AddDash (GraphArea, 768, 256, Shift*IsSh, Length, (T-ModbusADCGUI[i].MinMaxColorVal[0])*256/(ModbusADCGUI[i].MinMaxColorVal[1]-ModbusADCGUI[i].MinMaxColorVal[0]), ModbusADCGUI[i].MinMaxColorVal[2] , i);
		        IsSh=0;
		}
        	if (IsSh) AddDash (GraphArea, 768, 256, Shift, Length, 128, 0, 255);
*/
/*
        AddDash (GraphArea, 768, 256, Shift, Length, ((float)Data->MeasMeanIIn   -GraphParam.MeasMeanIIn_Min   )*256/(GraphParam.MeasMeanIIn_Max   -GraphParam.MeasMeanIIn_Min   ), GraphParam.MeasMeanIIn_Color   , 0);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasMeanIOut  -GraphParam.MeasMeanIOut_Min  )*256/(GraphParam.MeasMeanIOut_Max  -GraphParam.MeasMeanIOut_Min  ), GraphParam.MeasMeanIOut_Color  , 1);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasVIn       -GraphParam.MeasVIn_Min       )*256/(GraphParam.MeasVIn_Max       -GraphParam.MeasVIn_Min       ), GraphParam.MeasVIn_Color       , 2);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasVOut      -GraphParam.MeasVOut_Min      )*256/(GraphParam.MeasVOut_Max      -GraphParam.MeasVOut_Min      ), GraphParam.MeasVOut_Color      , 3);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPIn       -GraphParam.MeasPIn_Min       )*256/(GraphParam.MeasPIn_Max       -GraphParam.MeasPIn_Min       ), GraphParam.MeasPIn_Color       , 4);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPOut      -GraphParam.MeasPOut_Min      )*256/(GraphParam.MeasPOut_Max      -GraphParam.MeasPOut_Min      ), GraphParam.MeasPOut_Color      , 5);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasBr1Temper -GraphParam.MeasBr1Temper_Min )*256/(GraphParam.MeasBr1Temper_Max -GraphParam.MeasBr1Temper_Min ), GraphParam.MeasBr1Temper_Color , 6);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasBr2Temper -GraphParam.MeasBr2Temper_Min )*256/(GraphParam.MeasBr2Temper_Max -GraphParam.MeasBr2Temper_Min ), GraphParam.MeasBr2Temper_Color , 7);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPhaseDelay-GraphParam.MeasPhaseDelay_Min)*256/(GraphParam.MeasPhaseDelay_Max-GraphParam.MeasPhaseDelay_Min), GraphParam.MeasPhaseDelay_Color, 8);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasFaults    -GraphParam.MeasFaults_Min    )*256/(GraphParam.MeasFaults_Max    -GraphParam.MeasFaults_Min    ), GraphParam.MeasFaults_Color    , 9);
*/
  //      (*DialogEngine) (UPDATECANVAS, CanvasID, (unsigned long)(GraphArea));                 

    //    return 0;
//}

//int DedicatedUpdateGraphs (MsgDebugLogs *Data)
int DedicatedUpdateGraphs ( )
{
//cout<<NoDraw<<" : "<<TimestampStep<<endl;
        if (NoDraw || !TimestampStep) return 1;   //�訡�窮 ��諮: ��� �������� ����஥� ���� 蠣 ࠢ�� ���! (��᫥���� ������ ������� � ����砥� �⪫�祭�� ��⮮�ࠢ��).

        if (LastTimestamp > 0xFFFF) LastTimestamp = ModbusADC.Regs[ADC_TIMESTAMP];      //�㫥��� �窠 ������ ��䨪�

        unsigned short Delta = ModbusADC.Regs[ADC_TIMESTAMP] - LastTimestamp;      //���� ��⠩᪨� ᥪ�� ����� ����: �᫨ ����稭� �� ����� �������� �����, 祬 �� ���-���������, ������ ��९������� ⮦� �㤥� ��ࠡ�⠭ ���४⭮.
        int Shift = Delta / TimestampStep;   //������ �� �६���, ��ࠦ����� � ���ᥫ�� (�. �. � 楫��� 蠣�� ��⮮�ࠢ��).
        LastTimestamp += Shift*TimestampStep;     //���� ��⠩᪨� ᥪ�� ����� ���: ������� �� ������ �訡�� ���㣫����!
        LastTimestamp &= 0xFFFF;             //� ��� ⠬ ���-⠪� U16
        int Length = max(Shift, 1);     //���� �᫨ ����� �������� � �� �� �������, ��� ࠢ�� ����� ������� ���祪 �� �뢠��.
//cout<<Shift<<" pixels to shift, Len = "<<Length<<endl;
        double T;
        int IsSh=1;

        for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
        	if (ModbusADCGUI[i].MinMaxColorVal[0] < ModbusADCGUI[i].MinMaxColorVal[1])
        	{
                        switch (ModbusADCGUI[i].Size)
                        {
                                case 1:
                                        T=((double)(
                                                (  ((char*)(ModbusADC.Regs))[b]  )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 2:
                                        T=((double)(
                                                * ((unsigned short*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 4:
                                        T=((double)(
                                                * ((unsigned long*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                        }
		        AddDash (GraphArea, 768, 256, Shift*IsSh, Length, (T-ModbusADCGUI[i].MinMaxColorVal[0])*256/(ModbusADCGUI[i].MinMaxColorVal[1]-ModbusADCGUI[i].MinMaxColorVal[0]), ModbusADCGUI[i].MinMaxColorVal[2] , i);
		        IsSh=0;
		}
        	if (IsSh) AddDash (GraphArea, 768, 256, Shift, Length, 128, 0, 255);

/*        AddDash (GraphArea, 768, 256, Shift, Length, ((float)Data->MeasMeanIIn   -GraphParam.MeasMeanIIn_Min   )*256/(GraphParam.MeasMeanIIn_Max   -GraphParam.MeasMeanIIn_Min   ), GraphParam.MeasMeanIIn_Color   , 0);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasMeanIOut  -GraphParam.MeasMeanIOut_Min  )*256/(GraphParam.MeasMeanIOut_Max  -GraphParam.MeasMeanIOut_Min  ), GraphParam.MeasMeanIOut_Color  , 1);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasVIn       -GraphParam.MeasVIn_Min       )*256/(GraphParam.MeasVIn_Max       -GraphParam.MeasVIn_Min       ), GraphParam.MeasVIn_Color       , 2);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasVOut      -GraphParam.MeasVOut_Min      )*256/(GraphParam.MeasVOut_Max      -GraphParam.MeasVOut_Min      ), GraphParam.MeasVOut_Color      , 3);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPIn       -GraphParam.MeasPIn_Min       )*256/(GraphParam.MeasPIn_Max       -GraphParam.MeasPIn_Min       ), GraphParam.MeasPIn_Color       , 4);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPOut      -GraphParam.MeasPOut_Min      )*256/(GraphParam.MeasPOut_Max      -GraphParam.MeasPOut_Min      ), GraphParam.MeasPOut_Color      , 5);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasBr1Temper -GraphParam.MeasBr1Temper_Min )*256/(GraphParam.MeasBr1Temper_Max -GraphParam.MeasBr1Temper_Min ), GraphParam.MeasBr1Temper_Color , 6);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasBr2Temper -GraphParam.MeasBr2Temper_Min )*256/(GraphParam.MeasBr2Temper_Max -GraphParam.MeasBr2Temper_Min ), GraphParam.MeasBr2Temper_Color , 7);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasPhaseDelay-GraphParam.MeasPhaseDelay_Min)*256/(GraphParam.MeasPhaseDelay_Max-GraphParam.MeasPhaseDelay_Min), GraphParam.MeasPhaseDelay_Color, 8);
        AddDash (GraphArea, 768, 256,   0  , Length, ((float)Data->MeasFaults    -GraphParam.MeasFaults_Min    )*256/(GraphParam.MeasFaults_Max    -GraphParam.MeasFaults_Min    ), GraphParam.MeasFaults_Color    , 9);
*/
        (*DialogEngine) (UPDATECANVAS, CanvasID, (unsigned long)(GraphArea));                 

        return 0;
}

int GrabUserInput ()
{
        unsigned long Msg[4]={0, BM_GETCHECK};

        double T;
		//ToDo -- colors and sizes!

        return 0;
}

/*int RequestADC ()
{
        unsigned long Msg[4];

        int pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusADC));
        if (pU32)
        {
                memcpy (&ModbusADC, (int*)pU32, sizeof (ModbusADC));
                for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
                {
                        Msg[0]=ModbusADCGUI[i].ControlId;
                        switch (ModbusADCGUI[i].Size)
                        {
                                case 1:
                                        *((double*)(Msg+1))=((double)(
                                                (  ((char*)(ModbusADC.Regs))[b]  )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 2:
                                        *((double*)(Msg+1))=((double)(
                                                * ((unsigned short*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                                case 4:
                                        *((double*)(Msg+1))=((double)(
                                                * ((unsigned long*)(  ((char*)(ModbusADC.Regs)) + b  ) )
                                                        )) / ModbusADCGUI[i].Scale;                                     
                                break;
                        }

                        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                }
        } else cout<<"Error! No ADC received"<<endl;	//ToDo: Log line!!!

        pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusADCFlags));
        Msg[1]=BM_SETCHECK;
        Msg[3]=0;
        if (pU32)
        {
                memcpy (&ModbusADCFlags, (int*)pU32, sizeof (ModbusADCFlags));
                for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
                {
		        Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
		        Msg[2]=(ModbusADCFlags.Bytes[i/8]>>(i%8)) & 1;
		        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
		}
	} else cout<<"Error! No ADC Flags received"<<endl;	//ToDo: Log line!!!

/*        if (IncomingLogs->Len > sizeof (MsgDebugLogs) && (WarnUpdate&0x01) )
        {
                Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"���������� ������ �������, ��� �������� ������ ������ �������! ��������, ������ ������� ������������ �������� ����������������?";
                (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                WarnUpdate &= ~0x01;
        }

        if (DoLog) DedicatedLogToFile (IncomingLogs, "Logs: ");


        if (IncomingLogs->Len < 10) return 0; 
        if (IncomingLogs->LogFlagRsvd && (WarnUpdate&0x10) )
        {
                Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"� ���������� ������ ����������������� ���� ������ ���������! ��������, ������ ������� ������������ �������� ����������������?";
                (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                WarnUpdate &= ~0x10;
        }

        Msg[1]=BM_SETCHECK;
        Msg[3]=0;
        Msg[0]=MEA_POVERI  , Msg[2]=IncomingLogs->LogFlagOverPI    , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=MEA_MOVERI  , Msg[2]=IncomingLogs->LogFlagOverMI    , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=MEA_IOVERV  , Msg[2]=IncomingLogs->LogFlagOverVIn   , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=MEA_OOVERV  , Msg[2]=IncomingLogs->LogFlagOverVOut  , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=MEA_BOVERT  , Msg[2]=IncomingLogs->LogFlagOverHeatBr, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
        Msg[0]=MEA_GENFAULT, Msg[2]=IncomingLogs->LogFlagFault     , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                //� ����, �� ���� �ਢ������� ⨯�� ����� �㡮� �����. ��, � ����⢨⥫쭮 ������� ���� � ᬥ��� ��� ���, ��稭�� � ᬥ饭�� 1 (�. �. 4 ����). �������, � ���� �ﬨ, � �㬠� ��ᥬ���஬. �� ������稬�.
/*        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasFaults    ) + sizeof(IncomingLogs->MeasFaults    )) return 1; Msg[0]=MEA_FAULTS   , *((double*)(Msg+1))=IncomingLogs->MeasFaults      , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasMeanIIn   ) + sizeof(IncomingLogs->MeasMeanIIn   )) return 1; Msg[0]=MEA_IIN      , *((double*)(Msg+1))=IncomingLogs->MeasMeanIIn     , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasMeanIOut  ) + sizeof(IncomingLogs->MeasMeanIOut  )) return 1; Msg[0]=MEA_IOUT     , *((double*)(Msg+1))=IncomingLogs->MeasMeanIOut    , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasVIn       ) + sizeof(IncomingLogs->MeasVIn       )) return 1; Msg[0]=MEA_VIN      , *((double*)(Msg+1))=IncomingLogs->MeasVIn         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasVOut      ) + sizeof(IncomingLogs->MeasVOut      )) return 1; Msg[0]=MEA_VOUT     , *((double*)(Msg+1))=IncomingLogs->MeasVOut        , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasPIn       ) + sizeof(IncomingLogs->MeasPIn       )) return 1; Msg[0]=MEA_PIN      , *((double*)(Msg+1))=IncomingLogs->MeasPIn         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasPOut      ) + sizeof(IncomingLogs->MeasPOut      )) return 1; Msg[0]=MEA_POUT     , *((double*)(Msg+1))=IncomingLogs->MeasPOut        , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasBr1Temper ) + sizeof(IncomingLogs->MeasBr1Temper )) return 1; Msg[0]=MEA_BRT1     , *((double*)(Msg+1))=IncomingLogs->MeasBr1Temper   , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasBr2Temper ) + sizeof(IncomingLogs->MeasBr2Temper )) return 1; Msg[0]=MEA_BRT2     , *((double*)(Msg+1))=IncomingLogs->MeasBr2Temper   , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,MeasPhaseDelay) + sizeof(IncomingLogs->MeasPhaseDelay)) return 1; Msg[0]=MEA_PDELAY   , *((double*)(Msg+1))=IncomingLogs->MeasPhaseDelay  , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
        if (IncomingLogs->Len < offsetof(MsgDebugLogs,Timestamp     ) + sizeof(IncomingLogs->Timestamp     )) return 1; Msg[0]=MEA_TIMESTAMP, *((double*)(Msg+1))=IncomingLogs->Timestamp       , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
*/   //��䨪� ����ࠨ���� ⮫쪮 �᫨ ��諮 ��� �� ᠬ� ⠩��⠬�.
//        DedicatedUpdateGraphs ();
//
  //      return 0;
//}

int DoButton (int Button)
{
//        Message.Len=0;
  //      Message.Hdr=MSG_DEBUG | CAT_CFG;
        unsigned long Msg[4]={/*ACK_PERM*/-1, BM_SETCHECK, BST_CHECKED};
        char* NameSet[5]={0};
        char Name[MAX_PATH]="Log.xls";
        int i;
        int pU32;

        switch (Button)
        {
                case RESIZE:    //���� �� ������ ���뢠���� "Redraw", ��⮬� �� ���� ��䨪� �� �࠭���� � �������. ��⮬ �㤥� �� ���ࠧ��ਢ��� ��� ����� 誠��.
                        RearrangeGraphs(-1);    //�� �㭪�� �㤥� � ���饬 �࠭��� ����� ����稭 � ����ᮢ뢠�� �� ��६���⠡�஢����, � �� �㯮 �����.

                                double T;
                                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, STEP            ))) )) TimestampStep       = T*1000; else TimestampStep       = 0;
                return 1;
                
/*                case NOAUTO:
//                        NoAuto();
                        if (TimestampStep) TimestampStep = 0;
                        else
                        {
                                double T;
                                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_AUTO_REQ            ))) )) TimestampStep       = T*1000; else TimestampStep       = 0;
                        }
			RearrangeGraphs (TimestampStep);
                        Msg[0]=1;       //����� ⠩���
                        Msg[1]=TimestampStep; //�६� ⠩���
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);
                return 1;*/
        }

        return 0;
}

//extern "C" int __export __syscall PluginEngine (unsigned long uMsg, unsigned long wParam, unsigned long lParam)
extern "C" int __export WINAPI PluginEngine (long DiaId, unsigned long uMsg, unsigned long wParam, unsigned long lParam)
{
        char Text[256];
        unsigned long Msg[4]={/*LASTTYPE*/-1, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
if (!uMsg) cout<<"WM_NULL "<<wParam<<" "<<lParam<<endl;

        if (!uMsg && (!DialogEngine && !NetEngine || (unsigned long)DialogEngine==wParam && (unsigned long)NetEngine==lParam) )
        {
                DialogEngine=(FWAPI*)(wParam);
                NetEngine=(FWAPI*)(lParam);
                cout<<"Opening a dialog by a template "<<DialogEngine<<endl;
                WCB* PluginEnginePtr[2] = {PluginEngine, NULL};

//DiaTemplate=LoadResource(hinst, FindResource (hinst, "PLUGIN", RT_DIALOG));

		if ((*DialogEngine) (GETDIALOGDATA_HEADER, -1, (unsigned long)(ParamHeaderString)))
		{	//Data prepared by GETDIALOGDATA_HEADER and GETDIALOGDATA_LINE are guaranteed to be valid (as well as their pointers) until next call, so both functions are not thread-safe.
			double *Params;

		        for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
		        	if (ModbusADCGUI[i].INIKeyName[0])
				 if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)(ModbusADCGUI[i].INIKeyName)) ) )
				 {
					ModbusADCGUI[i].MinMaxColorVal[0] = Params[0];
					ModbusADCGUI[i].MinMaxColorVal[1] = Params[2];
					ModbusADCGUI[i].MinMaxColorVal[2] = Params[3];
//cout<<"Colors by INI: "<<Params[0]<<" "<<Params[2]<<" "<<Params[3]<<endl;
				 } else ModbusADCGUI[i].MinMaxColorVal[2] = 0xFFFF00 -1 + (ModbusADCGUI[i].MinMaxColorVal[1] = 256 + (ModbusADCGUI[i].MinMaxColorVal[0] = 0) );

//cout<<(int)(ModbusSETFlags.Bytes[0])<<" "<<(int)(ModbusSETFlags.Bytes[1])<<endl;

//			double *Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("TestParam")) );
//			if (!Params) cout<<"TestParam is not present in .INI!"<<endl;
//			else for (int i=0; i<4; i++) cout<<Params[i]<<" "; cout<<endl;
		}

cout<<"Received result "<<
                /*MyDialogID = */(*DialogEngine) (OPENDIALOG, (unsigned long)(DiaTemplate), (unsigned long)(&PluginEnginePtr))
<<endl;

                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusADC)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.

		CanvasID = (*DialogEngine) (CREATECANVAS, MyDialogID, NULL);

//                return (int)("Test point 3");
//		return -1;
		return 0;
        }

	if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam )
	{
cout<<"Received copy of "<<IncomingBuffer[0]<<" : "<<IncomingBuffer[1]<<endl;
		memcpy (&ModbusADC, IncomingBuffer, sizeof (ModbusADC));
		DedicatedUpdateGraphs();
	}

// TODO!!! SEND IT ON DEVICE RECONNECT!!!
// if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam ) ModbusMain.Count = TOTALREGS;    //Return to Max Supported

/*        if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam )
        {
                cout<<"Incoming data!"<<endl;
                if (IncomingBuffer[0]==(CAT_CFG|MSG_DEBUG))
                {
                        if (IncomingBuffer[1] < 8)      //�� �����, � �����
                        {
                                sprintf (Text, "������ �� ���������� ��������� ������ %i (�������������� %i)", IncomingBuffer[1], sizeof (MsgDebug));
                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                                Msg[0]=SENDBYREQ, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg) == BST_CHECKED) //� ��� ࠧ ��ண� �� ���.
                                        DoButton (SET_CURR);    //�������筮 ������ "��ࠢ��".
                                return 1;
                        }
                        
                        sprintf (Text, "������� ���������� ��������� ������ %i (�������������� %i)", IncomingBuffer[1], sizeof (MsgDebug));
                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        if (IncomingBuffer[1] > sizeof (MsgDebug))
                        {
                                Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"���������� ������ �������, ��� �������� ������ ������ �������! ��������, ������ ������� ������������ �������� ����������������?";
                                if (DoWarning) (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                                else
                                {
                                        Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[3]=Msg[2], Msg[2]=0;
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                }
                        }
                        Msg[0]=ACK_PERM, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg))        //�� ���� ����� ��諨, � ���� �ࠢ���� � ��ࠢ����묨.
                        {
                                GrabUserInput(&Message);
                                if (memcmp((char*)(&Message)+8, (char*)(IncomingBuffer)+8, min (IncomingBuffer[1]-8UL, 30UL-8UL)))      //�� ᠬ� ���� �� �����⥫쭮.
                                {
//                                      NoAuto();       //�४�頥� ��⮫���
                                        Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Error!", Msg[2]=(int)"���������� ���-����� �� ��������� � ������ �����!";
                                        if (DoWarning) (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                                        else
                                        {
                                                Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[3]=Msg[2], Msg[2]=0;
                                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        }
                                        DoWarning=0;
                                        return 1;       //�ࠢ����� �� ��諮. �� ������塞 ����, �⮡� �� ������ �ࠢ��쭮� ���ࠢ����.
                                } else {
                                        Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[2]=0, Msg[3]=(LPARAM)(LPCTSTR)Text;
                                        strcat (Text, ", ��������� � ������ �����.");
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        Msg[0]=ACK_PERM, Msg[1]=BM_SETCHECK, Msg[2]=(WPARAM)BST_UNCHECKED, Msg[3]=(LPARAM)0;
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        //�ࠢ����� ��諮. ������塞 ����, �⮡� ���ᮢ��� �� 䫠��.
                                }
                        }
                        Msg[0]=ACK_CURR, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg))
                        {
                                Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[2]=0, Msg[3]=(LPARAM)(LPCTSTR)Text;
                                strcat (Text, ", ���� ����� �� ��������� (�� ��� ��� ���-�������������)!");
                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                return 1;       //�᫨ ��� �ࠢ�����, � �� ������塞 ����, �⮡� �� ������, ᪠���, ॣ��୮� ⥫����ਥ�.
                        }
                } else if (IncomingBuffer[0]==(CAT_MODE|MSG_DEBUG))
                {
                        if (IncomingBuffer[1] < 8)      //�� �����, � �����
                        {
                                sprintf (Text, "������ �� ��������� ��������� ������ %i (�������������� %i)", IncomingBuffer[1], sizeof (MsgDebug));
                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                                Msg[0]=SENDBYREQ, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg) == BST_CHECKED) //� ��� ࠧ ��ண� �� ���.
                                        DoButton (SET_CURR);    //�������筮 ������ "��ࠢ��".
                                return 1;
                        }

                        sprintf (Text, "������� ��������� ��������� ������ %i (�������������� %i)", IncomingBuffer[1], sizeof (MsgDebug));
                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                        if (IncomingBuffer[1] > sizeof (MsgDebug))
                        {
                                Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"���������� ������ �������, ��� �������� ������ ������ �������! ��������, ������ ������� ������������ �������� ����������������?";
                                if (DoWarning) (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                                else
                                {
                                        Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[3]=Msg[2], Msg[2]=0;
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                }
                        }
                        Msg[0]=ACK_CURR, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg))        //�� ���� ����� ��諨, � ���� �ࠢ���� � ��ࠢ����묨.
                        {
                                GrabUserInput(&Message);
                                if (memcmp((char*)(&Message)+8, (char*)(IncomingBuffer)+8, min (IncomingBuffer[1]-8UL, 30UL-8UL)))      //�� ᠬ� ���� �� �����⥫쭮.
                                {
//                                      NoAuto();       //�४�頥� ��⮫���
                                        Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Error!", Msg[2]=(int)"���������� ���-����� �� ��������� � ������ �����!";
                                        if (DoWarning) (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                                        else
                                        {
                                                Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[3]=Msg[2], Msg[2]=0;
                                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        }
                                        DoWarning=0;
                                        return 1;       //�ࠢ����� �� ��諮. �� ������塞 ����, �⮡� �� ������ �ࠢ��쭮� ���ࠢ����.
                                } else {
                                        Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[2]=0, Msg[3]=(LPARAM)(LPCTSTR)Text;
                                        strcat (Text, ", ��������� � ������ �����.");
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        Msg[0]=ACK_CURR, Msg[1]=BM_SETCHECK, Msg[2]=(WPARAM)BST_UNCHECKED, Msg[3]=(LPARAM)0;
                                        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                        //�ࠢ����� ��諮. ������塞 ����, �⮡� ���ᮢ��� �� 䫠��.
                                }
                        }
                        Msg[0]=ACK_PERM, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                        if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg))
                        {
                                Msg[0]=LASTTYPE, Msg[1]=WM_SETTEXT, Msg[2]=0, Msg[3]=(LPARAM)(LPCTSTR)Text;
                                strcat (Text, ", ���� ����� �� ��������� (�� ��� ��� ���-�������������)!");
                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                return 1;       //�᫨ ��� �ࠢ�����, � �� ������塞 ����, �⮡� �� ������, ᪠���, ॣ��୮� ⥫����ਥ�.
                        }
                } else if (IncomingBuffer[0]==(CAT_LOGS|MSG_DEBUG))
                {
                        return ProcessLogs((MsgDebugLogs*)IncomingBuffer);      //"����� ��१�" ��� �뭥ᥭ�� ���� � ���騩 �⤥��� ������
                } else return 0;        //�� ��� �����
                
                DoWarning = 0;  //����� ������ �ᯫ뢠�饣� ���誠 �� ���� ࠧ �� �뢠��
                //����� ���ᨬ �, �� ��諮, � ���� ����� ���, ������� ᫠����� � �. �.

                Incoming = (MsgDebug*)IncomingBuffer;
                
                if (DoLog) LogToFile (Incoming, "Incoming: ");

                if (Incoming->Len >= offsetof(MsgDebug,AutoSend        ) + sizeof(Incoming->AutoSend        )) 
                {
                        RearrangeGraphs (Incoming->AutoSend);   //��� 誠�� ��䨪� == 蠣 ��⮮�ࠢ��
                }
                if (Incoming->Len >= offsetof(MsgDebug,GenFreq         ) + sizeof(Incoming->GenFreq         )) 
                {
                        Msg[0]=SLIDERFREQ, Msg[1]=TBM_SETPOS, Msg[2]=(WPARAM)(BOOL)TRUE, Msg[3]=Incoming->GenFreq;
                        (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                }
                if (Incoming->Len >= offsetof(MsgDebug,GenAmplit       ) + sizeof(Incoming->GenAmplit       )) 
                {
                        Msg[0]=SLIDERAMPL, Msg[1]=TBM_SETPOS, Msg[2]=(WPARAM)(BOOL)TRUE, Msg[3]=Incoming->GenAmplit;
                        (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
                }

                //� ����, �� ���� �ਢ������� ⨯�� ����� �㡮� �����. ��, � ����⢨⥫쭮 ������� ���� � ᬥ��� ��� ���, ��稭�� � ᬥ饭�� 1 (�. �. 4 ����). �������, � ���� �ﬨ, � �㬠� ��ᥬ���஬. �� ������稬�.
                if (Incoming->Len < offsetof(MsgDebug,AutoSend        ) + sizeof(Incoming->AutoSend        )) return 1; Msg[0]=SET_RSVD     , *((double*)(Msg+1))=Incoming->AutoSend         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenFreq         ) + sizeof(Incoming->GenFreq         )) return 1; Msg[0]=SET_FREQ     , *((double*)(Msg+1))=Incoming->GenFreq     /10.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenWForm        ) + sizeof(Incoming->GenWForm        )) return 1; Msg[0]=SET_WFORM    , *((double*)(Msg+1))=Incoming->GenWForm         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenAmplit       ) + sizeof(Incoming->GenAmplit       )) return 1; Msg[0]=SET_AMPL     , *((double*)(Msg+1))=Incoming->GenAmplit    *5.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenMeanILim     ) + sizeof(Incoming->GenMeanILim     )) return 1; Msg[0]=SET_MILIM    , *((double*)(Msg+1))=Incoming->GenMeanILim /10.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenPulseILim    ) + sizeof(Incoming->GenPulseILim    )) return 1; Msg[0]=SET_PLIM     , *((double*)(Msg+1))=Incoming->GenPulseILim/10.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenVOutLim      ) + sizeof(Incoming->GenVOutLim      )) return 1; Msg[0]=SET_VOLIM    , *((double*)(Msg+1))=Incoming->GenVOutLim   *5.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenVInLim       ) + sizeof(Incoming->GenVInLim       )) return 1; Msg[0]=SET_VILIM    , *((double*)(Msg+1))=Incoming->GenVInLim    *5.0, (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenSoftStartRamp) + sizeof(Incoming->GenSoftStartRamp)) return 1; Msg[0]=SET_RAMP     , *((double*)(Msg+1))=Incoming->GenSoftStartRamp , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Incoming->GenLoopsSwitch  )) return 1; Msg[0]=SET_LOOPS    , *((double*)(Msg+1))=Incoming->GenLoopsSwitch   , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

                if (Incoming->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Incoming->GenLoopsSwitch  ) + 2) return 1; 
                if (Incoming->ModeFlagRsvd)
                {
                        Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"� ���������� ������ ����������������� ���� ������ ���������! ��������, ������ ������� ������������ �������� ����������������?";
                        (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                }
                Msg[1]=BM_SETCHECK;
                Msg[3]=0;
                Msg[0]=SET_ILOOPON   , Msg[2]=Incoming->ModeFlagILoop, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_VLOOPON   , Msg[2]=Incoming->ModeFlagVLoop, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_FREQLOOPON, Msg[2]=Incoming->ModeFlagFLoop, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_FREQSCANON, Msg[2]=Incoming->ModeFlagFScan, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=SET_PLOOPON   , Msg[2]=Incoming->ModeFlagPLoop, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                //�᫨ �� ������ �� �⮣� ����, � ��� ��諨 �� ����ன�� � ����� ���-��ࠢ���� ᫠����� � ���-�⢥��� �� �����.
                Msg[0]=SLIDERSEND, Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg)) {Msg[1]=BM_SETCHECK, Msg[2]=BST_CHECKED, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);}  //�⠢�� ����� ���-��ࠢ�� ᫠���஢, �᫨ �� �⮣� �뫠 "���"
                Msg[0]=SENDBYREQ , Msg[1]=BM_GETCHECK, Msg[2]=Msg[3]=0;
                if ((*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg)) {Msg[1]=BM_SETCHECK, Msg[2]=BST_CHECKED, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);}  //�⠢�� ����� ���-��ࠢ�� �� ������, �᫨ �� �⮣� �뫠 "���"
                                                       
                if (Incoming->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Incoming->GenLoopsSwitch  ) + 4) return 1; 
                if (Incoming->LogFlagRsvd)
                {
                        Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"� ���������� ������ ����������������� ���� ������ ���������! ��������, ������ ������� ������������ �������� ����������������?";
                        (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                }
                Msg[1]=BM_SETCHECK;
                Msg[3]=0;
                Msg[0]=MEA_POVERI  , Msg[2]=Incoming->LogFlagOverPI    , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=MEA_MOVERI  , Msg[2]=Incoming->LogFlagOverMI    , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=MEA_IOVERV  , Msg[2]=Incoming->LogFlagOverVIn   , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=MEA_OOVERV  , Msg[2]=Incoming->LogFlagOverVOut  , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=MEA_BOVERT  , Msg[2]=Incoming->LogFlagOverHeatBr, (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                Msg[0]=MEA_GENFAULT, Msg[2]=Incoming->LogFlagFault     , (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);

                if (Incoming->Len < offsetof(MsgDebug,MeasMeanIIn   ) + sizeof(Incoming->MeasMeanIIn   )) return 1; Msg[0]=MEA_IIN      , *((double*)(Msg+1))=Incoming->MeasMeanIIn     , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasMeanIOut  ) + sizeof(Incoming->MeasMeanIOut  )) return 1; Msg[0]=MEA_IOUT     , *((double*)(Msg+1))=Incoming->MeasMeanIOut    , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasVIn       ) + sizeof(Incoming->MeasVIn       )) return 1; Msg[0]=MEA_VIN      , *((double*)(Msg+1))=Incoming->MeasVIn         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasVOut      ) + sizeof(Incoming->MeasVOut      )) return 1; Msg[0]=MEA_VOUT     , *((double*)(Msg+1))=Incoming->MeasVOut        , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasPIn       ) + sizeof(Incoming->MeasPIn       )) return 1; Msg[0]=MEA_PIN      , *((double*)(Msg+1))=Incoming->MeasPIn         , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasPOut      ) + sizeof(Incoming->MeasPOut      )) return 1; Msg[0]=MEA_POUT     , *((double*)(Msg+1))=Incoming->MeasPOut        , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasBr1Temper ) + sizeof(Incoming->MeasBr1Temper )) return 1; Msg[0]=MEA_BRT1     , *((double*)(Msg+1))=Incoming->MeasBr1Temper   , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasBr2Temper ) + sizeof(Incoming->MeasBr2Temper )) return 1; Msg[0]=MEA_BRT2     , *((double*)(Msg+1))=Incoming->MeasBr2Temper   , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasPhaseDelay) + sizeof(Incoming->MeasPhaseDelay)) return 1; Msg[0]=MEA_PDELAY   , *((double*)(Msg+1))=Incoming->MeasPhaseDelay  , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,MeasFaults    ) + sizeof(Incoming->MeasFaults    )) return 1; Msg[0]=MEA_FAULTS   , *((double*)(Msg+1))=Incoming->MeasFaults      , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                if (Incoming->Len < offsetof(MsgDebug,Timestamp     ) + sizeof(Incoming->Timestamp     )) return 1; Msg[0]=MEA_TIMESTAMP, *((double*)(Msg+1))=Incoming->Timestamp       , (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

                UpdateGraphs (Incoming);        //��䨪� ����ࠨ���� ⮫쪮 �᫨ ��諮 ��� �� ᠬ� ⠩��⠬�.

//for (int i=0; i<768*256*3; i++) GraphArea[i]=rand();
//AddDash (GraphArea, 768, 256, 10, 10, rand()%128    , 0x0000FF);
//AddDash (GraphArea, 768, 256,  0,  5, rand()%128+127, 0x00FF00);
//AddDash (GraphArea, 768, 256,  0, 10, rand()%256    , 0xFF0000);

/*              if (IncomingBuffer[0]==(CAT_LOGS|MSG_DEBUG))    //�ᥣ�� �஢��塞, ����� �� ���� ��᪮�쪮 ��ॣ���
                {
                        cout<<"Accepted!"<<endl;

                        char Text[256];
                        unsigned long Msg[4]={0, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
                        float Lamps[7]={0};
                        for (int i=0; i<6; i++)
                        {
                                Msg[0]=1001+i;  //���� ��⮬ ��-祫����᪨ ᤥ���
                                sprintf (Text, "����������� ������������ ����� %i = %i", i+1, IncomingBuffer[i+2]);
                                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
                                Lamps[6]+=IncomingBuffer[i+2];
                        }
                        for (int i=0; i<6; i++) Lamps[i]=(float)(IncomingBuffer[i+2])*2*3.1415926536/Lamps[6];

//                      for (int i=0; i<256*256*3; i++) GraphArea[i]=rand();
                        for (int y=0, i=0; y<256; y++)  //����஥��� �⠭������ ����ࠬ, ����筮, ⮦� �㤥� �� �३���થ, � �� � ������ ��㣥 ᢮�.
                         for (int x=0; x<256; x++)      //��� ⮣� �३���ન � �㦭�. ��७��� �� 室� ����. � ��⨬������, �ᥭ ����.
                         {
                                if ( (x-128)*(x-128)+(y-128)*(y-128)>127*127 ||
                                        x==128 && y==128 )      
                                {
                                        GraphArea[i++] = 192;
                                        GraphArea[i++] = 192;
                                        GraphArea[i++] = 192;
                                        continue;
                                }
                                float a=atan2(y-128,x-128)+3.1415926536, aa;
                                int n;
                                for (aa=n=0; n<6; aa+=Lamps[n++]) if (aa>a) break;
                                GraphArea[i++] = (n/4)*127;
                                GraphArea[i++] = (n/2)*127;
                                GraphArea[i++] = (n  )*127;
                         }
                        (*DialogEngine) (UPDATECANVAS, MyDialogID, (unsigned long)(GraphArea));                 
//for (int i=0; i<256*256*3; i++) GraphArea[i]=rand();
                }*/
//        }

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED) return DoButton(wParam&0xFFFF);

        if (uMsg==WM_INITDIALOG)
        {
		MyDialogID = lParam;
cout<<"Newly created Dialog ID = "<<MyDialogID<<endl;
//              memset (GraphArea, 192, 768*256*3);
//              (*DialogEngine) (UPDATECANVAS, MyDialogID, (unsigned long)(GraphArea));                 

                unsigned long Msg[4]={0, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, (LPARAM)0};

                for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
                {
			for (int j=0; j<3; j++)
			{
	                        Msg[0]=ModbusADCGUI[i].MinMaxColorId[j];
        	                *((double*)(Msg+1))=ModbusADCGUI[i].MinMaxColorVal[j];
//cout<<"Setting "<<ModbusADCGUI[i].MinMaxColorVal[j]<<endl;
                	        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

	                        Msg[0]=ModbusADCGUI[i].MinMaxColorVal[j];
        	                Msg[1]=(int)(ModbusADCGUI[i].ToolTip);
                	        (*DialogEngine) (ATTACHTOOLTIP, MyDialogID, (int)Msg);
                	}
                }

                double T;
                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, STEP            ))) )) TimestampStep       = T*1000; else TimestampStep       = 0;

                cout<<"Cleaning"<<endl;
                RearrangeGraphs(-1);
        }
        
//      return 0xDEADC0DE;
        return 0;
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        if (fdwReason==DLL_PROCESS_ATTACH)
        {

//              hinst=hinstDLL;
                DiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "LOG_GRA", RT_DIALOG));
                cout<<"Plug Init "<<DiaTemplate<<" "<<fdwReason<<endl;
//cout<<"Supported Size: "<<sizeof(MsgDebugLogs)<<endl;
        }

        return 1;
}
