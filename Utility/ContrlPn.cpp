#define OPEN_WATCOM_DIRTY_HACK
#include "ContrlPn.rc"
#endif
//#include <windows.h> 㦥 ����祭� � ��吝�� 堪�
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <math.h>
#include <process.h>

#include <commctrl.h>   //������ (᫠�����)

#include "DiaWrap.h"
#include "Protocol.h"
#include "ContrlPn.h"

//float WattsIn, WattsOut, LevelPercentage, EfficiencyRatio, Amperage, TT, TR,
//	BiggestAmperage, kEff;
//int  Status, IsConnected, BatteryMode;
/*
volatile _DXData DXData={7100, 1800, 0.85, 0.901, 5.11, 0, 150,
			5.11, .9,
			0, 0, 0};
/*/
volatile _DXData DXData={0};
// */
volatile _DXReq DXReq={0, 0, 0};
volatile float LoopedGraph[4096];
volatile int LoopedGraphIndex;
volatile int RequestedW, RequestedH;

HINSTANCE hinst;

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

double BiggestAmperage=0, kEff=1, Fullscreen=0;

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
        
        BiggestAmperage = *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, BIGGEST_I  )));
        kEff            = *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, EFFK       )));

        return 0;
}

int NotifyDirectXThread ()	//���� �뫮 ������ �� �᭮�� �������, � �� ��䮭��. � ��� ��⮬ � ᮫��.
{
        unsigned long Msg[4];
	double Eff = 1;

        for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
        {
//                Msg[0]=ModbusADCGUI[i].ControlId;
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

		switch (ModbusADCGUI[i].MinMaxColorId[2])
		{
			case MEA_W_IN_GRAPH_COLOR:
				DXData.WattsIn = *((double*)(Msg+1));
				Eff /= *((double*)(Msg+1));
			break;
			case MEA_W_OUT_GRAPH_COLOR:
				DXData.WattsOut = *((double*)(Msg+1));
				Eff *= *((double*)(Msg+1));
			break;
			case MEA_I_OUT_GRAPH_COLOR:
				DXData.Amperage = *((double*)(Msg+1));
				LoopedGraph[(LoopedGraphIndex+1)%4096]=*((double*)(Msg+1));
				LoopedGraphIndex=(LoopedGraphIndex+1)%4096;
				if (BiggestAmperage < DXData.Amperage) BiggestAmperage=DXData.Amperage;
				DXData.BiggestAmperage=BiggestAmperage;
			break;
			case MEA_Tb1_GRAPH_COLOR:
				DXData.TT = *((double*)(Msg+1));
			break;
			case MEA_Tb2_GRAPH_COLOR:
				DXData.TR = *((double*)(Msg+1));
			break;
			case MEA_STATUS_GRAPH_COLOR:
				DXData.Status = *((double*)(Msg+1));
			break;
		}

        }

if (DXData.WattsIn < 100) Eff = 0;

	DXData.EfficiencyRatio = Eff*kEff;

	if (BiggestAmperage) DXData.LevelPercentage = 100.0 * (.9*BiggestAmperage - DXData.Amperage) / (.9*BiggestAmperage);
	else DXData.LevelPercentage = 0;

        Msg[0]=BIGGEST_I;
        *((double*)(Msg+1))=BiggestAmperage;
        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

	DXData.kEff=kEff;
	//DXData.BatteryMode sets immediately on startup.

	return 0;
}

int LastConnState = 0;
void SyncGlobalsWithThread()
{
	if (DXReq.Close) (*DialogEngine)(SETSYSTEMSTATE, MyDialogID, API_DEAD);
	if (DXReq.Connect)
	{
		if (!LastConnState)
		{
			(*DialogEngine)(SETSYSTEMSTATE, MyDialogID, API_CONNECTED);
			LastConnState = 1;
		}
	} else {
		if (LastConnState)
		{
			(*DialogEngine)(SETSYSTEMSTATE, MyDialogID, API_ALIVE);
			LastConnState = 0;
		}
	}
	
	DXData.IsConnected = ((*DialogEngine)(GETSYSTEMSTATE, MyDialogID, 0) == API_CONNECTED);

	//DXReq.On �⤠���� �� ������ "���ﭨ� �����筮�� reg u16"
}

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

int DirectXEngine(HINSTANCE hThisInst);

static void DrawingThread(void *arglist)
{
	DirectXEngine((HINSTANCE) arglist);
        _endthread();
}

volatile HWND DXhWnd=NULL;

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
                
                case FULLSCREEN:
