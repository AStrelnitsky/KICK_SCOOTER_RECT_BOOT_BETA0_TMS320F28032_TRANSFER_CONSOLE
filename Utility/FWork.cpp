//���⥬� �㬥�樨 ���ᨩ: A.B.C.
//A -- ���訩 �����. ��������� ����砥� ����� ᮢ���⨬��� � ����஫��ࠬ� �� �஢�� ��⮪���. �������, ⠪ � ��⠭���� �� ��� 1.
//B -- �।��� �����. ��������� ����砥� ����� ᮢ���⨬��� �������� � �३���મ�. �. �. ��������� �ਤ���� ��� ����, �� ����஫��ࠬ ��९�訢�� �� �஦���.
//C -- ����訩 �����. ��������� ����砥� ��࠭���� ᮢ���⨬��� �� ��� ����� ������, �������� ����� ��஭��.
//����� ����� -- 0.0.0 (��� �裡 � ॠ��� ����஫��஬, ⮫쪮 � ���樥�).
//����� ��������, ��� �裡 � ॠ��� ����஫��஬ -- 0.0.1 (��� ������᭮�� ������).
//����� ��������, ��� �裡 � ॠ��� ����஫��஬ -- 1.0.1 (��� ������᭮�� ������, ����� ��ॢ��� �� ������).
//����� ��������, ��� �裡 � ॠ��� ����஫��஬ -- 1.1.1 (��� ������᭮�� ������, ������ ������⢥��� �������, ����� ��ॢ��� �� ������).

#define OPEN_WATCOM_DIRTY_HACK
#include "FWork.rc"
#endif
//#include <windows.h> 㦥 ����祭� � ��吝�� 堪�
#include <iostream.h>
#include <fstream.h>
#include <process.h>
#include <stdio.h>
#include <math.h>	//for NAN constant

#include <commctrl.h>   //������ (᫠�����)
#include <shlobj.h>     //���뢠誠 ��४�਩

#include "DiaWrap.h"
#include "Protocol.h"

HINSTANCE g_hInst;

//WCB *Client;
//int IsModal;    //����� �����ন���� ⮫쪮 ���� ������ � ����� ��������
//HWND DiaHdl;    //�� �⤠� ��� �������, �⮡� �� �뫮 ᮡ����� ������� ������ ������

struct
{
	WCB* ClientCallback;	//��������� ��ࠡ��稪 �������. � ⥮ਨ ����� �������� ��᪮�쪮 �/��� �ਢ易�� � ������ ࠧ�� ������� (⮣�� � ᯨ᪥ �㤥� �������騥�� 㪠��⥫�). -1 �� �३�� � ���� ����砥� "ᮧ���-�� ����� ���⠭��!", � -1 �� ��㣠 � �३�� ����砥� "����-���⠭樨 �� �����ন������" (��᫥���� ���� ⮫쪮 ��� ����, ��� ���⠭樨 ���� ������ ������).
	int  ClientInstance;	//���ᬠ�ਢ��� 㪠��⥫� �� ��ࠡ��稪 � ��� ����� ��� ����� 64-���� �����. ��� ࠧ�묨 ��ﬨ ��� ����� ��-ࠧ���� ���५�஢���.
	HWND ClientDiaHdl;	//����, ᮮ⢥�����饥 ��ࠡ��稪�. ��� �� �� ������ ����������, �������� ������⥫쭮 �������� ᯨ᮪ ���⮢묨 ��ࠡ��稪��� � �㫥�� ����� (�. �. ����� ������ ���뢠�� ��ࢮ� ����, �᫨ �� 㪠�뢠�� �� �� ��ࠡ��稪 -- ��१���襬 ���).
	BOOL ClientDiaIsModal;
}	PluginsSet [2048] = {0};//����譨�� �������� = �������. ����筮, ��������, �� �� ����� ᮮ⢥��⢮���� ���� (��������� ������), �� �� � �� ��०����� �� �⮬� ������.
int TotalPlugins = 0;

HWND Parent=NULL;	//����⥫�᪮� ���� ��� ��� ������� ���� -- ���� �३���ઠ.

struct
{
	HBITMAP WinBitmap;
	BITMAPINFO DIBFormat;
	HDC hDC,Comp_hDC;
	HWND DialogWnd;
}	Graphs [2048] = {0};
//int TotalGraphs=0;		//���� �㯮 ����� ����� ��� � ����� �������, ���� ⮦� �����頥� ��� ����� ������� ����� ���, �. �. �� ������ ���� �� ������.

struct
{
	int DialogId;
	unsigned long Addr, Size;
} Incoming [2048] = {0};
int TotalIncoming=0;

int Timeout=5000000;

#define MENUW 16        //���室�� ��ਮ�. ����筮, �㤥� ���������� �������� � �������� �� �����⥫��⢠�.
#define MENUH 120
#define MINVSIZE 11
#define MINHSIZE 7

int INIAlreadyOpen = 0;
fstream INIFile;

int ColOrder[1024], ColCount;	//ToDo: separate for each dialog (or, at least, check for being the same).
int LineOffset[8192], LineCount;
int ProcessINIHeader (char *Parameters)
{
	char Buffer[16384];
	char Request[16384];
	char *P, *Pp;
	int i, j;
			
	strcpy (Request, ((char*)(Parameters)));
	for (P=Request; *P; P++) if (*P == '	') *P=0;
	P[1]=0;	//double zero-term.

	//ToDo: different table formats for different dialogs, if possible.
	INIFile.getline (Buffer, 16384);
	cout<<"Actual header: "<<Buffer<<endl;

	for (P=Buffer; *P; P++) if (*P == '	') *P=0;
	P[1]=0;	//double zero-term.

	if ( !stricmp (Request, Buffer) )
	{
		cout<<"Dialog name matches: "<<Request<<endl;
		for (ColCount=0, P=Request+strlen(Request)+1; *P; P+=strlen(P)+1, ColCount++)
		{
			ColOrder[ColCount]=-1;
			for (j=0, Pp=Buffer+strlen(Buffer)+1; *Pp; Pp+=strlen(Pp)+1, j++)
			{
				if (!stricmp (P, Pp)) ColOrder[ColCount]=j;
			}
		}
cout<<"Parameter column order (in file):"; for (i=0;i<ColCount;i++) cout<<" "<<ColOrder[i]; cout<<endl;			
		//ToDo: suggest unused columns instead of missing ones (in order to support "Value_Mode_I" or "Value_Mode_F" instead of "Value")

		for (i=LineCount=0;i<8192;i++)
		{
			LineOffset[i] = INIFile.tellg();
			INIFile.getline (Buffer, 16384);
//cout<<"@"<<LineOffset[i]<<">"<<Buffer<<endl;
			P=Buffer;
			while (*P==' ') P++;	//Trim spaces
			if (*P<14) break;	//Null-term, CR, LF, CR/LF or Tab ( == empty parameter name).
			LineOffset[i] += P-Buffer;
//cout<<"@"<<LineOffset[i]<<"->"<<P<<endl;
			LineCount++;
		}

//INIFile.clear();
//for (i=0;i<LineCount;i++){INIFile.seekg(LineOffset[i]);INIFile.getline (Buffer, 16384);cout<<"@ "<<LineOffset[i]<<" Found "<<Buffer<<endl;}

		return 1;

	} else cout<<"Dialog "<<Request<<" does not match the table "<<Buffer<<endl;
	return 0;
}

double ValueBuffer[512];
double* ProcessINILine (char *KeyName)
{
	int i, j, k;
	char Buffer[16384], *P, *Pp, *Delim, Ch;

//cout<<"Searching "<<KeyName<<" in "<<LineCount<<endl;
	for (i=0; i<LineCount; i++)
	{
		INIFile.clear();

		INIFile.seekg(LineOffset[i]);
		INIFile.getline (Buffer, 16384);
//cout<<Buffer<<"?"<<KeyName<<endl;
		for (P=Buffer; *P!='	'; P++) if (*P==0) return NULL;
//else cout<<*P;
		Delim=P;
		do {*P=0; P--;} while (P>Buffer && *P==' ');//Trim spaces
//cout<<Buffer<<" "<<KeyName<<endl;
		if (stricmp(Buffer, KeyName)) continue;	//ToDo: QSort LineOffset in alphabetical order and search bit-by-bit (dichotomy).

		for (P=Delim+1; *P; P++) if (*P == '	') *P=0;
		P[1]=0;	//double zero-term.
		for (j=0; j<ColCount; j++)
		{
			if (ColOrder[j]<0)
			{
				ValueBuffer[j] = NAN;
				continue;
			}
			for (k=0, P=Delim+1;   k<ColOrder[j];   k++, P+=strlen(P)+1) if (*P==0) return NULL;

//			cout<<"found key "<<P<<endl;

			//ToDo: support $String, 0xHex etc. headers (Hex is needed for saving mostly; on loading, I can tell from # or 0x).
			while (*P==' ') P++;	//needed for detecting NAN
                        if (P[0]=='#')       //Color Hexadecimal
                        {
                        	Ch=P[-1];
                                P[-1]='0';	//There are always characters there (at least the key hame and it's null-term).
                                P[0]='x';
                                ValueBuffer[j] = strtod (P-1, &Pp);
                                P[-1]=Ch;	//restore the value;
                                Pp--;	//+1 accordingly to (P-1), but -2 because 0x is always processed, even in case of failure.
                        } else ValueBuffer[j] = strtod (P, &Pp);
                        if (P[0]=='0' && (P[1]=='x'||P[1]=='X')) Pp-=2;	//native 0x, not #
//cout<<(int)P<<":"<<(int)Pp<<endl;
                        if (P==Pp) ValueBuffer[j] = NAN;
		}
		return ValueBuffer;
	}

cout<<"Key Name not found: "<<KeyName<<endl;
	return NULL;
}

