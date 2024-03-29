#define OPEN_WATCOM_DIRTY_HACK
#include "FWUpdate.rc"
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
HGLOBAL DiaTemplate;//, BarDiaTemplate;
int* IncomingBuffer=NULL;

fstream Log;
int DoLog=0, DoWarning=0;

//int PrBarDiaId, BarState=0;
/*extern "C" int __export WINAPI PrBarDia (long DiaId, unsigned long uMsg, unsigned long wParam, unsigned long lParam)
{
//        char Text[256];
  //      unsigned long Msg[4]={HEX_BODY, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
//         	Msg[0]=wParam&0xFFFF;
  //       	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);

if (!uMsg) cout<<"Unexpected WM_NULL "<<wParam<<" "<<lParam<<endl;

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED)
        	BarState=2;	//abort

        if (uMsg==WM_INITDIALOG)
        {
		PrBarDiaId = lParam;
		BarState=1;
cout<<"Newly created Dialog ID = "<<PrBarDiaId<<endl;

        }
        
        return 0;
}

*/

unsigned long Hex (char* Text, int Count)
{
	unsigned long Val=0;
	for (int i=0; i<Count; i++)
	{
		Val*=16;
		if (Text[i]>='A'&&Text[i]<='F')
		{
			Val+=Text[i]-'A'+10;
		} else if (Text[i]>='a'&&Text[i]<='f')
		{
			Val+=Text[i]-'a'+10;
		} else  Val+=Text[i]-'0';
	}

	return Val;
}

int IsCPU2=0;

