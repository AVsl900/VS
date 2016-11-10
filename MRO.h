/** @file MRO.h
*   @brief MRO Header File
*   @date 07.07.2013
*   @version 01.0.00
*   
*   This file contains:
    - макросы
*   - Definitions
*   - enum
*   - struct
    - прототипы ф-ий
*   .
*/

#ifndef MRK_H
#define MRK_H

#include "stdtypes.h"
/*! 
   Битовые операции 
*/
#define SETB(x,y) (x |= (1<<y)) //(1<<y1)+(1<<y2)+(1<<y3)) /* Set bit y in byte x*/
//#define SETB(x,y,y1) (x |= (1<<y)+(1<<y1))
#define CLRB(x,y) (x &= (~(1<<y))) /* Clear bit y in byte x*/
#define EORB(x,y) (x ^= (1<<y)) /* EOR bit y in byte x*/
//#define CHKB(x,y) (x & (1<<y)) /* Check bit y in byte x*/
#define CHKB(x,y) ((x>>y) & 1) // Check bit y in byte x 
//((x>>y) & 1) меньше на 3 кода чем (x & (1<<y))


#define _nop __asm ("nop")
#define _cli __asm ("cli")
#define _sei __asm ("sei")
#define _wdr __asm ("wdr")

#define EMPT  0x20  //пусто на индикаторе
//#define DIG_CHAR(x) (x+0x30)    //пусто на индикаторе


/** 
\def K1
PORTA
\def K2 
PORTA
\def K3 
PORTA
\def K4 
PORTA
\def K5 
PORTA
\def K6 
PORTA
*/

//======= bits PORTA ======= 
#define   K1         0 
#define   K2         1
#define   K3         2
#define   K4         3 //
#define   K5         4
#define   K6         5 //
//======= bits PORTB =======  
#define   DB4         0 //DB0-на схеме
#define   DB5         1
#define   DB6         2
#define   DB7         3 //DB3
#define   DDRB_WRITE (DDRB|=0x0F)  //
#define   DDRB_READ  (DDRB&=0xF0)

//В новой плате конт RW PORTB_4 вместо PORTB_6
//#define   RW          6 //старая плата
#define   RW          4
//======= bits PORTC ======= -, -, out, out, in, in, in, in
#define   KEY1         0
#define   KEY2         1
#define   KEY3         2
#define   KEY4         3 // 
#define   RS           4 // 
#define   E            5 //
//======= bits PORTD ======= -, -, out, out, in, in, in, in
#define   RELE         0
#define   RXD1         2
#define   TXD1         3 // 
#define   DR1          5 //
//======= bits PORTE ======= out, out, out, out, out, out, out, in  
#define   RXD0         0
#define   TXD0         1
#define   DR0          2
#define   DIP1         3 // address
#define   DIP2         4 // 
#define   DIP3         5 //
#define   DIP4         6 //
#define   DIP5         7 //
//======= bits PORTG ======= out, out, out, out, out, out, out, in  
#define   DIP6         0 //

#define Off       0  //
#define Push      1  //0.1 sec
#define Push_2s   2  //
#define Push_6s   3  //>250
#define Push_10s  4
/** 
*   Время в 20 мс интервалах
*/
#define Delay_006s 60/20
#define Delay_01s  100/20
#define Delay_02s  200/20
#define Delay_03s  300/20
#define Delay_05s  500/20

#define Delay_1s  1000/20
#define Delay_1p5s  1500/20
#define Delay_2s  2000/20
#define Delay_6s  6000/20
#define Delay_10s  10000/20
#define Delay_15s  15000/20

//Время в 100 мс интервалах
#define Time_02s  200/100
#define Time_03s  300/100
#define Time_05s  500/100
#define Time_1s   1000/100
#define Time_1p5s 1500/100
#define Time_2s   2000/100
#define Time_3s   3000/100
#define Time_4s   4000/100
#define Time_5s   5000/100
#define Time_6s   6000/100
#define Time_8s   8000/100
#define Time_10s  10000/100
#define Time_60s  60000/100
#define Time_180s 180000/100
//#define Time_15s  15000/100


//для данных конфигураций
//#define MaxSection   4
#define MaxModule    8
#define MaxSecMod    16*8 //4 секции по 8 модулей
#define MaxFail      8 //4 секции по 8 модулей


//DisplayAllAlarm, APKAllAlarm, ReleAlarm
#define FailMRK      6     //приоритет - с большим номером бита
#define FailMRO      5
#define AlarmMRK     4
#define AlarmMRO     3     //=1 если DisplayAlarmMRK_copy!=0 DisplayAlarmMRO_copy
#define ResourceMRK  2
#define NetwMRK      1     //=1 если NetwMRK_copy!=0
#define SrabSection  0     //=1 если хоть один бит в DisplaySrabSection_copy[16]!=0



