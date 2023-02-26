#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <math.h>
#include <process.h>

#define   DIRECTDRAW_VERSION 0x0300
//#define   DIRECTDRAW_VERSION 0x0700
#include "dx_sdk\\ddraw.h"
#define  DIRECTINPUT_VERSION 0x0300
#include "dx_sdk\\dinput.h"

#include "ContrlPn.h"

extern volatile _DXData DXData;
extern volatile _DXReq DXReq;
extern volatile float LoopedGraph[4096];
extern volatile int LoopedGraphIndex;
extern volatile int RequestedW, RequestedH;

LPDIRECTDRAW lpDD=NULL;	//Our piece of DirectDraw;
LPDIRECTDRAWSURFACE S_Main=NULL;	//Two most common surfaces;
LPDIRECTDRAWSURFACE S_Back=NULL;
//int Width, Height, Brutto;
DDSURFACEDESC UserPreferred = {sizeof(DDSURFACEDESC), 0};
LPDIRECTDRAWPALETTE PalPtr=NULL;
PALETTEENTRY Palette[256]={0};
unsigned char PalRed[256];

#define RandLT(X) (rand()*(X)/(RAND_MAX+1))

#pragma pack (push)
#pragma pack (0)
	typedef struct
	{
		unsigned short Width, Height;
		unsigned short HorzSet, VertSet;
	} ModeLine;
#pragma pack (pop)
ModeLine *Modes = NULL;
int NModes = 0;

#define PALMASK 3
#define PAL16 1
#define PAL256 2
#define RGBMASK 28
#define RGB16 4
#define RGB24 8
#define RGB32 16

#define INITVIDEO 1
#define INITINPUT 2