const char* GetIntelHex(unsigned long DefaultEntryPoint)	//in Words
{
	char* NameSet[5]={0};
	char Name[MAX_PATH]="Intel.hex", Line[65536];
	unsigned long Msg[4];
	int i;
	fstream IntelHex;

	NameSet[0]=(char*)API_LOADFILE;
	NameSet[1]=Name;
	i=(*DialogEngine) (OPENFILEROLLER, MyDialogID, (int)NameSet);
	if (!i) return "Intel Hex\r\nLoad aborted!";       //���� �⪠����� ��� �訡�� ����� API_[BLABLABLA]
cout<<Name<<" "<<(Name+i)<<endl;        //����� pathname � �⤥�쭮� ���}

	unsigned long Offset=0, EntryP=0, EPDefined=0,TotalDataLines=0;
	IntelHex.open (Name,ios::in|ios::text);

	for(int Type=0; Type!=0x01;)
	{
		IntelHex.getline(Line, 65536);
		if (IntelHex.fail()) return "Intel Hex\r\nNo end marker!";
//cout<<Line<<endl;
		if (Line[0]!=':') return "Intel Hex\r\nLine not beginning with \":\"!";
		int Len = Hex(Line+1, 2);
		if (strlen(Line) != Len*2+11) return "Line length mismatches\r\ndata size!";
		int Addr = Hex(Line+3, 4);
		Type = Hex(Line+7, 2);

//ToDo: checksum!

//cout<<"Type "<<Type<<" Addr = "<<Offset<<"+"<<Addr<<" Size = "<<Len<<endl;

		switch (Type)
		{
			case 0x01:	//End marker
			break;

			case 0x00:
				if (Len%2) return "Intel Hex\r\n\Incomplete Short value!";
				TotalDataLines++;
			case 0x02:	//Ignore addresses and data;
			case 0x04:
			break;

			case 0x03:	//CS:IP entrypoint (in Words because of Code Composer addressing model)
				if (Len!=4) return "Intel Hex\r\n\Seg EntryP error!";
				EntryP  = Hex(Line+9 , 4) * 16;	//CS Paragraphs to bytes;
				EntryP += Hex(Line+13, 4);	//IP

				if (EPDefined) return "Intel Hex\r\n\ EntryP redefinition!"; else EPDefined=1;
				if (EntryP != DefaultEntryPoint)
				{
					Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"Entry point mismatches the default one!";
					(*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
				}
			break;
			case 0x05:	//Flat memory model entrypoint (in Words because of Code Composer addressing model)
				if (Len!=4) return "Intel Hex\r\n\Lin EntryP error!";
				EntryP  = Hex(Line+9 , 8);

				if (EPDefined) return "Intel Hex\r\n\ EntryP redefinition!"; else EPDefined=1;
				if (EntryP != DefaultEntryPoint)
				{
					Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"Entry point mismatches the default one!";
					(*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
				}
			break;

			default: return "Intel Hex\r\nUnknown type!";
		}
	}
	if (!TotalDataLines) return "Upload\r\nNo data lines!";
cout<<"EntryPoint (words): "<<EntryP<<endl;
	if (!EPDefined)
	{
		Msg[0]=MB_ICONWARNING|MB_OK|MB_APPLMODAL, Msg[1]=(int)"Warning!", Msg[2]=(int)"No entry point provided by the file! Default value will be used.";
		(*DialogEngine) (MESSAGEBOX, MyDialogID, (int)Msg);
		EntryP = DefaultEntryPoint; //(in Words because of Code Composer addressing model)
	}

	IntelHex.seekg(0);
        unsigned short Command[4]={2,1,0};	//Size, protocol and body.

	for (int j=0,k=0x08AA; j<9; j++,k=0)
	{
		Command[2] = k;
		i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
		cout<<"Prefix "<<j<<" got answer "<<i<<endl;

//				if (!i) return "Upload\r\nNo connection!";
		if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
		if (!j && i==0x80C0) return "Upload\r\n\Unknown keywords!";
		if ( j && i==0x80C1) return "Upload\r\n\Unknown keywords!";
		if (!j && i!=0xC0) return "Upload\r\n\Unknown response!";
		if ( j && i!=0xC1) return "Upload\r\n\Unknown response!";
	}

	Command[3] = EntryP;
	Command[2] = EntryP>>16;	//Warning! Watcom Sign Extension depends on Int type!
	Command[0] = 4;
	i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
	cout<<"Entry point got answer "<<i<<endl;

//	if (!i) return "Upload\r\nNo connection!";
	if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
	if (i==0x80C2) return "Upload\r\n\Wrong entry point!";
	if (i!=0xC2) return "Upload\r\n\Unknown response!";

	int ChunkShorts = 32;

unsigned short Buf[1024+2];	//ToDo: realloc (ChunkShorts*2)!

//	WCB* PrBarDiaPtr[2] = {PrBarDia, NULL};
//	(*DialogEngine) (OPENDIALOG, (unsigned long)(BarDiaTemplate), (unsigned long)(&PrBarDiaPtr));
	(*DialogEngine) (PROGRESSBAR, MyDialogID, 0);	//create progress bar;

	for(int Type=0, DL=0; Type!=0x01;)
	{
		IntelHex.getline(Line, 65536);
		if (IntelHex.fail()) return "Intel Hex\r\nNo end marker!";
//cout<<Line<<endl;
		if (Line[0]!=':') return "Intel Hex\r\nLine not beginning with \":\"!";
		int Len = Hex(Line+1, 2);
		if (strlen(Line) != Len*2+11) return "Line length mismatches\r\ndata size!";
		int Addr = Hex(Line+3, 4);
		Type = Hex(Line+7, 2);
//cout<<"Type "<<Type<<" Addr = "<<Offset<<"+"<<Addr<<" Size = "<<Len<<endl;

//ToDo: checksum!

		switch (Type)
		{
			case 0x01:	//End marker
			break;

			case 0x02:	//Seg addr
				if (Len!=2) return "Intel Hex\r\n\Seg Offset error!";
				Offset = Hex(Line+9, 4) * 16;	//Paragraphs to bytes;
			break;
			case 0x04:	//Ext addr
				if (Len!=2) return "Intel Hex\r\n\Lin Offset error!";
				Offset = Hex(Line+9, 4) * 65536;	//Paragraphs to bytes;
			break;

			case 0x03:	//Entry Points are ignored on this stage;
			case 0x05:
			break;

			case 0x00:	//Data chunk
				DL++;
				if (Len%2) return "Intel Hex\r\n\Incomplete Short value!";

//				for (int i=0; i<Len; i+=2)
//				{
//cout<<Offset+Addr+i<<": "<<Hex(Line+9+i*2, 4)<<endl;
//printf ("%Xh: %Xh\n",Offset+Addr+i,Hex(Line+9+i*2, 4));
//printf ("%Xh: %Xh\n",Offset+Addr*2+i,Hex(Line+9+i*2, 4));	//��� � ������ ������ ᫮����
//				}

				for (unsigned long ChunkOffset=0, Left=Len/2, ChunkAddr = Offset+Addr;
					ChunkOffset<Len/2;
					ChunkOffset+=ChunkShorts, Left-=ChunkShorts, ChunkAddr+=ChunkShorts)	//In Shorts.
				{
RetryFromSize:
					Command[2] = min(Left, ChunkShorts);
					Command[0] = 2;
					i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
					cout<<"Size got answer "<<i<<endl;
	
//					if (!i) return "Upload\r\nNo connection!";
					if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
					if (i==0x80C3) return "Upload\r\n\Wrong size!";
					if (i==0x81C3) goto RetryFromSize;
					if (i==0x81C5) return "Upload\r\n\Bogus Retry!";					
					if (i!=0xC3) return "Upload\r\n\Unknown response!";

RetryFromAddr:
printf ("%Xh?\n",ChunkAddr);	
					Command[3] = ChunkAddr;
					Command[2] = ChunkAddr>>16;	//Warning! Watcom Sign Extension depends on Int type!
					Command[0] = 4;
					i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
					cout<<"Write address got answer "<<i<<endl;
	
//					if (!i) return "Upload\r\nNo connection!";
					if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
					if (i==0x80C5) return "Upload\r\n\Wrong address!";
					if (i==0x81C3) goto RetryFromSize;
					if (i==0x81C5) goto RetryFromAddr;
					if (i!=0xC5) return "Upload\r\n\Unknown response!";
	
					Buf[0] = 2*min(Left, ChunkShorts);
					Buf[1] = 1;
					for (int w=0; w<min(Left, ChunkShorts); w++)
					{
						Buf[2+w]=Hex(Line+9+(w+ChunkOffset)*4, 4);
					}					
	
					i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Buf));
					cout<<"Write payload got answer "<<i<<endl;
	
//					if (!i) return "Upload\r\nNo connection!";
					if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
					if (i==0x80C6) return "Upload\r\n\Write failure!";
					if (i==0x81C3) goto RetryFromSize;
					if (i==0x81C5) goto RetryFromAddr;
					if (i!=0xC6) return "Upload\r\n\Unknown response!";
				}
				if (!(*DialogEngine) (PROGRESSBAR, MyDialogID, 100*DL/TotalDataLines))	//Update and check progress bar;
				{
					Command[2] = 0;
					Command[0] = 2;
					i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
					cout<<"Zero-term (abort) got answer "<<i<<endl;
					
//					if (!i) return "Upload\r\nNo connection!";
					if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
					if (i==0x80C4) return "Upload\r\n\Zero-term failure!";
					if (i!=0xC4) return "Upload\r\n\Unknown response!";

					return "Upload\r\n\User abort!";
				}
			break;

			default: return "Intel Hex\r\nUnknown type!";
		}
	}

	Command[2] = 0;
	Command[0] = 2;
	i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
	cout<<"Zero-term got answer "<<i<<endl;
	
//			if (!i) return "Upload\r\nNo connection!";
	if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
	if (i==0x80C4) return "Upload\r\n\Zero-term failure!";
	if (i!=0xC4) return "Upload\r\n\Unknown response!";
			

	IntelHex.close();
	return "Intel Hex\r\nUploaded!";
}

