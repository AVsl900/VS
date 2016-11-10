
/** @file MRO.c 
*   @brief Основные программы МРО
*   @date 23.06.2016
*   @version 01.00.00
*/


#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif

#include "inavr.h"
#include "ioavr.h"
#include "MRO.h"
#include "UART.h"
//#include "stdtypes.h"
#include "EEPROM.h"


uint8 Period_1s;            /**< кол-во секунд */
uint8 TimeNoPress;          /**<время ненажатия кнопок */
uint8 TimeAddr;             /**<время проверки адреса */
uint8 ReadAddr;             /**<прочитанный адрес в сети*/
uint8 LINE1[48],            /**<1 строка с 4-х кратным запасом для бег. строки+ добавочная информ. по окончмнию строки*/
LINE2[48],                  /**<2 строка с 4-х кратным запасом для бег. строки+ добавочная информ. по окончмнию строки*/
ADDLINE[12];/**<1, 2 строки с 4-х кратным запасом для бег. строки+ добавочная информ. по окончмнию строки*/
uint8 IndexResourse [16*8+1];/**<Индекс сортировки ресурса 0...3bit -MRK, 4..7bit- Module in section*/
uint8 IndexSrab [16*8+1];   /**<Индекс сортировки срабатываний 0...3bit -MRK, 4..7bit- Module in section*/
uint8 ReqMRK;               /**<запрос МРК*/                 
 
uint16 OnD4;        //отключение неисп. входов МРк при конфигурировании
uint8 OnCircMRO;    //отключение неисп. входов МРО при конфигурировании
uint8 NmbFreeMROIn; //для поиска  наименьшего свободного входа МРО в Н и МТ, он мигает
uint8 LCOn;         //для Н и ТМ установка секций, =1 -ЛС, =0 -нет секции  
 
//размыкание
uint16 DisplayFailMRK[2]; /**<[0]-цепь0(ряд1) 15...0 МРК, [1]-цепь1(ряд2) 15...0 МРК */
uint8 DisplayFailMRO;     /**<6 входов/цепей МРО(0-5)*/ 
/**время в аварии. При превышении для каждого из модулей- индикация ОТКАЗ*/
uint16 TimeFailMRK[32];       
uint16 TimeFailMRO[6];    /**<время в аварии.*/
uint16 TimeRele;          /**<время в аварии.*/
uint8 ReleMode;           /**<режим работы реле  RelePower (включения питания)/ RelePowerEnd (штатная работа)*/
/**отказ*/
uint16 DisplayAlarmMRK[2];    //0бит -МР-К0 ...15бит -МР-К15. [0]-цепь0(ряд1), [1]-цепь1(ряд2)
uint8 DisplayAlarmMRO;        /**<отказ*/
uint8 DisplayResourceMRK[16]; /**<выработка ресурса MRK>80%*/
uint8 NMBResource;
uint16 DisplayNetwMRK;        /**<нет связи с МРК0-15 дольше 10с*/
uint8 TimeNoAnswMRK[16];  /**<время ошибки связи*/    
uint8 TimeSrabMRO[6];     /**<время срабатывания МРО*/
extern uint16 NoAnswMRK;  /**<нет ответа МРК, побитово*/
extern uint16 NoClearMRK; /**<нет очистки МРК, побитово*/

uint8 NMBSrabSection;             /**<№ срабатывания. после показа заданное время в  ищем следующий NMBSrabSection*/
uint8 DisplaySrabSection[16];     /**<8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]*/
uint8 DisplaySrabSectionForAPK[16];
uint16 DisplayFailMRK_copy[2];    /**для последоватеьлной индикации всех типов аварий:
<0бит -МР-К0 ...15бит -МР-К15. [0]-цепь0(ряд1), [1]-цепь1(ряд2)*/
uint8 DisplayFailMRO_copy;        /**<последовательная индикации всех типов аварий:*/
uint16 DisplayAlarmMRK_copy[2];   /**<для последоватеьлной индикации всех типов аварий: 
0бит -МР-К0 ...15бит -МР-К15. [0]-цепь0(ряд1), [1]-цепь1(ряд2)*/
uint8 DisplayAlarmMRO_copy;       /**<последовательная индикации всех типов аварий:*/
uint8 DisplayResourceMRK_copy[16];/**<последовательная индикации всех типов аварий:*/
uint16 DisplayNetwMRK_copy;       /**<последовательная индикации всех типов аварий:*/
uint8 DisplaySrabSection_copy[16]; /**<8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]*/

uint16 DisplayAlarmMRK_APK[2];/**<последовательная индикации всех типов аварий:*/
uint8 DisplayAlarmMRO_APK;    /**<последовательная индикации всех типов аварий:*/
uint8 DisplayResourceMRK_APK[16];/**<последовательная индикации всех типов аварий:*/
uint16 DisplayNetwMRK_APK;/**<последовательная индикации всех типов аварий:*/
uint8 DisplaySrabSection_APK[16]; /**<последовательная индикации всех типов аварий:
8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]*/

/** отказы:  индикация, отправки в сеть, вкл.реле */
uint8 DisplayAllAlarm,  /**< отказы:  индикация*/
//APKAllAlarm,            /**< отказы:  отправки в сеть */
ReleAlarm;              /**< вкл.реле */
uint8 PrepClear,        /**<для полного стирания ЕЕПРОМ*/
PrepTest;               /**<для тестового режима*/

extern uint8 Init_Stage; /**<стадий инициализации дисплея*/
extern uint8 DATA_APK[];/**<данных аварий для отправки по APK*/
//uint8 NmbSectionSend[16];  //номера секторов для отправки, до 16 штук ? (уже есть в DATA_SEKT)
int8 NmbSendAPK, NmbCurrAPK;          //количество секций для передачи, текущий номер передачи

uint8 DATA_SECT_APK[4*16];    //для 16 секций - 16 секторов передачи (по 4 за посылку) 

extern uint8 RX_UART0_TEST[], RX_UART1_TEST[] ;

uint8 Resource80;       /**<наработка >80%*/
uint8 Event;            /**<наличие срабатываний*/

uint8 RunShift;         /**<сдвиг для бегущей строки*/

struct ConfigMRO CONFIGMRO;/**<Конфигурация МРО*/
struct DataMRK DATAMRK;   /**<Данные МРК последний найденный МРК = 0xFF*/
struct Button BUTTON;     /**<Кнопки*/

__flash uint8 TVersion [2] [12]={"___МР-О", "Версия_01.11"};/**< ТЕКСТ для индикатора */
__flash uint8 TAddress [2] [12]={"Aдрес_", "Скор._9600"};/**< ТЕКСТ для индикатора */
__flash uint8 TConfig [3] [12]={"Ожидание", "конфигурации", "Ошибка_конф."};/**< ТЕКСТ для индикатора */
__flash uint8 TVZU [5] [11]={"ВЗУ-ЭЦС-Е", "ВЗУ-ЭЦС-M", "ВЗУ-ЭЦС-Н", "ВЗУ-ЭЦС-TM", "ВЗУ-ЭЦС-"};/**< ТЕКСТ для индикатора */
__flash uint8 TSide [2][12]={"Сторона_", "Колич._рядов", /*"Кол._рядов_"*/};/**< ТЕКСТ для индикатора для Е и М*/
__flash uint8 TWidth [5] [11]={"Ширина_","630","660","900", "Нет"};
__flash uint8 TA630 [18] [10]={
"NA1-NA6","NA7-NA12","NA1-NA9","нет", "нет","нет",          //M1 ЛЦ0 ЛЦ1 ЛЦС0 ЛЦС1 РЦ0 РЦ1
"NA1-NA6","NA7-NA12","нет","нет","NA1-NA4","NA5-NA8",     //M2
"нет","нет","нет","нет","нет","нет"};                       //M3
__flash uint8 TA660 [18] [10]={
"NA1-NA8","NA9-NA16", "нет","нет","NA1-NA6","нет",
"нет","нет","нет","нет","нет","нет",
"нет","нет","нет","нет","NA1-NA18","нет"};
__flash uint8 TA900 [18] [10]={
 "NA1-NA10","NA11-NA20","NA1-NA12", "нет","NA1-NA4","NA5-NA8", 
 "NA1-NA10","NA11-NA20","нет","нет","NA1-NA4", "NA5-NA8",
 "нет","нет","нет","нет","нет","нет"};
 __flash uint8 TVZU_H [3] [12]={"Нет_А5-А8", "Конфигурацию", "__сохранить?"};
__flash uint8 TCircuit [6] [8]={"Цепь_","_ЛЦ","_ЛЦС","_РЦ","_ЛЦ/ЛЦС", "ЛЦ"};
__flash uint8 TSearch [4] [12]={"Поиск_МР-К","Найдены_МР-К", "-нет", "_....." };/**< ТЕКСТ для индикатора */
__flash uint8 TSection [13] [12]={"Секция_", "или_нет_секц", "РЦ_МР-К", "Модуль_А", "СЕК.", "МОД.A", "РЯД", "Нет_МР-К4", "МР-К","_-нет", "_-есть", "Нет_секции_", "Нет_ряда_2"};/**< ТЕКСТ для индикатора */
__flash uint8 TRes_Srab [5] [12]={"Выраб-ка_", "Срабат._", "Кол-во_сраб.", "Срабатываний", "нет" };/**< ТЕКСТ для индикатора */
__flash uint8 TResp [2] [12]={"СРАБОТАЛ","Модуль_" };/**< ТЕКСТ для индикатора */
__flash uint8 TNoContr [6] [12]={"Нет_контроля", "Отказ_модуля", "Кол-во_сраб.", "Нет_связи_с","Контроль_не", "производится" };/**< ТЕКСТ для индикатора */
__flash uint8 TLC_MR [12]={"МР-К_секц."};/**< ТЕКСТ для индикатора */
__flash uint8 TMod [13]={"МОДУЛИ:"};/**< ТЕКСТ для индикатора */
__flash uint8 TVirob [2] [12]={"ВЫРАБОТКА", "РЕСУРСА!"};/**< ТЕКСТ для индикатора */
__flash uint8 TZamen [2] [12]={"Замените", "модуль"};/**< ТЕКСТ для индикатора */
__flash uint8 TClear [9] [12]={"ОБНУЛИТЬ?","Обнулена_выр", "СТЕРЕТЬ?", "ВЕСЬ_ЕЕПРОМ", "СРАБАТЫВ.", "Данные", "Обнулены", "Стирание...", "Обнул_выраб?"};/**< ТЕКСТ для индикатора */
__flash uint8 TTest [18] [12]={"ПРОВЕРИТЬ?", "Check_of", "___display", "БбГгДдЖжЗзИи", "ЛлПпУуФфЦцЧч", "`_`_`_`_`_`_", "_`_`_`_`_`_`", "Тест_кнопок", "Тест_входов", 
"Тест_адреса","Тест_реле", "Реле_ДК_вкл", "Реле_ДК_выкл", "Тест_RS_МРО", "Тест_RS_МРК" ,  "Принято_","___ТЕСТ", "_ОКОНЧЕН"  };/**< ТЕСТ для индикатора */
__flash uint8 TDig [6] = {"123456"};

//вкл. входы ширины 630-900 для ЛЦ/ЛЦС/РЦ: 00-нет, 01- 0вх есть, 10-  1 вх.есть, 11 -оба есть
/*
const uint8 CheckPortW630[3][3]={
3,1,0,                   	  //M1 ЛЦ,ЛЦС,РЦ
3,0,3,                   	  //M2
0,0,0};                  	  //M3
const uint8 CheckPortW660[3][3]={//вкл. входы ширины 660 для ЛЦ/ЛЦС/РЦ. 00-нет, 01- 0вх есть, 10-  1 вх.есть, 11 -оба есть
3,0,1,                    	 //M1 ЛЦ,ЛЦС,РЦ
0,0,0,                    	 //M2
0,0,0};                   	 //M3
const uint8 CheckPortW900[3][3]={//вкл. входы ширины 900 для ЛЦ/ЛЦС/РЦ. 00-нет, 01- 0вх есть, 10-  1 вх.есть, 11 -оба есть
3,1,3,                    	 //M1 ЛЦ,ЛЦС,РЦ
3,0,3,                    	 //M2
0,0,1};                   	 //M3
*/
//Отключение старшего входа МРО комбинации
//M1, 630, ЛЦС
//М1, 900, ЛЦС

//Отключение старшего входа МРК комбинации
//М1, 660, РЦ
//М3, 900, РЦ




 const uint8 AllowWidth[3]={//разрешенные типы ширины для М1/М2/М3
 7,  		//0b00000111,    //M1, bit 2-900 1-660 0-630 
 5,  		//0b00000101,  	 //M2, bit 900 660 630 
 2}; 		//0b00000010     //M3, bit 900 660 630
 const uint8 AllowCircuit[3][3]={//разрешенные типы цепей для М1/М2/М3 630/660/900
 3, 5, 7, 	//0b00000011,  0b00000101,   0b00000111,       //M1:  630 660 900, bit 2-РЦ/1-ЛЦС/0-ЛЦ
 5, 0, 5, 	//0b00000101,  0b00000000,   0b00000101,       //M2:  630 660 900, bit РЦ/ЛЦС/ЛЦ
 1, 4, 0};  //0b00000001,  0b00000100,   0b00000000      //M3:  630 660 900, bit РЦ/ЛЦС/ЛЦ
 
 
extern unsigned char convert_to_ASCII(uint8 HalfByte);
//******************************************************************************
//Инициализация

/** @fn void initPort(void)
*   @brief инициализация портов ввода/вывода
*
*/
void initPort(void)
  {DDRB=(1<<DB4)|(1<<DB5)|(1<<DB6)|(1<<DB7) |(1<<RW) ;
   DDRC=(1<<RS)|(1<<E);
   PORTC=(1<<KEY1)|(1<<KEY2)|(1<<KEY3)|(1<<KEY4); //button +5V
   DDRD=(1<<RELE)|(1<<TXD1)|(1<<DR1);  //внешний RS485
   DDRE=(1<<TXD0)|(1<<DR0);            //внутр RS485
   PORTE=(1<<DIP1)|(1<<DIP2)|(1<<DIP3)|(1<<DIP4)|(1<<DIP5); //dip to +5V
   PORTG=(1<<DIP6);                    //dip to +5V
  }

/** @fn void initTimer(void)
*   @brief инициализация таймеров
*
*/
void initTimer(void)
  {OCR1A = 1000; //ждем (140 мс)
   StartT1;
//таймер контроля времени АПК-ДК - 1 сек между байтами и контроль времени передачи/приема   
   TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //пуск время. мах=OCR3A,  7372800/1024=7200Hz=140мкс
   OCR3A=Time1sUART;    //между байтами ждем не более 1 сек
   ETIMSK = (1<<OCIE3A);
//2-универсальный таймер - интервал 20 мс   
   OCR2  = 144;      //20 мсек
   TCCR2 = (1<<CS22)|(1<<CS20)|(1<<WGM21);  //мах=OCR2 7372800/1024= 138.9 mks
   TIMSK  = (1<<OCIE0)|(1<<OCIE1A)|(1<<OCIE2);
  }
 
/** @fn void initDATA(void)
*   @brief инициализация данных.
*
*   Проверяется наличие и сохранность конфигурации прибора в ЕЕПРОМе, 
*   загрузку оттуда одной из двух корректной копии. Также проверяются накопленные данные с МР-К. 
*   При первом включении  питания прибора (в ЕЕПРОМе - все данные 0xFF) накопленные данные с МР-К обнуляются 
*   ф-ей ClearDATAMRK()
*   При обнаружении ошибок ЕЕПРОМа устанавливаются флагы ошибок, к-ые в дальнейшем используются 
*   для принудительного перехода в режим конфигурации.
*/
void initDATA(void) //загружаем в CONFIGMRO 1 из 2х корректную ECONFIGMRO
  {uint8 i; 
   uint16 Error;
      
   i=0;
   CONFIGMRO=ECONFIGMRO[0].CONFIGMRO; //1-я копия
   if (CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-3)!=0)
    SETB(i,ErrorConfigMRO1);
    
   CONFIGMRO=ECONFIGMRO[1].CONFIGMRO; //2-я копия
   if (CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-3)!=0)
    {SETB(i,ErrorConfigMRO2);
     if CHKB(i,ErrorConfigMRO1)
      CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;  //восстанавливаем 1-ю копию
    }
    
   if ((ECONFIGMRO[0].CONFIGMRO.Error==0XFF)||(ECONFIGMRO[1].CONFIGMRO.Error==0XFF)) //если 1-е включение - стираем данные
    {ClearDATAMRK();   //полная очистка данных МРК в ЕЕПРОМ
     i=(1<<ErrorConfigMRO1)|(1<<ErrorConfigMRO2);             //Error
    }

   CONFIGMRO.Error=i;                    //сохраняем флаги ошибок
//   ECONFIGMRO[0].CONFIGMRO.Error=i; 
//   ECONFIGMRO[1].CONFIGMRO.Error=i;
 
//накопленные данные по МРK   
   Error=0;
   for(i=0;i<16;i++)
    {DATAMRK=EDATAMRK[i].DATAMRK; 
     if (CRC16(DATAMRK.Resource,sizeof(struct DataMRK))!=0)
      SETB(Error,i);
    }
   CONFIGMRO.ErrorDataMRK=Error;
//   ECONFIGMRO[0].CONFIGMRO.ErrorDataMRK=Error; 
//   ECONFIGMRO[1].CONFIGMRO.ErrorDataMRK=Error;
  }

/** @fn void ChekBusyDisplay(void)
*   @brief Проверка окончания выполнения операций дисплея
*
*/
void ChekBusyDisplay (void)
 {uint16 i=0;
CLRB(PORTC,E); 
SETB(PORTB,RW);   //displ-READ
DDRB_READ; 
  while (i<10000)
   {SETB(PORTC,E);//чтение DB7
    Delay(4);
    if (CHKB(PINB,DB7)==0)
     i=0xFFF0;    //end
    CLRB(PORTC,E); 
    Delay(5);
    
    SETB(PORTC,E);
    Delay(5);
    CLRB(PORTC,E); 
    Delay(4);
    i++;
   }
 }

/** @fn void InitDisplay(void)
*   @brief Однократная инициализация дисплея при старте
*
*/
 void InitDisplay(void)
 {
SETB(PORTB,RW);   //displ-Read 
DDRB_WRITE;
CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.

SETB(PORTC,E); //E: H,H->L Chip enable signal
Delay(10000);  //10 ms
CLRB(PORTC,E); //E: H,H->L Chip enable signal  
SETB(PORTC,E); //E: H,H->L Chip enable signal
Delay(10000);  //10 ms
CLRB(PORTC,E); //E: H,H->L Chip enable signal    

//FUNCTION  
DisplayFunction();
ChekBusyDisplay();
   
//ENTRY MODE  
   Write4bitDispl(0x00);
   Write4bitDispl(0x06);//запись 01 I/D S/H
 
  /*I/D IS THE INCREMENT/DECREMENT BIT.
When I/D is set to "1", the DDRAM Address is incremented by "1" when a character code is written into
or read from the DDRAM. An increment of 1 will move the cursor or blinking one step to the right.
When I/D is set to "0", the DDRAM is decremented by 1 when a character code is written into or read
from the DDRAM. A decrement of 1 will move the cursor or blinking one step to the left.
S: SHIFT ENTIRE DISPLAY CONTROL BIT
This bit is used to shift the entire display. When S is set to "1", the entire display is shifted to the right
(when I/D ="0") or left (when I/D ="1"). When S is set to "0", the display is not shifted.
Ex1 : I/D=1, S=1
  */
 }

/** @fn void InitDisplay2(void)
*   @brief Периодическая инициализация дисплея.
*
*   Контроль дисплея с периодической его переинициализацией для  устранения зависаний и сбоев от помех 
*   В зависимости от значения переменной Init_Stage выполняется 1 из 4 стадий инициализации дисплея. 
*   После выполнения одной из стадий Init_Stage инкрементируется для выполнения следующей стадии при 
*   следующем вызове подпрограммы. Т.о. на каждой стадии инициализации выделяется фиксированное время 20 мс 
*   для выполнения внуттренних команд дисплея, без затрат времени CPU на ожидание следующей операции
*/
void InitDisplay2(void)
 {
  if (Init_Stage>4)
   return;
  switch (Init_Stage)
   {
    case 0:
     SETB(PORTB,RW);   //displ-Read 
     DDRB_WRITE;
     CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
     SETB(PORTC,E); //E: H,H->L Chip enable signal
    break;

    case 1:
      CLRB(PORTC,E); //E: H,H->L Chip enable signal
    break;  

    case 2:
      SETB(PORTC,E); //E: H,H->L Chip enable signal
      //SETB(PORTB,RW);   //displ-WRITE
      //DDRB_READ;
    break; 
    
    case 3:
      CLRB(PORTC,E); //E: H,H->L Chip enable signal
      CLRB(PORTB,RW);   //displ-WRITE 
      DDRB_WRITE;
      DisplayFunction(); //FUNCTION  
   break;

   case 4:
//ENTRY MODE  
      Write4bitDispl(0x00);
      Write4bitDispl(0x06);//запись 01 I/D S/H
   break;
   }
 Init_Stage++; 
 }
 