enum Mode{Plug, AddressM, Config0, Config, Check, Ish, Inform, Response, Alarm, Clear, Test};
//Включение Адрес Конфигурация Проверка Исходный Просмотр Срабатывание Аварийный Стирание Тест
enum SubMode_Config{VZU_E, VZU_M, VZU_H, VZU_TM, Side, Width, Circuit, Search_MRK};
enum SubMode_Inform{Modul_cect, Modul_mod, Modul_inf, Resource_res, Resource_srab, Resource_inf_res, Resource_inf_srab, Modul_ADC};
enum SubMode_Alarm{NoContr, Breakdown, EndResource, NoNetw};
enum SubMode_Clear {Prepare, Execute, ExecuteEnd};
enum SubMode_Ish {NoDispl, DisplAPK};
enum SubMode_Test {TestPrep, TestDispl, TestButt,  TestIn, TestAddress, TestRelay, TestRS_MRO, TestRS_MRK,  TestEnd};

enum AddMode_Clear {No, ClearRes, ClearSrab, ClearResSrab, ClearMRK,ClearEEPROM };

enum ReleStart {RelePower=11, RelePowerEnd=0};
enum TypeM1 {M00, M1, M2, M3};
enum TypeWidth {W00, W630, W660, W900};
enum TypeCircuit {L00, LC, LCS, RC};


struct Regime
 {uint8 Mode;
  uint8 SubMode;
  uint8 ClearMode;      //тип стирания
  uint8 NmbLine1;       //бегущая строка для 1-й строки
  uint8 NmbLine2;       //бегущая строка для 2-й строки
  uint8 NmbSection;     //выбраная секция
  uint8 NmbModule;      //выбранный модуль в секции NmbCheckSection
  uint8 NmbRes;
  uint8 NmbSrab;        //номер след. ресурса или срабатыв.
  uint8 FlashDelay;     //задержка на мигание
  uint8 DisplDelay;     //задержка на индикацию (напр. при нескольких отказах - на каждый по 3 сек)
  uint8 RunStringDelay; //первая задержка на бегущую строку
  uint8 RunStringNext;   //след задержка на бегущую строку
//флаги
  unsigned BegConfig:1;      //начальное конфигурирование
  unsigned Flash:1;          //мигание
  unsigned RunFlash:1;       //для сравнения с предыдущим сост для бег. строки
  unsigned PermitAlarm:1;    //разрешение индикации аварий
  unsigned ErrorConfig:1;
  unsigned EndCheck:1;       //Последняя секция - сохранена, конец проверки
  //unsigned APKCompl:1;       //передача старых данных по апк - завершена
 };
extern struct Regime REGIME;



struct Button
 {uint16 TimeUp;
  uint16 TimeDwn;
  uint16 TimeChoice;
  uint16 TimeEsc;
  uint16 TimeUpDw;
  uint8 Press; //общий признак нажатия любой из кнопок
  uint8 Up;  //0-выкл, 1- кратко нажата
  uint8 Dwn;
  uint8 Choice;
  uint8 Esc;
  uint8 UpDwn;
 };
extern struct Button BUTTON;

//Байт ошибок Error
#define Srab               0 //>3 переполнение срабатываний модулей защиты
#define NoClear            1 //отсутствие сброса после чтения не 0 данных
#define ErrorCRC           2 //ошибка команды сброса (не корректный CRC)
#define ErrorRead          3 //ошибка чтения по данному адресу , напр. CRC. Сбрасывается после успешного чтения
#define ErrorConfigMRO1    4 //ошибка конфигурации MRO1(не верный CRC конфигурации)
#define ErrorConfigMRO2    5 //ошибка конфигурации MRO2(не верный CRC конфигурации)
#define ErrorDATAMRO1      6 //ошибка накопленных срабатываний MRO(не верный CRC конфигурации)
#define ErrorDATAMRO2      7

#define ErrorNoClear       10
//#define ErrorConfigMRK     6 //ошибка конфигурации MRK(не верный CRC конфигурации)
//#define NoConfigMRO        7 //нет конфигурации MRO