const char* DoButton (int Button)
{
        unsigned long Msg[4]={CPU2, BM_SETCHECK, BST_CHECKED};
        char* NameSet[5]={0};
        char Name[MAX_PATH]="Log.xls";
        int i;
        unsigned short Command[4]={2,1,0};	//Size, protocol and body.

        switch (Button)
        {
                case CPU2:
//	         	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);

			if (IsCPU2)
			{
				Command[2] = 0x0710;
				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
				cout<<"Command returned "<<i<<endl;
	
	//			if (!i) return "Switch to CPU I\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Switch to CPU I\r\nCommand timeout!";
				if (i==0x4085) return "Switch to CPU I\r\nUnknown command!";
				if (i!=0x4086) return "Switch to CPU I\r\n\Unknown response!";

				for (i=0; i<20; i++)
				{
					Sleep (500);
					Command[2] = 0x0710;
					int j = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(Command));
					if (j == 0x87) {IsCPU2=0;return "Switch to CPU II";}
				}
		return "Switch to CPU I\r\n\Failed to switch!";
			} else {
				Command[2] = 0x0700;
				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
				cout<<"Command returned "<<i<<endl;
	
	//			if (!i) return "Switch to CPU II\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Switch to CPU II\r\nCommand timeout!";
				if (i==0x8085) return "Switch to CPU II\r\nUnknown command!";
				if (i!=0x86) return "Switch to CPU II\r\n\Unknown response!";

				for (i=0; i<20; i++)
				{
					Sleep (500);
					Command[2] = 0x0700;
					int j = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(Command));
					if (j == 0x4087) {IsCPU2=1;return "Switch to CPU I";}
				}
			}
		return "Switch to CPU II\r\n\Failed to switch!";

                case CMD_RESET:
			Command[2] = 0x000f + (0x20-0xf)*IsCPU2;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Command returned "<<i<<endl;

