#pragma pack (push)
#pragma pack (0)

//������� 16 ��� (���������)
#define CAT_LOGS        0x0000  //���� (�������������)
#define CAT_CFG         0x0001  //������������ (����������) ������������ (�������������)
#define CAT_MODE        0x0002  //������������ ��������� (����� ������, �������������)
#define CAT_SECU        0xFFFF  //������� (�����, ������...), ��� ��� ����� ��� ��������� ���� ��������� ������������ ����� �������, �� ���������.

//������� 16 ��� (������� ������ 0, ��� ���������)
#define MSG_DEBUG       0xFFFF0000      //���������, ��� ������������������ �������. ��������� �� ���� ���������� �����.

//Logs
#define MSG_RESERVED    0x00000000      //��� 0 ��������� 0 -- ��������� "��� ���������"
#define MSG_TEMPER      0x00010000      //��� 1 ��������� 0 -- ���� �����������, ��������

//Cfg
#define MSG_MAXLAMPS    0x00000000      //��� 0 ��������� 1 -- ����� ����, ��������

//Mode and Cfg
struct MsgDebug //������� ������� ���������, ������� ��� �� ���������. �������� �������� CAT_LOGS, ���� ��� �� �� ������, �� ��� ������ �����.
{
        unsigned long   Hdr;    //=MSG_DEBUG | CAT_CFG ��� ������� ��������, MSG_DEBUG | CAT_MODE ��� ��������� ��������
        unsigned long   Len;    //������ ����������� � ����������� ��.

        unsigned short AutoSend;        //����������� ����� "��������, �� ����!" ������������ � �������� 10 � ���� ���������� = 0.

        unsigned short GenFreq;
        unsigned char GenWForm; 
        unsigned char Sparse;           //����� �� ������������ ������ ���� �������� ����
        unsigned short GenAmplit, GenMeanILim, GenPulseILim, GenVOutLim, GenVInLim, GenSoftStartRamp, GenLoopsSwitch;

        unsigned short ModeFlagILoop: 1;        //������� ������������ ��� � ����� ������������ ������������. ��� ������� ������������ "����������" (��� ����� �����). ��� ������������ �������� �������� � �������� ������.
        unsigned short ModeFlagVLoop: 1;
        unsigned short ModeFlagFLoop: 1;
        unsigned short ModeFlagFScan: 1;
        unsigned short ModeFlagPLoop: 1;
        unsigned short ModeFlagRsvd: 11;        //������ 0 �� ������ � ������ 0 �� ����� (�. �. "�� ��������").

        //��� ��� �������� ������� ����� ����������� � ������. ���� �� ����� ������ ����������, ���������� � ������ 30 ���� (�. �. �� ��� ����� �����).
        //����� ��, ����� ����� ��������� �� ������ �������� ("������"), Ctrl+X Ctrl+V ����� �� ���� �����.

        //���������� ����� ������ � ���������, �� �� ���� ����� ���������� � ������ � ������ 32, ����� ��������. ����� �� ����������, ���������� � ������ 30 ��� � ������ 32 � �����, �� � ���������.
        unsigned short LogFlagOverPI: 1;
        unsigned short LogFlagOverMI: 1;
        unsigned short LogFlagOverVIn: 1;
        unsigned short LogFlagOverVOut: 1;
        unsigned short LogFlagOverHeatBr: 1;
        unsigned short LogFlagRsvd: 10;         //������ 0 �� ������ � ������ 1 �� ����� (�. �. "�� ����������").
        unsigned short LogFlagFault: 1;

        //����������, ������ �����, ����� ������ ��� ������. � ���� ������ � Faults �������� (��� ���� ���������� ��� ���?)
        unsigned short MeasMeanIIn, MeasMeanIOut, MeasVIn, MeasVOut, MeasPIn, MeasPOut;
        unsigned char MeasBr1Temper, MeasBr2Temper;
        unsigned short MeasPhaseDelay, MeasFaults;
        //����� ���� 50 ����. ������������� ���-�� ������ �� ���� -- �� ����� ����� ������ �������� � �����, � ��������� (����� � "�������") ���������� �� ������� � �����������.
        //����� ����, ������ "������" ������ ������, ����� �������� ������ ����, ���� ���� �� ������������ (��� �������� �� ���� ���, ����� ������ ����������). � ���������� ����� �������� ���.
        unsigned short Timestamp;       //��� ���� ����� read-only.
        //����� 52 � ������� ����� (���� ���� ��� �� � ������� ����������, � ������� ����� ���� �� �����), � � ������� ����������� -- �� ���������������. ��������, ���� ��������� ����������, ����� �� ������� ������ ����������� ���������.
};

