//Моя обёртка на стандартный мастдайный диалог (тот самый предефайненный оконный класс).
//Диалог -- это самый минималистичный набор, его аналоги есть везде (даже в линуховом перле).
//Поэтому если мне взбредёт портировать это под никсы или какое-нибудь ведро, достаточно будет просто сделать таблицу трансляции оконных сообщений туда и обратно.

#define OPENDIALOG 0
#define MODALDIALOG 1
#define CLOSEDIALOG 2
#define SENDMDIALOG 3
#define OPENFILEROLLER 4
        #define API_OPENDIR 0   //SHBrowseForFolder
        #define API_SAVEFILE 1  //GetSaveFileName
        #define API_LOADFILE 2  //GetOpenFileName
        #define API_LOADFILES 3 //GetOpenFileName OFN_ALLOWMULTISELECT|OFN_EXPLORER
#define CREATECANVAS 5
#define UPDATECANVAS 6
#define REMOVECANVAS 7
#define MESSAGEBOX 8
#define TIMER 9
#define SETFOCUS 10
#define TEXTTODOUBLE 11
#define DOUBLETOTEXT 12
#define ATTACHTOOLTIP 13
#define GETDIALOGDATA_HEADER 14 //Fast filling from .INI file -- colums set
#define GETDIALOGDATA_LINE 15 //Fast filling from .INI file -- line
#define GETSYSTEMSTATE 16
#define SETSYSTEMSTATE 17
        #define API_DEAD 0	//По понятной причине никогда не возвращается GETSYSTEMSTATE. Разговорчивых покойников я на своём веку не припомню. При получении SETSYSTEMSTATE закрывает всю прогу целиком.
        #define API_ALIVE 1	//Жив, но отключен от сервера.
        #define API_CONNECTED 2	//Жив и подключен к серверу.
#define PROGRESSBAR 18	//0 to create progress bar, 100 to finish progress bar, returns TRUE if Abort hadn't been pressed.

#define SEND_MESSAGE 0
#define REGISTER_INCOMING 1
#define SEND_REQUEST 2

typedef BOOL __export WINAPI (WCB)  (long, unsigned long, unsigned long, unsigned long); //первый S32 -- даже не столько параметр, сколько часть самого указателя (WCB* указывает на функцию, а первый параметр -- на её инстанцию, вместе они образуют уникальный id, под другими ОС можно сделать и по-другому).
typedef BOOL __export WINAPI (FWAPI)(      unsigned long, unsigned long, unsigned long);
//typedef BOOL __export __syscall (WCB)(unsigned long, unsigned long, unsigned long);

