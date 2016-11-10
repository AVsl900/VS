// MRK

/** @file main.c 
*   @brief Главная ф-ия
*   @date 07.07.2014
*   @version 01.00.00
*
*   Файл содержит
  - Однократно, после reset: 
	  - инициализация watchdog,
	  - инициализация портов ввода/вывода initPort();
	  - инициализация таймеров initTimer();  
	  - инициализация UART InitUSART(); 
	  - Инициализация дисплея InitDisplay();
  - Основной цикл, включающий
	 - Выполнение подпрограмм, периодически, каждые 20 мс:
	    - Опрос кнопок Poll_button();
	    - Реакция на нажатия кнопок Poll_Regime();
	    - Опрос цифровых входов (размыканий) Poll_Srab();
	    - Контроль дисплея с периодической его переинициализацией для  устранения зависаний и сбоев от помех InitDisplay2(): в исходном режиме (индикация отключена) - каждые 2 сек, в остальных режимах ( индикация включена) - каждые 20 сек.
	 - Выполнение подпрограмм, периодически каждые 100 мс:
	    - Контроль временных интервалов индикации на дисплее мигающих символов FlashDisplay();
	    - Выдача информации на дисплей Display();
	    - Управление реле Rele();
	    - Связь  МР-О с АПК-ДК Link_UART_APK();
	    - Связь  МР-О с МР-К Poling_MRK();
	    - Анализ данных полученных от МР-К, запись ее в ЕЕПРОМ AddDataMRK()
	    - Запрос в МР-К на стирание полученной информации Clear_MRK();
  - Прерывание системного таймера каждые 20 мс TIMER2_COMP_vec(void) по которому инкрементируется переменная Period_20ms.

\note
Замечания по схеме:
1.+ С по питанию, дроссель, разрядники
2. С на кварц 33, должно быть 12-22
3. супервизор
4. ключ на индикатор
5. + конт. на чтение/запись индикатора- для контроля его состояния - есть

\par
\note 
В новой плате конт RW PORTB_4 вместо PORTB_6*/

/*  
доп. защита на запись в еепром - проверка EnEEPROM
*/

//UART постояно включен на прием, т.к. вывод RE заземлен!!!!!!
/*
//test
1.тестовый режим- нет кодировки ASCII.
//CONV_ASCII (&DATA_APK[0],&TX_UART1[1],20);
2. в посл. 16 байтах -передается код АЦП МРК 1 и 2
3. при просмотре срабатываний/ресурса Сек/мод: кн. up- просмотр AЦП - 8 данных
4. при передаче запроса от АПК - на индик. первые 
5. В исх. режиме кн. up- просмотр 8 байт данных ответа АПК
*/


//#define DEBUG_AVR_STUDIO_4   1   //отладка в авр студио

#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif

#include "inavr.h"
#include "ioavr.h"
#include "MRO.h"
#include "UART.h"

uint8 Period_display, /**< Время индикации */
Period_20ms, /**< Время в 20 мс интервалах */
Period_Test, /**< Время проверки дисплея \note каждые 2 сек*/
Period_Reset, /**< Время сброса дисплея \note каждые 20 сек*/
Init_Stage;/**< */
struct Regime REGIME;/**< Режим МР-О*/
extern uint8 LINE1[], LINE2[];
extern uint8 PrepClear, PrepTest;
extern uint16 TimeRele;
extern uint8 ReleMode;
uint8 Period_butt; /**< Время опроса кнопок*/