void ShowError()
{
	char *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
	MessageBox ( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}

#define PANEL_W 1920
#define PANEL_H 1080


#define TRANSPARENT_COLOR 0x1000000

enum
{
	PANEL,	//Текстура фоновой панели по очевидным причинам в стэке слоёв всегда идёт первой.

	TINY_DIGIT_0,
	TINY_DIGIT_1,
	TINY_DIGIT_2,
	TINY_DIGIT_3,
	TINY_DIGIT_4,
	TINY_DIGIT_5,
	TINY_DIGIT_6,
	TINY_DIGIT_7,
	TINY_DIGIT_8,
	TINY_DIGIT_9,

	MEDIUM_DIGIT_0,
	MEDIUM_DIGIT_1,
	MEDIUM_DIGIT_2,
	MEDIUM_DIGIT_3,
	MEDIUM_DIGIT_4,
	MEDIUM_DIGIT_5,
	MEDIUM_DIGIT_6,
	MEDIUM_DIGIT_7,
	MEDIUM_DIGIT_8,
	MEDIUM_DIGIT_9,
	MEDIUM_DIGIT_BLANK,

	BIG_DIGIT_0,
	BIG_DIGIT_1,
	BIG_DIGIT_2,
	BIG_DIGIT_3,
	BIG_DIGIT_4,
	BIG_DIGIT_5,
	BIG_DIGIT_6,
	BIG_DIGIT_7,
	BIG_DIGIT_8,
	BIG_DIGIT_9,

	POWER_STATUS_INACTIVE,
	POWER_STATUS_ST_GEN,
	POWER_STATUS_ST_INV,
	POWER_STATUS_READY,
	POWER_STATUS_POWER_ON,
	POWER_STATUS_BOOST,
	POWER_STATUS_ERROR,

	CONN_STATUS_OFF,
	CONN_STATUS_TRY,
	CONN_STATUS_ON,
	SETUP_SPROCKET,
	CLOSING_CROSS,

	BATTERY_SCALE,

	AMPERAGE_G_TEXTURE,

	LACUNA_COIL,
	EM_BASE,

	TOTAL_TEXTURES
} Images;

struct Texture
{
	char FileName [MAX_PATH];
	int Width, Height, IsVRAM;
	unsigned int ColorKeyBGRF;
	LPDIRECTDRAWSURFACE Surface;
} Textures[TOTAL_TEXTURES] =
{
{"_G_Panel_1920x1080.bmp", PANEL_W, PANEL_H},

{"_G_TinyDigit0.bmp", 64, 128},
{"_G_TinyDigit1.bmp", 64, 128},
{"_G_TinyDigit2.bmp", 64, 128},
{"_G_TinyDigit3.bmp", 64, 128},
{"_G_TinyDigit4.bmp", 64, 128},
{"_G_TinyDigit5.bmp", 64, 128},
{"_G_TinyDigit6.bmp", 64, 128},
{"_G_TinyDigit7.bmp", 64, 128},
{"_G_TinyDigit8.bmp", 64, 128},
{"_G_TinyDigit9.bmp", 64, 128},

{"_G_MediumDigit0.bmp", 64, 128},
{"_G_MediumDigit1.bmp", 64, 128},
{"_G_MediumDigit2.bmp", 64, 128},
{"_G_MediumDigit3.bmp", 64, 128},
{"_G_MediumDigit4.bmp", 64, 128},
{"_G_MediumDigit5.bmp", 64, 128},
{"_G_MediumDigit6.bmp", 64, 128},
{"_G_MediumDigit7.bmp", 64, 128},
{"_G_MediumDigit8.bmp", 64, 128},
{"_G_MediumDigit9.bmp", 64, 128},
{"_G_MediumDigitBl.bmp", 64, 128},

{"_G_BigDigit0.bmp", 64, 128},
{"_G_BigDigit1.bmp", 64, 128},
{"_G_BigDigit2.bmp", 64, 128},
{"_G_BigDigit3.bmp", 64, 128},
{"_G_BigDigit4.bmp", 64, 128},
{"_G_BigDigit5.bmp", 64, 128},
{"_G_BigDigit6.bmp", 64, 128},
{"_G_BigDigit7.bmp", 64, 128},
{"_G_BigDigit8.bmp", 64, 128},
{"_G_BigDigit9.bmp", 64, 128},

{"_G_PowerStatusInactive.bmp", 64, 128},
{"_G_PowerStatusStartGen.bmp", 64, 128},
{"_G_PowerStatusStartInv.bmp", 64, 128},
{"_G_PowerStatusReady.bmp", 64, 128},
{"_G_PowerStatusPowerOn.bmp", 64, 128},
{"_G_PowerStatusBoost.bmp", 64, 128},
{"_G_PowerStatusError.bmp", 64, 128},

{"_G_Button_Disconnected.bmp", 64, 128},
{"_G_Button_Connecting.bmp", 64, 128},
{"_G_Button_Connected.bmp", 64, 128},
{"_G_Button_Settings.bmp", 64, 128},
{"_G_Button_Close.bmp", 64, 128},

{"_G_BatteryScale.bmp", 64, 128},

{"_G_AmperageGraph.bmp", 64, 128},

{"_G_Lacuna_Coil.bmp", 64, 128},
{"_G_EM_Base.bmp", 64, 128} };

enum
{
	BACKGROUND,
	
	WIN_UPPER_DIGIT,
	WIN_LOWER_DIGIT,
	WOUT_UPPER_DIGIT,
	WOUT_LOWER_DIGIT,
	RATIO_UPPER_DIGIT,
	RATIO_LOWER_DIGIT,
	AMP_UPPER_DIGIT,
	AMP_LOWER_DIGIT,
	LEVEL_100_PERCENT,
	LEVEL_UPPER_DIGIT,
	LEVEL_LOWER_DIGIT,
	TT_UPPER_DIGIT,
	TT_MED_DIGIT,
	TT_LOWER_DIGIT,
	TR_UPPER_DIGIT,
	TR_MED_DIGIT,
	TR_LOWER_DIGIT,

	POWER_STATUS,
	CONN_STATUS,
	SETTINGS,
	CLOSEALL,
	LEVEL_STATUS,
	AMPERAGE_GRAPH,

	COIL_BGND,
	COIL_EM,

	TOTAL_CONTROLS
} Rectangles;

struct
{
	int PosX, PosY, WipeX, WipeY;
	Texture *CurrentTexture;
} Controls [TOTAL_CONTROLS] =
{
{0, 0, 0, 0, Textures + PANEL},

{185, 30+170, 	0,0,Textures + MEDIUM_DIGIT_0},
{272, 30+170, 	0,0,Textures + MEDIUM_DIGIT_1},
{392, 30+170,	0,0, Textures + MEDIUM_DIGIT_0},
{479, 30+170,	0,0, Textures + MEDIUM_DIGIT_1},
{20+234, 530+161, 	0,0,Textures + BIG_DIGIT_0},
{100+270, 530+161,	0,0, Textures + BIG_DIGIT_1},
{929, 30+164, 	0,0,Textures + MEDIUM_DIGIT_0},
{1013, 30+164, 	0,0,Textures + MEDIUM_DIGIT_1},
{1521-113-50+4 - (1601-124-54+4) + (1521-113-50+4), 30+164, 	0,0,Textures + MEDIUM_DIGIT_0},
{1521-113-50+4, 30+165, 0,0,Textures + MEDIUM_DIGIT_0},
{1601-124-54+4, 30+165, 0,0,Textures + MEDIUM_DIGIT_1},
{1000-243-64, 848,	0,0, Textures + TINY_DIGIT_0},
{1015-240-64, 848,	0,0, Textures + TINY_DIGIT_0},
{1030-237-64, 848,	0,0, Textures + TINY_DIGIT_1},
{1100-243, 848,		0,0, Textures + TINY_DIGIT_0},
{1115-240, 848,		0,0, Textures + TINY_DIGIT_0},
{1130-237, 848,		0,0, Textures + TINY_DIGIT_1},

{1800-67, 900-262,	0,0, Textures + POWER_STATUS_INACTIVE},
{300-6, 971,	0,0, Textures + CONN_STATUS_OFF},
{210, 965,	0,0, Textures + SETUP_SPROCKET},
{130, 966,	0,0, Textures + CLOSING_CROSS},
{1600-65-250, 377,	0,0, Textures + BATTERY_SCALE},
{800-108, 200+188,	1,0, Textures + AMPERAGE_G_TEXTURE},

{1172, 137,	0,0, Textures + LACUNA_COIL},
{1172, 137,	0,1, Textures + EM_BASE} };

int PStDefX=Controls[POWER_STATUS].PosX;
int PStDefY=Controls[POWER_STATUS].PosY;

int ScrollX=0, ScrollY=0;
int ScrW, ScrH, ScrBPP;
int /*ColorRM, ColorGM, ColorBM, */ColorRS, ColorGS, ColorBS, ColorRD, ColorGD, ColorBD;
int ColSea=0x006080, ColSky=0x0000FF;
char WaterLoop;
int OldFagSizeLimit=0;//debug!!	//some ancient video cards can't create vram surfaces bigger than screen res.

unsigned long CRandU=11111111;
#define RANDU (CRandU=CRandU*65539|0x7FFFFFFF)
#define RANDU_MAX 0x7FFFFFFF

//DD_OK в мастдае равно нулю, так что дополнительные действия для приведения к общему возврату "0 = ошибки нет, иначе код ошибки" не нужны.

HRESULT WINAPI V_CollectMode(LPDDSURFACEDESC desc, LPVOID p)
{
	int ThisMode;
	unsigned short *Set;
//cout<<"Looking for existing mode "<<desc->dwWidth<<" "<<desc->dwHeight<<endl;
	for (ThisMode=0;ThisMode<NModes;ThisMode++) if (Modes[ThisMode].Width  == max (desc->dwWidth,desc->dwHeight) && Modes[ThisMode].Height == min (desc->dwWidth,desc->dwHeight) ) break;
	if (ThisMode == NModes)
	{
		NModes++;
		Modes = (ModeLine*) realloc (Modes, NModes*sizeof(ModeLine));
		if (!Modes) return DDENUMRET_CANCEL;
		Modes[ThisMode].Width  = max (desc->dwWidth,desc->dwHeight);
		Modes[ThisMode].Height = min (desc->dwWidth,desc->dwHeight);
		Modes[ThisMode].VertSet = 0;
		Modes[ThisMode].HorzSet = 0;
	}
	

	if (desc->dwWidth > desc->dwHeight) Set = &(Modes[ThisMode].HorzSet); else Set = &(Modes[ThisMode].VertSet);

	switch (desc->ddpfPixelFormat.dwRGBBitCount)
	{
		case  4: *Set |= PAL16; break;
		case  8: *Set |= PAL256;break;
		case 16: *Set |= RGB16; break;
		case 24: *Set |= RGB24; break;
		case 32: *Set |= RGB32; break;
	}

	return DDENUMRET_OK;
}

int V_CreateSurfaces()
{
	HRESULT ddrval;

	DDSURFACEDESC ddsd={sizeof(DDSURFACEDESC),0};
	DDSCAPS ddscaps;

	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;

	ddrval = lpDD->CreateSurface( &ddsd, &S_Main, NULL );	//Creating Primary surface, notifying the Windows we're going to create Backbuffer surface later.
	if( ddrval != DD_OK ) return ddrval;
//cout<<"Main created"<<endl;

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	ddrval = S_Main->GetAttachedSurface(&ddscaps, &S_Back);	//Creating Backbuffer surface as we promised.
	if( ddrval != DD_OK ) return ddrval;
//cout<<"Back created"<<endl;

		//Почему-то не лочится (хотя это место нам и не нужно, но странно).
/*	ddrval = S_Main->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if(ddrval != DD_OK) return ddrval;
	Brutto = ddsd.lPitch;
	Width = ddsd.dwWidth;
	Height = ddsd.dwHeight;
	ddrval = S_Main->Unlock(NULL);
cout<<Width<<" x "<<Height<<" pitch "<<Brutto<<endl;
*/
	return ddrval;
}

LPDIRECTINPUT dinput;
LPDIRECTINPUTDEVICE keyboard;
int I_AttachDInput (HWND hWnd)
{
	HRESULT r = DirectInputCreate( 	GetModuleHandle(NULL), DIRECTINPUT_VERSION, &dinput, 0 ); 
	if (r) return r;
	r = dinput->CreateDevice( GUID_SysKeyboard, &keyboard, 0 );
	if (r) return r;
	r = keyboard->SetDataFormat( &c_dfDIKeyboard );
	if (r) return r;
	r = keyboard->SetCooperativeLevel( hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	if (r) return r;
	r = keyboard->Acquire();	//Это тебе после каждого возврата в окно повторять придётся!

	return r;
}

int DX_Init (int *RatioPromille, BOOL *IsVertical, int Services, HWND hWnd)
{
	switch (Services)
	{
		case INITVIDEO|INITINPUT:
		case INITVIDEO:	
			HRESULT ddrval;
		
		//DirectDrawEnumerate();
		
			ddrval = DirectDrawCreate( NULL, &lpDD, NULL );	//Generic giving our program the DD abilities.
			if( ddrval != DD_OK ) return ddrval;
			
			ddrval = lpDD->SetCooperativeLevel( hWnd        , DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);	//DOS-like full access via main dialog window

			if( ddrval != DD_OK ) return ddrval;
			if (Services|INITINPUT)
			{
				ddrval=I_AttachDInput (hWnd);
				if( ddrval != DD_OK ) return ddrval;
			}


			ddrval = V_CreateSurfaces();
			if( ddrval != DD_OK ) return ddrval;
		
			lpDD->GetDisplayMode(&UserPreferred);
			*RatioPromille = max (UserPreferred.dwWidth,UserPreferred.dwHeight) * 1000 / min (UserPreferred.dwWidth,UserPreferred.dwHeight);
			*IsVertical = UserPreferred.dwWidth < UserPreferred.dwHeight;
		
			ddrval = lpDD->EnumDisplayModes( 0, 0, NULL, V_CollectMode );	//Pass 1: getting max resolution;
			if(ddrval != DD_OK) return ddrval;
			if (!Modes) return -1;
		break;
		case INITINPUT:
			ddrval=I_AttachDInput (hWnd);
			if( ddrval != DD_OK ) return ddrval;
		break;
	}
	return 0;	//No error
}

//Creates read/write surfaces in SYSTEM memory (try to benchmark READING from video memory and you'll see why!)
int V_Create_R_W_Surface (DWORD w, DWORD h, LPDIRECTDRAWSURFACE *surf)
{
	DDSURFACEDESC desc={sizeof(desc)};
	HRESULT ddrval;

	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	desc.dwWidth = w;
	desc.dwHeight = h;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	ddrval=lpDD->CreateSurface( &desc, surf, 0 );

	if(ddrval != DD_OK) return ddrval;

	return 0;
}

//Creates write-only surface in VIDEO memory.
int V_Create_WO_Surface (DWORD w, DWORD h, LPDIRECTDRAWSURFACE *surf)
{
	DDSURFACEDESC desc={sizeof(desc)};
	HRESULT ddrval;

	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	desc.dwWidth = w;
	desc.dwHeight = h;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	ddrval=lpDD->CreateSurface( &desc, surf, 0 );

	if(ddrval != DD_OK) return ddrval;

	return 0;
}

int V_FadeOut_Everything(int mSec)
{
	DDSURFACEDESC CurMode = {sizeof(DDSURFACEDESC), 0};
	HRESULT ddrval;

	lpDD->GetDisplayMode(&CurMode);

	if (CurMode.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED1|DDPF_PALETTEINDEXED2|DDPF_PALETTEINDEXED4|DDPF_PALETTEINDEXED8|DDPF_PALETTEINDEXEDTO8))
	{	//Fading Out the screen palette
		0;	//ToDo
	} else {//Fading Out the RGB surface
		DDSURFACEDESC ddsd;
		HRESULT ddrval;
		LPDIRECTDRAWSURFACE S_Screenshot;
		int R, G, B, Rp, Gp, Bp, Rh, Gh, Bh, Rd, Gd, Bd, Rs, Gs, Bs, MinDiap;
		unsigned long Time, TD;
		int TimeToGo;
		char *Addr; int Size;

		Time = timeGetTime();

		V_Create_R_W_Surface (CurMode.dwWidth,CurMode.dwHeight,&S_Screenshot);

		ddrval = S_Screenshot->BltFast( 0, 0, S_Main, 0, DDBLTFAST_WAIT );
		if(ddrval != DD_OK) return ddrval;

		for (Rp=32,R=CurMode.ddpfPixelFormat.dwRBitMask;R;R<<=1) Rp--;
		for (Gp=32,G=CurMode.ddpfPixelFormat.dwGBitMask;G;G<<=1) Gp--;
		for (Bp=32,B=CurMode.ddpfPixelFormat.dwBBitMask;B;B<<=1) Bp--;
		for (Rh=-1,R=CurMode.ddpfPixelFormat.dwRBitMask;R;R>>=1) Rh++;
		for (Gh=-1,G=CurMode.ddpfPixelFormat.dwGBitMask;G;G>>=1) Gh++;
		for (Bh=-1,B=CurMode.ddpfPixelFormat.dwBBitMask;B;B>>=1) Bh++;
		R=CurMode.ddpfPixelFormat.dwRBitMask;
		G=CurMode.ddpfPixelFormat.dwGBitMask;
		B=CurMode.ddpfPixelFormat.dwBBitMask;
		MinDiap=min(Rh-Rp,min(Gh-Gp,Bh-Bp));
		Rd=Rh-MinDiap;
		Gd=Gh-MinDiap;
		Bd=Bh-MinDiap;

	//	ddrval = S_Main->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_DISCARDCONTENTS, NULL);
//		ddrval = S_Back->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_READONLY, NULL);

		for(TimeToGo=mSec;TimeToGo>0;TimeToGo-=TD)
		{
			memset (&ddsd, 0, sizeof (ddsd));
			ddsd.dwSize = sizeof( ddsd );

			ddrval = S_Screenshot->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
			if(ddrval != DD_OK) return ddrval;
			Addr = (char*) ddsd.lpSurface;
			Size = ddsd.dwHeight*ddsd.lPitch;

			TD = timeGetTime()-Time;
			TD = 255*TD/mSec;

			if (CurMode.ddpfPixelFormat.dwRGBBitCount>=24)
			{
//*
			    if (TD>255) TD = 255;
			    __asm
			    {
				mov esi, Addr
				mov ebx, TD
				mov ecx, Size
				shr ecx, 2
firsttry:			mov eax, ds:[esi]
				sub al, bl
				jnc Bt1
				sub al, al
Bt1:				sub ah, bl
				jnc Bt2
				sub ah, ah
Bt2:				bswap eax
				sub al, bl
				jnc Bt3
				sub al, al
Bt3:				sub ah, bl
				jnc Bt4
				sub ah, ah
Bt4:				bswap eax
				mov ds:[esi], eax
				add esi, 4
				loop firsttry
			    }
/*/
			    if (TD>255) TD = 255;
			    __asm
			    {
				mov esi, Addr
				mov ebx, TD
				mov ecx, Size
				shr ecx, 3
firsttry:			movq mm7, ds:[esi]
				psubusb mm7, 0x0a0a0a0a0a0a0a0a
				movq ds:[esi], mm7
				add esi, 8
				loop firsttry
			    }
//	*/
			} else {
				TD=(TD<<MinDiap)>>7;
				Rs=TD<<Rd;
				Gs=TD<<Gd;
				Bs=TD<<Bd;
				TD=(TD<<7)>>MinDiap;
				for (unsigned short *Pixel = (unsigned short *) Addr; Pixel < (unsigned short *)(Addr+Size); Pixel++)
				{
					if (((*Pixel)&R)<Rs) (*Pixel)&=~R; else (*Pixel)-=Rs;
					if (((*Pixel)&G)<Gs) (*Pixel)&=~G; else (*Pixel)-=Gs;
					if (((*Pixel)&B)<Bs) (*Pixel)&=~B; else (*Pixel)-=Bs;
				}
			}

			TD = TD*mSec/255;
			Time += TD;

			ddrval=S_Screenshot->Unlock(NULL);
			if(ddrval != DD_OK) return ddrval;

			ddrval = S_Back->BltFast( 0, 0, S_Screenshot, 0, DDBLTFAST_WAIT );
			if(ddrval != DD_OK) return ddrval;

			ddrval = S_Main->Flip( 0, DDFLIP_WAIT);
			if(ddrval != DD_OK) return ddrval;
		}

		return S_Screenshot->Release();
	}

	return 0;
}

int V_RedrawAll()
{
	HRESULT ddrval;
	RECT From, To, Full;
	int i, W, H;
	Texture *T;//, *F;

//			case 0: T=&Bgnd; F=&F_Bgnd; P=&Pre_Bgnd; break;
//			default: T=&Map; F=&F_Map;  P=&Pre_Map;

	for (i=0; i<TOTAL_CONTROLS; i++)
	{
		T=Controls[i].CurrentTexture;

		From.left  =Controls[i].WipeX;
		From.top   =Controls[i].WipeY;
		From.right =T->Width ;
		From.bottom=T->Height;
		To.left = Controls[i].PosX - ScrollX + Controls[i].WipeX;
		To.top  = Controls[i].PosY - ScrollY + Controls[i].WipeY;
		W = T->Width  - Controls[i].WipeX;
		H = T->Height - Controls[i].WipeY;

		if (To.left + W > ScrW)
		{
			From.right -= To.left + W - ScrW;
		}
		if (To.top + H > ScrH)
		{
			From.bottom -= To.top + H - ScrH;
		}

		if (To.left < 0)
		{
			From.left -= To.left;
			To.left=0;
		}
		if (To.top < 0)
		{
			From.top -= To.top;
			To.top=0;
		}
		
/*		From.left   = ScrollX;// - Controls[i].PosX;
		From.top    = ScrollY;// - Controls[i].PosY;
		From.right  = From.left + ScrW;
		From.bottom = From.top  + ScrH;
		To.left = 0;
		To.top  = 0;
	
		if (From.top <0)
		{
			To. top=-From.top;
			From.top =0;
		} else if (From.bottom > T->Height) {
			From.bottom = T->Height;
		}
		if (From.left<0)
		{
			To.left=-From.left;
			From.left=0;
		} else if (From.right > T->Width) {
			From.right = T->Width;
		}*/
	
		if (From.bottom-From.top>0 && From.right-From.left>0)
		{
			ddrval = S_Back->BltFast(To.left, To.top, T->Surface, &From, DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);
			if(ddrval != DD_OK) return ddrval;
		}

	}

	ddrval = S_Main->Flip( 0, DDFLIP_WAIT);
	return ddrval;
}

int V_SetMode (int W, int H, int D)
{
	if (S_Back) S_Back->Release();
	if (S_Main) S_Main->Release();

	lpDD->SetDisplayMode(W,H,D);
	
	//Тру пишет, что 4-битные DD не поддерживаются. Возможно, поддерживались между его древним Директом и NT, плюс м. б. Wine.
	//Стопудово поддерживались, в хедерах всё есть, надо дописывать.

	int err= V_CreateSurfaces();
	if (D>8 || err) return err;
	
	if (PalPtr) PalPtr->Release();
	PalPtr=NULL;
	if (D==8) lpDD->CreatePalette( DDPCAPS_8BIT | DDPCAPS_ALLOW256, Palette, &PalPtr, 0 );
	else if (D==4) lpDD->CreatePalette( DDPCAPS_4BIT, Palette, &PalPtr, 0 );
	if (err) return err;
	return S_Main->SetPalette(PalPtr);
}

#define MODE_CURRENT		0x00000001
#define MODE_NEAREST		0x00000002
#define MODE_BIGGER_ONLY	0x00000004
#define MODE_KEEP_ASPECT	0x00000008
#define MODE_RGB		0x00000010
#define MODE_256		0x00000020

int V_GetMode (int *W, int *H, int *D, unsigned int Mode)
{
	if (Mode & MODE_CURRENT)	//Replace incoming values with current mode settings
	{
		DDSURFACEDESC CurMode = {sizeof(DDSURFACEDESC), 0};
		lpDD->GetDisplayMode(&CurMode);

		*D=CurMode.ddpfPixelFormat.dwRGBBitCount;
		*W=CurMode.dwWidth;
		*H=CurMode.dwHeight;
	}

	if (Mode & MODE_NEAREST)	//Finding nearest existing
	{
		int Error = 0x7FFFFFFF;
		int i;
		for (i=0;i<NModes;i++)
		{
			if ((Mode & MODE_BIGGER_ONLY) && (Modes[i].Width < *W || Modes[i].Height < *H)) continue;
		}
	}

	return 0;
}

int DX_DeInit()
{
	//Как-то не поленись определять, был ли мальчик (детач-инпут же это делает!)

	if (S_Back) S_Back->Release();
	if (S_Main) S_Main->Release();
	if (PalPtr) PalPtr->Release();
	S_Back=NULL;
	S_Main=NULL;
	PalPtr=NULL;
	free (Modes);
	Modes = NULL;
	NModes = 0;
	if (lpDD)
	{
		lpDD->RestoreDisplayMode();
		lpDD->Release();
	}
	lpDD=NULL;

	if (keyboard)
	{
		keyboard->Unacquire();
		keyboard->Release(), keyboard=0;
	}

	return 0;
}

int RestR=0, RestG=0, RestB=0;

void ResetDithering() {RestR=0, RestG=0, RestB=0;}

unsigned int BGRtoCard (int BGR, int Exclude, int DivideByBGR)
{
	int B=(BGR&0xFF)    >> ColorBD;
	int G=(BGR&0xFF00)  >> ColorGD;
	int R=(BGR&0xFF0000)>> ColorRD;	//flags are omitted!
	int I, m, M, S, s;

	if (DivideByBGR)
	{
		B/=(DivideByBGR&0xFF)        ;	//maybe add rounding later
		G/=(DivideByBGR&0xFF00)  >>8 ;
		R/=(DivideByBGR&0xFF0000)>>16;
	}

	if (ScrBPP>8)
	{
		I = B<<ColorBS | G<<ColorGS | R<<ColorRS;
		if (I==Exclude) I^=1<<ColorGS;
		return I;
	}
//if (ScrBPP>4) return B<<ColorBS | G<<ColorGS | R<<ColorRS;
	if (ScrBPP>4)
	{
		R=max(0,min(255,R+RestR));
		G=max(0,min(255,G+RestG));
		B=max(0,min(255,B+RestB));

		for (I=PalRed[R], M=0x7FFFFFFF; I<256; I++)
		{
			if (I==Exclude) continue;
			s=Palette[I].peRed-R;
			S=s*s;
//if (S>M) cout<<R<<":"<<(int)(PalRed[R])<<"-"<<I<<" ";
			if (S>M) break;	//for pre-sorted palettes
//			if (S>M) continue;
			s=Palette[I].peGreen-G;
			S+=s*s;
			if (S>M) continue;
			s=Palette[I].peBlue-B;
			S+=s*s;
			if (S>M) continue;
			M = S;
			m = I;
		}
		for (I=PalRed[R]-1            ; I>=0 ; I--)
		{
			if (I==Exclude) continue;
			s=Palette[I].peRed-R;
			S=s*s;
//if (S>M) cout<<I<<" ";
			if (S>M) break;	//for pre-sorted palettes
//			if (S>M) continue;
			s=Palette[I].peGreen-G;
			S+=s*s;
			if (S>M) continue;
			s=Palette[I].peBlue-B;
			S+=s*s;
			if (S>M) continue;
			M = S;
			m = I;
		}

		RestR=R-Palette[m].peRed;
		RestG=G-Palette[m].peGreen;
		RestB=B-Palette[m].peBlue;
	} else {
		R=max(0,min(255,R+RestR));
		G=max(0,min(255,G+RestG));
		B=max(0,min(255,B+RestB));

		for (I=0, M=0x7FFFFFFF; I< 16; I++)
		{
			if (I==Exclude) continue;
			s=Palette[I].peRed-R;
			S=s*s;
//if (S>M) cout<<R<<"-"<<I<<" ";
			if (S>M) continue;
			s=Palette[I].peGreen-G;
			S+=s*s;
			if (S>M) continue;
			s=Palette[I].peBlue-B;
			S+=s*s;
			if (S>M) continue;
			M = S;
			m = I;
		}

		RestR=R-Palette[m].peRed;
		RestG=G-Palette[m].peGreen;
		RestB=B-Palette[m].peBlue;
	}
//ToDo: last min err + cur value + rand = dithering (needed for 16)
	return m;
}

int V_RestorePatch (Texture *Me)
{
	fstream Patch;
	HRESULT ddrval;
	int LS, x, y, Pix;
	DDSURFACEDESC ddsd={sizeof( ddsd ),0};
	char RGBbmphead[0x36], *Dst, BGR[3]={0};	//High is always 0
	unsigned int TargetColorCode = 0;	//not used or used for palette modes

	if (Me->ColorKeyBGRF&TRANSPARENT_COLOR && ScrBPP>8) TargetColorCode = BGRtoCard(Me->ColorKeyBGRF&0xFFFFFF, -1, 0);

	if (Me->Surface->IsLost()) Me->Surface->Restore();
	else return 0;

cout<<" (was lost) ";

	if (!Me->FileName) return 0;

cout<<" (has patch "<<Me->FileName<<") ";

	ddrval = Me->Surface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY, NULL);
	if(ddrval != DD_OK) return ddrval;
//	if (Me->MirrorFactor) Dst = (char*) ddsd.lpSurface;
	/*else*/ Dst = (char*) ddsd.lpSurface + ddsd.lPitch*(Me->Height-1);

	Patch.open (Me->FileName,ios::in|ios::binary);
	if (Patch.fail()) return -1;
	Patch.read (RGBbmphead,0x36);
	LS=(Me->Width*3+3)&0xFFFFFFFC;

	ResetDithering();

	for (y=0;y<Me->Height;y++)
	{
		Patch.seekg(*(int*)(RGBbmphead+0x0A) + LS*y);
		for (x=0;x<Me->Width;x++)
		{
			Pix=0;
			Patch.read((char*)&Pix,3);

			if (Me->ColorKeyBGRF&TRANSPARENT_COLOR)
			{
				if (Pix==(Me->ColorKeyBGRF&0xFFFFFF)) Pix = TargetColorCode;
				else Pix = BGRtoCard(Pix, TargetColorCode, 0);
			} else       Pix = BGRtoCard(Pix,              -1, 0);

			switch (ScrBPP)
			{
				case 32: ((int*)Dst)[x]=Pix; break;
				case 24: memcpy (Dst, &Pix, 3); break;
				case 16: ((short*)Dst)[x]=Pix; break;
				case  8: Dst[x]=Pix; break;
			}
		}

/*		if (Me->MirrorFactor) Dst += ddsd.lPitch; 
		else*/ Dst -= ddsd.lPitch;
	}
	Patch.close();

	ddrval = Me->Surface->Unlock(NULL);
	return ddrval;
}