//			if (!i) return "Reset\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Reset\r\nCommand timeout!";
			if (i==0x8085-0x4000*IsCPU2) return "Reset\r\nUnknown command!";
			if (i==0x80  +0x4000*IsCPU2) return "Reset\r\nOK!";
		return "Reset\r\n\Unknown response!";

                case CMD_RUN:
/*for (int i=0; i<101; i++)
{
cout<<i<<" "<<(*DialogEngine) (PROGRESSBAR, MyDialogID, i)<<endl;
if (!(*DialogEngine) (PROGRESSBAR, MyDialogID, i)) break;
Sleep (200);
}*/
			Command[2] = 0x0010-(0x10-0x0E)*IsCPU2;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Command returned "<<i<<endl;

//			if (!i) return "Run\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Run\r\nCommand timeout!";
			if (i==0x8085-0x4000*IsCPU2) return "Run\r\nUnknown command!";
			if (i==0x83  +0x4000*IsCPU2) return "Run\r\nOK!";
		return "Run\r\n\Unknown response!";

                case CMD_ERASE:
			Command[2] = 0x300+0x100*IsCPU2;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Command returned "<<i<<endl;

//			if (!i) return "Erase\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Erase\r\nCommand timeout!";
			if (i==0x8085-0x4000*IsCPU2) return "Erase\r\nUnknown command!";
			if (i!=0x81  +0x4000*IsCPU2) return "Erase\r\nUnknown response!";

			for (i=0; i<20; i++)
			{
				Sleep (500);
				Command[2] = 0x300+0x100*IsCPU2;
				int j = (*NetEngine) (SEND_REQUEST, MyDialogID, (unsigned long)(Command));
				if (j == 0x01) return "Erase\r\nOK!";
			}
                return "Erase\r\nFailed!";

                case CMD_UPLOAD:
			Command[2] = 0x0100+0x100*IsCPU2;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Command returned "<<i<<endl;

//			if (!i) return "Upload\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Upload\r\nCommand timeout!";
			if (i==0x8085-0x4000*IsCPU2) return "Upload\r\nUnknown command!";
			if (i==0x84  +0x4000*IsCPU2) return "Upload\r\nPrepared!";
		return "Upload\r\n\Unknown response!";

		case INTELHEX:
		case UPLOAD:
			char Body[65536], *P=Body;
			unsigned short Hex[13107+2]={0}, *V=Hex+2;	// (65536 ����� null-term) / 5, ��⮬� �� ����� 䨧��᪨ �� �������
			int SpaceTerminates = 0;	//������騩 �஡���� ᨬ��� ������ �������� ���ᨭ� ��� ��।���� �᫠ � "�⢥न��" ���.
			int N;

		        unsigned long Msg[4]={ENTRYP, WM_GETTEXT,65536,(LPARAM)(LPCTSTR)Body};
		        Msg[0]=ENTRYP;
	         	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
	         	unsigned long EntryP = strtoul (Body, &P, 0);
	         	if (P==Body || *P!=' '&&*P) if (Button == INTELHEX) return "Intel Hex\r\nEntry Point syntax error!"; else return "Upload\r\nEntry Point syntax error!";
	         	if (EntryP%2) if (Button == INTELHEX) return "Intel Hex\r\nOdd Entry Point!"; else return "Upload\r\nOdd Entry Point!";
	         	EntryP/=2;