/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvv PROGRESS BAR THREAD vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
volatile int PBarValue=0, PBarState=0;

BOOL __export PASCAL PrBarFunc (HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
	char Text[64];
        if (uMsg == WM_INITDIALOG)
        {
        	SetTimer (hWnd, 0, 100, NULL);
		PBarState = 1;
        }
        if (uMsg == WM_TIMER)
        {
        	if (PBarValue<100)
        	{
        		sprintf (Text, "%i%% Completed", PBarValue);
        		SetDlgItemText (hWnd, PBARSTATE, Text);
        	} else {
	        	PBarState = 0;
        		EndDialog (hWnd, NULL);
        	}
        }
        if (uMsg==WM_COMMAND && lParam && wParam==(BN_CLICKED<<16)+PBARABORT)
        {
        	PBarState = 0;
       		EndDialog (hWnd, NULL);
        }
        if (uMsg == WM_CLOSE) 
        {
        	PBarState = 0;
       		EndDialog (hWnd, NULL);
        }
        return FALSE;
}

static void ProgressBarThread(void *arglist)
{
	DialogBoxParam(GetModuleHandle(NULL),"PROGBAR",NULL,PrBarFunc, NULL);
        _endthread();
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^ PROGRESS BAR THREAD ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


int FileRoller (HWND DiaHdl, char* PathSet[], int Mode )
{
        OPENFILENAME Name={sizeof(OPENFILENAME), DiaHdl, /*hInstance*/ NULL, PathSet[1], 0, 0, 0, PathSet[0], MAX_PATH, NULL, 0, PathSet[3], NULL, OFN_EXPLORER, 0, 0, PathSet[2], 0, NULL, ""};

        switch (Mode)
        {
                case API_OPENDIR:
                        //��� �����⮢�⥫�� �⠯ ��吝��� 堪� (������� �⠡����� ����� 䠩�� � �. �., �� �� ��⮬ ᤥ����).
                case API_SAVEFILE:
                        Name.Flags |= OFN_EXPLORER|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
                        if ( !GetSaveFileName(&Name) ) return 0;
                        if (Mode == API_OPENDIR) PathSet[0][Name.nFileOffset]=0;        //��吝� 堪 -- ���㡨�� ��� 䠩��, ��⠢��� ⮫쪮 ��४���.
                        return Name.nFileOffset;        //�� �� ����� ���� �㫥��, ��⮬� �� ���� ��� ������ ��� ������ "c:\", ��� ����ᮬ ��� ������ "/". ����� pathname �� ��稭����� � ����� �������.
                break;

                case API_LOADFILES:
                        Name.Flags = OFN_ALLOWMULTISELECT;
                case API_LOADFILE:
                        Name.Flags |= OFN_FILEMUSTEXIST|OFN_EXPLORER|OFN_HIDEREADONLY  |  OFN_CREATEPROMPT;
                        if ( !GetOpenFileName(&Name) ) return 0;
                        return Name.nFileOffset;        //�� �� ����� ���� �㫥��, ��⮬� �� ���� ��� ������ ��� ������ "c:\", ��� ����ᮬ ��� ������ "/". ����� pathname �� ��稭����� � ����� �������.
                break;
        }

        return 0;       //��������� ⨯ �������.
}

//DirRoller (requires COM, blocks multithreading)
//BROWSEINFO Dir = {DiaHdl, NULL, (char*)(Parameters[0]), NULL /*""?*/, BIF_USENEWUI, NULL, 0, 0}; // Variable that receives the image associated with the selected folder. The image is specified as an index to the system image list. WAT???
//return SHBrowseForFolder (&Dir);

BOOL __export PASCAL WinFuncWrapper (HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
//        DiaHdl=hWnd;
	int ToDo;

	if (uMsg == WM_INITDIALOG)
	{
cout<<"Registering self as "<<lParam<<endl;
		PluginsSet[lParam].ClientDiaHdl = hWnd;
		if (lParam == TotalPlugins) TotalPlugins++;
	        return (*(PluginsSet[lParam].ClientCallback))(PluginsSet[lParam].ClientInstance, uMsg, wParam, lParam);
	}

	WCB *Client = NULL;
	int CliInst;
	for (int i=0; i<TotalPlugins; i++) if (PluginsSet[i].ClientDiaHdl == hWnd)
	{
		Client  = PluginsSet[i].ClientCallback;
		CliInst = PluginsSet[i].ClientInstance;
		ToDo=i;
		break;
	}
	if (!Client)
	{
//		cout<<"Warning! Dialog/plugin not found by hWnd in "<<TotalPlugins<<", Msg = "<<uMsg<<endl;
		return FALSE;	//����� ����� ᫨�� ��ࠡ��稪 ��㣮� � �३�� �������, ���ਢ FwWinFuncBeta (��ࠡ��뢠�� "���, �� �� ������" � ��ࠡ��稪� �३��), �� �� �।�⥫��⢮ -- ���ॡ���� ��㣠 � �३�� �� ᮢ������ �ࠪ��᪨ �� � ��.
	}

        switch (uMsg)
        {
        //�ॡ��� �࠯��஢:
                case WM_HSCROLL:
                case WM_VSCROLL:
                        if ((wParam&0xFFFF)==SB_THUMBPOSITION || (wParam&0xFFFF)==SB_THUMBTRACK) return (*Client)(CliInst, uMsg, wParam, GetDlgCtrlID((HWND)lParam));    //������� �� ࠡ���� � ����� ����. �����.
                return (*Client)(CliInst, uMsg, (wParam&0xFFFF)|(SendMessage ((HWND)lParam, TBM_GETPOS, 0, 0 )<<16), GetDlgCtrlID((HWND)lParam));        //�� ���� ��᫠ �� ��।����� ����稭�, �᫨ �� ����� ��।��� � ���� ��� ��।���.
        
        //�ॡ��� ��-�࠯��஢:
                case WM_CLOSE:
//                        ReleaseDC(hWnd,hDC);
  //                      ReleaseDC(hWnd,Comp_hDC);
    //                    DeleteObject(WinBitmap);
                break;
                case WM_PAINT:  //��� ���� ��� �ᠩ� �஢�����, �� ��㪠 �� ��뢠���� �� 㬥��襭�� ����
//cout<<"Blitter: "<<
                        BitBlt(Graphs[ToDo].hDC,MENUW,MENUH,768,256,Graphs[ToDo].Comp_hDC,0,0,SRCCOPY);
//cout<<endl;
                break;
                case WM_SIZE:
                	//ToDo: auto-resizing graphs.
//                        DeleteObject(WinBitmap);
  //                      DIBFormat.bmiHeader.biWidth=768;
    //                    DIBFormat.bmiHeader.biHeight=256;
      //                  WinBitmap=CreateCompatibleBitmap(hDC,768,256);  //���� �����, ࠧ��� 䨪�஢��. � 祫���� ������, ���� �ᨫ�� �� ����� ��ࠦ������ ��᫠, ��� ���� ॠ������� � �� ���窨 ���� ᤥ���� :)
//cout<<"ResBitmap: "<<WinBitmap<<endl;
                break;
        }

        return (*Client)(CliInst, uMsg, wParam, lParam);
}

int DialogAPIWrapper (HWND hWnd, unsigned long *Parameters)
{
        switch (Parameters[1])  //� ���� ��� �������� ������ �⠭������ ᮮ�饭�� �������, ᮮ⢥��⢥���, �� ����� ����஢��� �㤠 㣮���, �᫨ ���� ��.
        {
//              case WM_SETTEXT:	//���� � ᯨ᮪ �ᯮ��㥬�� �� 䠪�� WM_, �⮡� � ��砥 ����஢���� �� ����� ������� �筮 �����, �� �㦭� ���������஢���.
//              case WM_GETTEXT:
//              case BM_SETCHECK:
//              case BM_GETCHECK:
//              case TBM_SETRANGE: case TBM_GETPOS: case TBM_SETPOS:
                default:        //�� �����騩 ������ �࠯���� �� ���ॡ�������.
                                //���������騥 ������ (�⠩, 0) -- ��ࠢ�� � ������� ����.
                        if (!GetDlgItem(hWnd,Parameters[0])) return SendMessage (hWnd, Parameters[1], (WPARAM)(Parameters[2]), (LPARAM)(Parameters[3]));
                                //�������騥, ����筮, ������� ᢮� ᮮ�饭��.
                return SendMessage (GetDlgItem(hWnd,Parameters[0]), Parameters[1], (WPARAM)(Parameters[2]), (LPARAM)(Parameters[3]));
        }
        return 0;
}

volatile int RunningThread = 0;

sockaddr_in address={0};
//int TCPSocket=INVALID_SOCKET, SlaveSocket=INVALID_SOCKET;
volatile int TCPSocket=INVALID_SOCKET, SlaveSocket=INVALID_SOCKET;	//ToDo: ������� ��-��᪨ ����, �⮡� ��� ᠬ� ��५����� �� ����⨫쪨 � TCPSocket, ��䨣 ������� ������ �裡 ����� �।��� �� ������ ���⫠���������.
static unsigned long TCPBuffer[16380]={0};      //���� ����� � ᯥ�䨪�樨 ���� ��� 0x00000000 ���������� ��ன

#define SB_FREE 0	//���� ���� (�� 1 � 0 ��ॢ���� �३�, �� 3 � 4 � 0 ��ॢ���� ᫥���।).
#define SB_DATA 1	//� ���� �室�騥 ����� (�� 0 � 1 ��ॢ���� ᫥���।)
#define SB_REQ 2	//� ���� ����� ����஥� (��� �������� ������ �� �३�� � ������, �� 0 � 2 ��ॢ���� ᫥���। � ����).
#define SB_ANS 3	//� ���� �⢥� ������� (�� 2 � 3 ��ॢ���� �३�)
#define SB_FAIL 4	//� ���� ����⥫�� �⢥� (�� � ������ �� �������� ��� ⠪�� ������ � �����⮢������ ����, �� 2 � 4 ��ॢ���� �३�).
volatile unsigned long SlaveBuffer[16380]={0};
volatile int SlaveBufferState;	//� ������ ���� �ᯮ������ ��� ��।�� ����� ᮪��
#define ENGINE_TIMEOUT 50
//volatile int IsData=0;

/*static void WaitingThread(void *arglist)
{
cout<<"Listening TCP..."<<endl;
        fd_set onesocket;
        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        cout<<select (0xDEADC0DE, &onesocket, NULL, NULL, NULL)<<endl;
        IsData=1;
cout<<"Got something!"<<endl;
        PostMessage ((HWND)arglist, WM_NULL, 0xDEADC0DE, 0xDEADBEEF); //wake the dialog window
        _endthread();
}
*/

BOOL CreateServer (int *SocketTCPBase, sockaddr_in *address, int port)
{
        if (*SocketTCPBase!=INVALID_SOCKET) return FALSE;    //already created

        memset(address,0,sizeof(sockaddr_in));
        if ((*SocketTCPBase = socket (AF_INET, SOCK_STREAM,0 )) < 0) 
        {
                cout<<"Error creating: "<<GetLastError()<<endl;
                return FALSE;
        }
        address->sin_family = AF_INET;
        address->sin_port = htons(port); 
        address->sin_addr.s_addr = htonl (INADDR_ANY);
        if(bind(*SocketTCPBase,(struct sockaddr *)address, sizeof(struct sockaddr_in)) == -1)  
        {
                cout<<"Error binding: "<<GetLastError()<<endl;
                return FALSE;
        }
        if (listen (*SocketTCPBase,1)==SOCKET_ERROR)
        {
                cout<<"Error listening: "<<GetLastError()<<endl;
                return FALSE;
        }
        int Temp=1;     //any non-zero;
        if (ioctlsocket (*SocketTCPBase,FIONBIO,(unsigned long*)&Temp)==SOCKET_ERROR)
        {
                cout<<"Error setting nonblock mode: "<<GetLastError()<<endl;
                return FALSE;
        }
//      Temp=1;     //any non-zero; <-- AFAIK ioctlsocket is non-destructive
        if ( setsockopt (*SocketTCPBase, IPPROTO_TCP, TCP_NODELAY, (char*)&Temp, 4)==SOCKET_ERROR)
        {
                cout<<"Error setting nobuffer mode: "<<GetLastError()<<endl;
                return FALSE;
        }
        cout<<"Server created!"<<endl;

	return TRUE;
}

BOOL CheckForClients(int *SocketTCP, int SocketTCPBase)
{
        if (*SocketTCP==INVALID_SOCKET)
        {
                *SocketTCP=accept (SocketTCPBase,NULL,NULL);
                if (*SocketTCP==INVALID_SOCKET)
                {
                        cout<<"No clients yet."<<endl;
                        return FALSE;
                }
        }
        cout<<"Client is connected!"<<endl;

	return TRUE;
}

static void ServerThread(void *arglist)
{
	sockaddr_in address={0};
	int SocketTCP=INVALID_SOCKET, SocketTCPBase=INVALID_SOCKET;
	char B[1024];

//	AllowedIPs=SlaveBufferState;	//ToDo!
	int port=SlaveBufferState;	//��⮬ ��� ࠢ�� �뭥ᥬ ᮧ����� �ࢥ� �� �।�

	if ( !CreateServer(&SocketTCPBase, &address, port) ) _endthread();
	while ( !CheckForClients (&SocketTCP, SocketTCPBase) ) Sleep (100);

	cout<<"MCU client has been connected as "<<SocketTCP<<endl;

        fd_set onesocket;
        TIMEVAL TimeOut;
        FD_ZERO(            &onesocket);
        FD_SET (SocketTCP,  &onesocket);
        TimeOut.tv_sec = 5;
        TimeOut.tv_usec = 0;
        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) || recv (SocketTCP, B, 800, 0) == SOCKET_ERROR)
        {
                cout<<"Greeting string timed out!"<<endl;
                //��� �� ��ࠢ�塞 ���� �ॡ������ ������� �। � ��� 㤮���⢮७�� �����.
        } else {
		cout<<B<<endl;
		if (!strcmp(B,"REASONANCE_DEVICE_PVHOD_3.0_CORES_1\r\n")) cout<<"Greeting string matches!"<<endl;
		else cout<<"WARNING!!! GREETING STRING MISMATCH!!!"<<endl; //��� �� ��ࠢ�塞 ���� �ॡ������ ������� �। � ��� 㤮���⢮७�� �����.

		strcpy (B, "reasonance@1REASONANCE@2");
		send (SocketTCP, B, strlen(B)+1, 0);

	        FD_ZERO(            &onesocket);
	        FD_SET (SocketTCP,  &onesocket);
	        TimeOut.tv_sec = 5;
	        TimeOut.tv_usec = 0;
	        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) || recv (SocketTCP, B, 800, 0) == SOCKET_ERROR)
	        {
	                cout<<"Auth string timed out!"<<endl;
	                //��� �� ��ࠢ�塞 ���� �ॡ������ ������� �। � ��� 㤮���⢮७�� �����.
	        } else {
			cout<<B<<endl;
		}
	}
	

	TCPSocket = SocketTCP;

	while (RunningThread>0)
	{
Sleep (100);
	}

	RunningThread = 0;
        _endthread();
}