/** @fn void DisplayFunction(void)
*   @brief Инициализация дисплея, перевод в режим выбора ф-ии 
*
*/
 void DisplayFunction(void) 
   {CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
    Write4bitDispl(0x00); 
    Write4bitDispl(0x00);
    Write4bitDispl(0x00);
    Write4bitDispl(0x00);
    Write4bitDispl(0x00);
    ChekBusyDisplay();   
   
    Write4bitDispl(0x02);
    Write4bitDispl(0x02);
    Write4bitDispl(0x0A);
    ChekBusyDisplay();    
    OnDisplay(0,0,0);//DISPL ON/OFF

    ChekBusyDisplay();
    ClearDisplay();//Display Clear
   }
 

//******************************************************************************
//Индикация
 
/** @fn void Display(void)
*   @brief Выдача информации на дисплей. 
*
*   Для различных режимов (REGIME.Mode) и подрежимов (REGIME.SubMode) индикации и  работы МР-О осуществляется
*   передача информации в дисплей и также на этапах конфигурирования прибора - конфигурирование МР-О.
*/
void Display (void) //индикация
  {uint8 i, n, m;
  
/** На 1-м этапе выполняется:
        - Включение режима индикации дисплея OnDisplay(1,0,0) если режим работы не "исходный"
        - Декремент переменной REGIME.DisplDelay, контролирующей время индикации
*/  
   if (REGIME.Mode!=Ish)
    OnDisplay(1,0,0);         //DISPL ON
 
    
   if (REGIME.DisplDelay!=0)  
    {if ((CHKB(PINC,KEY1))&&(CHKB(PINC,KEY2))&&(CHKB(PINC,KEY3))&&(CHKB(PINC,KEY4)))//при нажатии кнопки - время не считается
      REGIME.DisplDelay--;
    }
  
/** На 2-м этапе контролируются аварии в штатных режимах работы МР-О (после конфигурирования и проверки конфигурации):
      Вызываются ф-ии проверяющие:
        - опрос наработки ресурса более 80%
        - Контроль времени отключения защиты  (контроль размыкания) CheckTimeFail() для 6 цифровых входов МР- О и 2 входов МР-К
        - Если разрешено индицировать аварии (установлен флагREGIME.PermitAlarm) то вызываются ф-ии обнаружения 
          всех типов аварий для их последовательной обязательной индикации и отсылке в АПК-ДК.
          - Обработка данных для последовательной индикации всех типов аварий
          - проверка размыкания в МР-О в CheckFail_MRO(), имеет наивысший приоритет, подавляющий другие типы аварий
          - проверка размыкания в МР-К в CheckFail_MRK(), имеет наивысший приоритет, подавляющий другие типы аварий%
          - проверка срабатывания 
          - проверка отказы в МР-О (размыкание более определенного времени)
          - проверка отказы в МР-К (размыкание более определенного времени) 
          - проверка наработки более 80%
          - проверка отсутствие связи с МР-К   
*/   
   if (REGIME.Mode>Check)       //проверки- только в штатных режимах для режима > Check)
    {Resource80=PollingResource();         //опрос наработки
     CheckTimeFail();           //контроль времени отключения защиты 
//не проверяем во время 3-х сек. индикации и после нажатия кнопки 5 сек     
     if ( (REGIME.DisplDelay==0)&&(REGIME.PermitAlarm==1) )  
      {DataDisplOrder();        //данные для последовательной индикации всех типов аварий:
       CheckFail_MRO();         //проверка  размыкания - приоритет по индикации над всеми 
       CheckFail_MRK();
       CheckSrab();             //проверка  срабатывания
       CheckAlarm_MRO();        //проверка  отказов MRO
       CheckAlarm_MRK();        //проверка  отказов MRK 
       CheckResource();         //проверка наработки более 80%/ 
       CheckNetw();
      }
    }
   
/** - Очистка массива памяти LINE1, LINE2 из к-ой данные копируются в дисплей для индикации 1-ой и 2-ой строки
*/    
   for (i=0;i<48;i++)//очистка экрана:
    {LINE1[i]=0x20;
     LINE2[i]=0x20;
    }
   for (i=0;i<12;i++)//очистка экрана:
    ADDLINE[i]=0x20;

/** На 3-м этапе в зависимости от режима работы/индикации (состояния REGIME.Mode) выполняется
*/
   switch (REGIME.Mode)
    {
/** - Режим включения Plug
    1. В дисплей загружается названия прибора и номер версии ПО в ф-ии
    2. Записывается время индикации данной информации
    3. В  initDATA() проверяется наличие и сохранность конфигурации прибора в ЕЕПРОМе
      При обнаружении ошибок ЕЕПРОМа устанавливаются флагы ошибок, к-ые в дальнейшем 
      используются для принудительного перехода в режим конфигурации
    4. Выбирается следующий режим работы/индикации 
*/    
    case Plug: //Включение
         LINE_Version();
         if (REGIME.DisplDelay==0)   
          {REGIME.DisplDelay=Time_2s;
           REGIME.Mode=AddressM;
           initDATA();
          }
     break;
/** - Режим индикации адреса AddressM
    1. В дисплей загружается сетевой адрес прибора и скорость работы 
    2. Устанавливается время индикации
    3. Выбирается следующий режим работы/индикации
*/    
     case AddressM:   //Адрес
         LINE_Address();
         if (REGIME.DisplDelay==0)   
          {REGIME.DisplDelay=Time_5s;
           REGIME.Mode=Config0;
          }
     break;
/** - Режим ожидания конфигурации Config0
    1. В дисплей загружается "ОЖИДАНИЕ КОНФИГУРАЦИИ" 
    2. Если включение питания -первое, т.е. конфигурирования не было - ожидается нажатие кнопок
       для перехода в след. этап конфигурирования Config
    3. Если МР-О ранее был сконфигурирован, по истечению времени ожидания включается 
       режим работы "исходный" с выключением дисплея
*/      
     case Config0: //1я Конфигурация. Еще не сконфигурирован, кнопки не нажимались
          CREATE_LINE1(&TConfig[0][0]); //"ОЖИДАНИЕ"
          CREATE_LINE2(&TConfig[1][0]); //"КОНФИГУРАЦИИ"
          DisplayAllAlarm=0;
          CONFIGMRO.Side=M1;
          CONFIGMRO.Width=W630;
          for (i=0;i<4;i++)
           CONFIGMRO.Circuit[i]=LC;
          if (CONFIGMRO.Error!=0)   //1-я конфигурация, ждем нажатия кнопок up/dwn
           break;
          else if (REGIME.DisplDelay==0)  //ожидание конфигурации - истекло   
           {REGIME.Mode=Ish;
            OnDisplay(0,0,0);//DISPL ON/OFF  -выкл.
           }
     break;
/** - Режим конфигурации Config
    1. В зависимости от значения REGIME.SubMode в дисплей загружается надпись 
       типа утройства "ВЗУ-ЭЦС-Е" / "ВЗУ-ЭЦС-М" / -Н / -ТМ или номер верхней секции / для М: ширина...цепь 
    2. запускается поиск МР-К
    3. Запускается SaveNmbSection() в к-ой в ЕЕПРОМ записываются 2 копии конфигурации МР-О, 
       включающие соответствующие номера секции для найденных МР-К
    4. На дисплее индицируются все найденные номера МР-К
*/      
     case Config: //Конфигурация
            REGIME.BegConfig=0;   //подготовка к проверке отключенного старшего входа МРО
            REGIME.ErrorConfig=0; //подготовка к проверке ошибки конфигурации 
            
            if (REGIME.SubMode<=VZU_TM)     //Индицируем все типы ВЗУ
             {if (REGIME.Flash==0)
               CREATE_LINE1(&TVZU[4][0]);
              else
               CREATE_LINE1(&TVZU[REGIME.SubMode][0]);  //ВЗУ-ЭЦС-Е...ВЗУ-ЭЦС-TM
              CopyDataLine(2);//копирует 2 в строку дисплея данные из LINE
             }
            else if (REGIME.SubMode==Side)//Сторона для -Е и кол-во модулей для М (секций)
             {
//  ver 1.11
              if (CONFIGMRO.Type==VZU_E)
               {CREATE_LINE1(&TVZU[0][0]);      //ВЗУ-ЭЦС-Е
                if (REGIME.Flash==0)            //номер мигает
                 CHAR_ADDLINE(CONFIGMRO.Side);       //hex to text до 99 -> ADDLINE
                CREATE_LINE2(&TSide[0][0]);     //Сторона_
               }
              else                              //для -М
                {if (REGIME.Flash==0)           //номер мигает
                  CHAR_ADDLINE(CONFIGMRO.Side); //hex to text до 99 -> ADDLINE 1, 2, 3
                 CREATE_LINE1(&TVZU[1][0]);     //ВЗУ-ЭЦС-M
                 CREATE_LINE2(&TSide[1][0]);    //Колич._рядов
                }
/* ver 1.11  -можно заменить на и откорр. TSide Кол._рядов_        
CREATE_LINE1(&TVZU[CONFIGMRO.Type][0]);      //ВЗУ-ЭЦС-Е / M
if (REGIME.Flash==0)           //номер мигает
 CHAR_ADDLINE(CONFIGMRO.Side); //hex to text до 99 -> ADDLINE 1, 2, 3
CREATE_LINE2(&TSide[CONFIGMRO.Type][0]);     //Сторона_ /Кол._рядов_
 */                 
             }
            else if (REGIME.SubMode==Width)         //только для М
             {CHAR_ADDLINE(CONFIGMRO.Side);         //hex to text до 99 -> ADDLINE 1, 2, 3
              CREATE_LINE1(&TVZU[1][0]);            //ВЗУ-ЭЦС-M
              
              i=CREATE_TEXT(&TWidth[0][0],&LINE2[0]);//Ширина_
              if (REGIME.Flash==0) //горит 0.5 сек, не горит 0.2 сек
               CREATE_TEXT(&TWidth[CONFIGMRO.Width][0],&LINE2[i]);    //630-900
              CopyDataLine(2);               
             }
            else if (REGIME.SubMode==Circuit)         //только для М
             {CHAR_ADDLINE(CONFIGMRO.Side);         //hex to text до 99 -> ADDLINE 1, 2, 3
              CREATE_LINE1(&TVZU[1][0]);            //ВЗУ-ЭЦС-M
              
              i=CREATE_TEXT(&TCircuit[0][0],&LINE2[0]);//Цепь_
              if (REGIME.Flash==0) //горит 0.5 сек, не горит 0.2 сек
               CREATE_TEXT(&TCircuit[CONFIGMRO.Circuit[0]][0],&LINE2[i]);    //ЛЦ РЦ
              CopyDataLine(2);  //2 строка             
             } 
             
            else if (REGIME.SubMode==Search_MRK)//Поиск МРК
             LINE_Searsh_MRK(); //отсюда переход в Check
            else //кнопки не нажимались, режим SubMode не установлен 
             { CREATE_LINE1(&TConfig[0][0]); //"ОЖИДАНИЕ", "КОНФИГУРАЦИИ"
               CREATE_LINE2(&TConfig[1][0]);
               REGIME.DisplDelay=Time_3s; //
             }
     break;
/** - Режим проверки секций МРК Check
    Индицируются номера подключенных секций, соответствующие номеру обнаруженных в сети МР-К
    Настраиваются секции ЛЦ, проверяется коректность входов МРО и МРК
    1. Для типов Н и ТМ: Всего - 8 секций. Сначала все найденные МРК распределяются по соотв. секциям 1-8      
на оставшиеся в порядке очереди распределяются входы МРО на к-ых 0. Можно секцию (не занятую МРК) отключить (нет секции)
или подкл. к входу МРО (ЛЦ). Если вх. МРО > оставшихся секций - "ошибка конф." Для Н может исп-ся вход МРК D4, для ТМ он отсутствует
    2. Для типов Е и М типы входов ЛЦ ЛЦС РЦ
*/     
     case Check:
     
     n=REGIME.NmbSection;
     uint8 i, k;
     
     Begin_Check();
     
    if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
     {i=CREATE_TEXT(&TSection[0][0],&LINE1[0]);
      if (REGIME.Flash==0)
       CHAR_LINE(n, &LINE1[i++]);   //Секция 1(мигает)
      else
       LINE1[i++]='_';      
      if (CHKB(CONFIGMRO.OnMRK,n)==0)  //нет МРК для такой секции 
       {  if ( CONFIGMRO.OnCircMRO==0 )     //все входы МРО -отключены
           CREATE_TEXT(&TSection[11][0],&LINE2[0]);  //нет секции -не мигает                   
          else
           {if (CHKB(LCOn,n-1)==1)      //каждый бит для 8 секций- есть ЛЦ =1 или НЕТ СЕКЦИИ =0 
             {if (REGIME.Flash==0)
               {CREATE_TEXT(&TCircuit[5][0],&LINE2[0]);        //ЛЦ
                //LINE2[2]=ConvHex(NmbFreeMROIn);           //Тот порт к-ый можно выбрать для данной секции - мигает.
               }
              DISPL_MRO_IN();               //Индикация включенных входов МРО во 2й строке
             }
            else
             {if (REGIME.Flash==0)            
               CREATE_TEXT(&TSection[11][0],&LINE2[0]);        //НЕТ_СЕКЦИИ, мигает
             }
           }
       }
      else //есть МРК для такой секции 
       {CLRB(LCOn,n-1);      //отключаем для данной секции работу с входами МРО
        CREATE_TEXT(&TCircuit[3][0],&LINE1[i]); //РЦ
       
        if(CONFIGMRO.Type==VZU_H)
         {//проверка входов 0,1 МРK1  для 1й секции,МРK2 для 2-ой, МРK3  для 3-ей
          if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -разомкнут 
           REGIME.ErrorConfig=1;
          else if(CHKB(DisplayFailMRK[1],n)==1)//D4 MRK -разомкнут 
           {CLRB(OnD4,n);   //отключаем вход D4 для дальнейшего контроля к-ый разомкнут
            CREATE_TEXT(&TVZU_H[0][0],&LINE2[0]); //нет А5...А8
           }
         } 
        else if(CONFIGMRO.Type==VZU_TM)    
         {if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -разомкнут 
           REGIME.ErrorConfig=1;
         }
       }

/*     if ( (REGIME.EndCheck==1)&&(REGIME.ErrorConfig==0) )    //закончили проверку 
       {CREATE_TEXT(&TVZU_H[1][0],&LINE1[0]);    //Конфигурацию
        CREATE_TEXT(&TVZU_H[2][0],&LINE2[0]);    //_сохранить?
       } */
       
      CopyDataLine(1);
     }
    else //тип - не VZU_H
     {
      i=CREATE_TEXT(&TSection[0][0],&LINE1[0]);
      if (REGIME.Flash==0)
       CHAR_LINE(n, &LINE1[i]);   //Секция 1 - ЛЦ
      i++; 
       
     if (CONFIGMRO.Type==VZU_M)  
      {CREATE_TEXT(&TCircuit[CONFIGMRO.Circuit[0]][0],&LINE1[i]);    //для М -из [0] ЛЦ ЛЦС РЦ
       CopyDataLine(1);
      }      
     else  //VZU_E
      {if (n>4) //ищем номер цепи в CONFIGMRO.Circuit[0-3] для 1-4 или 5-8 секции
        k=n-5;
       else
        k=n-1;
      
       if (CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0)//Если МРК - нет: ЛЦ/ЛЦС
        {if ( ((CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==1))||(k==3) ) //если оба входа МРО не подключены, или 4 или 8 секции
          {CopyDataLine(1); //ЛЦ/ЛЦС - в бег. строке не индицируем
           CLRB(OnCircMRO,k*2);
           CLRB(OnCircMRO,k*2+1);//и отключаем их от контроля
          }
         else
          {CREATE_TEXT(&TCircuit[4][0],&LINE1[i]); //для Е ЛЦ/ЛЦС
           RunDataLine(0, 3); //бег. строка
          }
        }         
       else//Если МРК - есть: РЦ
        {CREATE_TEXT(&TCircuit[3][0],&LINE1[i]); //РЦ - не мигает
         CopyDataLine(1);
        }         
      }
  
//проверка входов, 2я строка
     if (CONFIGMRO.Type==VZU_M)
      {if (CONFIGMRO.Circuit[0]==LC)
        {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //нет МРК для такой секции 
          {//проверка входов МРО 0,1 для 1й секции, 2,3 для 2-ой, 4,5 для 3-ей
           if (CHKB(PINA,(n-1)*2)==1) //0вход =1, номер секции начин. с 1 а пин с 0
            REGIME.ErrorConfig=1;
           else if (CHKB(PINA,(n-1)*2+1)==1)//0вход =0, 1вх =1
            {CLRB(OnCircMRO,(n-1)*2+1);   //отключаем вход для дальнейшего контроля к-ый разомкнут
             i=CREATE_TEXT(&TWidth[4][0],&LINE2[0]); //Нет
             k=6*(CONFIGMRO.Side-1)+1; //Порядок в TA630-900: М1- ЛЦ0 ЛЦ1 ЛЦС0 ЛЦС1 РЦ0 РЦ1, М2- ЛЦ0 ЛЦ1 ЛЦС0 ЛЦС1 РЦ0 РЦ1, 
             if (CONFIGMRO.Width==W630)
              CREATE_TEXT(&TA630[k][0],&LINE2[i]); //M1 630 ЛЦ1: NА7-NА12. M2: NА13-NА24...
             else if (CONFIGMRO.Width==W660)
              CREATE_TEXT(&TA660[k][0],&LINE2[i]); //M1 660 ЛЦ1: NА9-NА16
             else
              CREATE_TEXT(&TA900[k][0],&LINE2[i]); //M1 900 ЛЦ1: NА11-NА20
             ChangeNmb(n,LINE2);  //Замена символа N на Nmb 
            }
           else if (CHKB(PINA,(n-1)*2+1)==0)//0вход =0, 1вх =0//0вход =0, 1вх =0
            SETB(OnCircMRO,(n-1)*2+1);   //включаем вход для дальнейшего контроля к-ый замкнут
          }
         else
           REGIME.ErrorConfig=1; //2 строка загружается ошибка конфиг. после чего рестарт
        }
       else if  (CONFIGMRO.Circuit[0]==LCS)
        {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //нет МРК для такой секции 
          {//проверка входов МРО 0,1 для 1й секции, 2,3 для 2-ой, 4,5 для 3-ей
           if (CHKB(PINA,(n-1)*2)==1) //0вход =1
            REGIME.ErrorConfig=1;
           }
          else
           REGIME.ErrorConfig=1; //2 строка загружается ошибка конфиг. после чего рестарт
        }
       else //RC
        {ApplyMaskFailMRK();   //наложение масок использования и отключения входов МРК
         if (CHKB(CONFIGMRO.OnMRK,n)==0)  //нет МРК для такой секции 
          REGIME.ErrorConfig=1; //2 строка загружается ошибка конфиг. после чего рестарт
         else
          {//проверка входов 0,1 МРK1  для 1й секции,МРK2 для 2-ой, МРK3  для 3-ей
            if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -разомкнут 
             REGIME.ErrorConfig=1;
            else if(CHKB(DisplayFailMRK[1],n)==1)//D4 MRK -разомкнут 
             {CLRB(OnD4,n);   //отключаем вход D4 для дальнейшего контроля к-ый разомкнут
              i=CREATE_TEXT(&TWidth[4][0],&LINE2[0]); //Нет
              uint8 k=6*(CONFIGMRO.Side-1)+5; //Порядок в TA630-900: М1- ЛЦ0 ЛЦ1 ЛЦС0 ЛЦС1 РЦ0 РЦ1, М2- ЛЦ0 ЛЦ1 ЛЦС0 ЛЦС1 РЦ0 РЦ1,
              if (CONFIGMRO.Width==W630)
               CREATE_TEXT(&TA630[k][0],&LINE2[i]); 
              else if (CONFIGMRO.Width==W660)
               CREATE_TEXT(&TA660[k][0],&LINE2[i]); 
              else
               CREATE_TEXT(&TA900[k][0],&LINE2[i]);  //M1 900 РЦ1: NА5-NА8. M2: NА9-NА16...
              ChangeNmb(n,LINE2);  //Замена символа N на Nmb
            }
           else if(CHKB(DisplayFailMRK[1],n)==0)//D4 MRK -разомкнут 
            SETB(OnD4,n);
          }  
        }
      }
     else if (CONFIGMRO.Type==VZU_E)//VZU_E
      {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //нет МРК для такой секции 
        {if (k==3) //4 или 8 секции
          {CHAR_ADDLINE(n); 
           CREATE_LINE2(&TSection[11][0]);   //Нет секции
           break;
          }
         //анализируем входы МРО для действ. секций
          if ( (CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==0) )//0вход =1, 1вх=0
           REGIME.ErrorConfig=1;
          else if ( (CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==1) )//0вход =1, 1вх =1
           CREATE_TEXT(&TSection[11][0],LINE2);   //Нет секции
          else if ( (CHKB(PINA,k*2)==0)&&(CHKB(PINA,k*2+1)==1) )//0вход =0, 1вх =1 
           {CLRB(OnCircMRO,k*2+1);   //отключаем вход для дальнейшего контроля к-ый разомкнут. сброс бита не влияет на индикацию, т.к. пин - не изменился
            CREATE_TEXT(&TSection[12][0],LINE2);   //Нет ряда 2
           }
          else if ( (CHKB(PINA,k*2)==0)&&(CHKB(PINA,k*2+1)==0) )//0вход =0, 1вх =0
           SETB(OnCircMRO,k*2+1);
        }
       else     //анализируем входы МРК
        { if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -разомкнут
           REGIME.ErrorConfig=1;
          else if (CHKB(DisplayFailMRK[1],n)==1)
           {CLRB(OnD4,n);   //отключаем вход D4 для дальнейшего контроля к-ый разомкнут
            CREATE_TEXT(&TSection[12][0],LINE2);   //Нет ряда 2
           }
          else if  (CHKB(DisplayFailMRK[1],n)==0)
           SETB(OnD4,n);
        } 
      }
    }//тип - не VZU_H


      if (REGIME.ErrorConfig==1)
        {CREATE_TEXT(&TConfig[2][0],LINE2);//Ошибка_конф.
        }
      CopyDataLine(2);
     break;
     
     
/** - Режим исходный Ish. 
     Выключается дисплей
*/     
     case Ish:    //Исходный
/*      
//test 
if (REGIME.SubMode==DisplAPK)
 {for(i=0;i<4;i++)
 {uint8 Data;
   Data=DATA_APK[3+i];
   LINE1[3*i]=ConvHex(Data>>4);
   LINE1[3*i+1]=ConvHex(Data);
   LINE1[3*i+2]='_';
   Data=DATA_APK[3+4+i];
   LINE2[3*i]=ConvHex(Data>>4);
   LINE2[3*i+1]=ConvHex(Data);
   LINE2[3*i+2]='_';
  }
 CopyDataLine(1);//копирует в строку дисплея данные из LINE
 CopyDataLine(2);//копирует в строку дисплея данные из LINE
 OnDisplay(1,0,0);         //DISPL ON
 
 if (REGIME.DisplDelay==0)
  {REGIME.SubMode=NoDispl;
   OnDisplay(0,0,0);//DISPL ON/OFF  -выкл.
  }
 }
       
else 
*/
       OnDisplay(0,0,0);//DISPL ON/OFF  -выкл.
     break;
     
/**- Режим просмотра Inform.
      - Режим индикации ресурса и срабатывания для выбранного номера секции и модуля. 
      Если нет ни одного подключенного МР-К в дисплей выдается "Контроль не производится"
      На дисплее индицируется, в зависимости от значения REGIME.SubMode:
    1.Modul_cect  -номер секции,  мигает строка "СекцияХ"
    2.Modul_mod -номер модуля, мигает строка "МодульХ"
    3.Modul_inf - индикация данных модуля: "Ресурс хх%. Срабат. ууу"". Для отключенной секции - "Контроль не производится"
      - Режим индикации номера секции и модуля для сортированного от большего к меньшему 
      значению ресурса и срабатывания. В зависимости от значения REGIME.SubMode индицируется:
    1.Resource_res  -  выбор индикации с сортировкой по ресурсу,  мигает строка "Ресурс"
    2.Resource_srab - выбор индикации с сортировкой по срабатыванию, мигает строка "Кол-во_Сраб"
    3.Resource_inf_res - отсортированные значения ресурса с указанием секции и модуля
    4.Resource_inf_srab - отсортированные значения срабатываний с указанием секции и модуля
*/     
     case Inform: //Просмотр

      if (CONFIGMRO.OnMRK==0)  //не подключен ни один МРК
         {CREATE_LINE1(&TNoContr[4][0]); //Контроль
          CREATE_LINE2(&TNoContr[5][0]); //не производится
          if (REGIME.DisplDelay==0)
           REGIME.Mode=Ish; //по окончанию индикации переходим в Ish
          return;
         }
      
      n=FindNmbMRK(); //N выбраного МРК из N выбраной секции REGIME.NmbSection
      if (REGIME.NmbModule>7)
       REGIME.NmbModule=7;
      m=REGIME.NmbModule+1; //для индикации- +1, т.к. нет 0 модуля
      
      switch (REGIME.SubMode)
           {case Modul_cect:     //мигает СекцияХ
            default:
             if (REGIME.Flash==0) //горит 0.5 сек, не горит 0.2 сек
              ADDLINE[0]=0;
             else
              CHAR_ADDLINE(REGIME.NmbSection);//hex to text до 99 -> ADDLINE
             CREATE_LINE1(&TSection[0][0]);   //СекцияХ 
             CHAR_ADDLINE(m);                 //hex to text до 99 -> ADDLINE
             CREATE_LINE2(&TSection[3][0]);   //МодульХ
            break;
            case Modul_mod:                   //мигает МодульХ
             CHAR_ADDLINE(REGIME.NmbSection); //hex to text до 99 -> ADDLINE
             CREATE_LINE1(&TSection[0][0]);   //СекцияХ 
             
             if (REGIME.Flash==0)   //горит 0.5 сек, не горит 0.2 сек
              ADDLINE[0]=0;
             else
              CHAR_ADDLINE(m);                 //hex to text до 99 -> ADDLINE
             CREATE_LINE2(&TSection[3][0]);   //МодульХ
            break;
            
            case Modul_inf:                 //Ресурс хх%. Срабат. ууу". 
             if (CHKB(CONFIGMRO.OnMRK,n)==0)  //МРК - не подключен
              {CREATE_LINE1(&TNoContr[4][0]); //Контроль
               CREATE_LINE2(&TNoContr[5][0]); //не производится
               if (REGIME.DisplDelay==0)
                REGIME.SubMode=Modul_cect; //по окончанию индикации переходим в Modul_cect
               return;
              }
             ADDLINE[CHAR_ADDLINE(EDATAMRK[n].DATAMRK.Resource[m-1])]='%';//для индикации мод- +1, поэтому надо -1
             CREATE_LINE1(&TRes_Srab[0][0]);   //Ресурс
             
             INT_ADDLINE(EDATAMRK[n].DATAMRK.SrabMod[m-1]);  //для индикации мод- +1, поэтому надо -1      
             CREATE_LINE2(&TRes_Srab[1][0]);   //Срабат
            break;
/*            
//test            
case Modul_ADC:
 for(i=0;i<4;i++)
  {uint8 Data;
   Data=DATASUMMMRK[n].ADC[i];
   LINE1[3*i]=ConvHex(Data>>4);
   LINE1[3*i+1]=ConvHex(Data);
   LINE1[3*i+2]=',';
   Data=DATASUMMMRK[n].ADC[i+4];
   LINE2[3*i]=ConvHex(Data>>4);
   LINE2[3*i+1]=ConvHex(Data);
   LINE2[3*i+2]=',';
  }
 CopyDataLine(1);//копирует в строку дисплея данные из LINE
 CopyDataLine(2);//копирует в строку дисплея данные из LINE
break; 
*/
        
            case Resource_res:   //Мигает Ресурс
             if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
              CREATE_LINE1(&TRes_Srab[0][0]);   //Ресурс
             else
              CopyDataLine(1);//копирует в строку дисплея данные из LINE
             CREATE_LINE2(&TRes_Srab[2][0]);   //Кол-во_Сраб
            break;
            
            case Resource_srab:  //Мигает Кол-во_Сраб
             CREATE_LINE1(&TRes_Srab[0][0]);   //Ресурс
             if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
              CREATE_LINE2(&TRes_Srab[2][0]);   //Кол-во_Сраб
             else
              CopyDataLine(2);//копирует в строку дисплея данные из LINE
            break;
//Ресурс хх%. Сек.z Мод Ау            
            case Resource_inf_res:
             //if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
             ADDLINE[CHAR_ADDLINE(EDATAMRK[n].DATAMRK.Resource[m-1])]='%';//для индикации мод- +1, поэтому надо -1 
             CREATE_LINE1(&TRes_Srab[0][0]);                   //Ресурс
             
             i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
             LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция;
             LINE2[i++]=' ';                                      //
             i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);
             LINE2[i]=ConvHex(m);       //N модуля
             CopyDataLine(2);//копирует в строку дисплея данные из LINE
            break;
            
            case Resource_inf_srab:
             //if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
             if (Event==0)
              {CREATE_LINE1(&TRes_Srab[3][0]);  //Срабатываний
               CREATE_LINE2(&TRes_Srab[4][0]);  //нет
              }
             else
              {
             INT_ADDLINE(EDATAMRK[n].DATAMRK.SrabMod[m-1]); //для индикации мод- +1, поэтому надо -1 
             CREATE_LINE1(&TRes_Srab[1][0]);                   //Сраб 
             
             i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
             LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция;
             LINE2[i++]=' ';                                      //
             i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);
             LINE2[i]=ConvHex(m);       //N модуля
             CopyDataLine(2);//копирует в строку дисплея данные из LINE
             }
            break;
            }
     break;
     
