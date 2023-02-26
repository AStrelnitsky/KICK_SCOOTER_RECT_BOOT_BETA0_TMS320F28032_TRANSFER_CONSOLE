#define OPEN_WATCOM_DIRTY_HACK
#include "Log_Man.rc"
#endif
//#include <windows.h> 㦥 ����祭� � ��吝�� 堪�
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <math.h>

#include "DiaWrap.h"
#include "Protocol.h"

//HINSTANCE hinst;

FWAPI *DialogEngine=NULL, *NetEngine=NULL;
int MyDialogID=0;
HGLOBAL DiaTemplate;
int* IncomingBuffer=NULL;

fstream Log;
int DoLog=0, DoWarning=0;

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

//unsigned long LastTimestamp = 0xFFFFFFFF;
unsigned long TimestampStep = 0;    //�᫨ ����, � �� �ᮢ���. ����� ����, � ����� �� ���� �����६���� � ᫥���騬.
//int NoDraw = 1;                     //�� �뫮 �������� ����஥�. �� �ᮢ���.


struct
{
    double Scale;               //� ��� ��� ���� �� ॣ�����, � ����. ���⮬� ���ᮢ����� ����ﬨ� ����� (ᬥ饭�� �� ��易⥫쭮 2, ��� ������ �१ Size).
    int ControlId;
    int Size;
    char ToolTip[128];
    char INIKeyName[64];
}  ModbusADCGUI[] =
{
1000,   MEA_ADC_TIMESTAMP,2,      "Timestamp, S",                                            "",
10,     MEA_I_IN         ,2,      "������������ ���, �",                                     "MEA_I_IN",  
2 ,     MEA_V_IN         ,2,      "������� ����������, �",                                   "MEA_V_IN",  
10,     MEA_I_OUT        ,2,      "�������� ���, �",                                         "MEA_I_OUT", 
2 ,     MEA_V_OUT        ,2,      "�������� ����������, �",                                  "MEA_V_OUT", 
1 ,     MEA_W_IN         ,2,      "������������ ������� ��������, ��",                       "MEA_W_IN",  
1 ,     MEA_W_OUT        ,2,      "���������� ������� ��������, ��",                         "MEA_W_OUT", 
2 ,     MEA_P_IN         ,2,      "������������ �������, ���*�",                             "MEA_P_IN",  
1 ,     MEA_Tb1          ,1,      "����������� 1 ��������� ����������, C",                   "MEA_Tb1",   
1 ,     MEA_Tb2          ,1,      "����������� 2 ��������� ����������, C",                   "MEA_Tb2",   
1 ,     MEA_Tpfc         ,1,      "����������� ������ pfc, C",                               "MEA_Tpfc",  
1 ,     MEA_Tpfc_b       ,1,      "����������� ����� pfc, C",                                "MEA_Tpfc_b",
.1,     MEA_Light        ,2,      "���������� ������������, ��",                             "MEA_Light", 
10,     MEA_F_m          ,2,      "������� ������� �������, ���",                            "MEA_F_m",   
1 ,     MEA_PWM_m        ,2,      "������� �������� ���, %",                                 "MEA_PWM_m", 
10,     MEA_I_U_m        ,2,      "������� ����������� �������� ������������ �� ����, �",    "MEA_I_U_m", 
1 ,     MEA_Ph_I         ,2,      "������ �������� ���� ����, ��.",                          "MEA_Ph_I",  
10,     MEA_ei           ,2,      "������� �������� ������ ������������, �",                 "MEA_ei",    
10,     MEA_acc_i        ,2,      "������� �������� ������������ �����������, �",            "MEA_acc_i",
10,     MEA_PID_out      ,2,      "������� ��a����� ������ PID ����������, �",               "MEA_PID_out",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1 ,     MEA_reserve      ,2,      "������",                                                  "",
1,      MEA_STATUS       ,2,      "��������� ����������",                                     "MEA_STATUS"
};