//			printf ("%xh\n", EntryP);

			if (Button == INTELHEX)
			{
				const char* ErrorText = GetIntelHex (EntryP);
				(*DialogEngine) (PROGRESSBAR, MyDialogID, 100);	//Force close progress bar (even if aborted on error);
		return ErrorText;
			}

		        Msg[0]=ADDRESS;
	         	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
	         	unsigned long Addr = strtoul (Body, &P, 0);
	         	if (P==Body || *P!=' '&&*P) return "Upload\r\nAddress syntax error!";
	         	if (Addr%2) return "Upload\r\nOdd Address!";
	         	Addr/=2;
//			printf ("%xh\n", Addr);

		        Msg[0]=HEX_BODY;
	         	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
	         	P=Body;

			for (i=0; i<65536; i++,P++)
			{
				if (*P <= '9' && *P >= '0')
				{
					*V*=16;
					*V+=*P-'0';
					SpaceTerminates = 1;
				}
				else if (*P <= 'F' && *P >= 'A')
				{
					*V*=16;
					*V+=*P-'A'+10;
					SpaceTerminates = 1;
				}
				else if (*P <= 'f' && *P >= 'a')
				{
					*V*=16;
					*V+=*P-'a'+10;
					SpaceTerminates = 1;
				}
				else if (*P == ' ' || *P == '	')
				{
					if (SpaceTerminates) V++;
					SpaceTerminates = 0;
				}
				else	//Any term character, including null-term
				{
					if (SpaceTerminates) V++;
					break;
				}
			}
			N=V-Hex-2;
cout<<N<<" values"<<endl;
for (int j=0;j<N;j++) printf ("%xh ", Hex[j+2]);
cout<<endl;
			if (!N) return "Upload\r\nNo data!";
			for (int j=0,k=0x08AA; j<9; j++,k=0)
			{
				Command[2] = k;
				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
				cout<<"Prefix "<<j<<" got answer "<<i<<endl;

//				if (!i) return "Upload\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
				if (!j && i==0x80C0) return "Upload\r\n\Unknown keywords!";
				if ( j && i==0x80C1) return "Upload\r\n\Unknown keywords!";
				if (!j && i!=0xC0) return "Upload\r\n\Unknown response!";
				if ( j && i!=0xC1) return "Upload\r\n\Unknown response!";
			}

			Command[3] = EntryP;
			Command[2] = EntryP>>16;	//Warning! Watcom Sign Extension depends on Int type!
			Command[0] = 4;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Entry point got answer "<<i<<endl;

//			if (!i) return "Upload\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
			if (i==0x80C2) return "Upload\r\n\Wrong entry point!";
			if (i!=0xC2) return "Upload\r\n\Unknown response!";

#define CHUNK_SHORTS 32

			for (int Offset=0, Left=N; Offset<N; Offset+=CHUNK_SHORTS,Left-=CHUNK_SHORTS)	//In Shorts.
			{
				Command[2] = min(Left, CHUNK_SHORTS);
				Command[0] = 2;
				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
				cout<<"Size got answer "<<i<<endl;

//				if (!i) return "Upload\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
				if (i==0x80C3) return "Upload\r\n\Wrong size!";
				if (i!=0xC3) return "Upload\r\n\Unknown response!";

				Command[3] = Addr;
				Command[2] = Addr>>16;	//Warning! Watcom Sign Extension depends on Int type!
				Command[0] = 4;
				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
				cout<<"Write word address got answer "<<i<<endl;

				Addr += CHUNK_SHORTS;// *2;	//Warning! Destroys Addr!

//				if (!i) return "Upload\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
				if (i==0x80C5) return "Upload\r\n\Wrong address!";
				if (i!=0xC5) return "Upload\r\n\Unknown response!";

				Hex[0+Offset] = 2*min(Left, CHUNK_SHORTS); //N*2;	//��� � �����, ��⮬� �� ࠧ��� ��� �३���ઠ.
				Hex[1+Offset] = 1;	//Warning! Destroys Hex!	//��� ��⮪��� (0=������, 1=�ଢ���)

				i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Hex+Offset));
				cout<<"Write payload got answer "<<i<<endl;