/** - Режим индикации срабатывания  Response
      При срабатывании ни дисплее отображается "Сработал ХАУ", где Х - номер секции, У - номер модуля. 
      По окончанию отведенного для индикации  времени - переход в исходный режим
*/     
     case Response://Срабатывание
//  При срабатывании (+1): на 3-4 сек "сработал мод. ххАуу". Если несколько  - по 3 сек для каждого модуля
//надо добавить вкл реле на 180 сек  
   
      if (REGIME.DisplDelay==0)
       REGIME.Mode=Ish; //по окончанию индикации переходим в исходный режим
//"сработал мод. хxАу" -xx section, y-module -кроме Н и ТМ, для них - просто номер секции           
      CREATE_LINE1(&TResp[0][0]);   //СРАБОТАЛ
      
      i=CREATE_TEXT(&TResp[1][0],&LINE2[0]);     //Мод.
      i+=CHAR_LINE(REGIME.NmbSection, &LINE2[i] ); //номер секции
      if ( (CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M) )
       {LINE2[i++]='A';
        LINE2[i++]=ConvHex(REGIME.NmbModule+1);//номер модуля
       }  
      CopyDataLine(2);//копирует в строку дисплея данные из LINE
     break;
/**- Режим аварий Alarm.
      На дисплее отображается, в зависимости от значения REGIME.SubMode:
      1.NoContr  -нет контроля  "Нет_Контроля" с указанием номера секции и модуля,  при размыкании цепи контроля исправности МР-О либо МР-К
      2.Breakdown - отказ, "Отказ Модуля" с указанием номеров секции и модуля, разрыв цепи контроля исправности МР-О либо МР-К более 60 сек. Для различных конфигураций МР-О изменяется перечень номеров модулей в сообщении (т.к. к одному цифровому входу последовательно подключены несколько контролируемых модулей)
      3.EndResource - выработка ресурса, надписи "ВЫРАБОТКА. РУСУРСА" и "ЗАМЕНИТЕ МОДУЛЬ" с указанием номеров секции и модуля
      4.NoNetw -нет связи, надпись "нет связи" с указанием номеров секции
*/     
     case Alarm:  //Аварийный
// Нет контроля. Для ВЗУ-ЭЦС-Е Надпись "Нет контроля. Сек.хх Рядуу", 
//для ВЗУ-ЭЦС-М Надпись "Нет контроля. Секция х". Если востановилось -переход Исходный
      if (REGIME.DisplDelay==0)
       REGIME.Mode=Ish; //по окончанию индикации переходим в исходный режим
      switch (REGIME.SubMode)
           {case NoContr:      //Нет контроля
            default://размыкание цепи контроля исправности МР-О либо МР-К
            
            if (CONFIGMRO.Type==VZU_E)
             {if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
                CREATE_LINE1(&TNoContr[0][0]);   //Нет_Контроля
              else
                CopyDataLine(1);//выкл строки
              if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
               LINE2_SEC_R(); //2 строка:Сек NmbSection Ряд NmbModule
              else
               CopyDataLine(2);//выкл строки
             }
            else if (CONFIGMRO.Type==VZU_M)
             {CREATE_TEXT(&TNoContr[0][0],&LINE1[0]);   //Нет_Контроля
              goto   Type_VZU_M;
             }
            else    //Н и ТМ
             {if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
               CREATE_LINE1(&TNoContr[0][0]);   //Нет_Контроля
              else
                CopyDataLine(1);//выкл строки 
              i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
              LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция; //2 строка:Сек NmbSection Ряд NmbModule
              CopyDataLine(2);//копирует в строку дисплея данные из LINE
             }
                          
            break;
            
            case Breakdown:      //Отказ
//размыкание цепи контроля исправности МР-О либо МР-К более 60 сек            
            
            if (CONFIGMRO.Type==VZU_E)
             {CREATE_LINE1(&TNoContr[1][0]);   //Отказ_Модуля
              LINE2_SEC_R(); //2 строка:Сек NmbSection Ряд NmbModule
             }
            else if (CONFIGMRO.Type==VZU_M)//"ОТКАЗ МОДУЛЯ. СЕКЦИЯ х" 2."РЯД МОДУЛЕЙ: хауу - хаzz".
             {CREATE_TEXT(&TNoContr[1][0],&LINE1[0]);      //1я часть -Отказ_Модуля
Type_VZU_M:             
              CREATE_TEXT(&TMod[0],&LINE1[12]);            //2я часть Ряд_Модулей:
              i=CREATE_TEXT(&TSection[0][0],&LINE2[0]);    //1я часть -Секция
              CHAR_LINE(REGIME.NmbSection,&LINE2[i]);      //N выбраной секция hex to text до 99 -> LINE
              
              uint8 k;    //индекс 
              k = 6*(CONFIGMRO.Side-1);          //модуль М1, М2, М3
              k +=  (CONFIGMRO.Circuit[0]-1)*2;  //добавляем тип цепи
              if (REGIME.NmbModule==2)
               k += 1 ;                               //провереем номер модуля, если =2, +1

              i=12;
//определяем ширину:
              if (CONFIGMRO.Width==W630)
               i+=CREATE_TEXT(&TA630[k][0],&LINE2[i]); //M1 630 ЛЦ1: NА7-NА12. M2: NА13-NА24...
              else if (CONFIGMRO.Width==W660)
               i+=CREATE_TEXT(&TA660[k][0],&LINE2[i]); //M1 660 ЛЦ1: NА9-NА16
              else
               i+=CREATE_TEXT(&TA900[k][0],&LINE2[i]); //M1 900 ЛЦ1: NА11-NА20
 
              LINE2[i++]=0;
              ChangeNmb(REGIME.NmbSection,&LINE2[12]);
      
              
              RunString(12,2); //бегущая строка/ сдвиг 12, 2 строки
              SetAddrDisplay(0x00);         //1 строка 
              CopyDataDisplay(&LINE1[REGIME.NmbLine1]); //2 строки в 1-ой
              SetAddrDisplay(0x40);         //2 строка 
              CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //2 строки в 2-ой
             }
            else        //для Н и ТМ: "ОТКАЗ МОДУЛЯ. СЕКЦИЯ х"
             {CREATE_LINE1(&TNoContr[1][0]);   //Отказ_Модуля
              i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
              LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция; //2 строка:Сек NmbSection Ряд NmbModule
              CopyDataLine(2);//копирует в строку дисплея данные из LINE
             }             
            break;
            
             
            case EndResource:  //ВЫРАБОТКА. РУСУРСА
//выработка более 80% ресурса МР-К. Надпись 1."ВЫРАБОТКА. РУСУРСА!" 2."ЗАМЕНИТЕ. МОДУЛЬ ХАУУ" 
             CREATE_TEXT(&TVirob[0][0],&LINE1[0]);           //1я часть -ВЫРАБОТКА
             CREATE_TEXT(&TZamen[0][0],&LINE1[12]);          //2я часть ЗАМЕНИТЕ:
             CREATE_TEXT(&TVirob[1][0],&LINE2[0]);           //1я часть -РЕСУРСА
             i=12;
             i+=CREATE_TEXT(&TZamen[1][0],&LINE2[12]);       //2я часть МОДУЛЬ:
             i+=CHAR_LINE(REGIME.NmbSection,&LINE2[i]); //hex to text до 99 -> LINE
             if ((CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M))
              {LINE2[i++]='A';      //ЗАМЕНИТЕ. МОДУЛЬ ХАУУ
               i+=CHAR_LINE((REGIME.NmbModule+1),&LINE2[i]); //hex to text до 99 -> LINE
              }
             LINE2[i++]=0;
             
             RunString(12,2); //бегущая строка/ сдвиг 12, 2 строки
             SetAddrDisplay(0x00);         //1 строка 
             CopyDataDisplay(&LINE1[REGIME.NmbLine1]); //2 строки в 1-ой
             SetAddrDisplay(0x40);         //2 строка 
             CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //2 строки в 2-ой
            break;
            
            case NoNetw:   //ОТКАЗ(нет связи)
//пропадание связи с каким-либо модулем МР-К. Надпись "ОТКАЗ МОДУЛЯ. МР-Кхх"
             CREATE_LINE1(&TNoContr[3][0]);     //нет связи            
             ADDLINE[CHAR_ADDLINE(REGIME.NmbSection)]; //номер МР-К
             CREATE_LINE2(&TLC_MR[0]);     //МР-К
            break;
           }
     break;
     