struct
{
	int CheckboxId;
	char ToolTip[128];
} ModbusADCFlagsGUI[TOTAL_ADC_FLAG_BYTES*8] = {
MEA_CB_THD_Ext_T_High		,"����� ��������� ������ �� ����������� 1 �������� �������",
MEA_CB_THD_Vpwr_Low_R		,"����� ��������� ������ �� �������� ��������������",
MEA_CB_THD_Vpwr_High_R		,"����� ��������� ������ �� �������� ��������������",
MEA_CB_THD_Ipwr_High_R		,"����� ��������� ������ �� ���������� �� �������� ����",
MEA_CB_THD_HB1_V_High_R		,"����� ��������� ������ �� �������������� 1 ���������",
MEA_CB_THD_HB1_I_High_R		,"����� ��������� ������ �� ������� ���������� 1 ���������",
MEA_CB_THD_HB1_Iiso_High_R	,"����� ��������� ������ ������� ���� 1 ���������",
MEA_CB_THD_HB1_T_High_R		,"����� ��������� ������ �� ��������� 1 ���������",
MEA_CB_THD_HB2_V_High_R		,"����� ��������� ������ �� �������������� 2 ���������",
MEA_CB_THD_HB2_I_High_R		,"����� ��������� ������ �� ������� ���������� 2 ���������",
MEA_CB_THD_HB2_Iiso_High_R	,"����� ��������� ������ ������� ���� 2 ���������",
MEA_CB_THD_HB2_T_High_R		,"����� ��������� ������ �� ��������� 2 ���������",
MEA_CB_THD_Ext_T1_High_R	,"����� ��������� ������ �� ����������� 2 �������� �������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_SYSTEM_ERROR		,"����� ���������� ������",
MEA_CB_LINE_BREAK		,"����� ������ ������ �����",
MEA_CB_CHORT_CIRCUIT_LINE	,"����� ������ ��������� ��������� �� �����",
MEA_CB_HIGH_LEAKGAGE		,"����� ������ ���������� ������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������",
MEA_CB_reserve			,"������"
};


char ParamHeaderString[] = "MixedPlugin	Minimum	Current Value	Maximum	#Graph. Color";

int GrabUserInput ()      //�������� �� ����, ࠧ��� �� ���⠢��� (�� ��।������ �����祭��� �����, �. �. ����⮩ �������).
{
        unsigned long Msg[4]={0, BM_GETCHECK};

        double T;

		//��� �ॢ�⨫�� � ��ࢨ������ �����᪨� ������, �� � ࠧ�������� �������� ��� ������ ��ࠢ���� ᠬ �� ᥡ�.
        memset ((char*)&ModbusADCFlags+8, 0, sizeof(ModbusADCFlags)-8);
        for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
        	if (ModbusADCFlagsGUI[i].CheckboxId != MEA_CB_reserve) 
		        ModbusADCFlags.Bytes[i/8] |= ( 
		          (*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)&(Msg[0]=ModbusADCFlagsGUI[i].CheckboxId) ) != 0
		        )<<(i%8);
		else ModbusADCFlags.Bytes[i/8] |= 1<<(i%8);	//Don't reset unknown flags!

        return 0;
}