int V_TextureInit (Texture *Me, char *PatchName, int ReadAccess, unsigned int SourceCKeyBGRF, unsigned int MirrorFactor)	//Due to game genre, we don't need fast loading. Space optimisation is preferred in this function.
{
	fstream Patch;
	char RGBbmphead[0x36], *Dst;
	int LS, x, y, Pix;
	DDSURFACEDESC ddsd={sizeof( ddsd ),0};
	HRESULT ddrval;
	unsigned int TargetColorCode = 0;	//not used or used for palette modes

	if (SourceCKeyBGRF&TRANSPARENT_COLOR && ScrBPP>8) TargetColorCode = BGRtoCard(SourceCKeyBGRF&0xFFFFFF, -1, MirrorFactor);

	Me->IsVRAM= 0;	//1; //Не время для оптимизации скорости -- тем более такой, что может потребовать подгружать с диска текстуры.

	if (PatchName)
	{
		Patch.open (PatchName,ios::in|ios::binary);
		if (Patch.fail()) return -1;
		Patch.read (RGBbmphead,0x36);

		Me->Width=*(int*)(RGBbmphead+0x12);
		LS=(Me->Width*3+3)&0xFFFFFFFC;
		Me->Height=*(int*)(RGBbmphead+0x16);
	}

						//Remember some ancient video cards can't create vram surfaces bigger than screen res.
	if (ReadAccess || V_Create_WO_Surface (Me->Width,Me->Height,&(Me->Surface)) ) //<<-- order does matter!
		Me->IsVRAM=0;

	if (!Me->IsVRAM && V_Create_R_W_Surface (Me->Width,Me->Height,&(Me->Surface)) )//<<-- order does matter!
		return -1;

/*DDSCAPS WhatWeJustDid;		//Looks like DDraw don't create anything rather than create sysram texture if it's requested (and failed) to create a VRAM one;
Me->Surface->GetCaps(&WhatWeJustDid);
if (Me->IsVRAM && WhatWeJustDid.dwCaps & DDSCAPS_SYSTEMMEMORY) Me->IsVRAM=-1;	//Debug failure marker
if (!Me->IsVRAM && WhatWeJustDid.dwCaps & DDSCAPS_SYSTEMMEMORY) Me->IsVRAM=-2;	//Debug OK marker*/

//for (x=0;x<256;x++) cout<<(int)Palette[x].peRed<<" "<<(int)Palette[x].peGreen<<" "<<(int)Palette[x].peBlue<<endl;

	ddrval = Me->Surface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY, NULL);
	if(ddrval != DD_OK) return ddrval;
	if (MirrorFactor) Dst = (char*) ddsd.lpSurface;
	else              Dst = (char*) ddsd.lpSurface + ddsd.lPitch*(Me->Height-1);