/**- Режим стирания Clear.
      На дисплее отображается, в зависимости от значения REGIME.SubMode:
      1.Prepare - подготовка к стиранию. В зависимости от значения REGIME.ClearMode 
      в дисплей выдается запрос на стирание срабатываний  "СТЕРЕТЬ СРАБАТЫВ.?" или 
      на стирание всего ЕЕПРОМ "СТЕРЕТЬ ВЕСЬ ЕЕПРОМ?"
      2.Execute - выполнение стирания. В зависимости от REGIME.ClearMode в дисплей выдается 
      "Стирание... СРАБАТЫВ" или "Стирание... ВЕСЬ ЕЕПРОМ"
      3.ExecuteEnd - завершение стирание. В зависимости от REGIME.ClearMode в дисплей выдается 
      "Данные Обнулены" или "Обнулено" и далее номер обнуленной секции с номером модуля  
*/     
     case Clear:  //Стирание
      switch (REGIME.SubMode)
           {case Prepare://"ОБНУЛИТЬ? СЕК.хх Модуу".
            default:
             if (REGIME.ClearMode==ClearMRK)
              {CREATE_LINE1(&TClear[2][0]);     //СТЕРЕТЬ
               if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
                CREATE_LINE2(&TClear[4][0]);     //СРАБАТЫВ.?
               else
                CopyDataLine(2);//копирует в строку дисплея данные из LINE
              }
             else if (REGIME.ClearMode==ClearEEPROM)
              {CREATE_LINE1(&TClear[2][0]);     //СТЕРЕТЬ
               if (REGIME.Flash!=0) //горит 0.5 сек, не горит 0.2 сек
                CREATE_LINE2(&TClear[3][0]);     //ВЕСЬ_ЕЕПРОМ?
               else
                CopyDataLine(2);//копирует в строку дисплея данные из LINE
              }
             else
              {CREATE_LINE1(&TClear[8][0]);     //Обнул_выраб?
               LINE2_SEC_M(); //2 строка:Сек NmbSection Ряд NmbModule
              }
            break;
            
            case Execute://Обнуляется -
              CREATE_LINE1(&TClear[7][0]);     //Стирание...
              if (REGIME.ClearMode==ClearMRK)
               CREATE_LINE2(&TClear[4][0]);     //СРАБАТЫВ
              else if (REGIME.ClearMode==ClearEEPROM)
               CREATE_LINE2(&TClear[3][0]);     //ВЕСЬ_ЕЕПРОМ
              else 
                LINE2_SEC_M();                   //Сек NmbSection Ряд NmbModule
             
              if (REGIME.DisplDelay==0)
               {REGIME.SubMode=ExecuteEnd;
                REGIME.DisplDelay=Time_2s;     //задержка на индикацию
               }
            break;
            
            case ExecuteEnd://"ОБНУЛЕНО 
              if ((REGIME.ClearMode==ClearMRK)||(REGIME.ClearMode==ClearEEPROM))
                {if (REGIME.DisplDelay!=0)
                  {CREATE_LINE1(&TClear[5][0]);     //Данные
                   CREATE_LINE2(&TClear[6][0]);     //Обнулены
                  }
                 else
                  REGIME.Mode=Plug;
                }
               else               
                {if (REGIME.DisplDelay!=0)
                  {CREATE_LINE1(&TClear[1][0]);     //Обнулено
                   LINE2_SEC_M();                   //Сек NmbSection Ряд NmbModule
                   }
                 else
                  REGIME.Mode=Ish;
                }
            break;
           }
     break;
     
 /**- Режим тестирования Test.
      На дисплее отображается, в зависимости от значения REGIME.SubMode:
1.Проверка дисплея. Состоит из 4 тестов: 
•	Тест английского шрифта, на экране в течении 2 секунд появляется надпись «Check of display»
•	Тест кирилицы, на экране в течении 2 секунд появляется надпись состоящая из 12 маленьких и заглавных букв
•	Тест пикселов нечетных знакомест, в течении 2 секунд горят все пикселы нечетных знакомест
•	Тест пикселов четных знакомест, в течении 2 секунд горят все пикселы четных знакомест
2.Проверка органов управления (кнопок). На экране в верхней строке отображается надпись «Тест кнопок».
3.Проверка входов. На экране в верхней строке появляется надпись «Тест входов». Во второй строке показываются номера разомкнутых входов: «1 2 3 4 5 6», 
если вход замкнут – его номер не отображается
4.Проверка сетевого адреса. На экране в верхней строке отображается надпись «Тест адреса». В нижней строке указан адрес, например: «Адрес 32». 
5.Проверка реле ДК. На экране в верхней строке отображается надпись «Тест реле». При нажимании кнопки «Up» или  «Dwn» реле ДК включается и во второй строке 
появляется надпись «Реле ДК вкл». Если кнопки не нажаты - реле ДК выключено, а на индикаторе загорается «Реле ДК выкл»
6.Проверка интерфейса RS-485 связи МРО с АПК ДК. На экране в верхней строке появляется надпись «Тест RS МРО». Если по RS-485 в МРО послать байт, 
то он будет отображен во второй строке, например: «Принято 23». После приема принятого байта МРО ответит инкрементированным байтом, в данном случае – 0х24. 
7.Проверка интерфейса RS-485 связи МРО с МРК. На экране в верхней строке появляется надпись «Тест RS МРК». Если в МРО послать байт, 
то он будет отображен во второй строке, например: «Принято 23». После приема принятого байта МРО ответит инкрементированным байтом, в данном случае – 0х24. 
*/     
     case Test:  //Test
      switch (REGIME.SubMode)
       {case TestPrep:
        default:
         CREATE_LINE1(&TTest[0][0]);
         CopyDataLine(2);//копирует 2 в строку дисплея данные из LINE
         REGIME.DisplDelay=Time_8s;
        break; //TestPrep
        
        case TestDispl:
          if (REGIME.DisplDelay>Time_6s)   //на время задержки - индикация режима теста Индикатора
           {CREATE_LINE1(&TTest[1][0]);
            CREATE_LINE2(&TTest[2][0]);
           }
          else if (REGIME.DisplDelay>Time_4s)
           {CREATE_LINE1(&TTest[3][0]); //"БбГгДдЖжЗзИи"
            CREATE_LINE2(&TTest[4][0]); //ЛлПпУуФфЦцЧч
           }
          else if (REGIME.DisplDelay>Time_2s)
           {CREATE_LINE1(&TTest[5][0]);
            CREATE_LINE2(&TTest[5][0]);
           }
          else if (REGIME.DisplDelay!=0)
          {CREATE_LINE1(&TTest[6][0]);
           CREATE_LINE2(&TTest[6][0]);
          }
         else if (REGIME.DisplDelay==0)
           {REGIME.SubMode=TestButt;
           } 
        break;  //TestDispl
        
        case TestButt:
        CREATE_LINE1(&TTest[7][0]);    //Тест_кнопок
         
        if (BUTTON.TimeUp!=0)
         LINE2[0] =0xD9;    //UP
        if (BUTTON.TimeDwn!=0)
         LINE2[3] =0xDA;    //DOWN
        if (BUTTON.TimeEsc!=0)
         LINE2[5] ='<';
        if (BUTTON.TimeChoice!=0)
         LINE2[7] ='>';
        if  (BUTTON.TimeUpDw!=0)
         {LINE2[0] =0xD9;
          LINE2[3] =0xDA;
         }
        SetAddrDisplay(0x40);           //2 строка
        CopyDataExDisplay(&LINE2[0]);   //Копирование данных из буфера в Display без конвертации
        break;  //TestButt

        case TestIn:
        CREATE_LINE1(&TTest[8][0]);    //Тест_входов
        DISPL_MRO_IN();               //Индикация включенных входов МРО во 2й строке
        CopyDataLine(2);              //копирует в строку дисплея данные из LINE 
        break; //TestIn
        
        case TestAddress:
        CREATE_LINE1(&TTest[9][0]);    //Тест_адреса
        
        i=CREATE_TEXT(&TAddress[0][0],&LINE2[0]); //"Aдрес_", 
        CHAR_LINE(EAddrNet, &LINE2[i++]);
        CopyDataLine(2);//копирует в строку дисплея данные из LINE
        break;  //TestAddress
        
        case TestRelay:
        CREATE_LINE1(&TTest[10][0]);    //Тест_реле
        if ((BUTTON.TimeUp!=0)||(BUTTON.TimeDwn!=0))
          CREATE_LINE2(&TTest[11][0]);    //Реле_ДК_вкл
        else
          CREATE_LINE2(&TTest[12][0]);    //Реле_ДК_выкл
        break;  //TestAddress
        
        case TestRS_MRO:
        CREATE_LINE1(&TTest[13][0]);    //Тест_RS_МРО
        i=CREATE_TEXT(&TTest[15][0],&LINE2[0]);  //в тестовом режиме - для индикации посл. байта
        if (RX_UART1_TEST[1]!=0)
         {LINE2[i++]=convert_to_ASCII(RX_UART1_TEST[0]>>4);
          LINE2[i++]=convert_to_ASCII(RX_UART1_TEST[0]&0x0F);
          if  (RX_UART1_TEST[1]==Time_1s)
            {SETB(PORTD,DR1);                   //запрет приема по 1
             UDR1=(RX_UART1_TEST[0]+1);         //однократная передача 1 байта
             UART1.TXExec=1;                    //идет передача, не принимаем
            }
          else
           {UART1.TXExec=0;
            CLRB(PORTD,DR1);                    //разрешение приема
           }
          RX_UART1_TEST[1]--;
         }
        CopyDataLine(2);                //копирует в строку дисплея данные из LINE
        break;  //TestAddress
        
        case TestRS_MRK:
        CREATE_LINE1(&TTest[14][0]);    //Тест_RS_МРО
        i=CREATE_TEXT(&TTest[15][0],&LINE2[0]);  //в тестовом режиме - для индикации посл. байта
        if (RX_UART0_TEST[1]!=0)
         {LINE2[i++]=convert_to_ASCII(RX_UART0_TEST[0]>>4);
          LINE2[i++]=convert_to_ASCII(RX_UART0_TEST[0]&0x0F);
          if  (RX_UART0_TEST[1]==Time_1s)
           {SETB(PORTE,DR0);            //вкл. передачу
            UDR0=(RX_UART0_TEST[0]+1);       //однократная передача 1 байта 
            } 
         else
          CLRB(PORTE,DR0);        //вкл. передачу
         RX_UART0_TEST[1]--;
        }         
        CopyDataLine(2);                //копирование в строку дисплея
        REGIME.DisplDelay=Time_6s;
        break;  //TestAddress
        
        case TestEnd:
        CREATE_LINE1(&TTest[16][0]);    //Тест_RS_МРК
        CREATE_LINE2(&TTest[17][0]);
        if (REGIME.DisplDelay==0)
         {REGIME.Mode=Plug;
          REGIME.DisplDelay=Time_2s;
         }
        break;  //TestAddress
       }
       
     break;
    }
  }
  
/** @fn void Begin_Check(void)
*   @brief Предварительная конфигурация используемых входов МРО и МРК для всех типов ВЗУ  
*/
 void Begin_Check(void)
 {uint8 i, k, Type=CONFIGMRO.Type;
  if  (REGIME.BegConfig==0)  
   {REGIME.BegConfig=1;
    CONFIGMRO.OnCircMRO=0x3F;   //по умолчанию -включены 6 входов МРО
    CONFIGMRO.OnD3=0xFFFF;      //включаем входы D3 МРК
    CONFIGMRO.OnD4=0xFFFF;      //включаем входы D4 МРК
    LCOn=0x00;                  //каждый бит для 8 секций- есть ЛЦ =1 или НЕТ СЕКЦИИ =0. Выключаем все секции в режим ЛЦ, далее по мере проверок свободных входов МРО - включаем

  if ( (Type==VZU_H)||(Type==VZU_TM) ) 
   {for (i=0,k=0;i<6;i++)       //проверка входов МРО, не задействованные - отключаем//3*2=6 входов МРО            
     {if (CHKB(PINA,i)==1)      //0вход =1, номер секции начин. с 1 а пин с 0                          //
       {CLRB(CONFIGMRO.OnCircMRO,i);  //откл. вход МРО
        k++;                           //кол-во вкл. входов МРО
       }
     }
//ищем мин. свободный номер входа МРО для данной секции
   for (NmbFreeMROIn=1;NmbFreeMROIn<7;NmbFreeMROIn++) //если  NmbFreeMROIn=7 -то нет свободных, ошибка если нет хотя бы 1 МРК из 8 
     {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
       break;
     }
   if ( (NmbFreeMROIn>=7)&&((CONFIGMRO.OnMRK&0xFF)==0) )  
     REGIME.ErrorConfig=1; //ошибка если нет хотя бы 1 МРК из 8 и отсутствуют свободные входы
  }
 else if (Type==VZU_M)   //M, 
  {if (CONFIGMRO.Side==M1)  //M1
    CONFIGMRO.OnCircMRO=0x03;//для VZU_M1 - включены 2 входов МРО
   else if (CONFIGMRO.Side==M2)
    CONFIGMRO.OnCircMRO=0x0F;//для VZU_M1 - включены 4 входов МРО
            
   if ( ((CONFIGMRO.Side==M1)&&(CONFIGMRO.Circuit[0]==LCS))&&((CONFIGMRO.Width==W630)||(CONFIGMRO.Width==W900)) )
     CLRB(CONFIGMRO.OnCircMRO,1); //Отключение старшего входа МРО комбинации для M1, 630, ЛЦС. М1, 900, ЛЦС
 
   if ( ((CONFIGMRO.Circuit[0]==RC)&&(CONFIGMRO.Width==W660))
        &&((CONFIGMRO.Side==M1)||(CONFIGMRO.Side==M3)) )
     CONFIGMRO.OnD4=0; //отключаем все D4 для комбинации М1+М3, RC, W660
  }
  
//Для М и Е: Ищем совпадения номеров секций для найденный МРК и в МРО, если есть - в МРО их отключаем 
 if ( (Type==VZU_M)||(Type==VZU_E) )
  {k=FindMinNmbSection();
   for (i=0;i<3;i++)//3*2=6 входов МРО            
    {if (CHKB(CONFIGMRO.OnMRK,k+i)==1)  //есть МРК для такой секции                           //
       {CLRB(CONFIGMRO.OnCircMRO,i*2);  //откл. вход МРО с совпавшей секцией
        CLRB(CONFIGMRO.OnCircMRO,i*2+1);  //откл. вход МРО с совпавшей секцией
       }
    }
  }
  
    OnCircMRO=CONFIGMRO.OnCircMRO;     //для конфигурирования
    OnD4=CONFIGMRO.OnD4;      
   }
 }
  
  
/** @fn void LINE_Version(void)
*   @brief В дисплей загружается названия прибора и номер версии ПО в ф-ии 
*/
 void LINE_Version(void) 
 {ADDLINE[0]=0;
  CREATE_LINE1(&TVersion[0][0]); //"МР-О", "Версия_01.0"
  CREATE_LINE2(&TVersion[1][0]);
 }
 
/** @fn void LINE_Address(void)
*   @brief В дисплей загружается сетевой адрес прибора и скорость работы  
*/
 void LINE_Address(void)   
 {uint8 i;
  i=CREATE_TEXT(&TAddress[0][0],&LINE1[0]); //"Aдрес_", "Скор._9600"
  CHAR_LINE(EAddrNet, &LINE1[i]);
  CopyDataLine(1);//копирует в строку дисплея данные из LINE
  
  CREATE_LINE2(&TAddress[1][0]);
 }

 
/** @fn void LINE_Searsh_MRK(void)
*   @brief Подпрограмма выдает на дисплей строку "Поиск_МР-К" и вызывает Find_MRK(), 
    
    Find_MRK посылает запрос в сеть RS-485  по адресами от 1 до 16,  ждет ответ, 
    и получив его записывает в конфигурацию МР-О CONFIGMRO.OnMRK все имеющиеся на связи МР-К. 
*/
void LINE_Searsh_MRK(void) //поиск МРК
  {uint8 i, n, NMB_MRK;
   if  ((CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M))
    i=1; //макс.4 МРК
   else if  ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
    i=2; //макс.8 МРК
  else  
    i=4; //макс.16 МРК
  
   if (RunString(12,i)!=0) //"бегущая строка" (4 строки на 16 адресов) с шагом = 12
    {REGIME.Mode=Check;//переход к след. режиму-Check после однократного показа
     REGIME.NmbSection=FindMinNmbSection();
    }

   if (REGIME.ClearMode==0)   //выполняем - 1 раз
    {REGIME.ClearMode=1;
     CREATE_LINE1(&TSearch[0][0]);   //Поиск_МР-К
     CREATE_LINE2(&TSearch[3][0]);  //...
   
     Find_MRK();        //поиск МРК 2 раза. Приоритет 100%. Время 32*16=0.5с 
     //SaveNmbSection();  //сохраняем в ЕЕПРОМ Номера секции для МРК0…МРК3, МРО
     REGIME.NmbLine2=0;
    }
   else
    {CREATE_LINE1(&TSearch[1][0]);   //Найдены_МР-К
     if (CONFIGMRO.OnMRK==0) //МР-К не найдены
      {CREATE_LINE2(&TSearch[2][0]); //"-нет"
      }
     else
      {for(i=0,n=0; i<16; i++) //n- номер позиции в строке2
         {if CHKB(CONFIGMRO.OnMRK,i)
           {if (i==0)
             NMB_MRK=0x16;  //для 0го бита OnMRK адрес =16
            else 
             NMB_MRK=HEX_DEC(i);
            LINE2[n++]=ConvHex(NMB_MRK>>4);
            LINE2[n++]=ConvHex(NMB_MRK);
            LINE2[n++]=',';
           }
         }
      if (n<12) //если меньше 4х МРК, заполнена только 1я строка
        {for(i=0,n=12; i<12; ) //копируем 1ю строку во 2 строку для индикации бег. строки
           LINE2[n++]=LINE2[i++];
        }

       SetAddrDisplay(0x40);         //2 строка 
       CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //бег. строка: 4 части
      }
    }
 }

/** @fn void LINE2_SEC_R(void)
*   @brief Сохраняет во 2 строку N выбраной секции и ряд
*/
void LINE2_SEC_R(void) //2 строка:Сек NmbSection Ряд NmbModule
  {uint8 i;
   i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
   LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция;
   LINE2[i++]=' ';                                      //
   i+=CREATE_TEXT(&TSection[6][0],&LINE2[i]);      //Ряд
   LINE2[i]=ConvHex(REGIME.NmbModule);    //Ряд =1 или 2        
   CopyDataLine(2);//копирует в строку дисплея данные из LINE
  }

/** @fn void LINE2_SEC_M(void)
*   @brief Сохраняет во 2 строку N выбраной секции и номер модуля
*/
void LINE2_SEC_M(void) //2 строка:Сек NmbSection Мод NmbModule
  {uint8 i;
   i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //СЕК
   LINE2[i++]=ConvHex(REGIME.NmbSection);     //N выбраной секция;
   LINE2[i++]=' ';                                      //
   i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);      //Мод
   LINE2[i]=ConvHex(REGIME.NmbModule+1);//номер модуля, для индикации +1             
   CopyDataLine(2);//копирует в строку дисплея данные из LINE
  }

/** @fn void CopyDataLine(uint8 Line)
*   @brief Копирует в 1 или 2 строку дисплея данные из LINE1 или LINE2
    @param[in] Line
      - 0: копирование в 1 строка
      - ~0: копирование в 2 строка
*/
void CopyDataLine(uint8 Line)
 {if (Line==1)
   {SetAddrDisplay(0x00);         //1 строка 
    CopyDataDisplay(&LINE1[0]); //
   }
  else
   {SetAddrDisplay(0x40);         //2 строка 
    CopyDataDisplay(&LINE2[0]); //
   }
 }

 /** @fn void RunDataLine(uint8 Line, uint8 Shift)
*   @brief Бегущая строка. Копирует в 1 или 2 строку дисплея данные из LINE1 или LINE2, с максимальным сдвигом Shift каждые 0.5 сек
    @param[in] Line
      - 0: копирование в 1 строка
      - ~0: копирование в 2 строка
    @param[in] Shift - максимальное значение сдвига  
*/
void RunDataLine(uint8 Line, uint8 Shift)
 { 
   if (REGIME.RunFlash!=REGIME.Flash)   //каждые 0.3, 0.2 сек
    {REGIME.RunFlash=REGIME.Flash;
     if  (REGIME.Flash==1)              //каждые 0.3+0.2=0.5 сек
      {RunShift++;
       if (RunShift>Shift)
        RunShift=0;
       }
     }
  if (Line==0)
   {SetAddrDisplay(0x00);         //1 строка 
    CopyDataDisplay(&LINE1[RunShift]); 
   }
  else
   {SetAddrDisplay(0x40);         //2 строка 
    CopyDataDisplay(&LINE2[RunShift]); //
   }
 }
 
 
/** @fn void FlashDisplay(void)
*   @brief Контроль временных интервалов индикации на дисплее мигающих символов

    Периодическая установка флага REGIME.Flash в 1 на интервал 0.3 сек и сброс на 0.2 сек 
    -для формирования мигающих символов на дисплее
*/
void FlashDisplay(void)//горит 0.5 сек, не горит 0.5 сек
 {
  if (REGIME.FlashDelay!=0)//задержка на мигание
   REGIME.FlashDelay--; 
  else   
   {if (REGIME.Flash==0)
     {REGIME.FlashDelay=Time_03s;
      REGIME.Flash=1;
     }
    else
     {REGIME.FlashDelay=Time_02s;
      REGIME.Flash=0;
     }
   }
 }


/** @fn uint8 RunString(uint8 Shift, uint8 Nmb)
*   @brief "бегущая строка" 
    @param[in] Shift шаг бегущей строки в символах
    @param[in] Nmb шаг бегущей строки в строках
    \return     
             - 0 бегущая строка не завершена
             - 1 бегущая строка завершена
*/
uint8 RunString(uint8 Shift, uint8 Nmb) //REGIME.NmbLine1,2 - бегущая строка на сдвиг- Shift, кол-во сдвигов Nmb
 {if (REGIME.RunStringDelay!=0)
   REGIME.RunStringDelay--;
  else   
   {REGIME.RunStringDelay=REGIME.RunStringNext;     //"бегущая строка" (4 строки на 16 адресов) с шагом = 12
    REGIME.NmbLine1+=Shift;            //прошла 1 секунда, след. информация 
    REGIME.NmbLine2+=Shift;            //прошла 1 секунда, след. информация 
    if (REGIME.NmbLine1>=(Shift*Nmb))  //по 2 адреса в строке на 12 букв
     {REGIME.NmbLine1=0;
      REGIME.NmbLine2=0;
      return 1;                        //end
     }
   }
  return 0;
 }
/*
uint8 Delta (uint8 Data_summ, uint8 Data) //относительная разница
 {if (Data_summ>=Data)
   return Data_summ-Data;
  else
   return  (256-Data)+Data_summ;
 }
*/

/** @fn uint8 PollingResource(void)
*   @brief Опрос наработки >80% 
    \return 
                - 0 нет наработки >80%
                - 1 есть наработка >80%
*/
uint8 PollingResource(void)       //опрос наработки >80%
 {uint8 i,n,RES80=0;
  for(i=0;i<16;i++)              //по 16 МРК
   {DisplayResourceMRK[i]=0;
    if CHKB(CONFIGMRO.OnMRK,i)   //для включенных МРК
     {for(n=0;n<8;n++)           //по 8 модулям
       if (EDATAMRK[i].DATAMRK.Resource[n]>80) 
        {SETB(DisplayResourceMRK[i],n);
         RES80=1;
        }
     }
   }
  return RES80;
 } 
 
/** @fn void CheckTimeFail(void)
*   @brief Контроль времени (60с) отключения защиты и отсутствия связи 
    
    Контроль времени отключения защиты  (контроль размыкания) для 6 цифровых входов 
    МР- О и 2 входов МР-К с установкой флагов отказов в  массиве DisplayAlarmMRK[],
    контроль времени отсутствия cвязи с МР-К с установкой флагов отсутствия cвязи 
    в DisplayNetwMRK
*/
void CheckTimeFail(void) //контроль времени (60с) отключения защиты и отсутствия связи.
 {uint8 i;
  DisplayAlarmMRK[0]=0;
  DisplayAlarmMRK[1]=0;
  DisplayAlarmMRO=0;
  DisplayNetwMRK=0;
 
  for(i=0;i<16;i++)
     {if (CHKB(CONFIGMRO.OnMRK,i)==0)
       continue;
//отказы MRK 

      ApplyMaskFailMRK();   //наложение масок использования и отключения входов МРК
    
      if (CHKB(DisplayFailMRK[0],i)==0)
       TimeFailMRK[i]=0;
      else
       {if (TimeFailMRK[i]>Time_60s)
         SETB(DisplayAlarmMRK[0],i);//отказ
        else 
         TimeFailMRK[i]++;
       }
      if (CHKB(DisplayFailMRK[1],i)==0)
       TimeFailMRK[i+16]=0;
      else
       {if (TimeFailMRK[i+16]>Time_60s)
         SETB(DisplayAlarmMRK[1],i);//отказ
        else
         TimeFailMRK[i+16]++;
       }
//время отсутствия связи с МРК       
      if (CHKB(NoAnswMRK,i)==0)
       TimeNoAnswMRK[i]=0;
      else      
       {if (TimeNoAnswMRK[i]>Time_10s)
         {SETB(DisplayNetwMRK,i);
 //отключение индикации аварий для МРК с отсутствием связи (кроме выработки ресурса) 
          CLRB(DisplayFailMRK[0],i);      //размыкание МРК
          CLRB(DisplayFailMRK[1],i);
          CLRB(DisplayFailMRK_copy[0],i); //размыкание МРК
          CLRB(DisplayFailMRK_copy[1],i);
         }
        else
         TimeNoAnswMRK[i]++;
       }
     }
//отказы MRО    
   for(i=0;i<6;i++)
     {if (CHKB(CONFIGMRO.OnCircMRO,i)==0)
       continue;
      if (CHKB(DisplayFailMRO,i)==0)
       TimeFailMRO[i]=0;
      else
       {if (TimeFailMRO[i]>Time_60s)
         SETB(DisplayAlarmMRO,i);
        else
         TimeFailMRO[i]++;
       }
     }
  }

/** @fn void CheckFail_MRK(void)
*   @brief Проверка  размыкания 16 МРК  по 2 ряда. 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    отказ, выработка ресурса, отказ мрк/нет связи, Срабатывание
*/