struct  ConfigMRO
{uint8  Type;              //Конфигурация МР-О: Объект эксплуатации VZU_E/VZU_M / конфигурация  
 uint8  Side;               //Сторона для ВЗУ-ЭЦС-Е (1или2). Количество Рядов(секций) для ВЗУ-ЭЦС-М (1, 2, 3). Для Н и ТМ: каждый бит для 8 секций- есть ЛЦ =1 или НЕТ СЕКЦИИ =0
 uint8  Width;              //Ширина 630, 660, 900 для ВЗУ-ЭЦС-М:
 uint8  Circuit[4];        //цепи ЛЦ, ЛЦС, РЦ для ВЗУ-ЭЦС-М из 0, для Е из 0-3 для 1-4 или 5-8 секции
 uint16 OnMRK;             //Подключенные МР-К, до 16 номеров (с 0 по 15) - (для секций ЛЦ также отключаются) 0- откл, 1- вкл
 uint16 OnD3;              //Контроль входа D3 0- откл, 1- вкл
 uint16 OnD4;              //Контроль входа D4 0- откл, 1- вкл
 uint8  NmbSectionMRK[16]; //Номер секции для МРК0…МРК15
 uint8  NmbSectionMRO[6];  //Номер секции для 6 цепей контроля исправности 
 uint8  OnCircMRO;         //Подключенные цепи контроля исправности МР-О. 0- откл, 1- вкл
 uint16 CRC;
 uint8  Error;              //Флаги ошибок
 uint16 ErrorDataMRK;      //Флаги ошибок данных MRK (накопленных срабатываний) 
};
extern struct ConfigMRO CONFIGMRO;

struct  DataMRK   //накопленные данные с
{uint8 Resource[8];   //Выработка ресурса, %, 0 байт - модуль0
 uint16 SrabMod[8];  //Срабатывания модулей защиты: 0-1 байт - модуль0 … 6-7 байт - модуль7 
 uint8 FlagDamage; //Флаги неисправности группы модулей защиты 
 uint8 Error;
 uint16 CRC;
};
extern struct DataMRK DATAMRK;

struct  DataSummMRK   //накопленные (переписанные) данные со всех МРК
{uint8 Resource[8];   //Выработка ресурса, %, 0 байт - модуль0
 uint16 SrabMod[8];  //Срабатывания модулей защиты: 0-1 байт - модуль0 … 6-7 байт - модуль7 
 uint16 CRC;
 uint8 ADC[8];
};
extern struct DataSummMRK DATASUMMMRK[16], DATASUMMMRK_Copy;

struct  ReadMRK   //прочитанные данные с MRK
{uint8 Resource[8];   //Выработка ресурса, %, 0 байт - модуль0
 uint16 SrabMod;  //Срабатывания модулей защиты: 0,1 бит - модуль0  14,15 бит - модуль7 
 uint16 FlagDamage; //Флаги неисправности группы модулей защиты 
 uint16 CRC;
 uint16 Error;
 uint8 NMB_MRK;
};
extern struct ReadMRK READMRK;

/*
struct DataMRO //собственные данные по срабатыванию в МРО
{uint8 Flag;         //не исп-ся
 uint16 SrabMod[6];  //Срабатывания модулей защиты: 0-1 байт - модуль0 … 6-7 байт - модуль7
 uint16 CRC;
};
extern struct DataMRO DATAMRO;
*/

/*
 typedef volatile  struct  DataMRO
{uint8 FlagDamage; //Флаги неисправности группы модулей защиты 
 uint16 SrabMod[8];  //Срабатывания модулей защиты: 0,1 байт - модуль0 … 14,15 байт - модуль7 
 uint8 Resource[8];   //Выработка ресурса, %, 0 байт - модуль0
 uint8 Error;
 uint8 CRC[2];
}DATAMRO;
#define DATAMRO_0 ((DATAMRO *)0x0400)
DATAMRO_0->FlagDamage=5;
*/
//#define DATAMRO[0] ((DataMRO *)0x0400)


void initPort(void);
void initTimer(void);
void InitUSART(void);
void initDATA(void);


void Display (void);
void CopyDataLine(uint8 Line);//копирует в 1 или 2 строку дисплея данные из LINE1 или LINE2
void RunDataLine(uint8 Line, uint8 Shift); //Бегущая строка с максимальным сдвигом Shift каждые 0.5 сек. копирует в 1 или 2 строку дисплея данные из LINE1 или LINE2
void FlashDisplay(void);//горит 0.5 сек, не горит 0.2 сек
uint8 RunString(uint8 Shift, uint8 Nmb); //REGIME.NmbLine1,2 - бегущая строка на сдвиг- Shift, кол-во сдвигов Nmb
void Delay(uint16 n);