static void WaitingThread(void *arglist)
{
	sockaddr_in address={0};
	int SocketTCP=INVALID_SOCKET;//, SocketTCPBase=INVALID_SOCKET;
//	int port=SlaveBufferIsFree;
        int S, Combo=0;
        char B[1024];

	int TrN, Ver, Len, UId, A, N;

	SocketTCP=SlaveBufferState;
	SlaveBufferState = SB_FREE;

/*	if ( !CreateServer(&SocketTCPBase, &address, port) ) _endthread();
	while ( !CheckForClients (&SocketTCP, SocketTCPBase) ) Sleep (100);
*/

cout<<"Slave TCP listening the socket "<<SocketTCP<<endl;

	while (RunningThread>0)
	{
	        if ( (S=recv(SocketTCP, B, 1024, 0 )) != SOCKET_ERROR )
        	{
			TrN = B[0]*256+B[1];
			Ver = B[2]*256+B[3];
			Len = B[4]*256+B[5];
			UId = B[6];

			if (S-6 == Len) cout<<"Data size matches the TCP recv size! :)"<<endl;
			else cout<<"Data sizes mismatch! :("<<endl;
			cout<<"Transaction N. "<<TrN<<" version "<<Ver<<" Length "<<Len<<", "<<Combo<<"th in a row"<<endl;
			cout<<"Unit ID = "<<UId<<" body: cmd = "<<(int)(B[7])<<" data =";
			for (int i=8; i<S; i++) cout<<" "<<(int)(B[i]);
			cout<<endl;

if (S-6 != Len) continue;

			switch (B[7])
			{
/*				case 1:
					A = B[8]*256+B[9];
					N = B[10]*256+B[11];
					cout<<"Reading "<<N<<" bit regs from "<<A<<endl;
					
					B[8]=(N+7)/8;
					
					Len = B[8] + 3;
					B[4]=Len/256;
					B[5]=Len%256;
					
					send (SocketTCP, B, B[8]+9, 0);
				break;*/
				case 3:
//				case 4:
					A = B[8]*256+B[9];
					N = B[10]*256+B[11];
					cout<<"Reading "<<N<<" regs from "<<A<<endl;

					if (SlaveBufferState && Combo<=5) Sleep (ENGINE_TIMEOUT);
					if (SlaveBufferState || Combo>5)	//�� ��।� ������� 5 ���뫮� ���� �ய���, �� ��������� �᢮�������� �३��.
					{
						//��ନ�㥬 ����� �訡�� 6 (busy)
						B[4] = 0;
						B[5] = 3;
						B[7] |= 0x80;
						B[8] = 6;
						send (SocketTCP, B, 9, 0);
					} else {
						SlaveBuffer[0] = A+40001;
						SlaveBuffer[1] = N;
						SlaveBufferState = SB_REQ;
					        PostMessage ((HWND)arglist, WM_NULL, 0xDEADC0DE, 0xDEADBEEF); //wake the dialog window
						while (SlaveBufferState == SB_REQ);	//������� ⠩���⮢ ��� �� �।�ᬮ�७� -- �᫨ ��� �३� �� �⢥⨫ ������⥫쭮 ��� ����⥫쭮, ᯠ��� 㦥 ������ :( � �� ᨭ�஭��� ��१���� ������ �⮬�୨��, ����� ��������� �� ��᫮���� ��ࠡ�⪠� ⠩���⮢ -- �� ⠪�� ᥡ�... ����, ��� �ᮡ�� �㦤� ���� ��⠢��� ⠪.
						if (SlaveBufferState == SB_FAIL)
						{
							SlaveBufferState = SB_FREE;
							//��ନ�㥬 ����� �訡�� 4 (failure)
							B[4] = 0;
							B[5] = 3;
							B[7] |= 0x80;
							B[8] = 4;
							send (SocketTCP, B, 9, 0);
						} else {
if (SlaveBufferState == SB_ANS) cout<<"������� ����ᨫ��!"<<endl;	//An impossible error happened.
							SlaveBufferState = SB_FREE;
							//��ନ�㥬 ����� �⢥�
					
							B[8]=N*2;
							
							Len = N*2 + 3;
							B[4]=Len/256;
							B[5]=Len%256;

							for (int i=0; i<N*2; i+=2)
							{
								B[9 +i]=((unsigned short*)(SlaveBuffer))[4+i/2] / 256;
								B[10+i]=((unsigned short*)(SlaveBuffer))[4+i/2] % 256;
							}

							send (SocketTCP, B, N*2+9, 0);
						}
					}
				break;
				case 15:
					A = B[8]*256+B[9];
					N = B[10]*256+B[11];
					cout<<"Slave received "<<N<<" bits from "<<A<<endl;
					if (B[12] != (N+7)/8) cout<<"Data size does not match the bits count!"<<endl;
if (B[12] != (N+7)/8) continue;
//					for (int i=0; i<B[12]; i++) cout<<((int)(B[13+i]))<<" ";
//					cout<<endl;

					if (SlaveBufferState) Sleep (ENGINE_TIMEOUT);
					if (!SlaveBufferState)
					{
						for (int i=0; i<B[12]; i++)
			                        	((unsigned  char*)(SlaveBuffer))[8+i]  =  B[13+i];
						SlaveBuffer[0] = A+1;
						SlaveBuffer[1] = N;
						SlaveBufferState = SB_DATA;
					        PostMessage ((HWND)arglist, WM_NULL, 0xDEADC0DE, 0xDEADBEEF); //wake the dialog window
					} else cout<<"Dropped!"<<endl;
					
					B[4]=0;
					B[5]=6;
					send (SocketTCP, B, 12, 0);
				break;
				case 16:
					A = B[8]*256+B[9];
					N = B[10]*256+B[11];
					cout<<"Slave received "<<N<<" regs from "<<A<<endl;
					if (B[12] != N*2) cout<<"Data size does not match the reg. count!"<<endl;
if (B[12] != N*2) continue;

//					for (int i=0; i<N*2; i+=2) cout<<B[13+i]*256+B[14+i]<<" ";
//					cout<<endl;

					if (SlaveBufferState) Sleep (ENGINE_TIMEOUT);
					if (!SlaveBufferState)
					{
						for (int i=0; i<N*2; i+=2)
							((unsigned short*)(SlaveBuffer))[4+i/2]  =  B[13+i]*256+B[14+i];
						SlaveBuffer[0] = A+40001;
						SlaveBuffer[1] = N;
						SlaveBufferState = SB_DATA;
					        PostMessage ((HWND)arglist, WM_NULL, 0xDEADC0DE, 0xDEADBEEF); //wake the dialog window
					} else cout<<"Dropped!"<<endl;
					
					B[4]=0;
					B[5]=6;
					send (SocketTCP, B, 12, 0);
				break;
			}
		} else if (WSAGetLastError() == WSAEWOULDBLOCK)
			Combo = 0;
		else RunningThread = -2;	//Connection has been reset?
	}

	RunningThread = 0;
        _endthread();
}