/*cout<<ddsd.lPitch<<" "<<Me->Height<<endl;
cout<<ddsd.ddpfPixelFormat.dwRGBBitCount<<" "
<<ddsd.ddpfPixelFormat.dwRBitMask<<" "
<<ddsd.ddpfPixelFormat.dwGBitMask<<" "
<<ddsd.ddpfPixelFormat.dwBBitMask<<endl;
return 1;*/

	ResetDithering();

//printf ("All %x change to %i which are %x\r\n", CKeyBGRF, ActualFillCode, *((int*)(Palette+ActualFillCode)) );

	if (PatchName)
	{
		for (y=0;y<Me->Height;y++)
		{
			Patch.seekg(*(int*)(RGBbmphead+0x0A) + LS*y);
			if (Patch.fail())
			{
				memset (Dst,0,Me->Width*4);	//Header-only patch;
				PatchName = NULL;	//Invalidate PatchName for future use;
			} else {
				for (x=0;x<Me->Width;x++)
				{
					Pix=0;
					Patch.read((char*)&Pix,3);

					if (SourceCKeyBGRF&TRANSPARENT_COLOR)
					{
						if (Pix==(SourceCKeyBGRF&0xFFFFFF)) Pix = TargetColorCode;
						else Pix = BGRtoCard(Pix, TargetColorCode, MirrorFactor);
					} else       Pix = BGRtoCard(Pix, -1             , MirrorFactor);

					switch (ScrBPP)
					{
						case 32: ((int*)Dst)[x]=Pix; break;
						case 24: memcpy (Dst, &Pix, 3); break;
						case 16: ((short*)Dst)[x]=Pix; break;
						case  8: Dst[x]=Pix; break;
					}
				}
			}

			if (MirrorFactor) Dst += ddsd.lPitch;
			else              Dst -= ddsd.lPitch;
		}
		Patch.close();
	} else {
		for (y=0;y<Me->Height;y++)
		{
			memset (Dst,0,Me->Width*ScrBPP/8);	//Software-generated texture;
			if (MirrorFactor) Dst += ddsd.lPitch;
			else              Dst -= ddsd.lPitch;
		}
	}

	ddrval = Me->Surface->Unlock(NULL);
	if (ddrval) return ddrval;

