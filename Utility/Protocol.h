#pragma pack (push)
#pragma pack (0)

//Младшие 16 бит (категория)
#define CAT_LOGS        0x0000  //Логи (обсуждабельно)
#define CAT_CFG         0x0001  //Перманентная (зашиваемая) конфигурация (обсуждабельно)
#define CAT_MODE        0x0002  //Сиюсекундные настройки (режим работы, обсуждабельно)
#define CAT_SECU        0xFFFF  //Инфобез (логин, пароль...), все или почти все сообщения этой категории отправляются самим фреймом, не плагинами.

//Старшие 16 бит (младшие всегда 0, там категория)
#define MSG_DEBUG       0xFFFF0000      //Временный, для внутрилабораторной отладки. Относится ко всем категориям сразу.

//Logs
#define MSG_RESERVED    0x00000000      //Тип 0 категории 0 -- служебный "нет сообщения"
#define MSG_TEMPER      0x00010000      //Тип 1 категории 0 -- логи температуры, например

//Cfg
#define MSG_MAXLAMPS    0x00000000      //Тип 0 категории 1 -- число ламп, например

//Mode and Cfg
struct MsgDebug //Текущая рабочая структура, которая ещё не устоялась. Содержит немножко CAT_LOGS, хотя это не по феншую, но для дебага можно.
{
        unsigned long   Hdr;    //=MSG_DEBUG | CAT_CFG для зашивки настроек, MSG_DEBUG | CAT_MODE для сеансовых настроек
        unsigned long   Len;    //Сильно варьируется в зависимости от.

        unsigned short AutoSend;        //Специальный пакет "горшочек, не вари!" отправляется с размером 10 и этим параметром = 0.

        unsigned short GenFreq;
        unsigned char GenWForm; 
        unsigned char Sparse;           //Иначе всё выравнивание поедет мимо адресной шины
        unsigned short GenAmplit, GenMeanILim, GenPulseILim, GenVOutLim, GenVInLim, GenSoftStartRamp, GenLoopsSwitch;

        unsigned short ModeFlagILoop: 1;        //Порядок расположения бит в шорте определяется компилятором. Для Ваткома декларирован "правильный" (для наших целей). При портировании обратить внимание и прочесть мануал.
        unsigned short ModeFlagVLoop: 1;
        unsigned short ModeFlagFLoop: 1;
        unsigned short ModeFlagFScan: 1;
        unsigned short ModeFlagPLoop: 1;
        unsigned short ModeFlagRsvd: 11;        //Всегда 0 от контра и всегда 0 от компа (т. е. "не взводить").

        //Вот тут проходит граница между настройками и логами. Если не хотим ничего сбрасывать, отправляем с длиной 30 байт (т. е. по это самое место).
        //Опять же, когда будем разносить по разным плагинам ("набело"), Ctrl+X Ctrl+V будет по этой линии.

        //Переставил флаги повыше в структуре, мы их ведь будем отправлять с нулями и длиной 32, чтобы сбросить. Чтобы не сбрасывать, отправляем с длиной 30 или с длиной 32 и более, но с единицами.
        unsigned short LogFlagOverPI: 1;
        unsigned short LogFlagOverMI: 1;
        unsigned short LogFlagOverVIn: 1;
        unsigned short LogFlagOverVOut: 1;
        unsigned short LogFlagOverHeatBr: 1;
        unsigned short LogFlagRsvd: 10;         //Всегда 0 от контра и всегда 1 от компа (т. е. "не сбрасывать").
        unsigned short LogFlagFault: 1;

        //Дальнейшее, скорее всего, будет только для чтения. У меня только в Faults сомнения (его надо сбрасывать или нет?)
        unsigned short MeasMeanIIn, MeasMeanIOut, MeasVIn, MeasVOut, MeasPIn, MeasPOut;
        unsigned char MeasBr1Temper, MeasBr2Temper;
        unsigned short MeasPhaseDelay, MeasFaults;
        //Итого пока 50 байт. Резервировать что-то смысла не вижу -- всё равно можно всегда добавить в хвост, а структура (когда её "выносим") развалится на логовую и настроечную.
        //Кроме того, всякий "резерв" ставит вопрос, какое значение должно быть, если поле не используется (как отличить от того дня, когда резерв пригодится). С удлинением такой проблемы нет.
        unsigned short Timestamp;       //Вот этот точно read-only.
        //Итого 52 в сторону компа (если речь идёт не о зашитых настройках, в которых логов быть не может), а в сторону контроллера -- по обстоятельствам. Наверное, буду полностью отправлять, чтобы не грузить голову адаптивными размерами.
};