int ModbusSlaveID;
int TrN=0;

void Propagate (unsigned long *Buffer)
{
	for (int i=0; i<TotalIncoming; i++)
	{
cout<<Buffer[0]<<" : "<<Buffer[1]<<" vs "<<Incoming[i].Addr<<" : "<<Incoming[i].Size<<endl;
		if (Buffer[0]==Incoming[i].Addr && Buffer[1]==Incoming[i].Size)
		{
cout<<"Notifying plugin "<<Incoming[i].DialogId<<endl;
			(*(PluginsSet[Incoming[i].DialogId].ClientCallback))(PluginsSet[Incoming[i].DialogId].ClientInstance, WM_NULL, (int)(Buffer), Buffer[1]);
		}
	}
}

int Referendum (unsigned long *Buffer)
{
	for (int i=0; i<TotalIncoming; i++)
	{
cout<<Buffer[0]<<" : "<<Buffer[1]<<" vs "<<Incoming[i].Addr<<" : "<<Incoming[i].Size<<endl;
		if (Buffer[0]==Incoming[i].Addr && Buffer[1]==Incoming[i].Size)
		{
cout<<"Asking plugin "<<Incoming[i].DialogId<<endl;
			if (
				(*(PluginsSet[Incoming[i].DialogId].ClientCallback))(PluginsSet[Incoming[i].DialogId].ClientInstance, WM_NULL, (int)(Buffer), -((signed long int)(Buffer[1])))
			) return 1;
		}
	}

	return 0;
}

int FWProtocolWr (char* Body, int Size)
{
        fd_set onesocket;
        TIMEVAL TimeOut;
        int TCPLen;

if (TCPSocket == INVALID_SOCKET) return 0xFFFFFFFF;

        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        TimeOut.tv_sec = 0;
        TimeOut.tv_usec = 0;
        while (select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) )
        {
                cout<<"Unexpected data!!!"<<endl;
                if (recv (TCPSocket, (char*)TCPBuffer, 65520, 0) != SOCKET_ERROR)
                 cout<<"Data read"<<" "<<TCPBuffer[0]<<endl;
                MessageBeep(MB_ICONHAND);
        }

        for (int i=0; i<Size/2; i++) cout<<"Sending "<<((unsigned short*)(Body))[i]<<endl;
        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        TimeOut.tv_sec = Timeout/1000000;
        TimeOut.tv_usec = Timeout%1000000;
        if (!select (0xDEADC0DE, NULL, &onesocket, NULL, &TimeOut) ||
             send (TCPSocket,Body,Size,0) == SOCKET_ERROR)
        {
                cout<<"Timed out when writing!"<<endl;
                MessageBeep(MB_ICONHAND);
                return 0xFFFFFFFF;
        }
//        send (TCPSocket,Body,Size,0);

        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        TimeOut.tv_sec = Timeout/1000000;
        TimeOut.tv_usec = Timeout%1000000;
        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) ||
             (TCPLen = recv (TCPSocket, (char*)TCPBuffer, 65520, 0)) == SOCKET_ERROR)
        {
                cout<<"Timed out!"<<endl;
                MessageBeep(MB_ICONHAND);
                return 0xFFFFFFFF;
        }

        cout<<"Data read: "<<TCPLen<<" "<<((unsigned short*)(TCPBuffer))[0]<<endl;
	return ((unsigned short*)(TCPBuffer))[0];
}

int FWProtocolRd (unsigned short Command)
{
        fd_set onesocket;
        TIMEVAL TimeOut;
        int TCPLen;

if (TCPSocket == INVALID_SOCKET) return 0xFFFFFFFF;

        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        TimeOut.tv_sec = 0;
        TimeOut.tv_usec = 0;

		//�஢��塞, �� ��᫠�� �� ��-� ��� ����� (� ��� ࠧ -- ��⭮, ������ ���� "expected"). ���쪮 ��� �� Command, ��� ������ �� ⠪! (�� ���� �� ⠪��).
        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) ||
             (TCPLen = recv (TCPSocket, (char*)TCPBuffer, 65520, 0)) == SOCKET_ERROR)
        {
                cout<<"No data yet"<<endl;
                return 0xFFFFFFFF;
        }

/*		//������� �㭤����� ���䨪�樨 ��訢�� (����� ��᪮� ������).
        cout<<"Sending "<<Command<<endl;
        send (TCPSocket,(char*)(&Command),2,0);

        FD_ZERO(            &onesocket);
        FD_SET (TCPSocket,  &onesocket);
        TimeOut.tv_sec = 0;
        TimeOut.tv_usec = 100;
        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) ||
             (TCPLen = recv (TCPSocket, (char*)TCPBuffer, 65520, 0)) == SOCKET_ERROR)
        {
                cout<<"Timed out!"<<endl;
                MessageBeep(MB_ICONHAND);
                return 0xFFFFFFFF;
        }
*/
        cout<<"Data read: "<<TCPLen<<" "<<((unsigned short*)(TCPBuffer))[0]<<endl;
	return ((unsigned short*)(TCPBuffer))[0];

}