void CheckFail_MRK(void) //проверка  размыкания 16 МРК  по 2 ряда
  {uint8 i;
 //0бит -МР-К0 ...15бит -МР-К15. [0]-цепь0(ряд1), [1]-цепь1(ряд2)
   if ((DisplayFailMRK_copy[0]==0)&&(DisplayFailMRK_copy[1]==0))
    {CLRB(DisplayAllAlarm,FailMRK); 
      return;
    }
    
   for(i=0;i<16;i++)               //16 МРК 
    {if CHKB(DisplayFailMRK_copy[0],i)
      {CLRB(DisplayFailMRK_copy[0],i); //проиндицировали, следующий
       REGIME.NmbModule=1;             //номер ряда для найденного МР-К
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //номер секции для найденного МР-К
       REGIME.Mode=Alarm;              //Авария
       REGIME.SubMode=NoContr;         //отказ
       TimeSetDispl();
       return;
      }
     if CHKB(DisplayFailMRK_copy[1],i)
      {CLRB(DisplayFailMRK_copy[1],i);
       REGIME.NmbModule=2;             //номер ряда для найденного МР-К
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //номер секции для найденного МР-К
       REGIME.Mode=Alarm;              //Авария
       REGIME.SubMode=NoContr;         //отказ
       TimeSetDispl();
       return;
      }
    }
  }
 
/** @fn void CheckFail_MRO(void)
*   @brief Проверка  размыкания MRO. 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    отказ, выработка ресурса, отказ мрк/нет связи, Срабатывание
*/
void CheckFail_MRO(void) //проверка  размыкания MRO
   {uint8 i;
//индикация после индикации отключения MRK   
    i=DisplayAllAlarm&(0xFF<<FailMRO);//маска на большие приоритеты
    if ((i>(1<<FailMRO))||CHKB(DisplayAllAlarm,FailMRO==0))
     return;    
    if (DisplayFailMRO_copy==0) 
     {CLRB(DisplayAllAlarm,FailMRO); 
      return;
     }
    
    for(i=0;i<6;i++) //max 6 проверок если 1-ый NMBFail=0
     if CHKB(DisplayFailMRO_copy,i)
      {CLRB(DisplayFailMRO_copy,i);
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRO[i]; //№ секции- по 4 бита в ContrCircMRO 
       REGIME.NmbModule=(i&1)+1;      //номер ряда для найденного МР-К
       REGIME.Mode=Alarm;             //Авария
       REGIME.SubMode=NoContr;        //нет контроля
       TimeSetDispl();
       return;
      }
    
    }
 
/** @fn void CheckAlarm_MRK(void)
*   @brief проверка  отказов MRK. 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    выработка ресурса, отказ мрк/нет связи, Срабатывание
*/

void CheckAlarm_MRK(void) //проверка  отказов MRK
  {uint8 i;
   i=DisplayAllAlarm&(0xFF<<AlarmMRK);//маска на большие приоритеты
   if ((i>(1<<AlarmMRK))||CHKB(DisplayAllAlarm,AlarmMRK==0)) //индикация после индикации размыкания
    return;

   for(i=0;i<16;i++)               //для 16 МРК по 2 входа на каждый
    {if CHKB(DisplayAlarmMRK_copy[0],i)
      {CLRB(DisplayAlarmMRK_copy[0],i);
       REGIME.NmbModule=1; //номер ряда для найденного МР-К
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //номер секции для найденного МР-К
       REGIME.Mode=Alarm;             //Авария
       REGIME.SubMode=Breakdown;      //отказ
       TimeSetDisplLn();
       return;
      }
     else if CHKB(DisplayAlarmMRK_copy[1],i)
      {CLRB(DisplayAlarmMRK_copy[1],i);
       REGIME.NmbModule=2; //номер ряда для найденного МР-К
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //номер секции для найденного МР-К
       REGIME.Mode=Alarm;             //Авария
       REGIME.SubMode=Breakdown;      //отказ
       TimeSetDisplLn();
       return;
      }
     
     if ((DisplayAlarmMRK_copy[0]==0)&&(DisplayAlarmMRK_copy[1]==0))
      {CLRB(DisplayAllAlarm,AlarmMRK);
       return;
      }
    }
  }

/** @fn void CheckAlarm_MRO(void)
*   @brief проверка  отказов MRО. 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    выработка ресурса, отказ мрк/нет связи, Срабатывание
*/
void CheckAlarm_MRO(void) //проверка  отказов
   {uint8 i;
    i=DisplayAllAlarm&(0xFF<<AlarmMRO);//маска на большие приоритеты
    if ((i>(1<<AlarmMRO))||CHKB(DisplayAllAlarm,AlarmMRO==0)) //индикация после индикации отключения
     return;
    if (DisplayAlarmMRO_copy==0)
     {CLRB(DisplayAllAlarm,AlarmMRO);
      return;
     }

    for(i=0;i<6;i++) //max 6 проверок если 1-ый NMBFail=0
     if CHKB(DisplayAlarmMRO_copy,i)
      {CLRB(DisplayAlarmMRO_copy,i);
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRO[i]; //№ секции- по 4 бита в ContrCircMRO
       REGIME.NmbModule=(i&1)+1; //номер ряда для найденного МР-К
       REGIME.Mode=Alarm;             //Авария
       REGIME.SubMode=Breakdown;        //нет контроля
       TimeSetDisplLn();
       return;
      }
    }

/** @fn void CheckResource(void)
*   @brief проверка нараюотки более 80%. 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    отказ мрк/нет связи, Срабатывание
*/
void CheckResource(void) //проверка нараюотки более 80%
  {uint8 i, MRK, Module;
   i=DisplayAllAlarm&(0xFF<<ResourceMRK);//маска на большие приоритеты
   if ((i>(1<<ResourceMRK))||CHKB(DisplayAllAlarm,ResourceMRK==0)) //индикация после индикации отключения, отказов, 
    return; 
   
   for (i=0;i<16;i++)   //проверка по всем 16
    {MRK=DisplayResourceMRK_copy[i];
     if ((MRK!=0)&&(CHKB(CONFIGMRO.OnMRK,i)))
      goto NextCheck;
    }
   CLRB(DisplayAllAlarm,ResourceMRK);
   return;

NextCheck:
   for ( ;NMBResource<MaxSecMod; NMBResource++) //проверяем по модулям всех МР-К 
    {i=NMBResource>>3;                 //номер МР-К из всех модулей (16 МР-К * 8 модулей)
     if CHKB(CONFIGMRO.OnMRK,i)        //проверяем подключеный МР-К 
      {MRK=DisplayResourceMRK_copy[i]; //проверяем 8 модулей в одном МР-К
       Module=NMBResource-i*8;         //номер модуля из NMBResource
       if CHKB(MRK,Module)             //ищем модуль c наработкой >80%
        {if (TimeNoPress>0)      //Если нажата любая кнопка- (кнопка ВВОД) не переходим на следующий, если он есть, пока кнопка не будет отпущена
          CLRB(DisplayResourceMRK_copy[i],Module);
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //из № МР-К определяем № секции
         REGIME.NmbModule=Module;
         REGIME.Mode=Alarm;            //Переходим в Отказ, след. сраб. секцию не ищем 3 сек
         REGIME.SubMode=EndResource;   //ВЫРАБОТКА. РУСУРСА
         TimeSetDispl();
         return;                       //стоп когда нашли
        }
      }
     else
      NMBResource|=7; //следующий МРК (после NMBResource++ кратно 8)
    }
  }
 
/** @fn void CheckSrab(void)
*   @brief проверка срабатывания 
    
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    Если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm.
    Перекрывает: 
    отказ мрк/нет связи
*/
void CheckSrab(void) //проверка  срабатывания
 {uint8 i, MRK, Module;
//индикация после индикации отключения, отказов, наработки, нет в сети, 
  i=DisplayAllAlarm&(0xFF<<SrabSection);//маска на большие приоритеты
  if ((i>(1<<SrabSection))||CHKB(DisplayAllAlarm,SrabSection==0))
   return; // 
  
  for (i=0;i<16;i++)   //проверка по всем 16
    {MRK=DisplaySrabSection_copy[i];
     if ((MRK!=0)&&(CHKB(CONFIGMRO.OnMRK,i)))
      goto NextCheck;
    }
   CLRB(DisplayAllAlarm,SrabSection);
   return;

NextCheck:
//нужна периодич. обновление  DisplaySrabSection
//проверяем 4 секции по 8 модулей
  for (;NMBSrabSection<MaxSecMod; NMBSrabSection++ ) //проверяем по всем модулям всех МР-К кроме предыдущего
   { i=NMBSrabSection>>3;                 //номер МР-К из всех модулей (16 МР-К * 8 модулей)
     if CHKB(CONFIGMRO.OnMRK,i)           //проверяем подключеный МР-К 
      {MRK=DisplaySrabSection_copy[i];    //проверяем 8 модулей в одном МР-К
       Module=NMBSrabSection-i*8;         //номер модуля из NMBSrabSection
       if CHKB(MRK,Module)                //ищем сработавший модуль
        {CLRB(DisplaySrabSection_copy[i],Module);//сбрасываем бит для проверки последовательной индикации 
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //из № МР-К определяем № секции
         REGIME.NmbModule=Module;
         REGIME.Mode=Response;            //Переходим в Срабатывание, след. сраб. секцию не ищем 3 сек
         TimeSetDispl();
         return;                          //стоп когда нашли
        }
      }
     else
      NMBSrabSection|=7; //следующий МРК (после NMBResource++ кратно 8)
    }
  }
 
/** @fn void CheckNetw(void)
*   @brief проверка пропадание связи с каким-либо модулем МР-К 
   
    Если есть данный тип аварии- индикация, сброс бита в _copy, 
    Если все аварии данного типа проиндицированы- сброс бита в DisplayAllAlarm
  
*/
void CheckNetw(void) //пропадание связи с каким-либо модулем МР-К.
  {uint8 i;
   i=DisplayAllAlarm&(0xFF<<NetwMRK);//маска на большие приоритеты
   if ((i>(1<<NetwMRK))||CHKB(DisplayAllAlarm,SrabSection==0)) //индикация после индикации отключения, отказов, наработки,  
    return; 
   
   for (i=0;i<16; i++) //проверяем по всем модулям всех МР-К 
    {if CHKB(DisplayNetwMRK_copy,i)            //ищем сработавший модуль
      {CLRB(DisplayNetwMRK_copy,i);       //сбрасываем бит для проверки последовательной индикации 
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //из № МР-К определяем № секции
       REGIME.Mode=Alarm;          //Переходим в Отказ, след. сраб. секцию не ищем 3 сек
       REGIME.SubMode=NoNetw;      //ВЫРАБОТКА. РУСУРСА
       TimeSetDispl();
       break;                      //стоп когда нашли
      }
    }
   
   if (DisplayNetwMRK_copy==0)
    CLRB(DisplayAllAlarm,NetwMRK);
  }
  
/** @fn void TimeSetDispl(void)
*   @brief Стандартное время индикации для бегущей строки 

*/
//уменьшил время в 2 раза по сравнению с TimeSetDisplLn
void TimeSetDispl (void)
 {REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;
  REGIME.RunStringDelay=Time_1s;
  REGIME.RunStringNext=Time_1s;
  REGIME.DisplDelay=Time_2s;     //задержка на индикацию
 }

/** @fn void TimeSetDisplLn(void)
*   @brief Удлиненное время индикации для бегущей строки 

*/
void TimeSetDisplLn (void) //длинная индикация - 3 сек
 {REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;
  REGIME.RunStringDelay=Time_2s;
  REGIME.RunStringNext=Time_2s;
  REGIME.DisplDelay=Time_4s;     //задержка на индикацию
 }

/** @fn void DataDisplOrder(void)
*   @brief Данные для последовательной индикации всех типов аварий 

*/
void DataDisplOrder(void) //данные для последовательной индикации всех типов аварий:
  {uint8 i; //после индикации в _copy сброс бита (проиндицированного значения)
   if (DisplayAllAlarm!=0) //ждем окончания всех индикаций
    return;
   //uint16 OnMRK=CONFIGMRO.OnMRK;
//после индикации всех аварий по 3 сек -новый круг    
   DisplayAlarmMRK_copy[0]=DisplayAlarmMRK[0];  //ряд 1
   DisplayAlarmMRK_copy[1]=DisplayAlarmMRK[1];  //ряд 2
   if ((DisplayAlarmMRK_copy[0]!=0)||(DisplayAlarmMRK_copy[1]!=0))
    SETB(DisplayAllAlarm,AlarmMRK);
   DisplayAlarmMRO_copy=(DisplayAlarmMRO & CONFIGMRO.OnCircMRO);
   if (DisplayAlarmMRO_copy!=0)
    SETB(DisplayAllAlarm,AlarmMRO);
//[0] -ряд1, [1] - ряд2 для 16 МРК№ 15...0 

   ApplyMaskFailMRK(); //наложение масок использования и отключения входов МРК 
   DisplayFailMRK_copy[0]=DisplayFailMRK[0];
   DisplayFailMRK_copy[0]&=(~DisplayAlarmMRK_copy[0]);//после перехода размыкания DisplayFail в отказ DisplayAlarm: размыкания игнорируются и не индицируются
   DisplayFailMRK_copy[1]=DisplayFailMRK[1];
   DisplayFailMRK_copy[1]&=(~DisplayAlarmMRK_copy[1]);//после перехода DisplayFail в DisplayAlarm: размыкания игнорируются
   if ((DisplayFailMRK_copy[0]!=0)||((DisplayFailMRK_copy[1]!=0)))
    SETB(DisplayAllAlarm,FailMRK);
    
   DisplayFailMRO_copy=(DisplayFailMRO & CONFIGMRO.OnCircMRO);
   DisplayFailMRO_copy&=~DisplayAlarmMRO_copy;//после перехода DisplayFail в DisplayAlarm: размыкания игнорируются
   if (DisplayFailMRO_copy!=0)
    SETB(DisplayAllAlarm,FailMRO);
   
   DisplayNetwMRK_copy=(DisplayNetwMRK & CONFIGMRO.OnMRK); 
   if (DisplayNetwMRK_copy!=0)      
    SETB(DisplayAllAlarm,NetwMRK);
   
   for(i=0;i<16;i++)
    if CHKB(CONFIGMRO.OnMRK,i)
     {DisplaySrabSection_copy[i]=DisplaySrabSection[i]; //8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]
      DisplaySrabSection[i]=0; //переписали - очистили, т.к. это собитие однократное и редкое      
      if (DisplaySrabSection_copy[i]!=0)
       SETB(DisplayAllAlarm,SrabSection);
      DisplayResourceMRK_copy[i]=DisplayResourceMRK[i];
      if (DisplayResourceMRK_copy[i]!=0)
       SETB(DisplayAllAlarm,ResourceMRK);
     }
   
//Проверка приоритетного FailMRO или FailMRK: остальные отключаем
   if (CHKB(DisplayAllAlarm,FailMRO)||CHKB(DisplayAllAlarm,FailMRK))
    DisplayAllAlarm&=(1<<FailMRK)|(1<<FailMRO);

   NMBResource=0;
   NMBSrabSection=0;
   }
 
 /** @fn void ApplyMaskFailMRK(void)
*   @brief наложение масок использования и отключения входов МРК

*/
void ApplyMaskFailMRK(void) //
 {DisplayFailMRK[0]&=CONFIGMRO.OnMRK;
  DisplayFailMRK[0]&=CONFIGMRO.OnD3;
  DisplayFailMRK[1]&=CONFIGMRO.OnMRK;
  DisplayFailMRK[1]&=CONFIGMRO.OnD4;
 }
 
 
 
/** @fn void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size)
*   @brief Запись в EEPROM
    @param[in]  __eeprom *pEDATA: указатель на EEPROM
    @param[in]  *pDATA указатель на данные для записи
    @param[in]  Size размер данных
      
    Для сокращения времени записи - пишем только то что отличается
*/
void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size)
 {uint8 i;
  for (i=0; i<Size; i++)
   {if (*pEDATA!=*pDATA)//для сокращения времени - пищем только то что отличается
     *pEDATA=*pDATA;
    pEDATA++;
    pDATA++;
   }
 }

/** @fn void ClearDATAMRK(void)
*   @brief полная очистка данных МРК в ЕЕПРОМ

    0-ые данные защищаются  CRC
*/
void ClearDATAMRK(void) //полная очистка данных МРК в ЕЕПРОМ
 {uint8 i;
  uint8 EnEEPROM=0xA7;   //разрешение записи в еепром
  for (i=0; i<(sizeof(struct DataMRK)-2); i++)
   DATAMRK.Resource[i]=0;
  DATAMRK.CRC=CRC16(DATAMRK.Resource, sizeof (struct DataMRK)-2);
  for (i=0; i<16; i++)
   {__watchdog_reset();
    if (EnEEPROM==0xA7)
     WriteEEPROM(&EDATAMRK[i].DATAMRK.Resource[0], &DATAMRK.Resource[0], sizeof (struct DataMRK));
    else
     return;
   }
 }

/** @fn void ClearDATAEEPROM(void)
*   @brief полная очистка данных ЕЕПРОМ
   

*/
void ClearDATAEEPROM(void) //полная очистка данных МРК в ЕЕПРОМ
 {uint8 i;
  uint8 EnEEPROM=0xA5;   //разрешение записи в еепром
  for (i=0; i<(sizeof(struct ConfigMRO)); i++)
   CONFIGMRO.Type=0;
  if (EnEEPROM==0xA5)
   {//ECONFIGMRO[0].CONFIGMRO=CONFIGMRO;
    WriteEEPROM(&ECONFIGMRO[0].CONFIGMRO.Type, &CONFIGMRO.Type, sizeof (struct ConfigMRO));
    __watchdog_reset();
    //ECONFIGMRO[1].CONFIGMRO=CONFIGMRO;
    WriteEEPROM(&ECONFIGMRO[1].CONFIGMRO.Type, &CONFIGMRO.Type, sizeof (struct ConfigMRO));
    __watchdog_reset();
   }
  ClearDATAMRK();
 }

/** @fn void CopyDataDisplay(uint8 *Data)
*   @brief Копирование данных из буфера в Display с конвертацией в формат OLED дисплея
    @param[in] *Data указатель на данные

*/
void CopyDataDisplay(uint8 *Data) //копировать данные из буфера в Display с конвертацией в формат дисплея
 {uint8 ConvData, n, end=0;
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code.
  for (n=0,end=0;n<12;n++)
   {if (*(Data)==0)
     end=1; //после символа конца строки - запись пустых символов 0x20 
    if (end==0)
     ConvData=ConvFont(*(Data++)); //преобразование в формат OLED 
    else
     ConvData=0x20;
    
    Write4bitDispl(ConvData>>4);
    Write4bitDispl(ConvData&0x0F);
    
   }
 }
 
/** @fn void CopyDataExDisplay(uint8 *Data)
*   @brief Копирование данных из буфера в Display без конвертации
    @param[in] *Data указатель на данные

*/
void CopyDataExDisplay(uint8 *Data) //Копирование данных из буфера в Display без конвертации
 {uint8 n;
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code.
  for (n=0;n<12;n++)
   {Write4bitDispl((*Data)>>4);
    Write4bitDispl((*Data)&0x0F);
	Data++;
   }
 } 

/** @fn uint8 CREATE_TEXT(uint8 __flash *pLn, uint8 *pText)
*   @brief Копирование текста (12 символов) из FLASH в указанное место
    @param[in]  __flash *pLn указатель на текст в FLASH
    @param[in]  *pText указатель куда копировать
    \return  количество скопированных символов

*/ 
uint8 CREATE_TEXT (uint8 __flash *pLn, uint8 *pText) //из pLn в pText до 12 символов
 {uint8 n=0;
  do
   {*pText++ =*pLn++;
    n++;
   }
  while ((*pLn)&&(n<12));
   return n;
 }
 
 /** @fn uint8 InsNmb(uint8 Nmb, uint8 *pText)
*   @brief Замена символа N на Nmb. Для строки вида NA1-NA23 -> 1A1-1A23. Проверка до 12 знакомест
    @param[in]  Nmb - заменяемый символ
    @param[in]  *pText указатель куда копировать

*/ 
void ChangeNmb (uint8 Nmb, uint8 *pText) //
 {uint8 n=0;
  do
   {if (*pText=='N')
     *pText=convert_to_ASCII(Nmb);
    pText++;
    n++;
   }
  while ((*pText)&&(n<12)); 
 }
 

/** @fn uint8 CREATE_LINE1(uint8 __flash *pLn)
*   @brief Копирование текста (12 символов) из FLASH в LINE1 и далее в 1 строку дисплея
    @param[in]  __flash *pLn указатель на текст в FLASH
    \return  количество скопированных символов

    Дополнительно  в конце текста копируется инф-ия из ADDLINE.
    Формат: надпись, буквы(цифры) в char формате до 0 символа
*/
void CREATE_LINE1 (uint8 __flash *pLn) //формат: надпись, буквы(цифры) в char формате до 0 символа
 {uint8 n=0, i=0;
  do
   LINE1[n++] =*pLn++;
  while ((*pLn)&&(n<12));
  if (ADDLINE[0]!=0)
   while ((ADDLINE[i])&&(n<12)) 
    LINE1[n++]=ADDLINE[i++];  //копируем дополнит. инф-ию до конца строки
  LINE1[n] = 0;               //0-конец строки
  CopyDataLine(1);//копирует 2 в строку дисплея данные из LINE
 }
 
 