void LogToFile (MsgDebug* Data, char* Name)
{
        Log<<Name;
        if (Data->Len < offsetof(MsgDebug,AutoSend        ) + sizeof(Data->AutoSend        )) {Log<<endl; return;} Log<<"       "<<Data->AutoSend        ;
        if (Data->Len < offsetof(MsgDebug,GenFreq         ) + sizeof(Data->GenFreq         )) {Log<<endl; return;} Log<<"       "<<Data->GenFreq         ;
        if (Data->Len < offsetof(MsgDebug,GenWForm        ) + sizeof(Data->GenWForm        )) {Log<<endl; return;} Log<<"       "<<(int)(Data->GenWForm        );
        if (Data->Len < offsetof(MsgDebug,GenAmplit       ) + sizeof(Data->GenAmplit       )) {Log<<endl; return;} Log<<"       "<<Data->GenAmplit       ;
        if (Data->Len < offsetof(MsgDebug,GenMeanILim     ) + sizeof(Data->GenMeanILim     )) {Log<<endl; return;} Log<<"       "<<Data->GenMeanILim     ;
        if (Data->Len < offsetof(MsgDebug,GenPulseILim    ) + sizeof(Data->GenPulseILim    )) {Log<<endl; return;} Log<<"       "<<Data->GenPulseILim    ;
        if (Data->Len < offsetof(MsgDebug,GenVOutLim      ) + sizeof(Data->GenVOutLim      )) {Log<<endl; return;} Log<<"       "<<Data->GenVOutLim      ;
        if (Data->Len < offsetof(MsgDebug,GenVInLim       ) + sizeof(Data->GenVInLim       )) {Log<<endl; return;} Log<<"       "<<Data->GenVInLim       ;
        if (Data->Len < offsetof(MsgDebug,GenSoftStartRamp) + sizeof(Data->GenSoftStartRamp)) {Log<<endl; return;} Log<<"       "<<Data->GenSoftStartRamp;
        if (Data->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Data->GenLoopsSwitch  )) {Log<<endl; return;} Log<<"       "<<Data->GenLoopsSwitch  ;

        if (Data->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Data->GenLoopsSwitch  ) + 2) {Log<<endl; return;}
        Log<<"  "<<Data->ModeFlagILoop;
        Log<<"  "<<Data->ModeFlagVLoop;
        Log<<"  "<<Data->ModeFlagFLoop;
        Log<<"  "<<Data->ModeFlagFScan;
        Log<<"  "<<Data->ModeFlagPLoop;

        if (Data->Len < offsetof(MsgDebug,GenLoopsSwitch  ) + sizeof(Data->GenLoopsSwitch  ) + 4) {Log<<endl; return;}
        Log<<"  "<<Data->LogFlagOverPI    ;
        Log<<"  "<<Data->LogFlagOverMI    ;
        Log<<"  "<<Data->LogFlagOverVIn   ;
        Log<<"  "<<Data->LogFlagOverVOut  ;
        Log<<"  "<<Data->LogFlagOverHeatBr;
        Log<<"  "<<Data->LogFlagFault     ;

        if (Data->Len < offsetof(MsgDebug,MeasMeanIIn   ) + sizeof(Data->MeasMeanIIn   )) {Log<<endl; return;}    Log<<"        "<<Data->MeasMeanIIn   ;
        if (Data->Len < offsetof(MsgDebug,MeasMeanIOut  ) + sizeof(Data->MeasMeanIOut  )) {Log<<endl; return;}    Log<<"        "<<Data->MeasMeanIOut  ;
        if (Data->Len < offsetof(MsgDebug,MeasVIn       ) + sizeof(Data->MeasVIn       )) {Log<<endl; return;}    Log<<"        "<<Data->MeasVIn       ;
        if (Data->Len < offsetof(MsgDebug,MeasVOut      ) + sizeof(Data->MeasVOut      )) {Log<<endl; return;}    Log<<"        "<<Data->MeasVOut      ;
        if (Data->Len < offsetof(MsgDebug,MeasPIn       ) + sizeof(Data->MeasPIn       )) {Log<<endl; return;}    Log<<"        "<<Data->MeasPIn       ;
        if (Data->Len < offsetof(MsgDebug,MeasPOut      ) + sizeof(Data->MeasPOut      )) {Log<<endl; return;}    Log<<"        "<<Data->MeasPOut      ;
        if (Data->Len < offsetof(MsgDebug,MeasBr1Temper ) + sizeof(Data->MeasBr1Temper )) {Log<<endl; return;}    Log<<"        "<<(int)(Data->MeasBr1Temper );
        if (Data->Len < offsetof(MsgDebug,MeasBr2Temper ) + sizeof(Data->MeasBr2Temper )) {Log<<endl; return;}    Log<<"        "<<(int)(Data->MeasBr2Temper );
        if (Data->Len < offsetof(MsgDebug,MeasPhaseDelay) + sizeof(Data->MeasPhaseDelay)) {Log<<endl; return;}    Log<<"        "<<Data->MeasPhaseDelay;
        if (Data->Len < offsetof(MsgDebug,MeasFaults    ) + sizeof(Data->MeasFaults    )) {Log<<endl; return;}    Log<<"        "<<Data->MeasFaults    ;
        if (Data->Len < offsetof(MsgDebug,Timestamp     ) + sizeof(Data->Timestamp     )) {Log<<endl; return;}    Log<<"        "<<Data->Timestamp     ;

        Log<<endl;
}