int WINAPI NetworkWrapper (int Function, int DialogId, void* Parameters[])
{
        unsigned char Buff[65536], *B=Buff;
        int NReg, PayloadLen, TCPLen=0, Addr, i;
        fd_set onesocket;
        TIMEVAL TimeOut;

        switch (Function)
        {
                case SEND_MESSAGE:
//                	if (TCPSocket == INVALID_SOCKET) return NULL;

          /*              printf ("Outgoing header: %4Xh, %4Xh, length %8Xh (%u)\n", ((unsigned long*)(Parameters))[0]&0xFFFF, ((unsigned long*)(Parameters))[0]>>16, ((unsigned long*)(Parameters))[1], ((unsigned long*)(Parameters))[1]);
                        if ((((unsigned long*)(Parameters))[0] & 0xFFFF0000) == MSG_DEBUG)
                        {
                                printf ("Body:\n");
                                if (((unsigned long*)(Parameters))[1]<=8) printf (" none.\n");
                                for (int i=8; i<((unsigned long*)(Parameters))[1]; i++)
                                {
                                        printf (" %2Xh", ((unsigned char*)(Parameters))[i]);
                                        if ((i%2)  == 1) printf (" ");
                                        if ((i%4)  == 3) printf (" ");
                                        if (((i-8)%16) == 15) printf ("\n");
                                }
                                printf ("\n");
                        }
*/
                        Addr = ((unsigned long*)(Parameters))[0];
                        
                        if (Addr & 0x00010000) return FWProtocolWr( ((char*)(Parameters))+4, Addr & 0xFFFF );	//ToDo: �뭥�� ModbusProtocol � �⤥���� �㭪��, ��� �������筮��.
if (TCPSocket == INVALID_SOCKET) return NULL;	//�� ��⮪��� ���-⠪� ᫨誮� ࠧ��; ����� ��⠪������ � ��饥, �� ���� � ��� -- � ��� "�뭥�� �� ᪮���".
                      
                        if (Addr>40000)
                        {
                                *B++ = TrN / 256;
                                *B++ = TrN % 256;
                                TrN++;
                                *B++ = 0;
                                *B++ = 0;
                                NReg = ((unsigned long*)(Parameters))[1];
                                TCPLen = NReg*2 + 13;
                                PayloadLen = TCPLen - 6;
                                *B++ = PayloadLen/256;
                                *B++ = PayloadLen%256;
                                *B++ = ModbusSlaveID;
                                *B++ = 16;
                                *B++ = (Addr-40001)/256;
                                *B++ = (Addr-40001)%256;
                                *B++ = NReg/256;
                                *B++ = NReg%256;
                                *B++ = NReg*2;
                                for (i=0; i<NReg; i++)
                                {
                                        *B++ = ((unsigned short*)(Parameters))[4+i] / 256;
                                        *B++ = ((unsigned short*)(Parameters))[4+i] % 256;
                                }
                        } else {
                                *B++ = TrN / 256;
                                *B++ = TrN % 256;
                                TrN++;
                                *B++ = 0;
                                *B++ = 0;
                                NReg = ((unsigned long*)(Parameters))[1];
                                TCPLen = (NReg+7)/8 + 13;
                                PayloadLen = TCPLen - 6;
                                *B++ = PayloadLen/256;
                                *B++ = PayloadLen%256;
                                *B++ = ModbusSlaveID;
                                *B++ = 15;
                                *B++ = (Addr- 1)/256;
                                *B++ = (Addr- 1)%256;
                                *B++ = NReg/256;
                                *B++ = NReg%256;
                                *B++ = (NReg+7)/8;
                                for (i=0; i<(NReg+7)/8; i++)
                                  *B++ = ((unsigned char*)(Parameters))[8+i];

			}
			
                        FD_ZERO(            &onesocket);
                        FD_SET (TCPSocket,  &onesocket);
                        TimeOut.tv_sec = 0;
                        TimeOut.tv_usec = 0;
                        while (select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) )
                        {
                                cout<<"Unexpected data!!!"<<endl;
                                if (recv (TCPSocket, (char*)TCPBuffer, 65520, 0) != SOCKET_ERROR)
                                 cout<<"Data read"<<" "<<TCPBuffer[0]<<" "<<TCPBuffer[1]<<endl;
                                MessageBeep(MB_ICONHAND);
                        }

                        cout<<"Sending "<<TrN-1<<endl;
                        send (TCPSocket,(char*)Buff,TCPLen,0);

                        FD_ZERO(            &onesocket);
                        FD_SET (TCPSocket,  &onesocket);
                        TimeOut.tv_sec = 1;
                        TimeOut.tv_usec = 500000;
                        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) ||
                             (TCPLen = recv (TCPSocket, (char*)Buff, 65520, 0)) == SOCKET_ERROR)
                        {
                                cout<<"Timed out!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }

                        cout<<"Data read: "<<Buff[0]*256+Buff[1]<<endl;

for (int Bt=0; Bt<TCPLen; Bt++) printf ("%Xh ",Buff[Bt]);
cout<<endl;
                        //Converting Buff (Raw Modbus answer) to the proper data structure:

                        if (Buff[0]*256+Buff[1] != TrN-1)
                        {
                                cout<<"Wrong Transaction ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[2]*256+Buff[3] != 0)
                        {
                                cout<<"Wrong Modbus ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[4]*256+Buff[5] != TCPLen-6)
                        {
                                cout<<"Wrong payload size!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[6] != ModbusSlaveID)
                        {
                                cout<<"Wrong Slave ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[7] != 16 && Addr>40000)
                        {
                                cout<<"Answer "<<((int)(Buff[7]))<<" instead of 16!"<<endl;
                                MessageBeep(MB_ICONHAND);
                            //ToDo: process custom error code and partial compatibility HERE!
                                return 0;
                        }
                        if (Buff[7] != 15 && Addr<10001)
                        {
                                cout<<"Answer "<<((int)(Buff[7]))<<" instead of 15!"<<endl;
                                MessageBeep(MB_ICONHAND);
                            //ToDo: process custom error code and partial compatibility HERE!
                                return 0;
                        }
                        if (Buff[8]*256+Buff[9] != Addr - 40001 && Addr>40000)
                        {
                                cout<<"Wrong first register!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[8]*256+Buff[9] != Addr - 1  && Addr<10001)
                        {
                                cout<<"Wrong first bit register!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[10]*256+Buff[11] != NReg)
                        {
                                cout<<"Wrong register count!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }

                        TCPBuffer[0] = Addr;
                        TCPBuffer[1] = NReg;    //May be smaller if Custom Error Codes supported.
                return (int)TCPBuffer;

                case REGISTER_INCOMING:
                        cout<<"Reg "<<((unsigned long*)(Parameters))[0]<<" size "<<((unsigned long*)(Parameters))[1]<<endl;
			Incoming[TotalIncoming].DialogId = DialogId;
			Incoming[TotalIncoming].Addr = ((unsigned long*)(Parameters))[0];
			Incoming[TotalIncoming].Size = ((unsigned long*)(Parameters))[1];
			TotalIncoming++;
//cout<<TCPBuffer[0]<<" : "<<TCPBuffer[1]<<" vs "<<Incoming[0].Addr<<" : "<<Incoming[0].Size<<endl;
                return (int)TCPBuffer;

                case SEND_REQUEST:
                	if (TCPSocket == INVALID_SOCKET) return NULL;

                        Addr = ((unsigned long*)(Parameters))[0];

                        if (Addr & 0x00010000) return FWProtocolRd(Addr & 0xFFFF);	//ToDo: �뭥�� ModbusProtocol � �⤥���� �㭪��, ��� �������筮��.

//                        if (Addr>30000)
                        {
                                *B++ = TrN / 256;
                                *B++ = TrN % 256;
                                TrN++;
                                *B++ = 0;
                                *B++ = 0;
                                NReg = ((unsigned long*)(Parameters))[1];
                                TCPLen = 12;
                                PayloadLen = 6;
                                *B++ = PayloadLen/256;
                                *B++ = PayloadLen%256;
                                *B++ = ModbusSlaveID;
	                        if (Addr>40000)
	                        {
	                                *B++ = 3;
	                                *B++ = (Addr-40001)/256;
        	                        *B++ = (Addr-40001)%256;
                                } else if (Addr > 30000) {
	                                *B++ = 4;
                                	*B++ = (Addr-30001)/256;
                                	*B++ = (Addr-30001)%256;
                                } else {
                                	*B++ = 1;
                                	*B++ = (Addr-1)/256;
                                	*B++ = (Addr-1)%256;
                                }
                                *B++ = NReg/256;
                                *B++ = NReg%256;
                        }                       

                        FD_ZERO(            &onesocket);
                        FD_SET (TCPSocket,  &onesocket);
                        TimeOut.tv_sec = 0;
                        TimeOut.tv_usec = 0;
                        while (select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) )
                        {
                                cout<<"Unexpected data!!!"<<endl;
                                if (recv (TCPSocket, (char*)TCPBuffer, 65520, 0) != SOCKET_ERROR)
                                 cout<<"Data read"<<" "<<TCPBuffer[0]<<" "<<TCPBuffer[1]<<endl;
                                MessageBeep(MB_ICONHAND);
                        }

                        cout<<"Sending Req "<<TrN-1<<endl;
                        send (TCPSocket,(char*)Buff,TCPLen,0);

                        FD_ZERO(            &onesocket);
                        FD_SET (TCPSocket,  &onesocket);
                        TimeOut.tv_sec = 1;
                        TimeOut.tv_usec = 500000;
                        if (!select (0xDEADC0DE, &onesocket, NULL, NULL, &TimeOut) ||
                             (TCPLen = recv (TCPSocket, (char*)Buff, 65520, 0)) == SOCKET_ERROR)
                        {
                                cout<<"Timed out!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }

                        cout<<"Data read: "<<Buff[0]*256+Buff[1]<<endl;

                        //Converting Buff (Raw Modbus answer) to the proper data structure:
for (int Bt=0; Bt<TCPLen; Bt++) printf ("%Xh ",Buff[Bt]);
cout<<endl;

                        if (Buff[0]*256+Buff[1] != TrN-1)
                        {
                                cout<<"Wrong Transaction ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[2]*256+Buff[3] != 0)
                        {
                                cout<<"Wrong Modbus ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[4]*256+Buff[5] != TCPLen-6)
                        {
                                cout<<"Wrong payload size!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[6] != ModbusSlaveID)
                        {
                                cout<<"Wrong Slave ID!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[7] != 3 && Addr > 40000)
                        {
                                cout<<"Answer "<<((int)(Buff[7]))<<" instead of 3!"<<endl;
                                MessageBeep(MB_ICONHAND);
                            //ToDo: process custom error code and partial compatibility HERE!
                                return 0;
                        }
                        if (Buff[7] != 4 && Addr < 40001 && Addr > 30000)
                        {
                                cout<<"Answer "<<((int)(Buff[7]))<<" instead of 4!"<<endl;
                                MessageBeep(MB_ICONHAND);
                            //ToDo: process custom error code and partial compatibility HERE!
                                return 0;
                        }
                        if (Buff[7] != 1 && Addr < 10001)
                        {
                                cout<<"Answer "<<((int)(Buff[7]))<<" instead of 1!"<<endl;
                                MessageBeep(MB_ICONHAND);
                            //ToDo: process custom error code and partial compatibility HERE!
                                return 0;
                        }
                        if (Buff[8] != NReg*2 && Addr > 30000)
                        {
                                cout<<"Wrong array size!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (Buff[8] != (NReg+7)/8 && Addr < 10001)
                        {
                                cout<<"Wrong bit-field size!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        if (TCPLen-9 !=  NReg*2    && Addr > 30000
                         || TCPLen-9 != (NReg+7)/8 && Addr < 10001)
                        {
                                cout<<"Array size doesn't match total size!"<<endl;
                                MessageBeep(MB_ICONHAND);
                                return 0;
                        }
                        B = Buff+9;
                        if (Addr > 30000) for (i=0; i<NReg; i++)
                        {
                                ((unsigned short*)(TCPBuffer))[4+i]  = *B++;
                                ((unsigned short*)(TCPBuffer))[4+i] *= 256;
                                ((unsigned short*)(TCPBuffer))[4+i] += *B++;
                        } else for (i=0; i<(NReg+7)/8; i++)
                        	((unsigned  char*)(TCPBuffer))[8+i]  = *B++;

                        TCPBuffer[0] = Addr;
                        TCPBuffer[1] = NReg;    //May be smaller if Custom Error Codes supported.

	        	Propagate(TCPBuffer);

                return (int)TCPBuffer;
        }
        
        return 0;       
}

int ConnectToServer(HWND hWnd, int Ctrl, BOOL Quiet)
{
//                unsigned long i=TRUE;
                char Temp[16];
                
		int TCPSocket=INVALID_SOCKET;

    //            TCPSocket=socket (AF_INET, SOCK_STREAM,0);
      //          cout<<TCPSocket<<endl;
        //        ioctlsocket (UDPSocket, FIONBIO, &i);   //����������騩
          //      PC2Board.sin_family = Board2PC.sin_family = AF_INET;
            //    PC2Board.sin_port = htons(GetDlgItemInt(hWnd, MCU_PORT, (int*)&i, FALSE));
int i;
        ModbusSlaveID = GetDlgItemInt(hWnd, MCU_SID, (int*)&i, FALSE);
  //              GetDlgItemText (hWnd, MCU_IP, Temp, 15);
    //            PC2Board.sin_addr.s_addr = inet_addr(Temp);     //���砫� ����⠥� �� ��������
      //          Board2PC.sin_addr.s_addr = htonl(INADDR_ANY);
        //        cout<<"Bind: "<<bind(UDPSocket,(sockaddr*)&Board2PC,sizeof(sockaddr_in))<<endl;

//        if (TCPSocket!=INVALID_SOCKET) return 0;    //already created

        if ((TCPSocket = socket (AF_INET, SOCK_STREAM,0 )) < 0)
        {
                cout<<"Error creating: "<<GetLastError()<<endl;
                return INVALID_SOCKET;
        }
        memset(&address,0,sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(GetDlgItemInt(hWnd, Ctrl, (int*)&i, FALSE)); 

        GetDlgItemText (hWnd, MCU_IP, Temp, 15);
        i=inet_addr(Temp);
        if (i==INADDR_NONE)
        {
                cout<<"Invalid IP address!"<<endl;
                return INVALID_SOCKET;
        }

        address.sin_addr.s_addr = i;
        if(connect(TCPSocket,(struct sockaddr *)&address, sizeof(struct sockaddr_in))      )
        {
                cout<<"Error connecting: "<<GetLastError()<<endl;
                closesocket(TCPSocket);
                TCPSocket=SOCKET_ERROR;
                if (!Quiet) MessageBox( NULL,"TCP server not found!","Can't connect to MCU!",MB_OK | MB_TASKMODAL );
                return INVALID_SOCKET;
        }
        else cout<<"Connected to server"<<endl;
        i=TRUE;
        if (ioctlsocket (TCPSocket,FIONBIO,(unsigned long*)&i)==SOCKET_ERROR)
        {
                cout<<"Error setting nonblock mode: "<<GetLastError()<<endl;
        }
//      i=1;     //any non-zero; <-- AFAIK ioctlsocket is non-destructive
        if ( setsockopt (TCPSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&i, 4)==SOCKET_ERROR)
        {
                cout<<"Error setting nobuffer mode: "<<GetLastError()<<endl;
        }

        cout<<"Configured for Modbus."<<endl;
        return TCPSocket;
}

double TD;
int WINAPI WinAPIWrapper (int Function, int DialogId, void* Parameters[])
//int __syscall WinAPIWrapper (int Function, void* Dialog, void* Parameters[])
{
        char Temp[256],*T;
//	WCB *Client;
//	int CliInst;

        switch (Function)
        {

		//���뢠誨 ��� �������� �ॡ��� ��।�� ����� ����譨�� -- 㪠��⥫� �� ⥬�����.

                case OPENDIALOG:
			PluginsSet[TotalPlugins].ClientCallback = ((WCB**)(Parameters))[0];
			PluginsSet[TotalPlugins].ClientInstance = ((int* )(Parameters))[1];
			PluginsSet[TotalPlugins].ClientDiaIsModal = 0;
			//PluginsSet[TotalPlugins].ClientDiaHdl =
			if (!
				CreateDialogIndirectParam (GetModuleHandle(NULL), (LPCDLGTEMPLATE) DialogId, Parent, WinFuncWrapper, (LPARAM) TotalPlugins)
			) return 0;
//			if (!PluginsSet[TotalPlugins].ClientDiaHdl) return 0;
//			TotalPlugins++;
			
			cout<<TotalPlugins<<" dialogs/plugins so far"<<endl;
			
                return TotalPlugins;

/*                case MODALDIALOG:
//                        Client = (WCB*)(Parameters[0]);
			Client = ((WCB*)(Parameters))[0];
			CliInst= ((int*)(Parameters))[1];
			if (!DialogBoxIndirect(GetModuleHandle(NULL), (LPCDLGTEMPLATE) DialogId, Parent, WinFuncWrapper)) return 0;
                return 1; //same
*/
		//����� ��� �⠭���⭮ -- �����筮 ��᫠�� ����譨�, � �� ���� �����⥭ ��� � ����譨�64 ������� (�㭪�� � ���⠭��).

                case CLOSEDIALOG:
                        if (PluginsSet[DialogId].ClientDiaIsModal) return EndDialog (PluginsSet[DialogId].ClientDiaHdl,NULL);
                return DestroyWindow (PluginsSet[DialogId].ClientDiaHdl);

                case SENDMDIALOG:
                return DialogAPIWrapper (PluginsSet[DialogId].ClientDiaHdl, (unsigned long*)Parameters);

                case OPENFILEROLLER:
//                      if ( ((int*)(Parameters))[0] == API_OPENDIR) return DirRoller ( (char*)(Parameters+1) );
                return FileRoller ( PluginsSet[DialogId].ClientDiaHdl, (char* *)(Parameters+1), ((int*)(Parameters))[0] );

                case CREATECANVAS:     //ࠡ��� � ���⨭���� � ��������� ����稢�� � ����ࠪ�� �� ��஭� �३���ઠ, ��⮬� �� �� ���� ���⤠���.
                        Graphs[DialogId].WinBitmap=NULL; //Sets up on Resize;
                        Graphs[DialogId].hDC=GetDC(PluginsSet[DialogId].ClientDiaHdl);
                        Graphs[DialogId].Comp_hDC=CreateCompatibleDC(Graphs[DialogId].hDC);
                        memset (&(Graphs[DialogId].DIBFormat),0,sizeof(BITMAPINFO));
                        Graphs[DialogId].DIBFormat.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
                        Graphs[DialogId].DIBFormat.bmiHeader.biPlanes=1;
                        Graphs[DialogId].DIBFormat.bmiHeader.biBitCount=24;
                        Graphs[DialogId].DIBFormat.bmiHeader.biCompression=BI_RGB;
                        Graphs[DialogId].DIBFormat.bmiHeader.biSizeImage=0;
                        Graphs[DialogId].DIBFormat.bmiHeader.biXPelsPerMeter=1000;
                        Graphs[DialogId].DIBFormat.bmiHeader.biYPelsPerMeter=1000;
                        Graphs[DialogId].DIBFormat.bmiHeader.biClrUsed=0;
                        Graphs[DialogId].DIBFormat.bmiHeader.biClrImportant=0;
cout<<"DevCont: "<<DialogId<<" "<<Graphs[DialogId].hDC<<" "<<Graphs[DialogId].Comp_hDC<<endl;
                        Graphs[DialogId].DIBFormat.bmiHeader.biWidth=768;
                        Graphs[DialogId].DIBFormat.bmiHeader.biHeight=256;
                        Graphs[DialogId].WinBitmap=CreateCompatibleBitmap(Graphs[DialogId].hDC,768,256);  //���� �����, ࠧ��� 䨪�஢��. � 祫���� ������, ���� �ᨫ�� �� ����� ��ࠦ������ ��᫠, ��� ���� ॠ������� � �� ���窨 ���� ᤥ���� :)
cout<<"DefBitmap: "<<Graphs[DialogId].WinBitmap<<endl;
			Graphs[DialogId].DialogWnd=PluginsSet[DialogId].ClientDiaHdl;
                return DialogId;	//��⮬ ᤥ���� �����饭�� ����譨�� ���, � ���� �� ᮢ������ � ��������.
                
                case UPDATECANVAS:
//cout<<"Drawing "<<
                        SetDIBits(Graphs[DialogId].Comp_hDC,Graphs[DialogId].WinBitmap,0,256,(char*)Parameters,&(Graphs[DialogId].DIBFormat),DIB_RGB_COLORS);
//cout<<"Drawing "<<
                        SelectObject(Graphs[DialogId].Comp_hDC,Graphs[DialogId].WinBitmap);
                return RedrawWindow(PluginsSet[DialogId].ClientDiaHdl,NULL,NULL,RDW_INTERNALPAINT);

                case REMOVECANVAS:
                        ReleaseDC(PluginsSet[DialogId].ClientDiaHdl,Graphs[DialogId].hDC);
                        ReleaseDC(PluginsSet[DialogId].ClientDiaHdl,Graphs[DialogId].Comp_hDC);
                        DeleteObject(Graphs[DialogId].WinBitmap);
                        Graphs[DialogId].DialogWnd=NULL;	//� �� ����� ���� ��⮬ �㤥� ᮧ������ ���� ����.
                break;

                case MESSAGEBOX: return MessageBox (PluginsSet[DialogId].ClientDiaHdl, ((char**)(Parameters))[2], ((char**)(Parameters))[1], ((unsigned long*)(Parameters))[0]);
                case TEXTTODOUBLE:
                        GetDlgItemText (PluginsSet[DialogId].ClientDiaHdl, (unsigned long)Parameters, Temp, 256);
                        if (Temp[0]=='#')       //Color Hexadecimal
                        {
                                memmove (Temp+1, Temp, 255);
                                Temp[0]='0';
                                Temp[1]='x';
                        }
                        TD=strtod (Temp, &T);   //Including NaN and +-INF
                        if (T==Temp) *((__int64*)(&TD))=0x7FF8000000000000;     //NaN (value not recognised)
                return (int)(&TD);
                case DOUBLETOTEXT:
//cout<<DialogId<<" sets control "<<*((unsigned long*)(Parameters))<<endl;
                	if ( isnan(*((double*)(Parameters+1)) )) return SetDlgItemText (PluginsSet[DialogId].ClientDiaHdl, *((unsigned long*)(Parameters)), "N/A");
                	if (!fmod( *((double*)(Parameters+1)), 1.0 ))
                	{
	                        GetDlgItemText (PluginsSet[DialogId].ClientDiaHdl, *((unsigned long*)(Parameters)), Temp, 256);
        	                if (Temp[0]=='#')       //Color Hexadecimal
                			sprintf (Temp, "#%X", (int)(*((double*)(Parameters+1))) );
        	                else	sprintf (Temp,  "%i", (int)(*((double*)(Parameters+1))) );
                	} else  sprintf (Temp, "%f", *((double*)(Parameters+1)) );      //��, � ����, �� � ���� ��ᥬ���� ��������� �����, �� � ����⢨⥫쭮 ���� � ���� �ਢ������ 㪠��⥫� �� ���ᨢ 㪠��⥫�� �� ���� � 㪠��⥫� �� 㯠�������� �������� "᭠砫� U32, ��⮬ 64-��⭮� ������饥". ���, �� ������. ���, �஡��� � �⥭��� �⮣� �१ N ��� ���� �� �뫮. ��, ��ᥬ���� ��������� �����.
                return  SetDlgItemText (PluginsSet[DialogId].ClientDiaHdl, *((unsigned long*)(Parameters)), Temp);

                case ATTACHTOOLTIP:     //���⠩ �᤭, ���� �� �� ���-� 㤠���� ��। �����⨥� ᠬ��� ����.
                        TOOLINFO toolInfo = { 0 };
                        toolInfo.cbSize = sizeof(toolInfo);
                        toolInfo.hwnd = PluginsSet[DialogId].ClientDiaHdl;
                        toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                        toolInfo.uId = (UINT_PTR)GetDlgItem(PluginsSet[DialogId].ClientDiaHdl, *((unsigned long*)(Parameters)) );
                        toolInfo.lpszText = (char*)(Parameters[1]);
                        SendMessage(
                                CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP |TTS_ALWAYSTIP | /*TTS_BALLOON*/0x40, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, PluginsSet[DialogId].ClientDiaHdl, NULL, g_hInst, NULL),
                                        TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
                return NULL;

                case TIMER:
                        if (! ((unsigned long*)(Parameters))[1] ) return KillTimer (PluginsSet[DialogId].ClientDiaHdl, ((unsigned long*)(Parameters))[0]);
                return SetTimer (PluginsSet[DialogId].ClientDiaHdl, ((unsigned long*)(Parameters))[0], ((unsigned long*)(Parameters))[1], NULL);

		case GETDIALOGDATA_HEADER:
//			cout<<((int)(DialogId))<<" requests "<<((char*)(Parameters))<<endl;

			if (((int)(DialogId)) < 0 && INIAlreadyOpen)	//All future dialogs of the currently loading plug-in
				return ProcessINIHeader ((char*)(Parameters));
		return 0;

		case GETDIALOGDATA_LINE:
//			cout<<((int)(DialogId))<<" requests parameter "<<((char*)(Parameters))<<endl;

			if (((int)(DialogId)) < 0 && INIAlreadyOpen)	//All future dialogs of the currently loading plug-in
				return (   (int)(ProcessINILine ((char*)(Parameters)) )   );
		return 0;
		
		case SETFOCUS: return SetFocus (GetDlgItem (PluginsSet[DialogId].ClientDiaHdl, (unsigned long)Parameters) ) != NULL;

		case GETSYSTEMSTATE:
			if (RunningThread>0) return API_CONNECTED;
		return API_ALIVE; //���������� � ��直� ����� � �����, ��� ⮫쪮 ���⫨��� ��-� �� �ਯ����.

		case SETSYSTEMSTATE:
cout<<(int)(Parameters)<<endl;
			if ( ((int)(Parameters))==API_DEAD )
			{
				PostMessage (Parent, WM_CLOSE, 0, 0);
				return 1;
			}
			if ( ((int)(Parameters))==API_CONNECTED )
			{
				if (RunningThread) return MessageBeep(MB_ICONHAND) | 1;

				SlaveBufferState = SlaveSocket = ConnectToServer(Parent, MCU_MST_PORT, TRUE);	//�� �� ⮫쪮 �� �������, �⮡� 㬥����� ������⢮ ����⨫�� ����� �।���...
		                if (SlaveSocket == INVALID_SOCKET) return 0;

				RunningThread=1;
		                _beginthread(WaitingThread, 256, (void*)Parent);

				return 1;
			}
			if ( ((int)(Parameters))==API_ALIVE )
			{
				if (RunningThread<=0) return 0;	//Nothing to disconnect!
				RunningThread = -1;	//���, � ����� �஢�મ� �। �ᯥ�� ᠬ���������� ��-�� �訡�� :( ��᫥ ⮣�, ��� � �।� ������ �ࠢ� �� ᠬ������⨥, ��� ᨭ��� ���� ��९��뢠�� -- ���⮩ ������� �⮬�୨� �� ᯠᠥ�.
				while (RunningThread);	//�� ᠬ�� ���� ����� ᤥ���� ⠪, �⮡� �। �� ᠬ�����뢠���, � �ॡ���� � ���� ��� �������. ����� 墠�� ������ �⮬�୮��.
				closesocket(SlaveSocket);
			}
		return 0;

		case PROGRESSBAR:
			PBarValue = (int)(Parameters);
			if (!PBarValue)
			{
				if (!PBarState) _beginthread(ProgressBarThread, 256, NULL);
				while (!PBarState);
			}
			else if (PBarState >= 100)
			{
				while (PBarState);
return 1;
			}
		return PBarState;
        }

        return 0;
}

BOOL __export PASCAL FwWinFuncBeta (HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam)
{
        if (uMsg == WM_INITDIALOG)
        {
WSADATA TmpDta;
WSAStartup (0x0101,&TmpDta);

//SlaveBufferIsFree = 502; _beginthread(WaitingThread, 256, (void*)hWnd);

		INIFile.open ("DefaultCFG.CSV", ios::in|ios::binary);
		INIAlreadyOpen=1;
		if (INIFile.fail()) INIAlreadyOpen=0;	//�� ��� ���-� ���� ������ ��뢠�� ��� ������ ��㣮�� 䠩��

                Parent=hWnd;

		if (ProcessINIHeader ("MixedPlugin	Minimum	Current Value	Maximum	#Graph. Color"))
		{
			double* Param;
			char Temp[128];
			if (Param = ProcessINILine ("MCU_IP"))
			{
				sprintf (Temp, "%u.%u.%u.%u", ((int)(Param[0])), ((int)(Param[1])), ((int)(Param[2])), ((int)(Param[3])));
				SetDlgItemText (hWnd, MCU_IP, Temp);
			}
			if (Param = ProcessINILine ("MCU_SLAVE_PORT"))
			{
				SetDlgItemInt (hWnd, MCU_PORT, Param[1], FALSE);
			}
			if (Param = ProcessINILine ("MCU_MASTER_PORT"))
			{
				SetDlgItemInt (hWnd, MCU_MST_PORT, Param[1], FALSE);
			}
INIFile.seekg(ColCount=LineCount=0);
		}

		WCB* ClientCallback;
		int  ClientInstance;
#define DebCnt 1
//char Debug[DebCnt][256]={"Base_Set.dll","Log_Man.dll","Log_Gra.dll","ContrlPn.dll","FWUpdate.dll"};
char Debug[DebCnt][256]={"FWUpdate.dll"};
for (int DebN=0; DebN<DebCnt; DebN++, INIFile.seekg(ColCount=LineCount=0))
{
//                cout<<"FW Init "<<LoadLibrary("Plugin.dll")<<" "<<GetProcAddress(LoadLibrary("Plugin.dll"), "PluginEngine")<<endl;
                ClientCallback = (WCB*) (GetProcAddress(LoadLibrary(Debug[DebN]), "PluginEngine"));    //������ ��ࠡ��稪. � ���� ����� ���� ��� ��ࠡ��稪� � ��� �������...
//                PluginsSet[TotalPlugins].ClientDiaHdl = NULL;	//���� �� ����� �� ���� ����, �. �. ������ ⮫쪮-⮫쪮 ����㧨��� � ��� ᢮� ��ࠡ��稪.
                ClientInstance = (*ClientCallback) (-1, WM_NULL, (int)(WinAPIWrapper), (int)(NetworkWrapper));

                if (ClientInstance < 0)
                	MessageBox (hWnd, "This plugin can't be added twice!", "Warning", MB_OK);
	                else
	                {
//� 99% ��砥� ᠬ 䠪� �맮�� (*Client) (-1...) �� ⮫쪮 ᮧ���� ���� ���⠭�, �� � �-��ࠡ��쭮 �室�� � ᮧ����� ������� � ��᫥ 
//�⮣� ���� ⮫쪮 �஢����, �����猪 ��� � ⠪�� uID64 㦥 ����� � ���� ����� ���� (� ��� �� ��� ��� ��� ⠪ ᪠���� ��㤭�, � ����-� �筮 ���� ���� ��� NULL).
//���쪮 ᬮ��, ��� �� �-��ࠡ���� �맮�� �� TotalPlugins++ �� ��।ࠫ��� :)
				0;	//��������� ������� -- ��⮬.
				cout<<"Inst "<<ClientInstance<<endl;
	                }	//� �ࠢ��쭮 �� �⠫ ������ -- ��� �� ������� ��� ����譨� ��।���� ���? ��� ����� ᮧ����� ������� �� �㫥���� 㪠��⥫� � �㤥� ᮧ������ �������������� �������, � ������ �� ���� �� ���.
}

		INIAlreadyOpen=0;
		INIFile.close();
        }

        if (uMsg==WM_COMMAND && lParam && wParam==(BN_CLICKED<<16)+SET_TO) {int i; Timeout = GetDlgItemInt(hWnd, TIMEOUT, &i, FALSE);}

        if (uMsg==WM_COMMAND && lParam && wParam==(BN_CLICKED<<16)+IDOK)        //ToDo: ᠬ �३���� ���� �������� � ��� �� ���.
        {
        	if (IsDlgButtonChecked (hWnd,UPSERVER))
        	{
        		int i;
			if (RunningThread) return MessageBeep(MB_ICONHAND) | 1;
//			SlaveBufferState = (allowed IP)	//ToDo!
			SlaveBufferState = GetDlgItemInt(hWnd, MCU_PORT, &i, FALSE);
			RunningThread=1;
        		_beginthread(ServerThread, 256, (void*)hWnd);	//ToDo: �뭥�� ᮧ����� �ࢥ� �� �।�, ᤥ���� �����ਭ� ������-������� �� �����筮� �⮬�થ.
        		return TRUE;
        	}
        	
        	
//if (Opened) return MessageBeep(MB_ICONHAND) | 1;// else Opened=1; //��⮬ ��-��᪨ ᤥ���
//if (!ConnectToServer(hWnd)) return MessageBeep(MB_ICONHAND) | 1;
TCPSocket = ConnectToServer(hWnd, MCU_PORT, FALSE);

//                ((unsigned long*)(Temp))[0] = CAT_SECU | MSG_DEBUG;
  //              ((unsigned long*)(Temp))[1] = 8;        //���� ������� �������, ᮫��� ��஫��, ��祣�, ���� ���⮩ 奤�� �ਢ���⢨�.
    //            sendto (UDPSocket, Temp, 8, 0, (sockaddr*)&PC2Board,sizeof (sockaddr_in));

//		int i;
//		SlaveBufferIsFree = GetDlgItemInt(hWnd, MCU_MST_PORT, &i, FALSE);	//�� �� ⮫쪮 �� �������, �⮡� 㬥����� ������⢮ ����⨫�� ����� �।���...
		SlaveBufferState = SlaveSocket = ConnectToServer(hWnd, MCU_MST_PORT, FALSE);
                if (SlaveSocket != INVALID_SOCKET)
                {
                	RunningThread = 1;
                	_beginthread(WaitingThread, 256, (void*)hWnd);
                }

//                Opened=1;
        }
        if (uMsg == WM_NULL && SlaveBufferState == SB_DATA)
        {
        	memcpy (TCPBuffer, (unsigned long*)SlaveBuffer, sizeof (TCPBuffer));
        	Propagate(TCPBuffer);
//        	Propagate((unsigned long*)SlaveBuffer);
        	SlaveBufferState = SB_FREE;
        }
        if (uMsg == WM_NULL && SlaveBufferState == SB_REQ)
        {
        	memcpy (TCPBuffer, (unsigned long*)SlaveBuffer, sizeof (TCPBuffer));
        	if (Referendum(TCPBuffer))
        	{
	        	memcpy ((unsigned long*)SlaveBuffer, TCPBuffer, sizeof (TCPBuffer));
	        	SlaveBufferState = SB_ANS;
        	}
		else SlaveBufferState = SB_FAIL;
        }
/*        if (uMsg == WM_NULL && IsData)
        {
                IsData=0;
                cout<<"Attempting to read..."<<endl;
                if (recv (TCPSocket, (char*)TCPBuffer, 65520, 0) != SOCKET_ERROR)       //��� �� �� ��直� ��砩 㡥������, �� ᮪�� ����������騩 (�� IsData �⠢�� �� ᠬ�, ��砩�� �� �ਫ���)
                {       //������ 奤�� � ��᫠�� �� �ਭ��������� �ᥬ ॣ���
                        cout<<"Data read"<<" "<<TCPBuffer[0]<<" "<<TCPBuffer[1]<<endl;

                        //�᫨ ���� ����� � MSG_DEBUG (�� ��⥣�ਨ), � �㯨� ��� 奪ᮬ � ���誮. ���� -- ⮫쪮 奤��.
/*                        printf ("Incoming header: %4Xh, %4Xh, length %8Xh (%u)\n", UDPBuffer[0]&0xFFFF, UDPBuffer[0]>>16, UDPBuffer[1], UDPBuffer[1]);
                        if ((UDPBuffer[0] & 0xFFFF0000) == MSG_DEBUG)
                        {
                                printf ("Body:\n");
                                if (UDPBuffer[1]<=8) printf (" none.\n");
                                for (int i=8; i<UDPBuffer[1]; i++)
                                {
                                        printf (" %2Xh", ((unsigned char*)(UDPBuffer))[i]);
                                        if ((i%2)  == 1) printf (" ");
                                        if ((i%4)  == 3) printf (" ");
                                        if (((i-8)%16) == 15) printf ("\n");
                                }
                                printf ("\n");
                        }

                        if (UDPBuffer[0] == (CAT_SECU | MSG_DEBUG))
                        {
                                MessageBox (hWnd, "MCU accepted the login!", "OK", MB_OK);
                        } else {        //���� ������ ����, �� ᬮ�ਬ, �� �� ॣ��
                                (*Client) (WM_NULL, (int)(UDPBuffer), UDPBuffer[1]);    //��ண��, ��אַ� �맮�, ��� ������� ᮮ�饭��. ������ ��� ⮫쪮 ��᫥ ��ࠡ�⪨.
                                UDPBuffer[0]=UDPBuffer[1]=0;    //���� ����� � ᯥ�䨪�樨 ���� ��� 0x00000000 ���������� ��ன
                        }*/
            //    }
           //     _beginthread(WaitingThread, 256, (void*)hWnd);
       // }
        if (uMsg == WM_CLOSE || uMsg==WM_COMMAND && lParam && wParam==(BN_CLICKED<<16)+IDCANCEL) EndDialog (hWnd,NULL);
        return FALSE;
}

void main (int argc, char* argv[])
{
        DialogBoxParam(g_hInst=GetModuleHandle(NULL),"FWORK",NULL,FwWinFuncBeta,NULL);



//      cout<<((char*)(i))<<endl<<"Exitting..."<<endl;
        WSACleanup();

GetOpenFileName(NULL);  //���襡��� �㭪��, ��� �맮�� ���ன (��� �� �������) ����������஫� �� ��������� � ������ ������ � �ࠪ��ࠬ� �_�
                        //� ࠧ������ � ��������஬/�����஬ � ��⥫쭮 ������ ��� �� �, �� ���⠢��� ���� �ਡ����� � �����!
}