void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink);//запись 1DCB. DISPLAY D: ON=1/OFF=0. Cursor C: ON=1/OFF. Blinking B: of Cursor
void InitDisplay(void);
void InitDisplay2(void);
void DisplayFunction(void);
uint8 TestDisplay (void);
void ChekBusyDisplay (void);
void ClearDisplay(void);
void HomeDisplay(void);
//void FunkSetDisplay(void);
void Write4bitDispl(uint8 Data);
uint8 Read8bitDispl(void);
void SetAddrDisplay(uint8 Addr);
uint8 ConvFont(uint8 Letter);
uint8 ConvHex(uint8 Data); //hex to text
void CopyDataDisplay(uint8 *Data); //копировать данные из буфера в Display с конвертацией в формат дисплея
void CopyDataExDisplay(uint8 *Data);//Копирование данных из буфера в Display без конвертации
uint8 CREATE_TEXT (uint8 __flash *pLn, uint8 *pText);
void ChangeNmb (uint8 Nmb, uint8 *pText);//Замена символа N на Nmb
void CREATE_LINE1 (uint8 __flash *pLn); //формат: надпись, число в hex формате 
void CREATE_LINE2 (uint8 __flash *pLn); //формат: надпись, число в hex формате 
uint8 CHAR_ADDLINE (uint8 Data);//hex to text до 99 -> ADDLINE
void INT_ADDLINE (uint16 Data); //hex to text до 999 -> ADDLINE
void DISPL_MRO_IN(void);

void Begin_Check(void);
void LINE_Version(void);
void LINE_Address(void);
void LINE_Searsh_MRK(void);
uint8 FindMinNmbSection(void); //поиск наименьшего номера секции, рез-т в REGIME.NmbSection
uint8 FindMaxNmbSection(void); //поиск наибольшего номера секции, рез-т в REGIME.NmbSection


uint8 FindNmbMRK(void);//поиск номера MRK по № секции
void ClearDATAMRK(void);//полная очистка данных МРК в ЕЕПРОМ
void LINE2_SEC_R(void); //2 строка:Сек NmbSection Ряд NmbModule
void LINE2_SEC_M(void); //2 строка:Сек NmbSection Мод NmbModule

void Rele(void);  //управление реле

void ApplyMaskFailMRK(void);//наложение масок использования и отключения входов МРК
void CheckTimeFail(void); //контроль времени отключения защиты. Если>60 сек - отказ
void CheckFail_MRK(void); //проверка  размыкания 32 МРК 
void CheckFail_MRO(void); //проверка  размыкания 6 МРО
void CheckAlarm_MRK(void); //проверка  отказов 
void CheckAlarm_MRO(void); //проверка  отказов 
void CheckResource(void); //проверка нараюотки более 80%
void CheckSrab(void); //проверка  срабатывания
void CheckNetw(void); //пропадание связи с каким-либо модулем МР-К.
void TimeSetDispl (void);    //без бег. строки
void TimeSetDisplLn (void); //с бег. строкой

void DataDisplOrder(void); //данные для последовательной индикации всех типов аварий:
uint8 HEX_DEC (uint8 Data); //hex to dec 99
uint8 CHAR_LINE (uint8 Data, uint8 *pLn); //hex to text до 99 -> LINE
uint8 PollingResource(void); //опрос наработки

uint8 Delta (uint8 Data1, uint8 Data2); //относительная разница

void Poll_Regime (void); //действия на нажатия кнопок
void CheckTimeNoPress(void); //выкл. при превышении времени не нажатия
void Check_Button(uint8 Key, uint16 *Time, uint8 *Button );
void CheckAddress(void); //чтение адреса
void Poll_button (void);
void Poll_Srab (void);  //опрос срабатываний по 6 входам. Если более 60сек- отказ
void PrepSearsh(void); //подготовка к поиску МРК

void ChoiceSideDwn(void);//запись в CONFIGMRO Side
void ChoiceSideUp(void);//запись в CONFIGMRO Side
uint8 ChoiceDwn(uint8 Data, uint8 Allow);//запись в Data номера 1-3 при нажатии Dwn с проверкой разрешения этого в Allow
uint8 ChoiceUp(uint8 Data, uint8 Allow);//запись в Data номера 1-3 при нажатии Up с проверкой разрешения этого в Allow 

void SaveNmbSection(void);//сохраняем в ЕЕПРОМ Номера секции для МРК0…МРК3
void SORT_Res(void); //сортировка по ресурсу
uint8 SORT_Srab(void);//сортировка по срабатыванию
void AddDataMRK(void); //добавление данных в массив DATAMRK из полученных по RS485
 
void DataAPKOrder(void); //данные для последовательной отправки в сеть всех типов аварий:
void DataAlarmAPK(void); //формирование данных аварий для отправки по APK
//void CheckAPKAllAlarm(void);     //проверка наличия аварий 

void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size);
#endif