void DedicatedLogToFile (MsgDebugLogs* Data, char* Name)
{
        Log<<Name;
        for (int i=0; i<15; i++) Log<<" ";

        if (Data->Len < 10 ) {Log<<endl; return;}
        Log<<"  "<<Data->LogFlagOverPI    ;
        Log<<"  "<<Data->LogFlagOverMI    ;
        Log<<"  "<<Data->LogFlagOverVIn   ;
        Log<<"  "<<Data->LogFlagOverVOut  ;
        Log<<"  "<<Data->LogFlagOverHeatBr;
        Log<<"  "<<Data->LogFlagFault     ;

        if (Data->Len < offsetof(MsgDebugLogs,MeasFaults    ) + sizeof(Data->MeasFaults    )) {Log<<endl; return;}    Log<<"    "<<Data->MeasFaults    ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasMeanIIn   ) + sizeof(Data->MeasMeanIIn   )) {Log<<endl; return;}    Log<<"    "<<Data->MeasMeanIIn   ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasMeanIOut  ) + sizeof(Data->MeasMeanIOut  )) {Log<<endl; return;}    Log<<"    "<<Data->MeasMeanIOut  ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasVIn       ) + sizeof(Data->MeasVIn       )) {Log<<endl; return;}    Log<<"    "<<Data->MeasVIn       ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasVOut      ) + sizeof(Data->MeasVOut      )) {Log<<endl; return;}    Log<<"    "<<Data->MeasVOut      ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasPIn       ) + sizeof(Data->MeasPIn       )) {Log<<endl; return;}    Log<<"    "<<Data->MeasPIn       ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasPOut      ) + sizeof(Data->MeasPOut      )) {Log<<endl; return;}    Log<<"    "<<Data->MeasPOut      ;
        if (Data->Len < offsetof(MsgDebugLogs,MeasBr1Temper ) + sizeof(Data->MeasBr1Temper )) {Log<<endl; return;}    Log<<"    "<<(int)(Data->MeasBr1Temper );
        if (Data->Len < offsetof(MsgDebugLogs,MeasBr2Temper ) + sizeof(Data->MeasBr2Temper )) {Log<<endl; return;}    Log<<"    "<<(int)(Data->MeasBr2Temper );
        if (Data->Len < offsetof(MsgDebugLogs,MeasPhaseDelay) + sizeof(Data->MeasPhaseDelay)) {Log<<endl; return;}    Log<<"    "<<Data->MeasPhaseDelay;
        if (Data->Len < offsetof(MsgDebugLogs,Timestamp     ) + sizeof(Data->Timestamp     )) {Log<<endl; return;}    Log<<"    "<<Data->Timestamp     ;

        Log<<endl;
}

void ShowValues (void)
{
        unsigned long Msg[4];

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
}

void ShowFlags (void)
{
        unsigned long Msg[4];
        Msg[1]=BM_SETCHECK;
        Msg[3]=0;
        for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
        {
	        Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
	        Msg[2]=(ModbusADCFlags.Bytes[i/8]>>(i%8)) & 1;
	        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
	}
}