int main( void )
{
// #ifndef DEBUG_AVR_STUDIO_4
  /** - Однократно, после reset:
        - инициализация watchdog,
        - инициализация портов ввода/вывода initPort();
        - инициализация таймеров initTimer();  
        - инициализация UART InitUSART(); 
        - Инициализация дисплея InitDisplay();
  */
__disable_interrupt();
 __watchdog_reset();// инициализация watchdog  
WDTCR|=(1<<WDE)|(1<<WDCE);
_nop;
WDTCR=(1<<WDP0)|(1<<WDP1)|(1<<WDP2)|(1<<WDE); //1.9 сек

__enable_interrupt();

 initPort();  //инициализация портов ввода/вывода 
 initTimer(); //инициализация таймеров 
 InitUSART(); //инициализация UART 
 CLRB(PORTD,RELE);
//для полного стирания ЕЕПРОМ проверяем нажатие ВЫбор, Назад не должен быть нажатым
 PrepClear=0xBB;

 while (CHKB(PINC,KEY1)==0)
  {PrepClear++;
   if (PrepClear==0xA5)
    break;
  }
 //для входа в тестовый режим проверяем нажатие Назад
 PrepTest=0xDD; 
 while (CHKB(PINC,KEY2)==0)
  {PrepTest++;
   if ((PrepTest==0xAA)||(PrepClear==0xA5))
    break;
  }
 
 Delay(60000);  //60 ms задержка перед инициализацией LED 
 Delay(60000);  //60 ms
 __watchdog_reset();
 Delay(60000);  //60 ms задержка перед инициализацией LED 
 Delay(60000);  //60 ms
 __watchdog_reset(); 

 InitDisplay();  //Инициализация дисплея
 REGIME.Mode=Plug;
 REGIME.DisplDelay=Time_2s;
 ReleMode=RelePower;
 TimeRele=Time_2s-(300/100); //срабатывание 2c при включении - 0.3s на задержку дисплея

 TIFR=0XFF;    //сброс  флагов прерываний всех таймеров, без сброса - Т0 прерывание

 TestDisplay();
Init_Stage=5;
 while(1)
  {__watchdog_reset();
   __enable_interrupt();
/** - Основной цикл, включающий
      - Выполнение подпрограмм, периодически, каждые 20 мс:
        - Опрос кнопок Poll_button();
        - Реакция на нажатия кнопок Poll_Regime();
        - Опрос цифровых входов (размыканий) Poll_Srab();
        - Контроль дисплея с периодической его переинициализацией для  устранения зависаний и сбоев от помех InitDisplay2(): в исходном режиме (индикация отключена) - каждые 2 сек, в остальных режимах ( индикация включена) - каждые 20 сек.
      - Выполнение подпрограмм, периодически каждые 100 мс:
        - Контроль временных интервалов индикации на дисплее мигающих символов FlashDisplay();
        - Выдача информации на дисплей Display();
        - Управление реле Rele();
        - Связь  МР-О с АПК-ДК Link_UART_APK();
        - Связь  МР-О с МР-К Poling_MRK();
        - Анализ данных полученных от МР-К, запись ее в ЕЕПРОМ AddDataMRK()
        - Запрос в МР-К на стирание полученной информации Clear_MRK();
   */  
   if (Period_butt!=Period_20ms)
    {Period_butt=Period_20ms;
     Period_display++;
     Period_Test++;
     Poll_button();  // - опрос Кнопок 
     Poll_Srab();    // - опрос срабатываний по 6 входам
     Poll_Regime();  // - действия на нажатия кнопок
     
     
    if (Period_Test>Delay_2s)
     {Period_Test=0;
      if (REGIME.Mode==Ish) //в исх. состоянии- инициализируем дисплей каждые 2 сек
       Init_Stage=0;  //пуск InitDisplay
       //InitDisplay();
      else   //в остальных- инициализируем дисплей каждые 20 сек, проверяем - каждые 2 сек
       {Period_Reset++;
        if (Period_Reset>10) //каждые 20 сек
         {Period_Reset=0;
          Init_Stage=0;  //пуск InitDisplay
         }
        else if (TestDisplay()==0)
         Init_Stage=0;  //пуск InitDisplay
       }
      }
     InitDisplay2(); //каждые 20 мс если был пуск InitDisplay

    }
//каждые 100 мс
   if (Period_display>=Delay_01s)
    {Period_display=0;
     FlashDisplay();              //REGIME.Flash -горит 0.5 сек, не горит 0.2 сек
     if (Init_Stage>4) //во время инициализации- не мешаем
      Display();   //индикация
     Rele();       //управление реле
    }

   if (REGIME.Mode>=Check)//||(REGIME.Mode==Check2)&&(CONFIGMRO.Circuit[0]==RC)&&((CONFIGMRO.Type==VZU_M)))
    {//Link_UART_MRO(); //Дополнит. информация по 485
     Link_UART_APK();//485-АПК
     Poling_MRK();   //запрос/анализ ответа по всем МРК
     Clear_MRK();    //стирание полученной инфы в МРК если она !=0
    }
  }
}



//**************************   interrupt *****************************
/** - Прерывание системного таймера каждые 20 мс TIMER2_COMP_vec(void) по которому инкрементируется переменная Period_20ms.*/
#pragma vector=TIMER2_COMP_vect
__interrupt void TIMER2_COMP_vec(void) //0,02 sec
 {Period_20ms++;
 }