/*	if (Me->IsVRAM)
	{
		if (NumFragile == 1024) return -1;
		Fragile[NumFragile++] = Me;
		if (PatchName)
		{
			Me->PatchName = (char*) realloc (Me->PatchName, strlen (PatchName)+1);
			strcpy (Me->PatchName, PatchName);
		} else {
			free (Me->PatchName);
			Me->PatchName = NULL;
		}
	}*/

	Me->ColorKeyBGRF=SourceCKeyBGRF;
//	Me->MirrorFactor=MirrorFactor;

	if (SourceCKeyBGRF&TRANSPARENT_COLOR)
	{
		DDCOLORKEY ddck;
		ddck.dwColorSpaceLowValue = TargetColorCode;
		ddck.dwColorSpaceHighValue = TargetColorCode;
		return Me->Surface->SetColorKey( DDCKEY_SRCBLT, &ddck );
	}

	return 0;
}

/*BOOL TextureFree(Texture *Me)
{
	cout<<"IS VRAM? => "<<Me->IsVRAM<<endl;

	return TRUE;
}*/

int G_Init(HWND hWnd)
{
	int i,j;
	int NativeRatioPromille;	//Assuming user is not dumb and has default display ratio = native;
	BOOL IsVertical;	//I prefer to have just one flag instead of " A = A || A = 1/A " comparsions.

//	Texture Sky;	//Infinite distance, never scrolls.
//	Texture LandscapeObjects;	//Fewest scroll ratio, objects near horizon. The whole set.
//	Texture FarWind;	//Generative wind-indicating particle area.
/*	struct Background
	{
		Texture WindParticles;	//Snowflake, leaf, rain drop etc. The whole sprite set of the current particle type.
		Texture ObjectLayer;	//Background "map".
		Texture NPCs;		//Some activity on background layers. All NPCs have one sprite set for now.
	};
	Background *Backgrounds;	//Probably will change to fixed size later.*/

	cout<<"DX_Init: "<<DX_Init(&NativeRatioPromille, &IsVertical, INITVIDEO|INITINPUT, hWnd)<<endl;			//Engage DirectDraw;
	cout<<"Modes: "<<NModes<<", alloc "<<Modes<<endl;

	if (IsVertical) cout<<"Desktop is PORTRAIT"; else cout<<"Desktop is LANDSCAPE"; cout<<" with "<<(float)NativeRatioPromille/1000.0<<" aspect ratio "<<endl;
	for (i=0;i<NModes;i++)
	{
		cout<<Modes[i].Width<<" X "<<Modes[i].Height;
		if (Modes[i].HorzSet & PAL16) cout<<" vga16;";
		if (Modes[i].HorzSet & PAL256) cout<<" vga256;";
		if (Modes[i].HorzSet & RGB16) cout<<" RGB16;";
		if (Modes[i].HorzSet & RGB24) cout<<" RGB24;";
		if (Modes[i].HorzSet & RGB32) cout<<" RGB32;";
		cout<<" Vertical ";
		if (Modes[i].VertSet & PAL16) cout<<" vga16;";
		if (Modes[i].VertSet & PAL256) cout<<" vga256;";
		if (Modes[i].VertSet & RGB16) cout<<" RGB16;";
		if (Modes[i].VertSet & RGB24) cout<<" RGB24;";
		if (Modes[i].VertSet & RGB32) cout<<" RGB32;";
		cout<<endl;
	}
//	cout<<V_FadeOut_Everything(300)<<endl;	//Fading the desktop itself;

	ScrBPP=32;

	if (ScrBPP == 8)	//Palette depends on our decisions. We do it before creating surfaces and setting mode.
	{
		//Loading a pre-made palette for exact level style;
		fstream pal;
		pal.open ("test.pal",ios::in|ios::binary);	//Forced: 0=black, 255=white. For transparent planes, 0 is also transparent (make another black if really needed).
		for (int i=0; i<256; i++) pal.read (&(Palette[i].peRed),3);
		pal.close();

		for (int i=0; i<255; i++)	//Дебильноватая реализация пузырька, зато кодо-компактная более-менее.
		 for (int j=0; j<255-i; j++)	//Белый можно смело дёргать, если он forced -- всё равно у него максимальные и R, и G, никуда не денется. Как и чёрный всегда 0. Даже плюс есть -- им не обязательно быть 0 и 255, сами по местам разбегутся.
		  if (Palette[j].peRed>Palette[j+1].peRed || Palette[j+1].peRed==Palette[j].peRed&&Palette[j].peGreen>Palette[j+1].peGreen)
		  {
			char R=Palette[j].peRed; char G=Palette[j].peGreen; char B=Palette[j].peBlue;
			Palette[j].peRed=Palette[j+1].peRed; Palette[j].peGreen=Palette[j+1].peGreen; Palette[j].peBlue=Palette[j+1].peBlue;
			Palette[j+1].peRed=R; Palette[j+1].peGreen=G; Palette[j+1].peBlue=B;
		  }

		for (int i=0, m=0; i<256; i++) for (   ; m<=Palette[i].peRed; m++) PalRed[m]=i;

		ColorRD = 16;	//Assuming reading BGR .bmp
		ColorGD = 8;
		ColorBD = 0;
/*ColorRD = 21;	//uncomment for 3-3-2 "RGB palette", also make if (ScrBPP>8) return B<<ColorBS | G<<ColorGS | R<<ColorRS; work if ScrBPP>4 and comment bubble sort
ColorGD = 13;
ColorBD = 6;

ColorRS = 5;
ColorGS = 2;
ColorBS = 0;*/
	}
	if (ScrBPP == 4)	//Это вроде не так делается, а через вторичную палитру, которая с флагом 8BITENTRIES, и ссылается на нормальную.
	{
		//Loading a pre-made palette for exact level style;
		fstream pal;
		pal.open ("test_16.pal",ios::in|ios::binary);
		for (int i=0; i<16; i++) pal.read (&(Palette[i].peRed),3);	//Forced: 0=black.  For transparent planes, 0 is also transparent.
		pal.close();

		ColorRD = 16;	//Assuming reading BGR .bmp
		ColorGD = 8;
		ColorBD = 0;
	}

	cout<<V_SetMode(ScrW=RequestedW,ScrH=RequestedH,ScrBPP)<<endl;
//	cout<<V_SetMode(ScrW=1024,ScrH=768,ScrBPP)<<endl;
//	cout<<V_SetMode(ScrW=PANEL_W,ScrH=PANEL_H,ScrBPP)<<endl;

	if (ScrBPP > 8)	//RGB depends on video card, we do it after setting the proper mode.
	{
		DDPIXELFORMAT format = { sizeof(format) };
		if ((i=S_Back->GetPixelFormat(&format))!=DD_OK) return i;
		ColorRS = format.dwRBitMask;
		ColorGS = format.dwGBitMask;
		ColorBS = format.dwBBitMask;
/*cout<<"Masked by "<<ColorRS<<", "<<ColorGS<<", "<<ColorBS<<endl;
ColorRD=23;
ColorGD=15;
ColorBD=7;
ColorRS=16;
ColorGS=8;
ColorBS=0;*/

		for (i=31; i>=0; i--) if (ColorRS & (1<<i)) break;
		ColorRD = i;
		for (i=31; i>=0; i--) if (ColorGS & (1<<i)) break;
		ColorGD = i;
		for (i=31; i>=0; i--) if (ColorBS & (1<<i)) break;
		ColorBD = i;
		for (i=0; i<32; i++) if (ColorRS & (1<<i)) break;
		ColorRS=i;
		for (i=0; i<32; i++) if (ColorGS & (1<<i)) break;
		ColorGS=i;
		for (i=0; i<32; i++) if (ColorBS & (1<<i)) break;
		ColorBS=i;

/*		__asm
		{
			BSR eax, ColorRS;	//Что-то нечисто - 386й 386м, а на S200 (PIII) в этом месте прога тихо умирает. Хотя она старой ваткой собрана, новой - не умирала вроде.
			mov ColorRD, eax;	//Лол, а она и после сборки новой ваткой стала помирать!
			BSR eax, ColorGS;
			mov ColorGD, eax;
			BSR eax, ColorBS;
			mov ColorBD, eax;

			BSF eax, ColorRS;	//check the first CPU generation!
			mov ColorRS, eax;	//OK, it's 386+ generation :)
			BSF eax, ColorGS;
			mov ColorGS, eax;
			BSF eax, ColorBS;
			mov ColorBS, eax;
		}*/
//		ColorRM = format.dwRBitMask>>ColorRS;
//		ColorGM = format.dwGBitMask>>ColorGS;
//		ColorBM = format.dwBBitMask>>ColorBS;
		ColorRD = 23 - ColorRD + ColorRS;	//Assuming reading BGR .bmp
		ColorGD = 15 - ColorGD + ColorGS;
		ColorBD = 7  - ColorBD + ColorBS;
//		cout<<ColorRM<<", "<<ColorGM<<", "<<ColorBM<<" shifted by "<<ColorRS<<", "<<ColorGS<<", "<<ColorBS<<endl;
		cout<<ColorRD<<", "<<ColorGD<<", "<<ColorBD<<" shifted by "<<ColorRS<<", "<<ColorGS<<", "<<ColorBS<<endl;
	}


	for (i=0; i<TOTAL_TEXTURES; i++)
	{
//		cout<<"Init texture "<<Textures[i].FileName<<":  "<<V_TextureInit (Textures+i, Textures[i].FileName, 0, 0xFFFFFF|TRANSPARENT_COLOR, 0 );
cout<<"Init texture "<<Textures[i].FileName<<":  "<<V_TextureInit (Textures+i, Textures[i].FileName, 0, 0x000000|TRANSPARENT_COLOR, 0 );
		cout<<" Size "<< Textures[i].Width<<" X "<< Textures[i].Height<<endl;
	}

/*	if (!Sky.IsVRAM || !Bgnd.IsVRAM || !Map.IsVRAM )
	{
		cout<<"Can't create a surface bigger than the screen! Oldfag mode engaged."<<endl;
		OldFagSizeLimit=1;
	}*/

/*	if (OldFagSizeLimit)
	{
		F_Map.Width = ScrW; F_Map.Height = ScrH;
		F_Sky.Width = ScrW; F_Sky.Height = ScrH; 
		F_Bgnd.Width = ScrW; F_Bgnd.Height = ScrH; 
	} else {
		F_Map .Width = Pre_Map .Width; F_Map .Height = Pre_Map .Height; 
		F_Sky .Width = Pre_Sky .Width; F_Sky .Height = Pre_Sky .Height; 
		F_Bgnd.Width = Pre_Bgnd.Width; F_Bgnd.Height = Pre_Bgnd.Height; 
	}
*/
/*	cout<<"Init Fmap: " <<V_TextureInit (&F_Map , NULL, 0, 0xFFFFFF|TRANSPARENT_COLOR, 0x040402 ); cout<<" Size "<< F_Map.Width<<" X "<< F_Map.Height<<endl;
	cout<<"Init Fsky: " <<V_TextureInit (&F_Sky , NULL, 0, 0                         , 0x040402 ); cout<<" Size "<< F_Sky.Width<<" X "<< F_Sky.Height<<endl;
	cout<<"Init FBgnd: "<<V_TextureInit (&F_Bgnd, NULL, 0, 0xFFFFFF|TRANSPARENT_COLOR, 0x040402 ); cout<<" Size "<<F_Bgnd.Width<<" X "<<F_Bgnd.Height<<endl;
*/

	cout<<"draw-1: "<<V_RedrawAll()<<endl;
//ShowError();

	return 0;
}