int RequestADC ()
{
        int pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusADC));
        if (pU32)
        {
                memcpy (&ModbusADC, (int*)pU32, sizeof (ModbusADC));
                ShowValues ();
        } else cout<<"Error! No ADC received"<<endl;	//ToDo: Log line!!!

        pU32 = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(&ModbusADCFlags));
        if (pU32)
        {
                memcpy (&ModbusADCFlags, (int*)pU32, sizeof (ModbusADCFlags));
                ShowFlags();
	} else cout<<"Error! No ADC Flags received"<<endl;	//ToDo: Log line!!!

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
                case MEA_RESET:
//                      Msg[1]=BM_SETCHECK;
                        Msg[2]=BST_UNCHECKED;
	                for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
	                {
			        Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
			        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
			}
                return 1;

                case MEA_SET:
//                      Msg[1]=BM_SETCHECK;
                        Msg[2]=BST_INDETERMINATE;
	                for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
	                {
			        Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
			        (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
			}
                return 1;

		case IDOK:
//                        TrakDeadTime = timeGetTime();
//                        DoWarning = 1;
                        if (GrabUserInput()) return 1;  //�訡�� ����� �����, ���᫮�� ᨬ���� � �. �.
                        (*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(&ModbusADCFlags));
                return 1;

                case OPENCSV:
                        if (DoLog) return 1;
                        NameSet[0]=(char*)API_SAVEFILE;
                        NameSet[1]=Name;
                        i=(*DialogEngine) (OPENFILEROLLER, MyDialogID, (int)NameSet);
                        if (!i) return 1;       //���� �⪠����� ��� �訡�� ����� API_[BLABLABLA]
cout<<Name<<" "<<(Name+i)<<endl;        //����� pathname � �⤥�쭮� ���
                        Log.open (Name, ios::out|ios::text);
                        if (Log.fail())
                        {
                                Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"Can't open file!";
                                (*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
                                return 1;
                        }
                        DoLog=1;

                        Log<<"Name      Auto-send each...       Generator Freq  Generator Waveform      Generator A     Generator Mean I Limit  Generator Pulse I Limit Generator Output V Limit        Generator Input V Limit Generator Soft Start Ramp       Generator Loop Switch";
                        Log<<"  I Loop Flag     V Loop Flag     Freq Loop Flag  Freq Scan Flag  Power Loop Flag Pulse Overcurrent!      Mean Overcurrent!       Input Overvoltage!      Output Overvoltage!     Bridge Overheat!        Generic Fault!";
                        Log<<"  Mean Input I    Mean Output I   Input V Output V        Input Power     Output Power    Bridge 1 Temperature    Bridge 2 Temperature    Phase Delay     Faults  Timestamp"<<endl;
cout<<"Log opened!"<<endl;
                return 1;
                case CLOSECSV:
                        if (!DoLog) return 1;
                        Log.close();
                        DoLog=0;
cout<<"Log closed!"<<endl;
                return 1;
                
                case NOAUTO:
//                        NoAuto();
                        if (TimestampStep) TimestampStep = 0;
                        else
                        {
                                double T;
                                if (isfinite(T= *((double*)((*DialogEngine)(TEXTTODOUBLE, MyDialogID, SET_AUTO_REQ            ))) )) TimestampStep       = T*1000; else TimestampStep       = 0;
                        }
//			RearrangeGraphs (TimestampStep);
                        Msg[0]=1;       //����� ⠩���
                        Msg[1]=TimestampStep; //�६� ⠩���
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);
                return 1;
        }

        return 0;
}