/** @fn uint8 CREATE_LINE2(uint8 __flash *pLn)
*   @brief Копирование текста (12 символов) из FLASH в LINE2 и далее в 2 строку дисплея
    @param[in]  __flash *pLn указатель на текст в FLASH
    \return  количество скопированных символов

    Дополнительно  в конце текста копируется инф-ия из ADDLINE.
    Формат: надпись, буквы(цифры) в char формате до 0 символа
*/ 
void CREATE_LINE2 (uint8 __flash *pLn) //формат: надпись, буквы(цифры) в char формате до 0 символа
 {uint8 n=0, i=0;
  do
   LINE2[n++] =*pLn++;
  while ((*pLn)&&(n<12));
  if (ADDLINE[0]!=0)
   while ((ADDLINE[i])&&(n<12)) 
    LINE2[n++]=ADDLINE[i++];
  LINE2[n] = 0;               //0-конец строки
  CopyDataLine(2);//копирует 2 в строку дисплея данные из LINE
 }
 
/** @fn void HEX_DEC(uint8 Data)
*   @brief Конвертация из HEX в DEC формат до 99
    @param[in]  Data данные HEX
    \return  данные DEC

*/ 
uint8 HEX_DEC (uint8 Data) //hex to dec 99
 {uint8 n=Data/10;
  return Data+n*6;
 } 

/** @fn uint16 INT_DEC(uint16 Data)
*   @brief Конвертация из INT в DEC формат до 999
    @param[in]  Data данные INT
    \return  данные DEC

*/ 
uint16 INT_DEC (uint16 Data) //hex to dec 999 (380cycles) 
 {uint8 n;
  n=Data/100;
  return (n<<8)+HEX_DEC(Data-n*100);
 } 
 
/** @fn uint8 CHAR_LINE (uint8 Data, uint8 *pLn)
*   @brief Конвертация из hex в текст формат дисплея до 99
    @param[in]  Data данные hex
    @param[in]  *pLn указатель на место куда писать

    \return  кол-во знакомест

*/ 
uint8 CHAR_LINE (uint8 Data, uint8 *pLn) //hex to text до 99 -> LINE
 {uint16 n;
  if (Data>99)
   {n=INT_DEC(Data);
    *pLn++=ConvHex(n>>8);
    *pLn++=ConvHex(n>>4);
    *pLn++=ConvHex(n);
    *pLn=0;
    return 3;
   }
  if (Data>9)
   {n=HEX_DEC(Data);
    *pLn++=ConvHex(n>>4);
    *pLn++=ConvHex(n);
    *pLn=0;          //стоп записи
    return 2; //номер след. знакоместа
   }
  else
   {*pLn++=ConvHex(Data);
    *pLn=0;          //стоп записи
    return 1; //номер след. знакоместа
   }
 }
 
/** @fn uint8 CHAR_ADDLINE (uint8 Data)
*   @brief Конвертация из hex в текст формат дисплея до 99 в ADDLINE
    @param[in]  Data данные hex

    \return  кол-во знакомест
*/ 
uint8 CHAR_ADDLINE (uint8 Data) //hex to text до 99 -> ADDLINE
 {return CHAR_LINE(Data, &ADDLINE[0]);
 }
 
/** @fn INT_ADDLINE (uint16 Data)
*   @brief Конвертация из INT в текст формат дисплея до 999 в ADDLINE
    @param[in]  Data данные hex

    \return  кол-во знакомест
*/ 
void INT_ADDLINE (uint16 Data) //hex to text до 999 -> ADDLINE
  {if (Data>999)
    {ADDLINE[0]='1';
     ADDLINE[1]='0';
     ADDLINE[2]='0';
     ADDLINE[3]='0';
     ADDLINE[4]=0;
     return;
    }
   uint16 n;
   n=INT_DEC(Data); //75
   ADDLINE[0]=ConvHex(n>>8);
   ADDLINE[1]=ConvHex(n>>4);
   ADDLINE[2]=ConvHex(n);
   ADDLINE[3]=0;
  }

 /** @fn DISPL_MRO_IN(void)
*   @brief Индикация включенных входов МРО во 2й строке
    для режима Check и TestIn
*/
void DISPL_MRO_IN(void) 
{uint8 i;
 if (REGIME.Mode==Check)
  {for (i=0;i<6;i++)
    {if (CHKB(CONFIGMRO.OnCircMRO,i)!=0)
      LINE2[i+6] =TDig[i];
    }
   if  (REGIME.Flash!=0) 
    LINE2[NmbFreeMROIn+5]='_'; //мигает номер для ЛЦ
  }
 else //test 
  {for (i=0;i<6;i++)
    {if (CHKB(PINA,i)!=0)
      LINE2[i*2] =TDig[i];
    }
  }
}   
 
/** @fn uint8 FindMinNmbSection(void)
*   @brief поиск наименьшего номера секции в CONFIGMRO
    \return  наименьший номера секции
*/ 
uint8 FindMinNmbSection(void) //поиск наименьшего номера секции
 {uint8 i, Nmb, NmbSection;
  Nmb=99;    //для поиска наименьшего номера секции
  for(i=0; i<16; i++) 
   {NmbSection=CONFIGMRO.NmbSectionMRK[i];
    if (NmbSection==0)
     continue;
    if (NmbSection<Nmb)
     Nmb=NmbSection;
    if (NmbSection==1)
     return 1;
   }
  return Nmb;
 }

/** @fn uint8 FindMaxNmbSection(void)
*   @brief поиск наибольшего номера секции в CONFIGMRO
    \return  наибольший номера секции
*/ 
uint8 FindMaxNmbSection(void) //поиск наибольшего номера секции
 {uint8 i, Nmb, NmbSection;
  Nmb=1;    //для поиска наибольшего номера секции
  for(i=0; i<16; i++) 
   {NmbSection=CONFIGMRO.NmbSectionMRK[i];
    if (NmbSection==0)
     continue;
    if (NmbSection>Nmb)
     Nmb=NmbSection;
   }
  return Nmb;
 }
 
/** @fn uint8 FindNmbMRK(void)
*   @brief поиск номера MRK по № секции
    \return  номер MRK
*/ 
uint8 FindNmbMRK(void)//поиск номера MRK по № секции
 {uint8 i;
  for(i=0; i<16; i++) //n- номер позиции в строке2
   {if (CONFIGMRO.NmbSectionMRK[i]==REGIME.NmbSection)
     return i;
   }
  return 0xFF;       
 }

//**************************************************************************
//             OLED DISPLAY
/** @fn void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink)
*   @brief Управление дисплеем
    @param[in]  On вкл дисплея
    @param[in]  Cursor вкл курсора
    @param[in]  Blink вкл мигания
*/ 
void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink)//запись 1DCB. DISPLAY D: ON=1/OFF=0. Cursor C: ON=1/OFF. Blinking B: of Cursor
 {
  CLRB(PORTB,RW);   //displ-WRITE
  DDRB_WRITE;
  
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  uint8 Data;
  Write4bitDispl(0x00);
  Data=(1<<3);
  if (On!=0)
   Data|=(1<<2); //вкл. индикацию
  if (Cursor!=0)
   Data|=(1<<1);  //вкл. курсор
  if (Blink!=0)
   Data|=(1<<0);  //вкл. мигание
  Write4bitDispl(Data);

 }

/** @fn void ClearDisplay(void)
*   @brief Очистка дисплея
    
*/ 
void ClearDisplay(void)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;  
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Write4bitDispl(0x00);
  Write4bitDispl(0x01);//Clear
//обязательно - задержка  
  Delay(20000);   //20ms
 
 }
/** @fn void HomeDisplay(void)
*   @brief Установка в 0 указателя дисплея
    
*/ 
void HomeDisplay(void)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Write4bitDispl(0x00);
  Write4bitDispl(0x02);//Home
 }

/** @fn void FunkSetDisplay(void)
*   @brief Режим дисплея -инструкции
    
*/ 
void FunkSetDisplay(void)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Write4bitDispl(0x02);
  Write4bitDispl(0x02);
  Write4bitDispl(0x0A);
//запись NF00 N: When N=1, the 2-line. When N =0, the 1-line.  F: When F=0,  5 x 8 dot character font
//(FT1, FT0) = (1, 0), the ENGLISH_RUSSIAN CHARACTER FONT TABLE
 }
 
/** @fn void Write4bitDispl(uint8 Data)
*   @brief Запись 4 бит в дисплей
    @param[in]  Data данные для записи
*/ 
void Write4bitDispl(uint8 Data)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;

  PORTB&=0xF0;
  PORTB|=(Data&0x0F);
  SETB(PORTC,E);
  Delay(5);
  CLRB(PORTC,E); //запись Data
  Delay(10);
 }

/** @fn uint8 Read8bitDispl(void)
*   @brief Чтение 8 бит из дисплея
    \return  прочитанный байт
*/ 
uint8 Read8bitDispl(void)
 {uint8 Data;
  DDRB_READ;
  SETB(PORTB,RW);   //displ-READ
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code.
  SETB(PORTC,E);
  Delay(10);
  Data=(PINB&0x0F)<<4;
  CLRB(PORTC,E); 
  Delay(10);
  SETB(PORTC,E);
  Delay(10);
  Data|=(PINB&0x0F);
  CLRB(PORTC,E); 
  Delay(10);
  return Data;
 }

/** @fn void SetAddrDisplay(uint8 Addr)
*   @brief Установка начального адреса дисплея 
    @param[in]  Addr адрес
*/
void SetAddrDisplay(uint8 Addr)//
 {CLRB(PORTB,RW);   //displ-WRITE  
  DDRB_WRITE;
  
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Addr&=0x7F;     // max 127 
  Write4bitDispl((Addr>>4)|8);
  Write4bitDispl(Addr&0x0F);
 }

/** @fn uint8 TestDisplay (void)
*   @brief Проверка работоспособности дисплея  
    \return  
                - 1 корректный ответ
                - 0 не корректный ответ
*/
uint8 TestDisplay (void)
 {uint8 Data=0;
  SetAddrDisplay(0x7F);
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code
  Write4bitDispl(0x05);
  Write4bitDispl(0x07);
  SetAddrDisplay(0x7F);
  Data=Read8bitDispl();
  
  if (Data==0x57)
   {
CLRB(PORTB,7); 
    return 1;
   }
  else
   {
SETB(DDRB,7);
SETB(PORTB,7);
    return 0; 
   }
 }

//****************************************************************************** 
/** @fn uint8 ConvFont(uint8 Letter)
*   @brief Конвертация символа для дисплея   
    @param[in]  Letter Входной символ
    \return  код символа для дисплея  
                
*/
uint8 ConvFont(uint8 Letter)
{if (Letter=='_')
  return 0x20; //пробел
 else if (Letter=='`')
  return 0xFF;  //горят все LED  
 else if (Letter<0xC0)
  return Letter;
 else
  switch (Letter)
   {case 'А': return 'A'; 
    case 'Б': return 0xA0;
    case 'В': return 'B';
    case 'Г': return 0xA1;
    case 'Д': return 0xE0;
    case 'Е': return 'E';
    case 'Ё': return 0xA2;
    case 'Ж': return 0xA3;
    case 'З': return 0xA4;
    case 'И': return 0xA5;
    case 'Й': return 0xA6;
    case 'К': return 'K';
    case 'Л': return 0xA7;
    case 'М': return 'M';
    case 'Н': return 'H';
    case 'О': return 'O';
    case 'П': return 0xA8;
    case 'Р': return 'P';
    case 'С': return 'C';
    case 'Т': return 'T';
    case 'У': return 0xA9;
    case 'Ф': return 0xAA;
    case 'Х': return 'X';
    case 'Ц': return 0xE1;
    case 'Ч': return 0xAB;
    case 'Ш': return 0xAC;
    case 'Щ': return 0xE2;
    case 'Ъ': return 0xAD;
    case 'Ы': return 0xAE;
    case 'Ь': return 0xC4;
    case 'Э': return 0xAF;
    case 'Ю': return 0xB0;
    case 'Я': return 0xB1;
    
    case 'а': return 'a';
    case 'б': return 0xB2;
    case 'в': return 0xB3;
    case 'г': return 0xB4;
    case 'д': return 0xE3;
    case 'е': return 'e';
    case 'ё': return 0xB5;
    case 'ж': return 0xB6;
    case 'з': return 0xB7;
    case 'и': return 0xB8;
    case 'й': return 0xB9;
    case 'к': return 0xBA;
    case 'л': return 0xBB;
    case 'м': return 0xBC;
    case 'н': return 0xBD;
    case 'о': return 'o';
    case 'п': return 0xBE;
    case 'р': return 'p';
    case 'с': return 'c';
    case 'т': return 0xBF;
    case 'у': return 'y';
    case 'ф': return 0xE4;
    case 'х': return 'x';
    case 'ц': return 0xE5;
    case 'ч': return 0xC0;
    case 'ш': return 0xC1;
    case 'щ': return 0xE6;
    case 'ъ': return 0xC2;
    case 'ы': return 0xC3;
    case 'ь': return 0xC4;
    case 'э': return 0xC5;
    case 'ю': return 0xC6;
    case 'я': return 0xC7;
    default: return 0x20; //пробел
   }
 }

/** @fn uint8 ConvHex(uint8 Data)
*   @brief Конвертация hex 0...F в текст   
    @param[in]  Data hex 0...F
    \return  код символа 
                
*/
uint8 ConvHex(uint8 Data) //hex to text до 15
 {Data&=0x0F;
  if (Data<10)
   return Data+0x30;
  else
   switch (Data)
   {case 0x0A: return 'A';
    case 0x0B: return 'B';
    case 0x0C: return 'C';
    case 0x0D: return 'D';
    case 0x0E: return 'E';
    case 0x0F: return 'F';
    default: return 0x20; //пробел
   }
 }
//****************************************************************************** 
//Обслуживание Кнопок 
 
/** @fn void Poll_button (void)
*   @brief Опрос кнопок   
    
    Вызываются ф-ии:
  - чтение сетевого адреса МР-О, установленного перемычками для опроса системы 
    АПК-ДК по RS-485 CheckAddress(void). При изменении адреса и подтверждении его 
    значения долее 2 секунд - переменной адреса ReadAddr присваивается новый адрес и 
    также он записывается в ЕЕПРОМ в EAddrNet
  - проверка времени не нажатия кнопок для выключения экрана и перевод МР-О в исходный режим работы Ish CheckTimeNoPress()
  - проверка времени нажатия кнопок с присвоением каждой кнопке в структуре BUTTON значения времени нажатия Check_Button() 
  - Проверка времени одновременного нажатия 2-х кнопок (UP и DWN) с присвоением в структуре BUTTON значения времени нажатия
*/ 
void Poll_button (void)
 {CheckAddress();     //чтение адреса
  CheckTimeNoPress(); //выкл. при превышении времени не нажатия (для режима > Check2)
    
  Check_Button(KEY1,&BUTTON.TimeChoice, &BUTTON.Choice);
  Check_Button(KEY2,&BUTTON.TimeEsc, &BUTTON.Esc);
  Check_Button(KEY3,&BUTTON.TimeUp, &BUTTON.Up);
  Check_Button(KEY4,&BUTTON.TimeDwn, &BUTTON.Dwn);
  
//проверка 2х кнопок >5сек
  if ((CHKB(PINC,KEY3)==0)&&(CHKB(PINC,KEY4)==0)) //кнопка нажата
   {BUTTON.TimeUp=0;
    BUTTON.TimeDwn=0;//если нажаты обе кнопки ,по отдельности - сбрасываем
    TimeNoPress=0; //время не нажатия
    BUTTON.TimeUpDw+=1;
    
    if (BUTTON.TimeUpDw>=Delay_6s) //срабатывание- по достижению интервала 10s
     {BUTTON.Press=Push;
      BUTTON.UpDwn=Push_6s;
      BUTTON.TimeUpDw=0;
     }
    }
   else
    BUTTON.TimeUpDw=0;  
 }

/** @fn void CheckTimeNoPress (void)
*   @brief выкл. при превышении времени не нажатия для режима > Check2  
    
    Проверка времени не нажатия кнопок для выключения экрана и перевод МР-О в исходный режим работы Ish
*/
void CheckTimeNoPress(void) //выкл. при превышении времени не нажатия
 {Period_1s++;
  if ((Period_1s>Delay_1s)&&(REGIME.Mode>Check)&&(REGIME.Mode!=Alarm)&&(REGIME.Mode!=Config0)) //каждые 1s
   {Period_1s=0;
    TimeNoPress++;            //время не нажатия
    if (TimeNoPress>=0xFF)
     TimeNoPress=6;           //
    
    if ((REGIME.Mode==Alarm)&&(REGIME.SubMode==EndResource))
     {if (TimeNoPress>2)        //5s интервал для откл. индикации аварий после нажатия кнопки только для ситуации конца ресурса 
       REGIME.PermitAlarm=1;    //разрешение индикации аварий 
     }
    else
     REGIME.PermitAlarm=1;
     
    if (REGIME.Mode==Test)
     return;
    
    if ((TimeNoPress==15)&&(REGIME.Mode!=Alarm))      //15s
     REGIME.Mode=Ish;         //через 15 сек и раз в 256 сек выкл дисплей
   }
 }

/** @fn void CheckAddress (void)
*   @brief чтение сетевого адреса  МР-О 
    
    чтение сетевого адреса МР-О, установленного перемычками для опроса системы АПК-ДК по RS-485. 
    При изменении адреса и подтверждении его значения долее 2 секунд - переменной адреса ReadAddr 
    присваивается новый адрес и также он записывается в ЕЕПРОМ в EAddrNet
*/
void CheckAddress(void) //чтение адреса
 {uint8 NewAddr;
  NewAddr=(PINE>>3);
  NewAddr|=((PING&1)<<5);
  if (NewAddr==EAddrNet)//адрес не изменился
   {TimeAddr=0;
    return;
   }
  
  if (ReadAddr!=NewAddr)
   {ReadAddr=NewAddr;   
    TimeAddr=0;
   }
  else  
   {TimeAddr++;
    if ((TimeAddr>Delay_2s)&&(EAddrNet!=ReadAddr))
     EAddrNet=ReadAddr;
   }
 }

/** @fn void Check_Button(uint8 Key, uint16 *Time, uint8 *Button )
*   @brief Проверка времени нажатия кнопок с присвоением каждой кнопке в структуре 
    BUTTON значения времени нажатия.

    @param[in]  Key Входной сигнал для кнопки
    @param[in]  *Time время нажатия
    @param[in] *Button Название кнопки

    \return  *Button=*Time
*/
void Check_Button(uint8 Key, uint16 *Time, uint8 *Button )
 {if (CHKB(PINC,Key)==0)   //кнопка нажата
   {TimeNoPress=0;         //время не нажатия
    REGIME.PermitAlarm=0;  //запрет включения индикации аварий
    if  (*Time<Delay_15s)
     (*Time)+=1;
//команды 2 сек. подаются при нажатой кнопке по достижению этого времени
    if (*Time==Delay_1p5s)//когда нажата и =2сек ,считаем что это нажатие 2с  
     {*Button=Push_2s;
      BUTTON.Press=Push;
//      *Time=0;
     }
    else if (*Time==Delay_6s)
     {*Button=Push_6s;
      BUTTON.Press=Push;
     }
    else if (*Time==Delay_10s)
     {*Button=Push_10s;
      BUTTON.Press=Push;
     }     
   }
  else if (*Button==Off)   //кнопку нажали - отпустили
   {
//команда Push подается при отпускании кнопки     
    if ((*Time>Delay_01s)&&(*Time<Delay_1p5s))   //была нажата 2>t>0.1 сек, потом отпустили
     {*Button=Push;
      BUTTON.Press=Push;
     }
    *Time=0;
   }
 }

/** @fn void Poll_Regime(void)
*   @brief Действия на нажатия кнопок

    Для различных режимов индикации МР-О (переменная REGIME.Mode)  и подрежимов (переменная REGIME.SubMode) 
    работы МР-О проверяется время нажатия кнопок (структура  BUTTON) и осуществляется:
  - переход в другие режимы работы
  - Изменение переменных номера секций, модулей и т.п.
  - Выполнение ф-ий подготовки конфигурирования
  - Индикация режима стирания значений выработки ресурса или количества срабатываний в ЕЕПРОМ, само стирание  ЕЕПРОМ в подпрограммах очистки накопленных данных с МР-К в ЕЕПРОМ ClearDATAMRK() или ClearDATAEEPROM() - очистки конфигурации МР-О в ЕЕПРОМ с очисткой данных МРК в ЕЕПРОМ.
  - Очистка структуры BUTTON после ее предыдущей обработки 
*/
 