struct MsgDebugLogs     //понемногу выкристаллизовывается
{
        unsigned long   Hdr;    //=MSG_DEBUG | CAT_LOGS
        unsigned long   Len;    //30 от контра к компу, 12 от компа к контру (обычно сбрасывать приходится всё сразу, в т. ч. faults)

        //Oтправлять с нулями, чтобы сбросить. Чтобы не сбрасывать, отправляем с единицами.
        unsigned short LogFlagOverPI: 1;
        unsigned short LogFlagOverMI: 1;
        unsigned short LogFlagOverVIn: 1;
        unsigned short LogFlagOverVOut: 1;
        unsigned short LogFlagOverHeatBr: 1;
        unsigned short LogFlagRsvd: 10;         //Всегда 0 от контра и всегда 1 от компа (т. е. "не сбрасывать").
        unsigned short LogFlagFault: 1;

        unsigned short MeasFaults;      //отправлять 0x0000, чтобы сбросить, и 0xFFFF, чтобы не сбрасывать.
        unsigned short MeasMeanIIn, MeasMeanIOut, MeasVIn, MeasVOut, MeasPIn, MeasPOut;
        unsigned char MeasBr1Temper, MeasBr2Temper;
        unsigned short MeasPhaseDelay;
        unsigned short Timestamp;
};

//Security
#define MSG_LOGIN       0x00000000      //Тип 0 категории FFFF: логин (от ПК к контроллеру).
struct MsgLogin //Передаётся с размером 8 для логаута
{
        unsigned long   Hdr;    //=CAT_SECU|MSG_LOGIN
        unsigned long   Len;    //=4*sizeof(unsigned long), пока не определились с грамотным выяснением ReplyTo через винапи. 

        unsigned long   WhoAreYou;      //MCU IP
        unsigned long   Ports;          //MCU Port in lower word, PC port in upper word (WiP)
//      unsigned long   ReplyTo;        //PC IP (WiP)
};      //В качестве рандомного зерна при вычислении подписи указывается ноль, т. е. SHA-256 подпись считается только по самому пакету и закрытому паролю.

#define MSG_SEED        0x00000001      //Тип 1 категории FFFF: логин (от контроллера к ПК).
struct MsgSeed  //Передаётся с размером 8 для подтверждения логаута
{
        unsigned long   Hdr;    //=CAT_SECU|MSG_LOGIN
        unsigned long   Len;    //=10*sizeof(unsigned long)

        unsigned long   RndSeed[8];     //Рандомное зерно, необходимое для остальных пакетов (т. е. они считаются как SHA-256 от пакета, закрытого пароля и этого зерна).
};


//Ну и так далее, у каждой категории может быть 65536 разных конкретных структур, причём каждая структура может от версии к версии удлиняться, не теряя "совместимости сверху".
//Если указан размер 0 (т. е. размер меньше восьмибайтного заголовка, чего быть не может) -- это "волшебное значение", которое означает, что структуру мы спрашиваем, а не устанавливаем.
//Но в ответ и на запрос, и на установку структуры возвращается ответ с содержащимися данными, что позволяет управляющему ПО сравить их и сделать выводы (например, структура может быть короче, если контр поддерживает только прошлую версию).
//После структуры идёт SHA-256 подпись из восьми U32, которая не является частью структуры (т. е. не отображена в её длине).
//Длина U32 на всякий случай (мало ли, переведём фрейм на TCP или какой-нибудь IR, а протокол-то останется). Пока старшие U16 рассматриваем как резервные 4 байта.





//Умолчальные порты (может, потом поменяем)
//#define PC_TO_BOARD 2233
//#define BOARD_TO_PC 7


#pragma pack (pop)