//DirectXEngine(hinst);
			if (DXhWnd) return 1;	//already exists!
			DXhWnd = (HWND)-1;	//First of all, block re-opening the DX window.
			if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, FULLSCREENW  ))) )) RequestedW = T+.1; else RequestedW = 1920;
			if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, FULLSCREENH  ))) )) RequestedH = T+.1; else RequestedH = 1080;

                        SyncGlobalsWithThread();

	                Msg[0]=1;       //����� ⠩���
        	        Msg[1]=10; //�६� ⠩���
                	(*DialogEngine) (TIMER, MyDialogID, (int)Msg);

                	_beginthread(DrawingThread, 256, (void*)hinst);	//The thread itself will reset DXhWnd to zero when needed.
                return 1;
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

			if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("DetectedIOut")) ) )
				BiggestAmperage = Params[2];
				else BiggestAmperage = 0;

			if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("AdditionalLoss_kEff")) ) )
				kEff = Params[2];
				else kEff=1;

			if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("IsBattery")) ) )
				DXData.BatteryMode = Params[2];
				else DXData.BatteryMode=1;

			if ( Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("Fullscreen")) ) )
				Fullscreen = Params[2];
				else Fullscreen = 0;

//cout<<(int)(ModbusSETFlags.Bytes[0])<<" "<<(int)(ModbusSETFlags.Bytes[1])<<endl;

//			double *Params = (double*) ( (*DialogEngine) (GETDIALOGDATA_LINE, -1, (unsigned long)("TestParam")) );
//			if (!Params) cout<<"TestParam is not present in .INI!"<<endl;
//			else for (int i=0; i<4; i++) cout<<Params[i]<<" "; cout<<endl;
		}

cout<<"Received result "<<
                /*MyDialogID = */(*DialogEngine) (OPENDIALOG, (unsigned long)(DiaTemplate), (unsigned long)(&PluginEnginePtr))
<<endl;

                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusADC)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.
		Msg[0]=40218;
		Msg[1]=1;
                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(Msg)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.

		CanvasID = (*DialogEngine) (CREATECANVAS, MyDialogID, NULL);

//                return (int)("Test point 3");
//		return -1;
		return 0;
        }

	if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam )
	{
cout<<"Received copy of "<<IncomingBuffer[0]<<" : "<<IncomingBuffer[1]<<endl;
		if (IncomingBuffer[0] != ModbusADC.Addr)
		{
cout<<"Error: address not "<<ModbusADC.Addr<<endl;
			return 0;
		}
		memcpy (&ModbusADC, IncomingBuffer, sizeof (ModbusADC));
		NotifyDirectXThread();
		DedicatedUpdateGraphs();
		return 1;
	}

	if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]== - lParam )
	{
cout<<"Requested copy of "<<IncomingBuffer[0]<<" : "<<IncomingBuffer[1]<<endl;
		if (IncomingBuffer[0] != 40218)
		{
cout<<"Error: address not "<<40218<<endl;
			return 0;
		}
		IncomingBuffer[2] = DXReq.On;
		return 1;
	}

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED) return DoButton(wParam&0xFFFF);

        if (uMsg==WM_TIMER)
        {
  //              if (wParam == 1)
    //            {
                        Msg[0]=1;       //����� ⠩���
                        Msg[1]=0;       //�६� ⠩��� = 0 (�⪫���� ⠩���)
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);

			if (DXhWnd)	//���� ���� ��� ������
			{
	                        SyncGlobalsWithThread();
                        
        	                Msg[0]=1;       //����� ⠩���
                	        Msg[1]=10; //�६� ⠩���
                        	(*DialogEngine) (TIMER, MyDialogID, (int)Msg);
                        }

                        return 1;
//                }
        }

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
                Msg[0]=BIGGEST_I;
                *((double*)(Msg+1))=BiggestAmperage;
	        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);
                Msg[0]=EFFK;
                *((double*)(Msg+1))=kEff;
	        (*DialogEngine) (DOUBLETOTEXT, MyDialogID, (int)Msg);

                double T;
                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, STEP            ))) )) TimestampStep       = T*1000; else TimestampStep       = 0;

                cout<<"Cleaning"<<endl;
                RearrangeGraphs(-1);

		if (Fullscreen) DoButton(FULLSCREEN);
        }
        
//      return 0xDEADC0DE;
        return 0;
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        if (fdwReason==DLL_PROCESS_ATTACH)
        {

              hinst=hinstDLL;
                DiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "CONTRLPN", RT_DIALOG));
                cout<<"Plug Init "<<DiaTemplate<<" "<<fdwReason<<endl;
//cout<<"Supported Size: "<<sizeof(MsgDebugLogs)<<endl;
        }

        return 1;
}