void Poll_Regime (void) //действия на нажатия кнопок
 {if (BUTTON.Press==Off)
   return;
  uint8 n, DATA;
  uint8 EnEEPROM=0;   //разрешение записи в еепром
  switch (REGIME.Mode)
   {case Plug:    //проверяем полное стирание
/*для глобального стирания: 1.при включении питания нажать Выбор>5сек. 
2.выбрать стирать МРК или весь ЕЕПРОМ. 3. Нажать UpDwn> 2 сек.  */  
     if ((PrepClear==0xA5)&&(BUTTON.Choice>=Push_10s))
      {PrepClear=0;
       REGIME.Mode=Clear;
       REGIME.SubMode=Prepare;
       REGIME.ClearMode=ClearMRK;
	   break;
      }
	 if ((PrepTest==0xAA)&&(BUTTON.Esc>=Push_6s))
      {REGIME.Mode=Test;
       REGIME.SubMode=TestPrep;
	   break;
      } 
    
    case Config0:
     if ( ((CONFIGMRO.Error!=0)&&((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push)||(BUTTON.UpDwn>=Push)) )   //1-я конфигурация, ждем нажатия кнопок up/dwn
          ||(BUTTON.UpDwn>=Push_6s) )
      {REGIME.Mode=Config;
       REGIME.EndCheck=0;
       REGIME.SubMode=VZU_E;
      }
     break;
    
    case Config:         //Конфигурация
     n=REGIME.NmbSection;
        switch (REGIME.SubMode)
         {case VZU_E:
          case VZU_M:
          case VZU_H:
          case VZU_TM:
               if (BUTTON.Up>=Push)
                {if(REGIME.SubMode<VZU_TM) //Изменение  Е  или М-"Up или Dwn". 
                  REGIME.SubMode++;
                 else  
                  REGIME.SubMode=VZU_E;
                 }
               else if  (BUTTON.Dwn>=Push)
                {if (REGIME.SubMode!=VZU_E)
                  REGIME.SubMode--;
                 else
                  REGIME.SubMode=VZU_TM;
                 }
               else if (BUTTON.Choice>=Push)
                {CONFIGMRO.Type=REGIME.SubMode;
                 if ( (REGIME.SubMode==VZU_E)||(REGIME.SubMode==VZU_M) )
                  REGIME.SubMode=Side;
                 else                   //для Н и ТМ
                  PrepSearsh();     //поиск МРК и переход в Check
                }
               else if (BUTTON.Esc>=Push)
                REGIME.Mode=Plug;
          break;
          
          case Side:    //сторона для -Е, кол-во рядов(секций) для -М
               if (BUTTON.Up>=Push)
                ChoiceSideUp();     //изменение в CONFIGMRO Side
               else if (BUTTON.Dwn>=Push)
                ChoiceSideDwn();    //изменение в CONFIGMRO Side
               else if (BUTTON.Choice>=Push_2s)
                {if (CONFIGMRO.Type==VZU_E)
                  PrepSearsh(); //подготовка к поиску МРК
                 else
                  {REGIME.SubMode=Width;
                   if (CHKB(AllowWidth[CONFIGMRO.Side-1],0)==0) //проверка разрешенных
                    {CONFIGMRO.Width=2;
                     if (CHKB(AllowWidth[CONFIGMRO.Side-1],1)==0)
                      CONFIGMRO.Width=3;
                    }
                  }
                }
               else if (BUTTON.Esc>=Push_2s)
                {REGIME.Mode=Config;
                 REGIME.SubMode=CONFIGMRO.Type; //конфигурация МРО: VZU_E/VZU_M
                 CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;  //восстановление данных из еепром              
                }
           break;
           
           case Width:      //для -М
              if (BUTTON.Up>=Push)
                CONFIGMRO.Width=ChoiceUp(CONFIGMRO.Width, AllowWidth[CONFIGMRO.Side-1]);     //изменение в CONFIGMRO Width для ранее выбранного типа М
               else if (BUTTON.Dwn>=Push)
                CONFIGMRO.Width=ChoiceDwn(CONFIGMRO.Width, AllowWidth[CONFIGMRO.Side-1]);     //изменение в CONFIGMRO Width для ранее выбранного типа М
               else if (BUTTON.Choice>=Push_2s)
                 {REGIME.SubMode=Circuit;
                  if (CHKB(AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1],0)==0) //проверка разрешенных
                   {CONFIGMRO.Circuit[0]=LCS;
                    if (CHKB(AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1],1)==0) 
                     CONFIGMRO.Circuit[0]=RC;
                   }
                 }
               else if (BUTTON.Esc>=Push_2s)
                REGIME.SubMode=Side;
           break;
           
           case Circuit:    //для -М    //изменение в CONFIGMRO Width для ранее выбранного типа М и ширины Width
               if (BUTTON.Up>=Push)
                CONFIGMRO.Circuit[0]=ChoiceUp(CONFIGMRO.Circuit[0], AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1]);     //изменение в CONFIGMRO Circuit РЦ/ЛЦС/ЛЦ для ранее выбранного типа М
               else if (BUTTON.Dwn>=Push)
                CONFIGMRO.Circuit[0]=ChoiceDwn(CONFIGMRO.Circuit[0], AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1]);    //изменение в CONFIGMRO Circuit РЦ/ЛЦС/ЛЦ для ранее выбранного типа М
               else if (BUTTON.Choice>=Push_2s)
                PrepSearsh(); //подготовка к поиску МРК
               else if (BUTTON.Esc>=Push_2s)
                REGIME.SubMode=Width;
           break;
         }
   break;
   
   case Check: //Проверка для H, TM, М и Е
          n=REGIME.NmbSection;
          if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) ) //для -H, TM 
           { if ( (BUTTON.Up==Push)||(BUTTON.Dwn==Push) ) 
               {if (CHKB(CONFIGMRO.OnMRK,n)==0)//нет МРК для данной секции
                 {if (CHKB(LCOn,n-1)==0)
                   SETB(LCOn,n-1); //ЛЦ
                  else
                   CLRB(LCOn,n-1);  //Нет секции
                  
                  if (NmbFreeMROIn>6)               //всего 6 входов, если закончились - сброс
                   CLRB(LCOn,n-1);  //Нет секции
                 }
                else 
                 CLRB(LCOn,n-1);  //есть МРК для данной секции  
               }
            }
          
        if (REGIME.ErrorConfig==1)
         {if (BUTTON.UpDwn>=Push_6s)
           {REGIME.Mode=Plug;
            REGIME.EndCheck=0;
            REGIME.DisplDelay=Time_1s;
            ECONFIGMRO[0].CONFIGMRO.Error=0XFF;//запрет выхода из ожидания конфигурирования, т.к. была ошибка, и надо провести полный цикл конфигурации
           }
         }
        else if ( ((BUTTON.Choice>=Push_2s)&&(REGIME.NmbSection==FindMaxNmbSection()))
            || ( (BUTTON.Choice>=Push_2s)&&(CONFIGMRO.Type==VZU_M)&&(CONFIGMRO.Side==M2)&&(CONFIGMRO.Width==W630)&&(CONFIGMRO.Circuit[0]==RC) ) )//уникальный случай - в М2-630-РЦ одна 1 секция  
          {if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.EndCheck==0) )//Проверяем была ли сохранена последняя  8 секция
             break;
           REGIME.Mode=Ish;
 //для последней секции если не нажимали UP, сохраняем ее         
          // if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
           // if ( (CHKB(CONFIGMRO.Side,REGIME.NmbSection-1)!=0)&&(NmbFreeMROIn<7) ) //если выбран ЛЦ и 6 входов
            // CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //присваиваем номеру входа МРО - номер секции, к-ую он контролирует
            
//записываем последнюю 8 секцию если не сохранили по нажатию кнопки UP
            if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.NmbSection==FindMaxNmbSection())&&(CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0) )
             if (NmbFreeMROIn<7) //6 входов
               CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //присваиваем номеру входа МРО - номер секции, к-ую он контролирует

//подтверждение окончания проверки данных -запись корректного CRC в  ECONFIGMRO
//окончательное формирование входов МРО
           CONFIGMRO.OnCircMRO&=OnCircMRO;
           CONFIGMRO.OnD4&=OnD4;//приведение контроля выводов МРК в соответствии с тем что настроен опри конфигурировании 
           SaveNmbSection();  //сохраняем в ЕЕПРОМ Номера секции для МРК0…МРК3, МРО 
           //CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;
           CONFIGMRO.CRC=CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-5); //запись 
           ECONFIGMRO[0].CONFIGMRO.CRC=CONFIGMRO.CRC;
           ECONFIGMRO[1].CONFIGMRO.CRC=CONFIGMRO.CRC;
          }
      else if ( (CONFIGMRO.Type==VZU_M)&&(CONFIGMRO.Side==M2)&&(CONFIGMRO.Width==W630)&&(CONFIGMRO.Circuit[0]==RC))
       break;//уникальный случай - в М2-630-РЦ на 2 секции на самом деле одна 1я секция
      else if ( (BUTTON.Up>=Push_2s)&&(REGIME.ErrorConfig!=1) )
          {//для последней секции, если она сохранена, бит окончания проверки EndCheck
              if (REGIME.NmbSection==FindMaxNmbSection())
               REGIME.EndCheck=1;
              else 
               REGIME.EndCheck=0;
              
           if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.NmbSection<=FindMaxNmbSection())&&(CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0) ) //для -H, TM установка ЛЦ ил нет (если это не МРК)
            {if (CHKB(LCOn,REGIME.NmbSection-1)!=0) //если выбран ЛЦ 
              {if (NmbFreeMROIn<7) //6 входов
                {CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //присваиваем номеру входа МРО - номер секции, к-ую он контролирует
                 for (NmbFreeMROIn++;NmbFreeMROIn<7;NmbFreeMROIn++)//ищем след. свободный вход МРО
                   {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
                     break;
                   }
                  
                 }
              }
             uint8 NmbInMRO, NmbInMRK;             //незадействованные свободные входы
             for (NmbInMRO=0,n=NmbFreeMROIn-1;n<7;n++)
              {if (CHKB(CONFIGMRO.OnCircMRO,n)!=0)
                NmbInMRO++;
              }
             for (NmbInMRK=0,n=REGIME.NmbSection;n<9;n++) //число МРК влияющих на оставшиеся входы МРО (ели NmbSection>MRK, то эти МРК не влияют)
              {if (CHKB(CONFIGMRO.OnMRK,n)!=0)
                NmbInMRK++;
              }
             if ( NmbInMRO>(8-REGIME.NmbSection-NmbInMRK) )
              REGIME.ErrorConfig=1;       //если незадействованных свободных входов < оставшихся секций -ошибка 
            }

           if (NmbFreeMROIn>6)
             NmbFreeMROIn=6; 
           REGIME.Mode=Check;  //проверка след секции (для Е и М, для H, TM -постоянно в этом режиме
           if (REGIME.NmbSection<FindMaxNmbSection() ) 
            REGIME.NmbSection++;
          }
      else if ((BUTTON.Esc>=Push_2s)&&(REGIME.ErrorConfig!=1)&&(REGIME.NmbSection!=FindMinNmbSection()))
          {if (REGIME.EndCheck==0)
            REGIME.NmbSection--; //проверка пред секции
           REGIME.EndCheck=0;
          //для Н и ТМ надо вычесть  NmbFreeMROIn
           if (CHKB(LCOn,REGIME.NmbSection-1)!=0) //если выбран ЛЦ, или занят под МРК
            for (NmbFreeMROIn--;NmbFreeMROIn!=1;NmbFreeMROIn--)//ищем пред. свободный вход МРО
             {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
               break;
             }
          }
   break;
   
   case Ish:        //Исходный   
       if (BUTTON.Choice==Push)   //Режим помодульного просмотра
         {REGIME.Mode=Inform;
          REGIME.SubMode=Modul_cect;
         }
       else if (BUTTON.Choice>=Push_2s)//Режим просмотра выработки ресурса и количества срабатываний
         {REGIME.Mode=Inform;
          REGIME.SubMode=Resource_res;
         }
/*       
//test
else if (BUTTON.Up>=Push)
 {REGIME.SubMode=DisplAPK;
  REGIME.DisplDelay=Time_2s;     //задержка на индикацию
 }
*/
       if ((REGIME.Mode==Inform)&&(CONFIGMRO.OnMRK==0))  //не подключен ни один МРК
        REGIME.DisplDelay=Time_2s;     //задержка 2 сек если модуль не подкл
        REGIME.NmbSection=FindMinNmbSection();  //наименьший номер секции
        REGIME.NmbModule=0;                     //наименьший номер секции
   break;
   
   case Inform:   //Просмотр
      switch (REGIME.SubMode)
       {case Modul_cect:                           //выбор секции
         if (BUTTON.Up>=Push)
          {n=FindMaxNmbSection();
           if (REGIME.NmbSection<n)
            REGIME.NmbSection++;
           else
            REGIME.NmbSection=FindMinNmbSection();  //наименьший номер секции
          }
         else if (BUTTON.Dwn>=Push) //секции начинаются с 1
          {n=FindMinNmbSection();  //наименьший номер секции
           if (REGIME.NmbSection>n)
            REGIME.NmbSection--;
           else
            REGIME.NmbSection=FindMaxNmbSection();  //наибольший номер секции
          }
         else if (BUTTON.Choice>=Push)    //детальный просмотр
          REGIME.SubMode=Modul_mod;
         else if (BUTTON.Esc>=Push)    //выход
          REGIME.Mode=Ish;
        break; 
        
        case Modul_mod:                   //выбор модуля в секции NmbSection
         if (BUTTON.Up>=Push)
          {if (REGIME.NmbModule<(MaxModule-1))//макс 8 модулей
            REGIME.NmbModule++;
           else
            REGIME.NmbModule=0;
          }
         else if (BUTTON.Dwn>=Push)
          {if (REGIME.NmbModule!=0)
            REGIME.NmbModule--;
           else
            REGIME.NmbModule=MaxModule-1;
          }
         else if (BUTTON.Choice>=Push)    //детальный просмотр
          {REGIME.SubMode=Modul_inf;
           REGIME.DisplDelay=Time_2s;     //задержка 2 сек если модуль не подкл
          }
         else if (BUTTON.Esc>=Push_2s)    //выход
          REGIME.Mode=Ish;
         else if (BUTTON.Esc>=Push)       //переход в смену № модуля в секции
          REGIME.SubMode=Modul_cect;
        break;
        
        case Modul_inf:                   //просмотр модуля
         if (BUTTON.Esc==Push)
          REGIME.SubMode=Modul_mod;
         else if (BUTTON.Esc>=Push_2s)
          REGIME.Mode=Ish;
         else if (BUTTON.Choice>=Push_6s)
          {REGIME.Mode=Clear;
           REGIME.SubMode=Prepare;
           REGIME.ClearMode=ClearRes;
           REGIME.DisplDelay=Time_3s;
          }
         break;
/*         
//test
else if (BUTTON.Up>=Push)
 REGIME.SubMode=Modul_ADC;         
        break;
        
case Modul_ADC:                   //просмотр АЦП
if (BUTTON.Esc>=Push)
 REGIME.SubMode=Modul_cect;
break;        
*/        
        case Resource_res:                //выбор срабатывание/ресурс
         if ((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push))
          REGIME.SubMode=Resource_srab;
         else if (BUTTON.Choice>=Push)
          {REGIME.SubMode=Resource_inf_res;
           REGIME.NmbRes=0;  //номер с макс. срабатыванием
           goto Res;
          }
         else if (BUTTON.Esc>=Push)
          REGIME.Mode=Ish;
        break;
         
        case Resource_srab: 
         if ((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push))
          REGIME.SubMode=Resource_res;
         else if (BUTTON.Choice>=Push)
          {REGIME.SubMode=Resource_inf_srab;
           REGIME.NmbSrab=0;  //номер с макс. срабатыв.
           goto Srabat;
          }
         else if (BUTTON.Esc>=Push)
          REGIME.Mode=Ish;
        break;
      
        case Resource_inf_res:
         if (BUTTON.Esc==Push)
          {REGIME.SubMode=Resource_inf_srab;
           REGIME.NmbSrab=0;  //номер с макс. срабатыванием
           goto Srabat;
          }
         else if (BUTTON.Esc>=Push_2s)
          REGIME.Mode=Ish;
         else if ((BUTTON.Dwn>=Push)&&(REGIME.NmbRes<MaxSecMod))
           REGIME.NmbRes++;
         else if ((BUTTON.Up>=Push)&&(REGIME.NmbRes!=0))
          REGIME.NmbRes--;
         /* отсюда- не стираем
         else if (BUTTON.UpDwn>=Push_2s)
          {REGIME.Mode=Clear;
           REGIME.SubMode=Prepare;
           REGIME.ClearMode=ClearRes;
           //REGIME.ClearMode=ClearResSrab;//стираем все
           REGIME.DisplDelay=Time_3s;
          }
         */
Res:         
         SORT_Res(); //сортировка по ресурсу,
         DATA=IndexResourse[REGIME.NmbRes];//0...3bit -MRK, 4..7bit- Module in section
         if ((DATA==0xFF)&&(REGIME.NmbRes!=0)) //F секция, F модул - нет такого, т.е. конец списка
          {REGIME.NmbRes--;
           DATA=IndexResourse[REGIME.NmbRes];//0...3bit -MRK, 4..7bit- Module in section
          }
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[(DATA>>4)];
         REGIME.NmbModule=DATA&0X0F;     //Module in section
        break;
        
        case Resource_inf_srab:
         if (BUTTON.Esc==Push)
          {REGIME.SubMode=Resource_inf_res;
           REGIME.NmbRes=0;  //номер с макс. срабатыванием
           goto Res;
          }
         else if (BUTTON.Esc>=Push_2s)
          REGIME.Mode=Ish;
         else if ((BUTTON.Dwn>=Push)&&(REGIME.NmbSrab<MaxSecMod))
           REGIME.NmbSrab++;
         else if ((BUTTON.Up>=Push)&&(REGIME.NmbSrab!=0))
          REGIME.NmbSrab--;
         /*
         else if (BUTTON.UpDwn>=Push_2s)
          {REGIME.Mode=Clear;
           REGIME.SubMode=Prepare;
           REGIME.ClearMode=ClearSrab;
           REGIME.DisplDelay=Time_3s;
          }
         */
Srabat:         
         Event=1;
         if (SORT_Srab()==0) //сортировка по срабатываниям
          {Event=0;                 //нет срабатываний
           break;
          }
         DATA=IndexSrab[REGIME.NmbSrab];//0...3bit -MRK, 4..7bit- Module in section
         if ((DATA==0xFF)&&(REGIME.NmbSrab!=0)) //F секция, F модул - нет такого, т.е. конец списка
          {REGIME.NmbSrab--;
           DATA=IndexSrab[REGIME.NmbSrab];//0...3bit -MRK, 4..7bit- Module in section
          }
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[(DATA>>4)];
         REGIME.NmbModule=DATA&0X0F;     //Module in section
        break;
       }
   break;
   
   case Response: //Срабатывание
   break;
   
   case Alarm:
      if (REGIME.SubMode==EndResource)//ресурс >80%
       {if (BUTTON.Choice>=Push_10s)
         {REGIME.Mode=Clear;
          REGIME.SubMode=Prepare;
          REGIME.ClearMode=ClearRes;
          REGIME.DisplDelay=Time_3s;
         }
        break;
       }
      else if (BUTTON.Choice==Push)   //Режим помодульного просмотра
       {REGIME.Mode=Inform;
        REGIME.SubMode=Modul_cect;
       }
      else if (BUTTON.Choice>=Push_2s)//Режим просмотра выработки ресурса и количества срабатываний
       {REGIME.Mode=Inform;
        REGIME.SubMode=Resource_res;
       }
      n=FindMinNmbSection();  //наименьший номер секции
      if (REGIME.NmbSection<n)   
       REGIME.NmbSection=n;
   break;
   
   case Clear:
      if (BUTTON.Esc>=Push)
       {REGIME.ClearMode=No;
        REGIME.Mode=Ish;
        break;       
       }
      else if (REGIME.SubMode==Prepare)
       {if ((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push))
         {if (REGIME.ClearMode==ClearMRK)
           REGIME.ClearMode=ClearEEPROM;
          else if (REGIME.ClearMode==ClearEEPROM)
           REGIME.ClearMode=ClearMRK;
         }
        else if (BUTTON.UpDwn>=Push_2s)
         {if (REGIME.ClearMode==ClearMRK)
//т.к. процесс полного стирания длительный, начать индикацию надо здесь,           
           {CREATE_LINE1(&TClear[7][0]);     //Стирание...
            CREATE_LINE2(&TClear[4][0]);     //СРАБАТЫВ
            ClearDATAMRK();                //полная очистка данных МРК в ЕЕПРОМ
           }
          else if (REGIME.ClearMode==ClearEEPROM)
           {CREATE_LINE1(&TClear[7][0]);     //Стирание...
            CREATE_LINE2(&TClear[3][0]);     //ВЕСЬ_ЕЕПРОМ
            ClearDATAEEPROM();                //полная очистка конфигурации ЕЕПРОМ +данных МРК
           }
          else
           {switch (REGIME.ClearMode)
            {//case ClearRes:
             case ClearSrab:
             case ClearResSrab:
              EnEEPROM=0xB6;
              n=FindNmbMRK();
              DATAMRK=EDATAMRK[n].DATAMRK;
              DATAMRK.SrabMod[REGIME.NmbModule]=0;
              DATAMRK.Resource[REGIME.NmbModule]=0;
              DATAMRK.CRC=CRC16(DATAMRK.Resource, sizeof (struct DataMRK)-2);
              __watchdog_reset();
              if (EnEEPROM==0xB6) //доп. защита на запись в еепром
               //EDATAMRK[n].DATAMRK=DATAMRK;
               WriteEEPROM(&EDATAMRK[n].DATAMRK.Resource[0], &DATAMRK.Resource[0], sizeof (struct DataMRK));
              __watchdog_reset();
             break;
  
             case ClearRes:
              EnEEPROM=0xB7;
              n=FindNmbMRK();
              DATAMRK=EDATAMRK[n].DATAMRK;
              DATAMRK.Resource[REGIME.NmbModule]=0;
              DATAMRK.CRC=CRC16(DATAMRK.Resource, sizeof (struct DataMRK)-2);
              __watchdog_reset();
              if (EnEEPROM==0xB7) //доп. защита на запись в еепром
               WriteEEPROM(&EDATAMRK[n].DATAMRK.Resource[0], &DATAMRK.Resource[0], sizeof (struct DataMRK));
              __watchdog_reset();
             break;
             /*
             case ClearSrab:
              n=FindNmbMRK();
              DATAMRK=EDATAMRK[n].DATAMRK;
              DATAMRK.SrabMod[REGIME.NmbModule]=0;
              DATAMRK.CRC=CRC16(DATAMRK.Resource, sizeof (struct DataMRK)-2);
              EDATAMRK[n].DATAMRK=DATAMRK;
             break; */
           }
         }
         REGIME.SubMode=Execute;
         REGIME.DisplDelay=Time_2s;     //задержка на индикацию
        }
      }
   break; //Clear
  
   case Test:
      if (BUTTON.Esc>=Push_2s)
       {REGIME.Mode=Ish;
        break;       
       } 
       
      if (REGIME.SubMode==TestPrep)
       {if ((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push))
         REGIME.SubMode=TestDispl;
        else if (BUTTON.Esc>=Push)
         REGIME.Mode=Ish;
       }
       
       
      if (BUTTON.Choice>=Push_2s)
       {if (REGIME.SubMode<TestEnd)
         REGIME.SubMode++;
        else
         {REGIME.SubMode=0;
          REGIME.Mode=Ish;
         }
       }         
      
   break; //Test   
   }//switch(REGIME.Mode)
  BUTTON.Press=Off;
  BUTTON.Up=Off;
  BUTTON.Dwn=Off;
  BUTTON.Choice=Off;
  BUTTON.Esc=Off;
  BUTTON.UpDwn=Off;
 }