struct MsgDebugLogs     //��������� ���������������������
{
        unsigned long   Hdr;    //=MSG_DEBUG | CAT_LOGS
        unsigned long   Len;    //30 �� ������ � �����, 12 �� ����� � ������ (������ ���������� ���������� �� �����, � �. �. faults)

        //O��������� � ������, ����� ��������. ����� �� ����������, ���������� � ���������.
        unsigned short LogFlagOverPI: 1;
        unsigned short LogFlagOverMI: 1;
        unsigned short LogFlagOverVIn: 1;
        unsigned short LogFlagOverVOut: 1;
        unsigned short LogFlagOverHeatBr: 1;
        unsigned short LogFlagRsvd: 10;         //������ 0 �� ������ � ������ 1 �� ����� (�. �. "�� ����������").
        unsigned short LogFlagFault: 1;

        unsigned short MeasFaults;      //���������� 0x0000, ����� ��������, � 0xFFFF, ����� �� ����������.
        unsigned short MeasMeanIIn, MeasMeanIOut, MeasVIn, MeasVOut, MeasPIn, MeasPOut;
        unsigned char MeasBr1Temper, MeasBr2Temper;
        unsigned short MeasPhaseDelay;
        unsigned short Timestamp;
};

//Security
#define MSG_LOGIN       0x00000000      //��� 0 ��������� FFFF: ����� (�� �� � �����������).
struct MsgLogin //��������� � �������� 8 ��� �������
{
        unsigned long   Hdr;    //=CAT_SECU|MSG_LOGIN
        unsigned long   Len;    //=4*sizeof(unsigned long), ���� �� ������������ � ��������� ���������� ReplyTo ����� ������. 

        unsigned long   WhoAreYou;      //MCU IP
        unsigned long   Ports;          //MCU Port in lower word, PC port in upper word (WiP)
//      unsigned long   ReplyTo;        //PC IP (WiP)
};      //� �������� ���������� ����� ��� ���������� ������� ����������� ����, �. �. SHA-256 ������� ��������� ������ �� ������ ������ � ��������� ������.

#define MSG_SEED        0x00000001      //��� 1 ��������� FFFF: ����� (�� ����������� � ��).
struct MsgSeed  //��������� � �������� 8 ��� ������������� �������
{
        unsigned long   Hdr;    //=CAT_SECU|MSG_LOGIN
        unsigned long   Len;    //=10*sizeof(unsigned long)

        unsigned long   RndSeed[8];     //��������� �����, ����������� ��� ��������� ������� (�. �. ��� ��������� ��� SHA-256 �� ������, ��������� ������ � ����� �����).
};


//�� � ��� �����, � ������ ��������� ����� ���� 65536 ������ ���������� ��������, ������ ������ ��������� ����� �� ������ � ������ ����������, �� ����� "������������� ������".
//���� ������ ������ 0 (�. �. ������ ������ �������������� ���������, ���� ���� �� �����) -- ��� "��������� ��������", ������� ��������, ��� ��������� �� ����������, � �� �������������.
//�� � ����� � �� ������, � �� ��������� ��������� ������������ ����� � ������������� �������, ��� ��������� ������������ �� ������� �� � ������� ������ (��������, ��������� ����� ���� ������, ���� ����� ������������ ������ ������� ������).
//����� ��������� ��� SHA-256 ������� �� ������ U32, ������� �� �������� ������ ��������� (�. �. �� ���������� � � �����).
//����� U32 �� ������ ������ (���� ��, �������� ����� �� TCP ��� �����-������ IR, � ��������-�� ���������). ���� ������� U16 ������������� ��� ��������� 4 �����.





//����������� ����� (�����, ����� ��������)
//#define PC_TO_BOARD 2233
//#define BOARD_TO_PC 7


#pragma pack (pop)