//				if (!i) return "Upload\r\nNo connection!";
				if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
				if (i==0x80C6) return "Upload\r\n\Write failure!";
				if (i!=0xC6) return "Upload\r\n\Unknown response!";
			}

			Command[2] = 0;
			Command[0] = 2;
			i=(*NetEngine) (SEND_MESSAGE, MyDialogID, (unsigned long)(Command));
			cout<<"Zero-term got answer "<<i<<endl;
			
//			if (!i) return "Upload\r\nNo connection!";
			if (i==0xFFFFFFFF) return "Upload\r\nData timeout!";
			if (i==0x80C4) return "Upload\r\n\Zero-term failure!";
			if (i!=0xC4) return "Upload\r\n\Unknown response!";

		return "Upload\r\nWritten!";
        }

        return NULL;
}


extern "C" int __export WINAPI PluginEngine (long DiaId, unsigned long uMsg, unsigned long wParam, unsigned long lParam)
{
        char Text[256];
        unsigned long Msg[4]={HEX_BODY, WM_SETTEXT,0,(LPARAM)(LPCTSTR)Text};
if (!uMsg) cout<<"WM_NULL "<<wParam<<" "<<lParam<<endl;

        if (!uMsg && (!DialogEngine && !NetEngine || (unsigned long)DialogEngine==wParam && (unsigned long)NetEngine==lParam) )
        {
                DialogEngine=(FWAPI*)(wParam);
                NetEngine=(FWAPI*)(lParam);
                cout<<"Opening a dialog by a template "<<DialogEngine<<endl;
                WCB* PluginEnginePtr[2] = {PluginEngine, NULL};

                /*MyDialogID = */(*DialogEngine) (OPENDIALOG, (unsigned long)(DiaTemplate), (unsigned long)(&PluginEnginePtr));
//                                 (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&Message)) );    //���, ������ �� ��।�� ���� ��ࢮ�� �������, ⠬ ⨯ ᮮ�饭��, ����� �� �㤥� �ਭ�����; �� ��஬ ������� -- �᫮ ����⨯��� ������-�����, �⮡� �����, ᪮�쪮 ����訢���.
//                Message.Hdr=MSG_DEBUG|CAT_MODE; //��ன "���" ⨯
//                IncomingBuffer = (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ModbusMain)) );    //���� �� �� ��ॣ����஢���� ᮮ�饭�� ����, ����� �� ��� �맮�� �����.
                                                //��⨩ ⨯, ��⮬ �뭥ᥬ � ��㣮� ���
//                                 (int*) ( (*NetEngine) (REGISTER_INCOMING, MyDialogID, (unsigned long)(&ResetLogs)) );

                return 0;//(int)("Test point 1");
        }

        if (uMsg==WM_COMMAND && lParam && wParam>>16==BN_CLICKED)
         if (Msg[3]=(LPARAM)(LPCTSTR)(DoButton(wParam&0xFFFF)))//I love C :)
         {
         	Msg[0]=wParam&0xFFFF;
         	(*DialogEngine) ( SENDMDIALOG, MyDialogID, (int)Msg);
         }

        if (uMsg==WM_INITDIALOG)
        {
		MyDialogID = lParam;
cout<<"Newly created Dialog ID = "<<MyDialogID<<endl;

        }
        
//      return 0xDEADC0DE;
        return 0;
}

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        if (fdwReason==DLL_PROCESS_ATTACH)
        {
//              hinst=hinstDLL;
                DiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "FWUPDATE", RT_DIALOG));
//                BarDiaTemplate=LoadResource(hinstDLL, FindResource (hinstDLL, "PROGBAR", RT_DIALOG));
                cout<<"Plug Init "<<DiaTemplate<</*" "<<BarDiaTemplate<<*/" "<<fdwReason<<endl;
//cout<<"Supported Size: "<<sizeof(MsgDebugLogs)<<endl;
        }

        return 1;
}