void V_PrintDigits (int Value, int Digits, int FirstControl, int FontOffset, int MinDigits)
{
	int i, d;
	for (i=0; i<Digits; i++)
	{
		d=Value%10;

		Controls[FirstControl+Digits-1-i].CurrentTexture = Textures + FontOffset + d;
		if (i>=MinDigits && !Value) Controls[FirstControl+Digits-1-i].WipeX = Controls[FirstControl+Digits-1-i].CurrentTexture->Width;
		else Controls[FirstControl+Digits-1-i].WipeX = Controls[FirstControl+Digits-1-i].WipeY = 0;

		Value/=10;
	}
	if (Value) for (i=0; i<Digits; i++) Controls[FirstControl+Digits-1-i].WipeY = Controls[FirstControl+Digits-1-i].CurrentTexture->Width/2;
}

int V_PropagateColoredGraph (float *LoopedGraph, int LoopedGraphIndex, int LoopedGraphSize, Texture *Texture)
{
	DDSURFACEDESC ddsd={sizeof( ddsd ),0};
	HRESULT ddrval;
	static float Snapshot[16384], Max;
	int Width = Texture->Width - 1;
	int Height = Texture->Height;
	int i, *Row, ColumnColor;
	char *Ptr;
	unsigned int Transparent = BGRtoCard(Texture->ColorKeyBGRF&0xFFFFFF, -1, 0);
	unsigned int Edge;

	i=LoopedGraphIndex - (Width-1);
	memcpy (Snapshot+max(0,-i), LoopedGraph+max(0,i), sizeof(float)*(Width+min(0,i)));
	if (i<0) memcpy (Snapshot, LoopedGraph+LoopedGraphSize+i, sizeof(float)*(-i));

	for (Max=.00000000000001,i=0; i<Width; i++) if (Max<Snapshot[i]) Max=Snapshot[i];
	for (i=0; i<Width; i++) Snapshot[i] = (Height-1) * (Snapshot[i]/Max);

	ddrval = Texture->Surface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if(ddrval != DD_OK) return ddrval;
	Ptr = (char*) ddsd.lpSurface;

	for (int j=0; j<Height; j++)
	{
		Row = (int*)(Ptr + j*ddsd.lPitch);
		if (!j) Edge = Row[0]; 
		ColumnColor = Row[0];
		for (i=0; i<Width; i++)
		{
			if (Height-j-1 > Snapshot[i]) Row[1+i] = Transparent;
				else if (Snapshot[i] - (Height-j-1) < 3) Row[1+i] = Edge;
				else Row[1+i] = ColumnColor;
		}
	}

	ddrval = Texture->Surface->Unlock(NULL);
	return ddrval;
}