//extern "C" int __export __syscall PluginEngine (unsigned long uMsg, unsigned long wParam, unsigned long lParam)
extern "C" int __export WINAPI PluginEngine (long DiaId, unsigned long uMsg, unsigned long wParam, unsigned long lParam)
{
        char Text[256];
        unsigned long Msg[4]={LASTTYPE, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
//if (!uMsg) cout<<"WM_NULL "<<wParam<<" "<<lParam<<" "<<IncomingBuffer<<" ";
//if (IncomingBuffer) cout<<IncomingBuffer[0]<<" : "<<IncomingBuffer[1]<<endl;

        if (!uMsg && (!DialogEngine && !NetEngine || (unsigned long)DialogEngine==wParam && (unsigned long)NetEngine==lParam) )
        {
                DialogEngine=(FWAPI*)(wParam);
                NetEngine=(FWAPI*)(lParam);
                cout<<"Opening a dialog by a template "<<DialogEngine<<endl;
                WCB* PluginEnginePtr[2] = {PluginEngine, NULL};

                /*MyDialogID = */(*DialogEngine) (OPENDIALOG, (unsigned long)(DiaTemplate), (unsigned long)(&PluginEnginePtr));

                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusADC)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.
                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusADCFlags)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.

                return 0;//(int)("Test point 2");
        }

	if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam )
	{
cout<<"Received copy of "<<IncomingBuffer[0]<<" : "<<IncomingBuffer[1]<<endl;
		if (IncomingBuffer[0] == ModbusADC.Addr)
		{
			memcpy (&ModbusADC, IncomingBuffer, sizeof (ModbusADC));
			ShowValues();
		} else if (IncomingBuffer[0] == ModbusADCFlags.Addr)
		{
			memcpy (&ModbusADCFlags, IncomingBuffer, sizeof (ModbusADCFlags));
			ShowFlags();
		}
	}

// TODO!!! SEND IT ON DEVICE RECONNECT!!!
// if (!uMsg && IncomingBuffer && (unsigned long)IncomingBuffer==wParam && IncomingBuffer[1]==lParam ) ModbusMain.Count = TOTALREGS;    //Return to Max Supported

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED) return DoButton(wParam&0xFFFF);

        if (uMsg==WM_TIMER)     //��ࠡ�⪠  ��⮮���a �� ⠩����.
        {
  //              if (wParam == 1)
    //            {
                        Msg[0]=1;       //����� ⠩���
                        Msg[1]=0;       //�६� ⠩��� = 0 (�⪫���� ⠩���)
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);

                        RequestADC();
                        
                        Msg[0]=1;       //����� ⠩���
                        Msg[1]=TimestampStep; //�६� ⠩���
                        (*DialogEngine) (TIMER, MyDialogID, (int)Msg);
                        
                        return 1;
//                }
        }

        if (uMsg==WM_INITDIALOG)
        {
		MyDialogID = lParam;
cout<<"Newly created Dialog ID = "<<MyDialogID<<endl;

                unsigned long Msg[4]={0, BM_SETCHECK, (WPARAM)BST_INDETERMINATE, (LPARAM)0};
                for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
		{
	                Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
	                (*DialogEngine) (SENDMDIALOG, MyDialogID, (int)Msg);
		}
                for (int i=0; i<TOTAL_ADC_FLAG_BYTES*8; i++)
		{
                        Msg[0]=ModbusADCFlagsGUI[i].CheckboxId;
                        Msg[1]=(int)(ModbusADCFlagsGUI[i].ToolTip);
                        (*DialogEngine) (ATTACHTOOLTIP, MyDialogID, (int)Msg);
		}

                for (int i=0,b=0; b<TOTAL_ADC*2; b+=ModbusADCGUI[i++].Size)
                {
                        Msg[0]=ModbusADCGUI[i].ControlId;
                        Msg[1]=(int)(ModbusADCGUI[i].ToolTip);
                        (*DialogEngine) (ATTACHTOOLTIP, MyDialogID, (int)Msg);
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
                DiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "LOG_MAN", RT_DIALOG));
                cout<<"Plug Init "<<DiaTemplate<<" "<<fdwReason<<endl;
//cout<<"Supported Size: "<<sizeof(MsgDebugLogs)<<endl;
        }

        return 1;
}