/** @fn void ChoiceSideDwn(void)
*   @brief запись в CONFIGMRO номера стороны при нажатии DWN 
    
*/
void ChoiceSideDwn(void)//запись в CONFIGMRO Side
 {
  switch (CONFIGMRO.Side)
   {case 3:
     CONFIGMRO.Side=2;
    break;
    default:
    case 2:
     CONFIGMRO.Side=1;
    break;
   }
 }

/** @fn void ChoiceSideUp(void)
*   @brief запись в CONFIGMRO номера стороны при нажатии UP 
    
*/
void ChoiceSideUp(void)//запись в CONFIGMRO Side
 {switch (CONFIGMRO.Side)
   {case 1:
     CONFIGMRO.Side=2;
    break;
    case 2:
     if (CONFIGMRO.Type==VZU_E)
      break;    //для Е -до 2х
     CONFIGMRO.Side=3;
    break; 
   }
 }

  /** @fn void ChoiceUp(void)
*   @brief запись в Data номера 1-3 при нажатии UP с проверкой разрешения этого в Allow 
    максим. глубина - до 7
*/
 uint8 ChoiceUp(uint8 Data, uint8 Allow)//проверка корректности при нажатии UP с проверкой разрешения этого в Allow 
{switch (Data)
    {case 0:
     case 1:
      if (CHKB(Allow,1)==1) //разрешен переход на 2
       return 2;
      else if (CHKB(Allow,2)==1)
       return 3;
      break;
      case 2:
       if (CHKB(Allow,2)==1) //разрешен переход на 3
        return 3;
      break;
     default:
     return Data;
    }
 return Data;
} 

  /** @fn void ChoiceDwn(void)
*   @brief запись в Data номера 1-3 при нажатии Dwn с проверкой разрешения этого в Allow 
    максим. глубина - до 7
*/
 uint8 ChoiceDwn(uint8 Data, uint8 Allow)//проверка корректности при нажатии UP 
{switch (Data)
    {case 0:
     case 3:
      if (CHKB(Allow,1)==1) //разрешен переход на 2
       return 2;
      else if (CHKB(Allow,0)==1)//разрешен переход на 1
       return 1;
      break;
      case 2:
       if (CHKB(Allow,0)==1)
        return 1;
      break;
      default:
      return Data;
    }
 return Data;
} 
 
 
/** @fn void SaveNmbSection(void)
*   @brief сохраняем в ЕЕПРОМ Номера секции для МРК0…МРК3 
    
*/
void SaveNmbSection(void)  //сохраняем в ЕЕПРОМ Номера секции для МРК0…МРК3
 {
//запись в еепром
  CONFIGMRO.Error=0;
  CONFIGMRO.CRC=0; //расчет и запись - после прохождениия проверки 
  //CONFIGMRO.CRC=CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-5);
  
  __watchdog_reset();
  //ECONFIGMRO[0].CONFIGMRO=CONFIGMRO;
  WriteEEPROM(&ECONFIGMRO[0].CONFIGMRO.Type, &CONFIGMRO.Type, sizeof (struct ConfigMRO));
  __watchdog_reset();
  //ECONFIGMRO[1].CONFIGMRO=CONFIGMRO;
  WriteEEPROM(&ECONFIGMRO[1].CONFIGMRO.Type, &CONFIGMRO.Type, sizeof (struct ConfigMRO));
  __watchdog_reset();
 }

/** @fn void PrepSearsh(void)
*   @brief подготовка к поиску МРК 
    
*/
void PrepSearsh(void) //подготовка к поиску МРК
 {REGIME.SubMode=Search_MRK; //после всех установок-переход на поиск МРК
  REGIME.ClearMode=0; //сброс счетчика прохождения поиска МРК (однократное) 
  ReqMRK=0;
  REGIME.RunStringDelay=Time_4s;//вкл. индикации
  REGIME.RunStringNext=Time_4s;//вкл. индикации
  REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;         //бегущая строка для 2-й строки -сначала
//  REGIME.NmbSection=FindMinNmbSection(); //поиск наименьшего номера секции
 }

/** @fn void SORT_Res(void)
*   @brief сортировка по ресурсу 
    
*/
void SORT_Res(void) //сортировка по ресурсу
//47000cycle for 32 data!=0 (6.4 ms for 7.3MHz
 {uint8 i, n, ni, nimax; //секция, модуль
  uint8 Data_curr, Data_next;
  uint8 IndexResourse0 [16*8]; //0...3bit -MRK, 4..7bit- Module in section
 
//заполняем индексы отличные от 0 и для включенных MRK

  for(i=0, ni=0; i<16; i++) //1800 cycle for 32 data!=0
   {if CHKB(CONFIGMRO.OnMRK,i)
     {for(n=0; n<8; n++)
       {if (EDATAMRK[i].DATAMRK.Resource[n]!=0)
         IndexResourse0[ni++]=(i<<4)+n;//адрес МРК + модуль в МРК
       }
     }
   }
  nimax=ni;   //№ последнего индекса
  uint8 m;
  for(i=0; i<nimax; i++)
   {Data_curr=0;
    m=i; 
    for(n=0; n<(nimax); n++)
     {if (IndexResourse0[n]!=0XFF)
       {Data_next=EDATAMRK[IndexResourse0[n]>>4].DATAMRK.Resource[IndexResourse0[n]&0X0F];
        if (Data_next>Data_curr)
         {Data_curr=Data_next;
          m=n;
         }
       }
     }
    IndexResourse[i]=IndexResourse0[m];//0...3bit -MRK, 4..7bit- Module in section
    IndexResourse0[m]=0xFF;   //индекс использован
   }
  IndexResourse[i]=0xFF; //посл. данное
 }

/** @fn void SORT_Srab(void)
*   @brief сортировка по срабатыванию 
    
*/
uint8 SORT_Srab(void) //сортировка по срабатыванию
//47200cycle for 32 data!=0 (6.4 ms for 7.3MHz
 {uint8 i, n, m, ni, nimax, full=0; //секция, модуль
  uint16 Data_curr, Data_next;
  uint8 IndexSrab0 [16*8];    //0...4 Section, 4..6 Module in sect
//заполняем индексы отличные от 0 и для включенных MRK
  for(i=0, ni=0; i<16; i++)//2000 cycle for 32 data!=0
   {if CHKB(CONFIGMRO.OnMRK,i)
     {for(n=0; n<8; n++)
       {if (EDATAMRK[i].DATAMRK.SrabMod[n]!=0)
         {IndexSrab0[ni++]=(i<<4)+n;//адрес МРК + модуль в МРК
          full=1;//есть не 0 срабатывания
         }
       }
     }
   }
  nimax=ni;   //№ последнего индекса
 
  for(i=0; i<nimax; i++)
   {Data_curr=0;
    m=i; 
    for(n=0; n<(nimax); n++)
     {if (IndexSrab0[n]!=0XFF)
       {Data_next=EDATAMRK[IndexSrab0[n]>>4].DATAMRK.SrabMod[IndexSrab0[n]&0X0F];
        if (Data_next>Data_curr)
         {Data_curr=Data_next;
          m=n;
         }
       }
     }
    IndexSrab[i]=IndexSrab0[m];
    IndexSrab0[m]=0xFF;   //индекс использован
   }
  IndexSrab[i]=0xFF; //посл. данное
  return full;
 }
//****************************************************************************** 
/** @fn void Poll_Srab(void)
*   @brief опрос срабатываний по 6 входам. Если более 60сек- отказ 
    
*/
void Poll_Srab (void)  //опрос срабатываний по 6 входам. Если более 60сек- отказ
 {uint8 i; 
/*
  if ((Period_20ms==Period_Srab))//||(REGIME.Mode<=Check)) //каждые 20ms и только в штатных режимах
   return;
  Period_Srab=Period_20ms;
*/ 
  for (i=0; i<6; i++)
   {if CHKB(PINA,i)
     {if (TimeSrabMRO[i]<0xFF)
       TimeSrabMRO[i]++;
     }
    else
     {if (TimeSrabMRO[i]>0)
       TimeSrabMRO[i]--;
     }
    
    if (TimeSrabMRO[i]>Delay_05s)   //>0.5 сек
     SETB(DisplayFailMRO,i);
    else if (TimeSrabMRO[i]==0) 
     CLRB(DisplayFailMRO,i);
   }
 }



//****************************************************************************** 



/** @fn void AddDataMRK(void)
*   @brief добавление данных в массив DATAMRK из полученных по RS485 
    
    Из ЕЕПРОМ МР-О для данного МР-К копируются накопленные данные в структуру DATAMRK. Далее в нее добавляются принятые от МР-К  (структура READMRK): 
  - флаги размыкания (отключения защиты) FlagDamage.
  - количество срабатываний (для 8 модулей). Если они не нулевые, то добавляются в массив DATAMRK.SrabMod[], их количество ограничивается значением 999. Дополнительно устанавливаются флаги для индикации данной аварии на дисплее DisplaySrabSection и передачи в АПК-ДК DisplaySrabSectionForAPK
  - выработанноый ресурс (для 8 модулей). Они добавляются в массив DATAMRK.Resource[] и имеют ограничение 99%.
    Для обновленных данных рассчитывается CRC, после чего структура DATAMRK  для этого МР-К копируется в ЕЕПРОМ EDATAMRK[].

*/
 void AddDataMRK(void) //добавление данных в массив DATAMRK из полученных по RS485
 {uint8 i, NMB=READMRK.NMB_MRK;
  uint8 SrabMRK;
  uint8 EnEEPROM=0x95;
  DATAMRK=EDATAMRK[NMB].DATAMRK;
  DATAMRK.FlagDamage=READMRK.FlagDamage;
//провести оюработка ошибок?  
  DATAMRK.Error=READMRK.Error;
  if CHKB(NoClearMRK,NMB)
   SETB(DATAMRK.Error,ErrorNoClear); //ошибка стирания
  
//срабатывание- однократное, поэтому накапливаем 
  for (i=0;i<8;i++)
   {SrabMRK=(READMRK.SrabMod)>>(i*2);//по 2 бита на модуль
    SrabMRK&=3;
    if (SrabMRK!=0)      
     {DATAMRK.SrabMod[i]+=SrabMRK;
      if (DATAMRK.SrabMod[i]>999)
       DATAMRK.SrabMod[i]=999;
      SETB(DisplaySrabSection[NMB],i);
      SETB(DisplaySrabSectionForAPK[NMB],i);
      SETB(ReleAlarm,SrabSection);
     }
 
    if (READMRK.Resource!=0)      
     {DATAMRK.Resource[i]+=READMRK.Resource[i];
      if (DATAMRK.Resource[i]>99)
       DATAMRK.Resource[i]=99;
     }
   }
  DATAMRK.CRC=CRC16(DATAMRK.Resource,sizeof(struct DataMRK)-2);
  __watchdog_reset();
  if (EnEEPROM==0x95)
   //EDATAMRK[NMB].DATAMRK=DATAMRK;
   WriteEEPROM(&EDATAMRK[NMB].DATAMRK.Resource[0], &DATAMRK.Resource[0], sizeof (struct DataMRK));
  __watchdog_reset();
 }

/** @fn void DataAPKOrder(void)
*   @brief данные для последовательной отправки в сеть всех типов аварий 
    
    определяет порядок передачи данных (в одном ответе могут не уместиться все аварии 
    и надо отправить все предыдущие аварии перед тем как начать обрабатывать новые). 
*/
void DataAPKOrder(void) //данные для последовательной отправки в сеть всех типов аварий:
  {uint8 i; //после индикации в _copy сброс бита (проиндицированного значения)
   
//DisplayFailMRK_APK[0]=DisplayFailMRK[0] & CONFIGMRO.OnMRK;   //только те, к-ые включены в OnMRK;
//DisplayFailMRK_APK[1]=DisplayFailMRK[1] & CONFIGMRO.OnMRK;
//DisplayFailMRO_APK=(DisplayFailMRO & CONFIGMRO.OnCircMRO);
//откл. цепей DisplayFailMRK_APK - не передается, только аварии   
DisplayAlarmMRK_APK[0]=DisplayAlarmMRK[0] & CONFIGMRO.OnMRK;
DisplayAlarmMRK_APK[1]=DisplayAlarmMRK[1] & CONFIGMRO.OnMRK;
DisplayAlarmMRO_APK=(DisplayAlarmMRO & CONFIGMRO.OnCircMRO);
   
   DisplayNetwMRK_APK=(DisplayNetwMRK & CONFIGMRO.OnMRK);
   for(i=0;i<16;i++)
    if CHKB(CONFIGMRO.OnMRK,i)
     {DisplaySrabSection_APK[i]=DisplaySrabSectionForAPK[i]; //8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]
DisplaySrabSectionForAPK[i]=0;      //переписали - очистили, т.к. это собитие однократное и редкое
      DisplayResourceMRK_APK[i]=DisplayResourceMRK[i];
     }
    else
     {DisplaySrabSection_APK[i]=0; //8 модулей в 16 секциях: в режиме срабатывания, если бит=1: индикация на 3 сек "сработал Модуль... СЕКЦИЯ[16].модуль[бит]
      DisplayResourceMRK_APK[i]=0;
     }
  }
   
/** @fn void DataAlarmAPK(void)
*   @brief формирование данных аварий для отправки по APK 
    1. Если все данные для АПК были ранее переданы (NmbSendAPK==0) -формируем новую посылку. Вычисляем колич. секций для передачи NmbSendAPK.
    Заполняем DATA_SEСT_APK[4][16], все 16 секторов для передачи 4 секций за 1 посылку, по 4 байта на секцию и сектор   
    2. Формируем передачу в АПК DATA_APK[4*4=16], переписывая туда из заполненных DATA_SEСT_APK[4][16], до тех пор пока не исчерпаются все данные NmbSendAPK
    
*/
void DataAlarmAPK(void)     //формирование данных аварий для отправки по APK
 {uint8 i, n, Alarm;               //определяем номера секций где есть аварии
 
//проверяем новые данные  
  if (NmbSendAPK<0) //передача старых данных по апк - завершена
   {NmbSendAPK=0;
    NmbCurrAPK=0;
    DataAPKOrder();     //переписываем новые данные
   
    for (i=0; i<(16*4); i++) //общее кол-во переписанных номеров 
       DATA_SECT_APK[i]=0;     //первоначальная очистка всех секций для передачи

//откл. цепей DisplayFailMRK_APK - не передается, только аварии      
//проверка данных 16 секций подкл. к МРК и заполнение до 16 секторов DATA_SEСT_APK[4][16] если есть аварии
    for (i=0; i<16; i++)                        //для 16 секций, заполняем DATA_SEСT_APK
     {Alarm=0;
      if CHKB(CONFIGMRO.OnMRK,i)                //проверка для вкл. МР-К
       {if (DisplaySrabSection_APK[i]!=0)       //проверяем срабатывание
         {DATA_SECT_APK[(NmbSendAPK*4)+1]=DisplaySrabSection_APK[i]; //Флаги срабатывания модулей защиты данной секции 
          Alarm++;
         }
        if (DisplayResourceMRK_APK[i]!=0)       //проверяем выработку ресурса
         {DATA_SECT_APK[(NmbSendAPK*4)+2]=DisplayResourceMRK_APK[i]; //Флаги выработки более 80% ресурса
          Alarm++;
         }
//определяем номера секций где есть размыкание МРК
        if CHKB(DisplayAlarmMRK_APK[0],i)  //по 2 входа на МРК [0]-1ряд, [1]-2ряд
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],0);
          Alarm++;
         }
        if CHKB(DisplayAlarmMRK_APK[1],i)
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],1);
          Alarm++;
         }
        if CHKB(DisplayNetwMRK_APK,i) //нет сети
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],7);
          Alarm++;
         }
        if (Alarm!=0)
         {DATA_SECT_APK[(NmbSendAPK*4)+0]=CONFIGMRO.NmbSectionMRK[i];//записываем номер секции
          NmbSendAPK++;                                      //если есть авария, переходим к след сектору
         }
       }
//очистка того что переписали
   DisplayResourceMRK_APK[i]=0;
   DisplaySrabSection_APK[i]=0; 
   CLRB(DisplayNetwMRK_APK,i);        
   CLRB(DisplayAlarmMRK_APK[0],i);
   CLRB(DisplayAlarmMRK_APK[1],i);            
     }
//для 6 входов МРО, но не более 16 секторов, заполняем DATA_SEСT_APK      
//6 входов МРО могут быть для от 1 до 6 секций 
    for (n=0; ((n<6)&&(NmbSendAPK<16)); )   
     {if (CHKB(CONFIGMRO.OnCircMRO,n))                      //проверка для вкл. МР-O
       {if (CHKB(DisplayAlarmMRO_APK,n))                    //есть размыкание в МРО?
         {i=CONFIGMRO.NmbSectionMRO[n];
          DATA_SECT_APK[(NmbSendAPK*4)+0]=CONFIGMRO.NmbSectionMRO[n]; //новый номер
          DATA_SECT_APK[(NmbSendAPK*4)+1]=0;                //Флаги срабат. для МРО нет
          DATA_SECT_APK[(NmbSendAPK*4)+2]=0;                //Флаги выработки более 80% ресурса. для МРО нет
          SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],n);          //установка бита для нового или старого номера секции , если уже есть          
         }
        n++;
        if (i!=CONFIGMRO.NmbSectionMRO[n])      //если изменился номер секции  
          NmbSendAPK++;                         //новый сектор передачи АПК
       }
      else
       n++;      
     }
   DisplayAlarmMRO_APK=0;  //очистка того что переписали в МРО
   }
  else //передача по 4 сектора (из максимум 16)
   {if (NmbSendAPK>16)
     NmbSendAPK=0;      //данных не больше чем на 16 секторов
    for (i=0; i<16; i++) //очистка данных секторов
      DATA_APK[i+ShiftAPK]=0;
      
    for (i=0; (i<4)&&(NmbSendAPK<0); i++) //копирование данных секторов
     {for (n=0; n<4; n++)
       {DATA_APK[(i*4)+n+ShiftAPK]=DATA_SECT_APK[(NmbCurrAPK*4)+n];
       }
      NmbSendAPK--;
      NmbCurrAPK++;
     }
   }  

//проверка наличия аварий для первого включения питания передаем 0xFF
  if (PrepClear==0xBB)
   {PrepClear=0;
    for (i=0; i<16; i++) //очистка данных секторов
     DATA_APK[i+ShiftAPK]=0;
    DATA_APK[15+ShiftAPK]=0xFF;
   } 
 }

/** @fn void Delay(uint16 n)
*   @brief Задержка в мкс
    \param[in] n Задержка в мкс
    
*/
 void Delay(uint16 n) //0.5 mks
 {for(;n;n--);
 }

/** @fn Rele(void)
*   @brief управление реле
    В тестовом режиме - управляется о ткнопки ВВЕРХ
*/
void Rele(void)  
 {if (TimeRele!=0)
   {TimeRele--;
    return;
   }
  if ((REGIME.Mode==Test)&&(REGIME.SubMode==TestRelay))
   {if ((BUTTON.TimeUp!=0)||(BUTTON.TimeDwn!=0))
     SETB(PORTD,RELE);      //вкл
    else
     CLRB(PORTD,RELE);     //выкл
    return; 
   }

  switch (ReleMode)
   {case RelePower:
     ReleMode=RelePowerEnd;
     TimeRele=Time_2s;
     SETB(PORTD,RELE);      //вкл
    break;
      
   case RelePowerEnd:
   default:
     if ((DisplayAlarmMRK[0]!=0)||(DisplayAlarmMRK[1]!=0)||(DisplayAlarmMRO!=0)
      ||(DisplayNetwMRK!=0)||(Resource80!=0))
      {CLRB(PORTD,RELE);     //выкл
      }
     else
      SETB(PORTD,RELE);      //вкл
     if CHKB(ReleAlarm,SrabSection)
      {CLRB(ReleAlarm,SrabSection); 
       TimeRele=Time_180s; //вкл на 180 сек
       CLRB(PORTD,RELE);     //выкл
      }
    break;
   }
 }
 

 