//*
//char *Map=NULL;
struct EMPixel
{
	unsigned short X, Y;
	unsigned long AnimNum;
} *PixelList;
int PixelCount=0;
int *EMFGr=NULL;
int FirstColor = 0;
float LastSpeed = 0;
int DelayCount = 0;
int V_DrawEMField (float Speed, Texture *Texture)
{
	DDSURFACEDESC ddsd={sizeof( ddsd ),0};
	HRESULT ddrval;
	int Width = Texture->Width;
	int Height = Texture->Height-1;
	unsigned int Transparent = BGRtoCard(Texture->ColorKeyBGRF&0xFFFFFF, -1, 0);
	int i, *Row, ColumnColor, CurrColor;
	char *Ptr;

	if (Speed > 1.0) Speed = 1.0;
	if (Speed) Speed = max (Speed, .04);
	if ((Speed > LastSpeed * 1.5 || Speed < LastSpeed / 1.5) && DelayCount < 10)
	{
		Speed=LastSpeed;
		DelayCount++;
	} else {
		LastSpeed=Speed;
		DelayCount=0;
	}

	Speed *=0x00080000;
	FirstColor = ((int)(FirstColor+Speed)) & 0xFFFFFF;

	ddrval = Texture->Surface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if(ddrval != DD_OK) return ddrval;
	Ptr = (char*) ddsd.lpSurface;

	if (!EMFGr)
	{
//		Map=(char*)malloc(Width*Height*3);
		EMFGr =(int*)malloc(Width*sizeof(int));

		memcpy (EMFGr, Ptr, Width*sizeof(int));
		for (int j=0; j<Height; j++)
		{
			Row = (int*)(Ptr + (j+1)*ddsd.lPitch);
			for (i=0; i<Width; i++)
			{			//RGB as a whole 24-bit integer
//				Map[j*Width*3+i*3  ]=(Row[i]>>ColorBS)&0xFF;
//				Map[j*Width*3+i*3+1]=(Row[i]>>ColorGS)&0xFF;
//				Map[j*Width*3+i*3+2]=(Row[i]>>ColorRS)&0xFF;
				if (Row[i]) PixelCount++;
			}
		}

		PixelList = (EMPixel*) malloc (sizeof(EMPixel)*PixelCount);

		PixelCount=0;
		for (int j=0; j<Height; j++)
		{
			Row = (int*)(Ptr + (j+1)*ddsd.lPitch);
			for (i=0; i<Width; i++)
			{			//RGB as a whole 24-bit integer
				if (!Row[i]) continue;
				PixelList[PixelCount].X = i;
				PixelList[PixelCount].Y = j;
				PixelList[PixelCount].AnimNum = (Row[i]>>ColorRS)&0xFF;
				PixelList[PixelCount].AnimNum *= 256;
				PixelList[PixelCount].AnimNum += (Row[i]>>ColorGS)&0xFF;
				PixelList[PixelCount].AnimNum *= 256;
				PixelList[PixelCount].AnimNum += (Row[i]>>ColorBS)&0xFF;
				PixelCount++;
			}
		}
		cout<<"Pixels in EM anim.:"<<PixelCount<<endl;
	}

//	for (int j=0; j<Height; j++)
	for (int j=0,k=0; k<PixelCount; k++)
	{
		j=PixelList[k].Y;
		Row = (int*)(Ptr + (j+1)*ddsd.lPitch);
//		for (i=0; i<Width; i++)
		i=PixelList[k].X;
		{
			CurrColor = PixelList[k].AnimNum;//Map[j*Width*3+i*3]+Map[j*Width*3+i*3+1]*256+Map[j*Width*3+i*3+2]*256*256;
//			if (!CurrColor) continue;
//CurrColor &= 0xFF0000;
			CurrColor    += 0x01000000 - FirstColor;
			CurrColor    %= 0x00200000;
			if (CurrColor < Speed*3)
			{
				CurrColor *= Width-1;
				CurrColor /= (Speed*3);
				CurrColor = EMFGr[CurrColor];
			} else CurrColor=Transparent;
//CurrColor >>= 16;
			Row[i]=CurrColor;
		}
	}

	ddrval = Texture->Surface->Unlock(NULL);
	return ddrval;
}

/*/float Alpha = 0;
int V_DrawEMField (float Speed, Texture *Texture)
{
	DDSURFACEDESC ddsd={sizeof( ddsd ),0};
	HRESULT ddrval;
	int Width = Texture->Width;
	int Height = Texture->Height;
	int i, *Row, ColumnColor;
	char *Ptr;
	unsigned int Transparent = BGRtoCard(Texture->ColorKeyBGRF&0xFFFFFF, -1, 0);

	if (Speed > 1.0) Speed = 1.0;
	Alpha += Speed;

	ddrval = Texture->Surface->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	if(ddrval != DD_OK) return ddrval;
	Ptr = (char*) ddsd.lpSurface;

	for (int j=0; j<Height; j++)	//Нарочно написано предельно тупо, чтобы тормозило примерно как будущая демосценка (та будет сильно посложнее).
	{
		Row = (int*)(Ptr + j*ddsd.lPitch);
		for (i=0; i<Width; i++)
		{
			Row[i] = Transparent;
		}
	}

	for (float Beta = Alpha; Beta < Alpha+.3; Beta+=0.001)
	{
		int X=cos(Beta)*Width/3 + Width/2;
		int Y=sin(Beta)*Width/3 + Height/2;

		Row = (int*)(Ptr + Y*ddsd.lPitch);
		Row[X] = 0x7F7F7F7F;
	}

	ddrval = Texture->Surface->Unlock(NULL);
	return ddrval;
}// */

int Active=0;
unsigned long LastT;

int DX_Restore()
{
	if (S_Main->IsLost()) S_Main->Restore();
	if (S_Back->IsLost()) S_Back->Restore();

//	for (int i=0; i<NumFragile; i++) cout<<"Restored "<<V_RestorePatch(Fragile[i])<<" Size "<<Fragile[i]->Width<<" X "<<Fragile[i]->Height<<endl;

	V_RedrawAll();
	Active=1;
	LastT = timeGetTime();

	return keyboard->Acquire();
}

static char key[256];
#define SPEED 24
//#define SPEED 100


//int Test=0, NTest=0; float FTest = 10;
int HoldedKey=0;

int G_Reaction ()
{
	int i, j;
	int sx=0, sy=0;

	if (S_Main->IsLost()) DX_Restore();

	if (timeGetTime()-LastT < 16) return 0;		//ToDo (possibly): if we have Estimated Drawing Time < 16/2, we can draw interpolated (smooth motion) frames. But, actually, 60 fps does not require such a thing.
	if (timeGetTime()-LastT > 16384) LastT = timeGetTime();	//Will not happen normally (we have Time Guards on all application reactiveation codes);

	for (;	timeGetTime()-LastT >= 16;	LastT+=16)	//One Gameplay Step ahead per each 16 mS
	{
		i=keyboard->GetDeviceState( sizeof(key), &key );	//If physics, not graph, slow the system down -- "blind control" may help a bit. So we refresh input each COMPUTED, not each DRAWN frame.
		if ( key[DIK_LEFT ] & 0x80 ) sx-=SPEED;
		if ( key[DIK_RIGHT] & 0x80 ) sx+=SPEED;
		if ( key[DIK_UP   ] & 0x80 ) sy-=SPEED;
		if ( key[DIK_DOWN ] & 0x80 ) sy+=SPEED;

if ( key[DIK_R    ] & 0x80 ) V_RedrawAll();

//V_RedrawAll();

	}

	ScrollX += sx;
	if (ScrollX < 0) ScrollX = 0;
	if (ScrollX > PANEL_W - ScrW) ScrollX = PANEL_W - ScrW;
	ScrollY += sy;
	if (ScrollY < 0) ScrollY = 0;
	if (ScrollY > PANEL_H - ScrH) ScrollY = PANEL_H - ScrH;

/*if (Test) 
{
LoopedGraph[LoopedGraphIndex=(LoopedGraphIndex+1)%4096]=FTest;
DXData.WattsIn=DXData.WattsOut=NTest*20;
DXData.Amperage=(NTest%100)/10.0;
DXData.EfficiencyRatio=DXData.LevelPercentage = ((float)(NTest%101))/100.0;
DXData.TT = DXData.TR = NTest/10;
DXData.Status=(NTest/25)%10;
NTest++;
FTest += (rand()-RAND_MAX/2)/(RAND_MAX/2);
if (FTest<0) FTest+=5;
if (FTest>30) FTest-=5;
}*/

	if (!DXData.IsConnected)
		DXData.WattsIn=DXData.WattsOut=DXData.LevelPercentage=DXData.EfficiencyRatio=DXData.Amperage=DXData.TT=DXData.TR=DXData.BiggestAmperage=DXData.Status=0;

	V_PrintDigits (DXData.WattsIn / 100, 2, WIN_UPPER_DIGIT, MEDIUM_DIGIT_0, 2);
	V_PrintDigits (DXData.WattsOut / 100, 2, WOUT_UPPER_DIGIT, MEDIUM_DIGIT_0, 2);
	V_PrintDigits (min( 99,   DXData.EfficiencyRatio * 100 ), 2, RATIO_UPPER_DIGIT, BIG_DIGIT_0, 2);
	V_PrintDigits (min(100, i=DXData.LevelPercentage * 100 ), 3, LEVEL_100_PERCENT, MEDIUM_DIGIT_0, 1);
	Controls[LEVEL_STATUS].WipeY = max(0, Controls[LEVEL_STATUS].CurrentTexture->Height - 2 - 93 * ((i+10)/20) );
	if (DXData.BiggestAmperage == 0 || i>1000 || DXData.Status == 0 || DXData.IsConnected == 0)
	{
		Controls[LEVEL_STATUS].WipeY = Controls[LEVEL_STATUS].CurrentTexture->Height;
		Controls[LEVEL_100_PERCENT].CurrentTexture =
		Controls[LEVEL_UPPER_DIGIT].CurrentTexture =
		Controls[LEVEL_LOWER_DIGIT].CurrentTexture = Textures + MEDIUM_DIGIT_BLANK;
	}
	//Текстура катушек должна идти в стэке слоёв после текстуры фона катушек, а текстура фона катушек -- после всего, имеющего отношение к батарейке (шрифты, сама батарейка...)
	V_PrintDigits (DXData.Amperage*10, 2, AMP_UPPER_DIGIT, MEDIUM_DIGIT_0, 2);
	V_PrintDigits (DXData.TT, 3, TT_UPPER_DIGIT, TINY_DIGIT_0, 1);
	V_PrintDigits (DXData.TR, 3, TR_UPPER_DIGIT, TINY_DIGIT_0, 1);

	if (HoldedKey == POWER_STATUS)
	{
		Controls[POWER_STATUS].PosX=PStDefX+2;
		Controls[POWER_STATUS].PosY=PStDefY+2;
//DXData.Amperage=0;
	}
	else if (DXReq.On)
	{
		Controls[POWER_STATUS].PosX=PStDefX;
		Controls[POWER_STATUS].PosY=PStDefY;
//DXData.Amperage += .01;
//DXData.BiggestAmperage=8;
	} else {
		Controls[POWER_STATUS].PosX=PStDefX-5;
		Controls[POWER_STATUS].PosY=PStDefY-5;
	}
	V_PrintDigits (min(DXData.Status,6), 1, POWER_STATUS, POWER_STATUS_INACTIVE, 1);
	V_PrintDigits (DXReq.Connect+DXData.IsConnected, 1, CONN_STATUS, CONN_STATUS_OFF, 1);

	V_PropagateColoredGraph ((float*)LoopedGraph, LoopedGraphIndex, 4096, Controls[AMPERAGE_GRAPH].CurrentTexture);

	if (DXData.BatteryMode)
	{
		Controls[COIL_BGND].WipeY = Controls[COIL_BGND].CurrentTexture->Height;
		Controls[COIL_EM].WipeY = Controls[COIL_EM].CurrentTexture->Height;
	} else {
		Controls[COIL_BGND].WipeY = 0;
		Controls[COIL_EM].WipeY = 1;
		V_DrawEMField (DXData.Amperage/(DXData.BiggestAmperage + 1E-50), Controls[COIL_EM].CurrentTexture);
	}

	V_RedrawAll();

	if ( key[DIK_ESCAPE] & 0x80 ) return 1;

	return 0;
}

extern volatile HWND DXhWnd;

int C_ProcessControl (int NControl)
{
	HoldedKey = NControl;
	switch (NControl)
	{
		case CONN_STATUS:
			DXReq.Connect = !(DXReq.Connect);	//Передать фреймворку команду на подключение
		break;
		case POWER_STATUS:
//			if (HoldedKey == POWER_STATUS) break;
			DXReq.On = !(DXReq.On);	//Передать главным тредом по следующему запросу.
		break;
		case SETTINGS:
			PostMessage ((HWND)DXhWnd, WM_CLOSE, 0, 0);	//Закрыть только окно красивого интерфейса.
		break;
		case CLOSEALL:
			DXReq.Close = 1;	//Передать фреймворку команду на полное закрытие всего.
		break;
	}

	return 0;
}

int C_GetClickedControl (int X, int Y)
{
	int i;
	for (i=TOTAL_CONTROLS-1; i>BACKGROUND; i--)
	{
		if (Controls[i].PosX > X) continue;
		if (Controls[i].PosY > Y) continue;
		if (Controls[i].PosX + Controls[i].CurrentTexture->Width  < X) continue;
		if (Controls[i].PosY + Controls[i].CurrentTexture->Height < Y) continue;
		return i;
	}
	return i;
}

LRESULT CALLBACK WindowFunc(HWND hWnd,UINT message, WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
		case WM_LBUTTONDOWN:
//cout<<LOWORD(lParam)<<" "<<HIWORD(lParam)<<endl;
			C_ProcessControl( C_GetClickedControl ( LOWORD(lParam) + ScrollX, HIWORD(lParam) + ScrollY ) );
		break;
		case WM_LBUTTONUP:
			HoldedKey = 0;
		break;
		case WM_TIMER:
			KillTimer (hWnd, 1);
//SetFocus(hWnd);
//cout<<"Re-focus"<<endl;
			DX_Restore();
		break;
		case WM_CREATE:
//SetFocus(hWnd);
//Sleep(3000);
			if (G_Init(hWnd)) {DX_DeInit(); cin>>WaterLoop; PostQuitMessage (0);}
			LastT = timeGetTime();
			Active=1;
		break;
		case WM_COMMAND:
		break;
		case WM_KILLFOCUS:
//cout<<"unfocus!"<<endl;
			KillTimer (hWnd, 1);
			Active=0;
		return DefWindowProc (hWnd,message,wParam,lParam);
	//	break;
		case WM_SETFOCUS:
//cout<<"focus!"<<endl;
			if (!Active) SetTimer (hWnd, 1, 1000, NULL);
		return DefWindowProc (hWnd,message,wParam,lParam);
	//	break;
		case WM_CLOSE:
			Active=0;
			DX_DeInit();
		return DefWindowProc (hWnd,message,wParam,lParam);
	//	break;
		case WM_DESTROY:
			PostQuitMessage (0);
		break;		

		default:
			return DefWindowProc (hWnd,message,wParam,lParam);
	}

	return 0;
}

HWND BlackBgndWin = NULL;
HBRUSH BlackBrush = NULL;
WNDCLASS BlackBgndClass = {0};
const char ClassName[] = "BlackDecoration1x1";

//int WINAPI WinMain (HINSTANCE hThisInst,HINSTANCE hPrevInst, LPSTR lpszArgs,int nWinMode)
int DirectXEngine(HINSTANCE hThisInst)
{	
	MSG msg;

	BlackBgndClass.hInstance = hThisInst;
	BlackBgndClass.lpszClassName = ClassName;
	BlackBgndClass.lpfnWndProc = WindowFunc;
	BlackBgndClass.style = 0;//WS_BORDER|WS_THICKFRAME;
//	BlackBgndClass.hIcon = LoadIcon (NULL,IDI_APPLICATION);
	BlackBgndClass.hIcon = LoadIcon (hThisInst,"ContrlPnIcon");
//cout<<"?"<<	BlackBgndClass.hIcon<<endl;
	BlackBgndClass.hCursor = LoadCursor (NULL, IDC_ARROW);
	BlackBgndClass.lpszMenuName = NULL;
	BlackBgndClass.cbClsExtra = 0;
	BlackBgndClass.cbWndExtra = 0;
	BlackBgndClass.hbrBackground= BlackBrush =CreateSolidBrush(0x000000);
	if ( !RegisterClass (&BlackBgndClass) ) return (int)(DXhWnd=NULL);

	DXhWnd = CreateWindowEx (WS_EX_TOPMOST|/*WS_EX_TOOLWINDOW|*/WS_EX_WINDOWEDGE,ClassName,"BlackBackground",0,0,0,1,1,HWND_DESKTOP,NULL,hThisInst,NULL);
	if (!DXhWnd) return 0;

//MoveWindow(DXhWnd,0,0,RequestedW,RequestedH,true);

	int WnHnd;
	WnHnd = GetWindowLong(DXhWnd, GWL_STYLE);
	WnHnd = WnHnd & (~WS_CAPTION);
	SetWindowLong(DXhWnd, GWL_STYLE, WnHnd);
	
//	WnHnd = GetWindowLong(DXhWnd, GWL_EXSTYLE);
//	WnHnd = WnHnd | WS_EX_TOOLWINDOW;
//	SetWindowLong(DXhWnd, GWL_EXSTYLE, WnHnd);

//	MoveWindow(DXhWnd,0,0,20,20,true);

        ShowWindow (DXhWnd,SW_SHOW);
        UpdateWindow (DXhWnd);

	while (1)
	{
		if (Active) if(G_Reaction()) PostMessage ((HWND)DXhWnd, WM_CLOSE, 0, 0);

		if (!PeekMessage (&msg, NULL,0,0,PM_REMOVE)) continue;
		if (msg.message == WM_QUIT) break;
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	UnregisterClass(BlackBgndClass.lpszClassName, BlackBgndClass.hInstance);	//MSDN: bgnd brushes are auto-removed.

	DXhWnd = NULL;
	return msg.wParam;
}
