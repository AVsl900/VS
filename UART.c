
/** @file UART.c 
*   @brief программы интерфейса UART
*   @date 07.07.2014
*   @version 01.00.00
*/

#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif

#include "inavr.h"
#include "ioavr.h"
#include "UART.h"
#include "MRO.h"


struct Uart UART0,  /**<Опрос МР-К*/
UART1;              /**< СВязь с АПК-ДК*/
struct ReadMRK READMRK; /**< Прочитанные данные с МР-К*/
struct DataSummMRK DATASUMMMRK[],/**< Прочитанные суммарные данные с МР-К*/
DATASUMMMRK_Copy ;/**< Копия суммарных данные с МР-К*/
/** скорость UART*/
__flash uint16 UBRR[10] ={FOSC/16/2400-1, FOSC/16/4800-1, FOSC/16/9600-1, FOSC/16/14400-1, FOSC/16/19200-1, FOSC/16/28800-1, FOSC/16/38400-1, FOSC/16/57600-1, FOSC/16/115200-1, FOSC/16/1200-1};
/** Задержка для 1.5 byte*/
__flash uint8 RTU_DELAY[10] ={45, 23, 11, 8, 6,  4,  3, 2, 1, 90}; 
//7.37MHz:1024=138mks. сброс на максимуме OCR0A
//1200-20ms, 2400-10, 4800-5 ms, 9600-2.5 байта=2.5 мс, 14400- 1.7, 19200 - 1.25, 28800-0.8, 38400 -0.6
/** Задержка для 2 byte*/  
__flash uint8 RTU_ANSW[10] ={60, 30, 15, 10, 8,  5, 4, 3, 1, 120}; 
/**к-во байт для чтения DataType: [0]-текущ, 1- сумм*/
__flash uint8 NmbByteReadMRK[2]= {16, 34};      
/**адрес для чтения DataType: [0]-текущ, 1- сумм*/
__flash uint8 BeginAddrMRK[2]= {0x00, 0x20};    
   //в сумме передача через 2+1.5=3.5 байта

/**Таблица номеров секторов MRK_ECSE*/
__flash uint8 TableNmbSectionMRK_ECSE[2][4]={ //0- нет такого
1,2,3,4, //сторона=1
5,6,7,8};//сторона=2
/**Таблица номеров секторов MRK_ECSM*/
__flash uint8 TableNmbSectionMRK_ECSM[4]={0,1,2,3,}; //для М1 -1 секция, М2 -2, М3 -3

uint8 NMB_UART_RX0, /**<Текущий номер байта приема*/
NMB_UART_TX0,       /**<Текущий номер байта передачи*/
MAX_NMB_UART0,      /**<количество байт передачи*/
NMB_UART1,          /**<Текущий номер байта приема*/
NMB_UART_TX1,       /**<Текущий номер байта передачи*/
MAX_NMB_UART1;      /**<количество байт передачи*/

uint8 RX_UART0[48];/**<Буфер приема внутр RS485 -MRK*/
uint8 TX_UART0[16];/**<Буфер передачи внутр RS485 -MRK*/
extern uint8 ReqMRK; //запрос МРК
/**№ MRK для 0-Current, 1-Summ*/
uint8 NMB_MRK[2];    //0-Current, 1-Summ
uint16 NoAnswMRK; /**<нет ответа после 2-х запросов*/
uint16 NoClearMRK;
extern uint16 DisplayFailMRK[2];       //[0]-цепь0(ряд1) 15...0 МРК, [1]-цепь1(ряд2) 15...0 МРК 

uint8 RX_UART1[48];/**<Буфер приема внешн RS485 -АПК-ДК*/
uint8 TX_UART1[48];/**<Буфер передачи внешн RS485 -АПК-ДК*/
uint8 DATA_APK[24];/**<Данные по авариям АПК-ДК*/

uint8 RX_UART0_TEST[2], RX_UART1_TEST[2]; 

extern __eeprom uint8 EAddrNet;/**Адрес МР-О в сети АПК-ДК*/

/** @fn void InitUSART(void)
*   @brief инициализация USART

    USART0 -внутр RS485, USART1 -внешн RS485 
*/
void InitUSART(void)
 {//0-внутр RS485
  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);       //|(1<<UPM0);
  UBRR0H = (uint8)(UBRR[S9600]>>8);     // Set baud rate
  UBRR0L = (uint8)UBRR[S9600];
  UCSR0B=(1<<TXEN0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<RXCIE0);   //Enable TX+receiver
// Set frame format: 8data, 1stop bit  2stop b:|(1<<USBS)
// UPM1=1, UPM0=0 - parity even, 11 - odd
  
  //1-внешн RS485
  UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);       //|(1<<UPM0);
  UBRR1H = (uint8)(UBRR[S9600]>>8);     // Set baud rate
  UBRR1L = (uint8)UBRR[S9600];
  UCSR1B=(1<<TXEN1)|(1<<TXCIE1)|(1<<RXEN1)|(1<<RXCIE1);   //Enable TX+receiver
 } 

/** @fn void Find_MRK(void)
*   @brief Поиск по UART всех подсоединенных МР-К (при конфигурировании прибора), конфигурирование входов МРО

    На время поиска 16 МР-К работа ПО (кроме прерываний) приостанавливается.
    Заполняется массив номеров секций CONFIGMRO.NmbSectionMRK[]: для исполнений 
    ВЗУ-ЭЦС-Е и ВЗУ-ЭЦС-М для 4-х МР-К в массив загружаются номера секций из таблицы. 
    Также загружаются номера секций для 6 входов МР-О. 

    Проводится максимум  2 запроса для каждого МР-К, ответивший МР-К вторично не опрашивается. 
    Ответившие МР-К записываются в CONFIGMRO.OnMRK как подключенные. 
    Опрос проводится след. образом: 
    - выключается прием. 
    - формируется массив  TX_UART0[] для 1-ой попытки запроса среди тех 16 номеров 
      МР-К, номер соответствующей секции к-го был присвоен ранее и не равен 0 (нет секций с номером 0)
    - Очищается буфер приема RX_UART0[], устанавливается таймаут ожидания ответа CNT1 
      и запускается передача. Ожидаем конец передачи или истечение времени на передачу.
    - Включается прием,  устанавливается таймаут ожидания ответа. Ожидаем конец приема 
      или истечение времени на прием
    - Проверяется корректность полученных данных в буфере приема RX_UART0[]. 
      Если данные правильные - устанавливается бит подключённого МР-К.
    - Выключается прием, формируется пауза 100 мс для опроса след. МР-К
    - Опрос повторяется для 16 МР-К, после чего запускается 2-я попытка опроса не ответивших МР-К. 

*/
void Find_MRK(void) //опрос МРК 0-15 2 раза. Приоритет 100%. Время 32*16=0.5с
 {uint8 i, Nmb;
  uint16 CRC_TX;
/** - Обнуляем неиспользуемые МРК в CONFIGMRO.NmbSectionMRK  */
  for(i=0;i<16;i++)                       //обнуляем неиспользуемые МРК 
    CONFIGMRO.NmbSectionMRK[i]=0;
  
  if (CONFIGMRO.Type==VZU_M) //для VZU_M cекций 1 для М1, 2 для М2,3 для М3
   {for(i=1;i<=CONFIGMRO.Side;i++)
     {CONFIGMRO.NmbSectionMRK[i]=i;
      CONFIGMRO.NmbSectionMRO[2*i-2]=i;
      CONFIGMRO.NmbSectionMRO[2*i-1]=i;
     }
   }
  else if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
   {for(i=1;i<=8;i++)
     CONFIGMRO.NmbSectionMRK[i]=i;
    for(i=0;i<6;i++)
     CONFIGMRO.NmbSectionMRO[i]=0xFF;
   }
  else                         //VZU_E -все 4 секции
    for(i=0;i<4;i++)
     {Nmb=TableNmbSectionMRK_ECSE[(CONFIGMRO.Side-1)][i];
      CONFIGMRO.NmbSectionMRK[Nmb]=Nmb;     //адрес МРК всегда = номеру секции
      if (i<3)                              //MRO: 6 входов - 3 секции
       {CONFIGMRO.NmbSectionMRO[2*i]=Nmb;
        CONFIGMRO.NmbSectionMRO[2*i+1]=Nmb;
       }
     }
  
  CONFIGMRO.OnMRK=0;
  CLRB(UCSR0B,RXEN0);          //выкл. приемник
//   - Формируется массив  TX_UART0[] для 1-ой попытки запроса среди тех 16 номеров 
//    МР-К, номер соответствующей секции к-го был присвоен ранее и не равен 0 (нет секций с номером 0)
  for (i=0;i<2;i++)//2 опроса с накоплением данных
   {TX_UART0[1]=0x03;
    TX_UART0[2]=0x00;
    TX_UART0[3]=BeginAddrMRK[0];      //адрес регистра (начало)
    TX_UART0[4]=0x00;
    TX_UART0[5]=0x01;              //1 слово

// - Ищем с №1 среди тех к-ые есть в списке секций, т.к. адрес МРК всегда = номеру секции    
    for (NMB_MRK[0]=0;NMB_MRK[0]<16;NMB_MRK[0]++)//МРК с адресами от 1 до 16
     {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])  //ответившие не опрашиваем
       continue;
      if (CONFIGMRO.NmbSectionMRK[NMB_MRK[0]]==0) //0 секции и 0 МРК нет, не ищем 
       continue;//адрес МРК всегда = номеру секции
      TX_UART0[0]=NMB_MRK[0];
      CRC_TX=CRC16(&TX_UART0[0],6) ;    
      TX_UART0[6]=(uint8)CRC_TX;
      TX_UART0[7]=(uint8)(CRC_TX>>8);
      MAX_NMB_UART0=8;
      CLR_BUF(&RX_UART0[0],8);
      NMB_UART_TX0=1;

// - Пуск Т1. контроль времени передачи =OCR1A*139мкс      
      OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[0]*4; //ждем 2*14*4=56 байт(112 мс)
      StartT1; 
      TCNT1=0;//сброс таймер ожидания ответа

      UART0.TXBeg=1;               //начинаем передачу (+задержка 3 байт)
      UART0.TXCmpl=0;
      UART0.RX_TIME=0;
// - Задержка перед началом передачи 3 байта и старт передачи*/
      TCNT0=0;                      //таймер времени между байтами
      StartT0;                      //вкл. Т0, :1024
      OCR0=(RTU_DELAY[S9600]*2);    //3 байта 
//  - Ждем конца передачи и контроль время*/    
      while((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) //ждем конца передачи и контроль время
       __watchdog_reset();
      UART0.RX_TIME=0;              //таймер ожидания ответа
      UART0.RXCmpl=0;
      SETB(UCSR0B,RXEN0);           //вкл. приемник
      CLR_BUF(&RX_UART0[0],16);     //сигнал на приемник постоянно подан, желат. очистить буфер
      UART0.RXBeg=0;
      NMB_UART_RX0=0;
      TCNT1=0;                      //сброс таймер ожидания ответа
//  - Ждем конца приема и контроль время */      
      while ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) //ждем конца приема и контроль время
       __watchdog_reset();
      if (UART0.RXCmpl==1)       //закончили опрос
// - Проверяется корректность полученных данных в буфере приема RX_UART0[].         
       {if ((RX_UART0[0]==NMB_MRK[0])&&(RX_UART0[1]==0x03)&&(RX_UART0[2]==2))
         {if (CRC16(&RX_UART0[0],7)==0) //успешный ответ
           {SETB(CONFIGMRO.OnMRK,NMB_MRK[0]); //включаем найденные МРК
// - Если МРК есть, вход МРО- отключается (адрес МРК всегда = номеру секции) 
//Ищем совпадения номеров секций для найденный МРК и в МРО, если есть - в МРО их отключаем  
/*            for (i=0;i<6;i++)//3*2=6 входов МРО            
             {if (CONFIGMRO.NmbSectionMRO[i]==CONFIGMRO.NmbSectionMRK[NMB_MRK[0]])                          //
               CLRB(CONFIGMRO.OnCircMRO,i);  //откл. вход МРО с совпавшей секцией
             }
*/
           }
         }
// - Выключается прием, формируется пауза 100 мс для опроса след. МР-К/      
        CLRB(UCSR0B,RXEN0);      //выкл. приемник
        UART0.RXCmpl=0;
        while (UART0.RX_TIME==0) //ждем интервал 100мс для след. посылки
         __watchdog_reset();
       }
     }
   }
 }

//нужен старт опроса
/** @fn void Poling_MRK(void)
*   @brief Связь  МР-О с МР-К 

    Ф-ия опрашивает подключенные МР-К (соответствующие установленным битам в CONFIGMRO.OnMRK). 
    Для чтения данных используется 3-я, для записи - 6-я   ф-ия MODBUS, см. ПРОТОКОЛ ОБМЕНА МР-О - МР-К
    Для МР-К с номером READMRK.NMB_MRK вычитываются 3-ей ф-ей (см. Табл. Регистры данных МР-К)
  - текущие данные - флаги выработки ресурса, срабатывания защиты и неисправности группы модулей
  - суммарные данные - накопленные процент выработки ресурса и количество срабатываний
    Очередность опроса текущих/суммарных данных - один суммарный после всех текущих: 
    текущие данные опрашиваются для всех подключенных МР-К (номер опрашиваемого МР-К - в NMB_MRK[0]), 
    после чего опрашивается суммарные данные для одного МР-К (номер опрашиваемого МР-К - в NMB_MRK[1]) и т.д.
    Для обеспечения надежного подсчета количества срабатываний и выработки ресурса реализован механизм 
    чтения/стирания из МР-К текущих данных. В период между запросами от МР-О, МР-К - накапливает данные по авариям. 
    По запросу с МР-О МР-К выдает накопленные текущие аварии. Если в прочитанных текущих данных присутствуют флаги 
    аварий, количества срабатываний (до 3 шт. на каждый модуль) и выработки ресурса (до 99% на каждый модуль)  
    то  МР-О посылает в МР-К код успешного приема данных по которому МР-К обнуляет регистры текущих аварий. 
    В качестве кода обнуления используется CRC (ячейка 0х06) этих текущих данных, к-ое необходимо записать 6-й ф-ей 
    в ячейку с адресом 0х110, см. табл. Регистры данных МР-К.

    Ф-ия завершается если / после: 
  - 1. неподключены все МР-К в сети RS-485 (CONFIGMRO.OnMRK = 0), или включен режим посылки запроса / получения ответа для стирания МР-К.
  - 2. по какой либо причине нет опроса МР-К (напр. после выключения - включения питания), то он включается ф-ей SendPoling_MRK().

    <b> Если нет выхода из ф-ии, это означает что включен режим приема или передачи: </b>
      - 1.Режим приема AnswRX. Проверяется окончание времени приема (UART0.RX_TIME) и если оно истекло, то окончен ли сам прием (UART0.RXCmpl).

    Если время приема исчерпано, а завершения приема нет - устанавливается флаг ошибки ответа NoAnswMRK для опрашиваемого МР-К, 
    посылается повторный запрос (до 5 попыток в режиме опроса текущих данных). По исчерпанию 5 попыток опроса текущих данных МР-К,
    к-ый не отвечает, выбирается следующий МР-К для опроса.

    Если ответ успешный, т.е. по истечению времени приема (UART0.RX_TIME) есть признак  завершения приема (UART0.RXCmpl), проверяется 
    правильность принятых из МР-К данных, включая значение CRC. Если данные корректны для режима типа данных CheckDataMRK:
          - HaveSummData - режим суммарных данных накопленных в  МР-К. Информация переписывается в массив DATASUMMMRK[] МР-О 
          - HaveData - режим текущих данных. Проверяется успешность операции стирания, и если она была выполнена, то после этого из 
    полученных новых данных ф-ей AddDataMRK() информация добавляется в массив DATAMRK[]. Если стирание не прошло, посылается 
    команда очистки  SendClear_MRK(). Так обеспечивается устранение повторных добавлений одних и тех же данных в память МР-О, 
    в случае если по каким либо причинам не была выполнена команда обнуления данных МР-К.

    Далее формируется запрос к следующему МР-К. 
    После опроса текущих данных для всех  МР-К посылается запрос суммарных данных для одного МР-К.
	
      - 2.Режим передачи SendTX. Проверяется окончание времени передачи (UART0.RX_TIME) и успешное завершение передачи (UART0. TXCmpl). 
    При выполнении одного из этих условий -переход в режим приема и настройки таймера CNT1 на контроль времени приема 

*/
void Poling_MRK(void) //запрос / анализ ответа по всем МРК для текущих или суммарных данных
 {uint8 i;
  
  if ((CONFIGMRO.OnMRK==0)||(UART0.Regime==SendTXClear)||(UART0.Regime==AnswRXClear))
   return;
  if ((UART0.Regime==SendTX)||(UART0.Regime==AnswRX))//включаем UART0 если он был выкл.
   ;//ок, 
  else //запуск в работу если был выкл
   {UART0.DataType=Current;
    SendPoling_MRK();   //передача
   }
 
//режим приема  
  if (UART0.Regime==AnswRX)      //был режим приема, проверяем его окончание
   {//if ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) 
    if (UART0.RX_TIME==0)
     return;                     //ждем конца приема и контроль время
    if (UART0.RXCmpl==1)         //прием завершен, анализ приема
     {if ((RX_UART0[0]==NMB_MRK[UART0.DataType])&&(RX_UART0[1]==0x03)&&(RX_UART0[2]==NmbByteReadMRK[UART0.DataType])
         &&(CRC16(&RX_UART0[0],NmbByteReadMRK[UART0.DataType]+5)==0))  //успешный ответ
       {CLRB(NoAnswMRK,NMB_MRK[UART0.DataType]);
        
        switch (CheckDataMRK())        //проверка текущих/суммарных данных
         {case HaveSummData:
if (NMB_MRK[1]<16) //чтобы не перегрузить буфер
             DATASUMMMRK[NMB_MRK[1]]=DATASUMMMRK_Copy;//копирование суммарных данных
          case NoData:
Next_MRK:   
            ReqMRK=0;              //для нового МРК обнуляем счетчик повторных запросов
            NMB_MRK[0]++;
            for(i=0 ; i<16; NMB_MRK[0]++,i++) //поиск след. МРК
             {if (NMB_MRK[0]>15)      //провели цикл опроса, обновляем данные для отправки
               {NMB_MRK[0]=0;          
//раз в цикл опроса всех МРК - запрашиваем суммарный для одного МРК               
                NMB_MRK[1]++;
                if (NMB_MRK[1]>15)      //провели цикл опроса, обновляем данные для отправки
                 NMB_MRK[1]=0;
                for(i=0 ; i<16; NMB_MRK[1]++,i++)
                 {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[1])
                   {UART0.DataType=Summ;    //сумма
                    SendPoling_MRK();      //передача 
                    return;
                   }
                 }
               }
              if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
               break;              //нашли вкл. МРК
             }
            
           if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
            {UART0.DataType=Current;    //сумма
             SendPoling_MRK();      //передача
            }
           return;//case NoData

          case HaveData:
            if (CHKB(NoClearMRK,READMRK.NMB_MRK)==0)//предыдущее стирание - успешно
             AddDataMRK();         //добавление данных в массив DATAMRK из полученных по RS485
            ReqMRK=0;
            SendClear_MRK();      //послать команду стереть
          return;//case HaveData
          case NoCorr:
            goto Next_Req;
         }
       }
      else              //ответ неверный, повторный запрос
       goto Next_Req;
     }
    else                //нет приема
Next_Req:       
     {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
       {SETB(NoAnswMRK,NMB_MRK[0]);//ошибка ответа
        ReqMRK++;
        if (ReqMRK<6)     //повторный запрос
         {UART0.DataType=Current;    //сумма
          SendPoling_MRK();//передача
         }
       }
      goto Next_MRK;
     }
   }
//режим передачи   
  else if (UART0.Regime==SendTX) 
   {if ((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) 
     return;            //ждем конца передачи и контроль время
//передачу закончили, прием + контроль времени приема
    TCNT1=0;            //сброс таймер ожидания ответа
    UART0.RXBeg=0;   
    NMB_UART_RX0=0;
//таймер ожидания ответа      
    UART0.RX_TIME=0;             
    //OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[UART0.DataType]*3; //ждем 2*14*2=112байт 
OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[UART0.DataType]*8; //ждем 2*14*15=420байт (0.4 сек) 
//если опрашивать чаще, почемуто мешает  
    UART0.Regime=AnswRX;         //
    SETB(UCSR0B,RXEN0);          //вкл. приемник
   }
 }


/** @fn void Clear_MRK(void)
*   @brief Cтирание в МР-К полученной информации (если она есть)

    Ф-ия контролирует передачу и ответ МР-К на запрос по стиранию  текущих данных для МР-К с номером READMRK.NMB_MRK. 
    Проверяется выключены ли все МР-К в сети RS-485 (по данным CONFIGMRO.OnMRK), включен ли режим приема/передачи текущих данных. 
    Если одно из условий выполняется - ф-ия завершается. Если нет выхода из ф-ии, это означает что включен режим приема/передачи стирания:
  - Режим приема AnswRXClear. Проверяется окончание времени приема (UART0.RX_TIME) и если оно истекло, то окончен ли сам прием (UART0.RXCmpl). 
    Если время приема исчерпано, а завершения приема нет - устанавливается флаг ошибки ответа и осуществляется переход в режим опроса  
    текущих данных, в котором контролируется успешность стирания инф-ии. Если ответ успешный, проверяется правильность принятых данных, 
    включая значение CRC. Если данные корректны, сбрасывается флаг ошибки ответа и осуществляется переход в режим опроса  текущих данных.
  - Режим передачи SendTXClear. Проверяется окончание времени передачи (UART0.RX_TIME) и окончена ли сама передача (UART0. TXCmpl). 
    В случае выполнения одного из этих условий - окончание передачи с переходом в режим приема и настройки таймера CNT1 на контроль времени приема.

 */
void Clear_MRK(void) //стирание полученной инфы в МРК если она !=0
 {if ((CONFIGMRO.OnMRK==0)||(UART0.Regime==SendTX)||(UART0.Regime==AnswRX))
   return;
  
//посылаем команду стереть, до 2х раз
  if (UART0.Regime==AnswRXClear)    //был режим приема, проверяем его окончание
   {//if ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) 
    if (UART0.RX_TIME==0)
     return;
    if (UART0.RXCmpl==1)            //ждем приема, анализ приема
     {if ((RX_UART0[0]==READMRK.NMB_MRK)&&(RX_UART0[1]==0x06)
          &&(RX_UART0[2]==ClearAddrH)&&(RX_UART0[3]==ClearAddrL)
          &&(CRC16(&RX_UART0[0],NmbByteClearMRK)==0))  //успешный ответ, т.е стерли
       {CLRB(NoAnswMRK,READMRK.NMB_MRK);
        CLRB(NoClearMRK,READMRK.NMB_MRK); //успешное стирание
        ReqMRK=0;
       }
      else //ошибка, новый запрос
       {SETB(NoAnswMRK,READMRK.NMB_MRK);   //нет ответа 
        SETB(NoClearMRK,READMRK.NMB_MRK);  //ошибка стирания
       }
     }
    else //превышено время ожидания ответа с МРК -20мс
     {SETB(NoAnswMRK,READMRK.NMB_MRK);   //нет ответа 
      SETB(NoClearMRK,READMRK.NMB_MRK);  //ошибка стирания
     }
    
    UART0.Regime=SendTX;
    UART0.DataType=Current;
    SendPoling_MRK();           //переход в режим опроса МРК, 
   }
//режим передачи     
  else if (UART0.Regime==SendTXClear) 
   {if ((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) 
     return;            //ждем конца передачи и контроль время
//передачу закончили, прием + контроль времени приема
    TCNT1=0;            //сброс таймер ожидания ответа
    UART0.RXBeg=0;   
    NMB_UART_RX0=0;
//таймер ожидания ответа      
    UART0.RX_TIME=0;
    //OCR1A = RTU_ANSW[S9600]*NmbByteClearMRK*3; //ждем 8*2*3= (или 45) байт
OCR1A = RTU_ANSW[S9600]*NmbByteClearMRK*10; //ждем 2*8*15=240байт  (240 мс)   
    UART0.Regime=AnswRXClear;        
    SETB(UCSR0B,RXEN0);          //вкл. приемник
   }
 }

/** @fn void SendPoling_MRK(void)
*   @brief Подготовка передачи для опроса МР-К и ее старт  

*/
void SendPoling_MRK (void)
 {  uint16 CRC_TX;
    CLRB(UCSR0B,RXEN0);          //выкл. приемник
    TX_UART0[0]=NMB_MRK[UART0.DataType];
    TX_UART0[1]=0x03;
    TX_UART0[2]=0x00;
    TX_UART0[3]=BeginAddrMRK[UART0.DataType];      //адрес регистра
    TX_UART0[4]=0x00;
    TX_UART0[5]=NmbByteReadMRK[UART0.DataType]/2;     //к-во слов чтения
    CRC_TX=CRC16(&TX_UART0[0],6) ;
    TX_UART0[6]=(uint8)CRC_TX;
    TX_UART0[7]=(uint8)(CRC_TX>>8);
    MAX_NMB_UART0=8;
    UART0.Regime=SendTX;
    
    UART0.TXBeg=1;               //начинаем передачу (+задержка 3 байт)
    UART0.TXCmpl=0;
    UART0.RX_TIME=0;
//задержка на  RTU_DELAY  
    TCNT0=0;                     //таймер времени между байтами
    StartT0; //вкл. Т0, :1024
    OCR0=RTU_DELAY[S9600]*2;     //3 байта
//посылка первого байта    
    NMB_UART_TX0=1;
    NMB_UART_RX0=0;
    CLR_BUF(&RX_UART0[0],16);
 }

/** @fn void SendClear_MRK(void)
*   @brief Подготовка передачи для стирания МР-К и ее старт  

*/
void SendClear_MRK (void)
 {  uint16 CRC_TX;
    CLRB(UCSR0B,RXEN0);          //выкл. приемник
    TX_UART0[0]=READMRK.NMB_MRK;
    TX_UART0[1]=0x06;
    TX_UART0[2]=ClearAddrH;
    TX_UART0[3]=ClearAddrL;      //адрес регистра
    TX_UART0[4]=(uint8)(READMRK.CRC>>8);
    TX_UART0[5]=(uint8)READMRK.CRC;
    CRC_TX=CRC16(&TX_UART0[0],6) ;
    TX_UART0[6]=(uint8)CRC_TX;
    TX_UART0[7]=(uint8)(CRC_TX>>8);
    MAX_NMB_UART0=8;
    UART0.Regime=SendTXClear;
    
    
    UART0.TXBeg=1;               //начинаем передачу (+задержка 3 байт)
    UART0.TXCmpl=0;
    UART0.RX_TIME=0;
//задержка на  RTU_DELAY 
    TCNT0=0;                     //таймер времени между байтами
    StartT0; //вкл. Т0, :1024
    OCR0=RTU_DELAY[S9600]*2;     //3*2 байта
//посылка первого байта    
    NMB_UART_TX0=1;
    NMB_UART_RX0=0;
    CLR_BUF(&RX_UART0[0],16);
 }

/** @fn void CheckDataMRK(void)
*   @brief Копирование в МР-О текущих и суммарных данных полученных от МР-К

    Данные,  полученные от МР-К (массив RX_UART0[]) проверяются на корректность. 
    В случае неправильного значения СRC ф-ия возвращает ошибку NoCorr. Если данные корректные:
  - В режиме опроса текущих данных Current - массив RX_UART0[] транслируется в структуру READMRK 
    (для дальнейшей записи в ЕЕПРОМ МР-О). Проверяется необходимость добавления новых данных к 
    хранящимся в ЕЕПРОМ: если имеются срабатывания или процент выработки ресурса ф-ия возвращает 
    HaveData (есть данные для суммирования), в противном случае - NoData (нет новых данных). 
    <b>Дополнительно: </b> В каждом из 16 МР-К есть 2 входа контроля отказа, соответствующие 1-му и 2-му ряду модулей. Для индикации аварий и отсылки аварий в АПК-ДК, при наличии флага отказа (контроль размыкания) устанавливаются флаги отказа в одном из 16 битах (для одного из 16 МР-К) DisplayFailMRK[0] для 1-го ряда и DisplayFailMRK[1] второго ряда. 
  - В режиме опроса суммарных данных - массив RX_UART0[] транслируются в структуру DATASUMMMRK. 
    Ф-ия возвращает HaveSummData.

*/
uint8 CheckDataMRK(void)        //копирование текущих/суммарных данных
 {uint8 i;
  if (UART0.DataType==Current)//копирование текущих данных
   {if (CRC16(&RX_UART0[3],sizeof(struct ReadMRK)-3)!=0)//некорр. ответ
     return NoCorr;
    READMRK.NMB_MRK=NMB_MRK[0];  //временная копия READMRK
    for(i=0; i<8; i++)
     READMRK.Resource[i]=RX_UART0[i+3];       //Данные начиная с 3 байта
    
    READMRK.SrabMod=(RX_UART0[8+3]<<8)+RX_UART0[9+3];
    READMRK.FlagDamage=(RX_UART0[10+3]<<8)+RX_UART0[11+3]; 
//отключение- длительное, поэтому не накапливаем а записываем текущее состояние
  if CHKB(READMRK.FlagDamage,0)
   SETB(DisplayFailMRK[0],READMRK.NMB_MRK);
  else
   CLRB(DisplayFailMRK[0],READMRK.NMB_MRK);
  if CHKB(READMRK.FlagDamage,1)
   SETB(DisplayFailMRK[1],READMRK.NMB_MRK);
  else
   CLRB(DisplayFailMRK[1],READMRK.NMB_MRK);
      
    READMRK.CRC=(RX_UART0[12+3]<<8)+RX_UART0[13+3];   //последовательность - наоборот?
    READMRK.Error=(RX_UART0[14+3]<<8)+RX_UART0[15+3];
   
    for (i=0;i<8;i++)                    //есть хоть 1 данное для суммирования?
     {if (READMRK.Resource[i]!=0)
       {i=0xff;
        break;
       }
     }
    
    if ((READMRK.SrabMod==0)&&(i!=0xff))//&&(READMRK.FlagDamage==0)) //есть хоть 1 данное для суммирования?
     return NoData; //нет новых данных, FlagDamage- отдельно в ЕЕПРОМ не записывается
    else
     return HaveData; //есть данные для суммирования
   }
  else                        //копирование суммарных данных
   {NMB_MRK[0]=0;             //после чтения суммы ,читаем текущий, старт с 0 адреса
    UART0.DataType=Current;   //закончили опрос суммарных, переходим к текущим
    if (CRC16(&RX_UART0[3],8+16+2)!=0)
     return NoData;           //ответ неверный, продолжаем опрос текущих
//ошибка компилятора?
    
    for(i=0; i<8; i++)        //временная копия
     {DATASUMMMRK_Copy.Resource[i]=RX_UART0[i+3];       //Данные начиная с 3 байта
      DATASUMMMRK_Copy.SrabMod[i]=(RX_UART0[2*i+8+3])*256+RX_UART0[2*i+9+3];
      DATASUMMMRK_Copy.ADC[i]=RX_UART0[i+8+16+2+3];
     }
/*
    for(i=0; i<8; i++)        //временная копия
     DATASUMMMRK_Copy.Resource[i]=RX_UART0[i+3];//Данные начиная с 3 байта
    for(i=0; i<8; i++) 
     DATASUMMMRK_Copy.SrabMod[i]=uint16((RX_UART0[i+8+3])<<8)+RX_UART0[i+9+3];
    for(i=0; i<8; i++)
     DATASUMMMRK_Copy.ADC[i]=RX_UART0[i+8+16+2+3];
*/
    DATASUMMMRK_Copy.CRC=(RX_UART0[8+16+3])*256+RX_UART0[8+16+4];
    return HaveSummData;
   }
 }

/** @fn void Link_UART_MRO(void)
*   @brief Копирование в МР-О текущих и суммарных данных полученных от МР-К

3 вида запросов: 
1.конфигурация (адреса 0х00...0х10) адрес 0, кол-во 32байт
2.накопленные данные. Начало 0х20 адрес, размер (34)26 байт. Адреса 0х20, 0х36, 0х58, 0х7А...  ,0х21E-0х23F
3.накопленные данные собственные. Начало- 0х240 адрес, размер 28 байт. Адреса 0х240,  0х25С, 0х278,  0х294... , 0x3Е4...0х3FF

*/ 

void Link_UART_MRO(void) //внешн RS485 для опроса MRО
 {
 }

/** @fn void Link_UART_APK(void)
*   @brief Связь  МР-О с АПК-ДК 

    Ф-ия формирует ответ на стандартный запрос, см. ПРОТОКОЛ ОБМЕНА МР-О  и  
    системы автоматизированного диспетчерского контроля АПК ДК

    <b> Ф-ия проверяет: </b> 
  - Флаги завершения передачи UART1.TXCmpl или превышение времени приема UART1.RX_TIME. 
    В случае наличия флагов -из режима передачи переход в режим прием, обнуляется структура управления данным 
    UART1, останавливается таймер контроля времени TCNT3 и выход из ф-ии (передача завершена или ошибка приема)
  - Флаги отсутствия окончания приема UART1.RXCmpl или продолжения передачи UART1.TXExec. При наличи флагов 
    -выход из ф-ии (ждем окончания приема или передачи)
    Если после проверка флагов нет выхода из ф-ии, это означает окончание приема информации от APK-ДК. 
    Данные раскодируются из формата ASCII ф-ией CONV_ASCII_UN() и проверяется корректность LRC с помощью 
    ф-ии LRC(). При наличии ошибки - устанавливается флаг ошибки,  стартует новый прием с выходом из этой ф-ии. 
    Если прием запроса от  АПК-ДК был корректный, готовятся данные для отправки в сеть всех типов аварий 
    с помощью DataAPKOrder() определяющей порядок передачи данных (в одном ответе могут не уместиться все 
    аварии и надо отправить все предыдущие аварии перед тем как начать обрабатывать новые). Далее выполняется 
    кодирование аварий ф-ей DataAlarmAPK() согласно протокола обмена с АПК-ДК. Данные по авариям располагаются 
    в массиве DATA_APK[].
    Из массива DATA_APK[] данные перекодируются в формат ASCII ф-ей CONV_ASCII() в массив TX_UART1[], 
    устанавливается количество байт на передачу MAX_NMB_UART1, устанавливается таймаут времени контроля 
    передачи в TCNT3 и запускается передача.
*/

void Link_UART_APK(void)
 {//если есть ошибка - стираем 
   
  
  if ((UART1.TXCmpl==1)||(UART1.RX_TIME==1))//||(UART1.RXERR==1))  //завершили передачу или превышено время или неверный прием
   {UART1.TXCmpl=0;
    UART1.TXExec=0;
    UART1.TXBeg=0;
    UART1.RXBeg=0;
    UART1.RXCmpl=0;
    UART1.RX_TIME=0;
    UART1.RXERR=0;
    NMB_UART1=0;

    CLRB(PORTD,DR1);    //разрешение приема
    TCNT3=0;            //сброс контроля времени
    TCCR3B=0;           //стоп таймер
    return;
   }
  if ((UART1.RXCmpl==0)||(UART1.TXExec==1)) //не приняли посылку или идет передача
   return;
//приняли посылку, проверяем
  UART1.RXCmpl=0;
  CONV_ASCII_UN(&RX_UART1[1],&RX_UART1[1],5); //раскодируем ASCII
  if   (RX_UART1[5]!=LRC(&RX_UART1[1], 4)) //проверяем LRC
   {UART1.RXERR=1;      //ошибка, продолжаем прием
    UART1.RXBeg=0;
    UART1.RX_TIME=0;
    CLRB(PORTD,DR1);    //разрешение приема
    TCNT3=0;            //сброс контроля времени
    TCCR3B=0;           // стоп таймер
    return;
   }
//формируем ответ 
  SETB(PORTD,DR1);      //запрет приема по 1
  DataAlarmAPK();       //формирование данных аварий для отправки по APK
  DATA_APK[0]=EAddrNet;  //addr0
  DATA_APK[1]=0X03;     //3 функция
  DATA_APK[2]=0X20;     //размер 
//DataAPKOrder();       //данные для последовательной отправки в сеть всех типов аварий:

  DATA_APK[19]=LRC(&DATA_APK[0], 19);

//конвертирование данных в ASCII
  CONV_ASCII (&DATA_APK[0],&TX_UART1[1],20);//в 40 байт
/*  
//test 
uint8 i;
{for (i=0;i<8;i++)
  DATA_APK[i+20]=DATASUMMMRK[1].ADC[i];
 for (i=0;i<8;i++)
  DATA_APK[i+28]=DATASUMMMRK[2].ADC[i];
  
 for (i=0;i<40;i++)
  TX_UART1[i+1]=DATA_APK[i];
 
 if (REGIME.Mode==Ish)
  {REGIME.SubMode=DisplAPK;
   REGIME.DisplDelay=Time_2s;     //задержка на индикацию
  }
}
//test  
*/ 
  TX_UART1[41]=0x0D; 
  TX_UART1[42]=0x0A;
  MAX_NMB_UART1=43;
  UART1.RXERR=0;
  NMB_UART_TX1=1;
  UART1.RXBeg=0;
  UART1.TXBeg=1;  //старт передачи
  UART1.TXExec=1; //идет передача
//запуск таймера на передачу, при превышении - принудительный переход на прием  
  TCNT3=0;
  TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //пуск время. 
  OCR3A=TimeStartUART1;    
 }

/** @fn uint8 convert_ASCII_byte(uint8 ASCII_byte)
*   @brief convert_ASCII_byte 
    \param[in] ASCII_byte байт в формате ASCII (0-9, A-F)
    \return  
        упакованный полубайт
*/
uint8 convert_ASCII_byte(uint8 ASCII_byte)
{
uint8 Temp;
  if ( (ASCII_byte>0x2F) && (ASCII_byte<0x3A) ) Temp = ASCII_byte & 0x0F; // проверка символа от 0 до 9
  else {
        Temp = ASCII_byte &  0xDF;	                // перевод в большой регистр буквы от A-F
	if ( (Temp>0x40) && (Temp<0x47) ) Temp -= 0x37; // проверка символа из CHAR в A до F
	else  Temp = 0x0F; // символ не в диапазоне
  }
return(Temp);	
}

/** @fn void CONV_ASCII_UN(uint8 *sourse,uint8 *dest,uint8 len)
*   @brief Кодирует в ASCII из адреса *sourse в *dest длиной len
    \param[in]  *sourse из адреса
    \param[in]  *dest  в адрес
    \param[in]  len  длиной
*/
void CONV_ASCII_UN(uint8 *sourse,uint8 *dest,uint8 len) //len=8, 39 us
 {uint8 i=0;
  while (len--)
   { *(dest)=convert_ASCII_byte(*(sourse+i))<<4;
      i++;
     *(dest++)|=convert_ASCII_byte(*(sourse+i));
      i++;
   }
 }

/** @fn convert_to_ASCII(uint8 HalfByte)
*   @brief Кодирует полубайт в ASCII 
    \param[in]  HalfByte
    \return  ASCII байт          
*/
uint8 convert_to_ASCII(uint8 HalfByte)
{HalfByte&=0X0F;
 if (HalfByte<0X0A)
  return HalfByte+0X30;
 else 
  return HalfByte+0X37;
}

/** @fn void CONV_ASCII(uint8 *sourse,uint8 *dest,uint8 len)
*   @brief Кодирует в ASCII из адреса *sourse в *dest длиной len
    \param[in]  *sourse полубайты для конвертации
    \param[in]           *dest
    \param[in]           len         
*/
void CONV_ASCII(uint8 *sourse,uint8 *dest,uint8 len)
 {//uint8 i=0;
  while (len--)
   { *(dest++)=convert_to_ASCII( (((*sourse)>>4)&0X0F) );
     *(dest++)=convert_to_ASCII( ((*sourse++)&0X0F) );
   }
 }

/** @fn uint8 LRC(uint8* data , uint16 len)
*   @brief расчет LRC
    \param[in]  *data полубайты для конвертации
    \param[in]           len длина 
    \return LRC
*/
uint8 LRC(uint8* data , uint16 len)
 {uint8 LRC=0;
  while( len-- )
   LRC+=*(data++);
 return  0xFF - LRC + 1;
 }

/** @fn CLR_BUF(uint8 *Data, uint8 NMB)
*   @brief очистка буфера с адреса
    \param[in]  *Data адрес
    \param[in]   NMB длина 
*/
 void CLR_BUF(uint8 *Data, uint8 NMB)
 {while (NMB--)
   *(Data+NMB)=0;
 }


/** 
Таблица расчета CRC
*/
__flash unsigned int TableCRCM_H[]={
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;
/** 
Таблица расчета CRC
*/
__flash unsigned int TableCRCM_L[]={
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

/** @fn uint16 CRC16(uint8 *buff,uint16 size)
*   @brief расчет CRC16 с адреса
    \param[in]  *buff адрес
               size длина
    \return СRC
*/
uint16 CRC16(uint8 *buff,uint16 size)
//1 1: C07E 780mks (100b)/4MHz- (medium) 705mks (high)
 {unsigned char uchCRCHi = 0xFF ;	// high CRC byte
  unsigned char uchCRCLo = 0xFF ;	// low CRC byte
  unsigned uIndex ;			// will index into CRC
  while (size--)		// pass through message buffer
   {uIndex = uchCRCHi ^ *buff++ ;	/* calculate the CRC */
    uchCRCHi = uchCRCLo ^ TableCRCM_H[uIndex] ;
    uchCRCLo = TableCRCM_L[uIndex] ;
   }
  return (uchCRCHi | uchCRCLo << 8 ) ;
}

/*
uint16 CRC16E(__eeprom uint8 *buff,uint16 size)
//1 1: C07E 780mks (100b)/4MHz- (medium) 705mks (high)
 {unsigned char uchCRCHi = 0xFF ;	// high CRC byte
  unsigned char uchCRCLo = 0xFF ;	// low CRC byte
  unsigned uIndex ;			// will index into CRC
  while (size--)		// pass through message buffer
   {uIndex = uchCRCHi ^ *buff++ ;	// calculate the CRC 
    uchCRCHi = uchCRCLo ^ TableCRCM_H[uIndex] ;
    uchCRCLo = TableCRCM_L[uIndex] ;
   }
  return (uchCRCHi | uchCRCLo << 8 ) ;
}
*/



//**************************   interrupt *****************************

// UART0 MRK
/** @fn void TIMER0_COMP_vec(void)
*   @brief таймер контроля времени между байтами -МРК
    
Таймер останавливается: 
  - В режиме старта приема (после получения первого байта и установления флага 
начала приема UART0.RXBeg), выполнение данного прерывания свидетельствует о превышении 
времени ожидания приема следующего байта (2 интервала), т.е. фиксируется конец приема и 
устанавливается флаг UART0.RXCmpl свидетельствующий о конце приема
  - В режиме старта передачи (UART0.TXBeg==1) данное прерывание используется для формирования 
задержки между концом приема и началом передачи. Выход порта включает передачу, в UDR0 
отсылается 1-й байт 
*/
#pragma vector=TIMER0_COMP_vect //таймер контроля времени между байтами -МРК
__interrupt void TIMER0_COMP_vec(void)

{StopT0; //stop timer
 if (UART0.RXBeg==1) 
  {UART0.RXBeg=0; 
   UART0.RXCmpl=1;// интервал больше 1.5 байта -конец приема
  }
//после установки TXBeg=1, выжидаем 2байта и начинаем передачу (в сумме передача через 2+1.5=3.5 байта) 
 else if (UART0.TXBeg==1)   
  {UART0.TXBeg=0;
   SETB(PORTE,DR0);        //вкл. передачу
   UDR0=TX_UART0[0];       //отправляем по RS485
  }
}

/** @fn void TIMER1_COMPA_vec(void)
*   @brief Таймер контроля времени ответа (приема) с МР-К 
    
  По прерыванию устанавливается флаг превышения времени UART0.RX_TIME
*/
#pragma vector=TIMER1_COMPA_vect //контроль времени приема -МРК
__interrupt void TIMER1_COMPA_vec(void)
 {//StopT1;         //стоп счетчик
  UART0.RX_TIME=1; //время превышено
 }

/** @fn void USART0_RXC_interrupt(void)
*   @brief Прием по UART с МРК
    
    Сбрасывается таймер контроля времени приема байта TIMER0.
    Если принят первый байт - устанавливается флаг начала приема UART0.RXBeg, 
    запускается TIMER0 с таймаутом ожидания приема следующего байта (время приема 2 байт).
    Принятые байты (до 48 штук с момента старта) - записываются в массив RX_UART0[]  
*/
#pragma vector=USART0_RXC_vect //опрос МРК
 __interrupt void USART0_RXC_interrupt(void) 
{uint8 DATA=UDR0;
    if (RX_UART0_TEST[1]!=(Time_1s-1))
    {RX_UART0_TEST[0]=DATA;  //в тестовом режиме - для индикации посл. байта
     RX_UART0_TEST[1]=Time_1s;     //в тестовом режиме - 1 сек индикации
    }
   TCNT0=0; //сброс таймера контроля времени между байтами
//начали чтение, запускаем контроль времени между байтами
   if (UART0.RXBeg==0)
    {UART0.RXBeg=1;
     OCR0=(RTU_DELAY[S9600]*2);
     StartT0; //вкл. Т0
    }
   if (NMB_UART_RX0<48) //пишем до окончания буфера
    *(&RX_UART0[NMB_UART_RX0++])=DATA ;
}
/** @fn void USART0_TXC_interrupt(void)
*   @brief Передача по UART в МРК 

    Отправка данных в UART0 до тех пор, пока их номер NMB_UART_TX0 не превысит  
    значения MAX_NMB_UART0. После чего передача останавливается, в UART0.TXCmpl 
    записывается флаг завершения передачи 
*/

#pragma vector=USART0_TXC_vect //опрос МРК 
 __interrupt void USART0_TXC_interrupt(void)
 {if (NMB_UART_TX0<MAX_NMB_UART0)
   UDR0 = TX_UART0[NMB_UART_TX0++];
  else               //стоп передачи
   {CLRB(PORTE,DR0); //выкл. передачу
    UART0.TXCmpl=1;  //передача завершена
   }
 }

// UART1 APK
/** @fn void TIMER3_COMPA_vec(void)
*   @brief Таймер контроля времени ответа с АПК-ДК  

    - В режиме старта приема (после получения первого байта и установления флага 
      начала приема UART1.RXBeg), выполнение данного прерывания свидетельствует 
      о превышении времени ожидания приема следующего байта (1 сек), т.е. фиксируется 
      конец приема и устанавливается флаг UART0.RX_TIME  свидетельствующий о конце приема
    - В режиме старта передачи (UART1.TXBeg==1) данное прерывание используется для 
      формирования задержки между концом приема и началом передачи. Здесь подается сигнал 
      включающий передачу и отсылается 1-й байт 0x3A в UDR1. Устанавливается также таймаут 
      1 сек. в CNT3 на контроль времени передачи.

*/
#pragma vector=TIMER3_COMPA_vect //таймер контроля времени АПК-ДК - 1 сек между байтами
__interrupt void TIMER3_COMPA_vec(void)
 {//
  if (UART1.RXBeg==1) 
  {UART1.RX_TIME=1;
  }
//после установки TXBeg=1, выжидаем 2байта и начинаем передачу (в сумме передача через 2+1.5=3.5 байта) 
 else if (UART1.TXBeg==1)   
  {UART1.TXBeg=0;
   UDR1=0x3A;              //отправляем по RS485
   OCR3A=Time1sUART;        //1 сек на передачу, при превышении - принудительный переход на прием 
  }
 }

/** @fn void USART1_RXC_interrupt(void)
*   @brief Прием по UART с АПК-ДК

      Для связи с  АПК-ДК используется ASCII MODBUS с большим допустимым интервалом 
      времени между принятыми байтами 1 сек. Для уменьшения времени обработки и реакции 
      на ошибку приема в прерывании анализируется каждый принятый байт посылки, и при его 
      некорректности сбрасывается флаг начала приема UART1.RXBeg,  устанавливается флаг 
      ошибки UART1.RXERR и прием начинается заново. 
  - Сбрасывается счетчик контроля времени между байтами CNT3
  - Т.к. приемник UART все время открыт на прием он принимает передаваемые данные. 
    Для отключении реакции на прием во время передачи  (UART1.TXExec=1) происходит выход из прерывания 
  - Проверяется имеется ли начало приема (UART1.RXBeg). Если нет - ожидается  стартовый байт 
    (0x3A), устанавливается флаг начала приема UART1.RXBeg  и запускается таймер контроля 
    времени между байтами CNT3.
  - Если имеется начало приема, проверяется корректность принятых байт: адреса в сети, номер 
    ф-ии  MODBUS и т.д. Также контролируется контрольная сумма LRC и код окончания посылки -байты 0x0D и 0x0A. 
  - После получения кода окончания посылки устанавливается флаг завершения приема UART1.RXCmpl,
    таймер CNT3 - останавливается.
  - Все корректные данные записываются в массив RX_UART1[].

*/
#pragma vector=USART1_RXC_vect //APK
 __interrupt void USART1_RXC_interrupt(void) 
{unsigned char DATA=UDR1;
   TCNT3=0;             //сброс контроля времени
   if (UART1.TXExec==1) //идет передача, не принимаем
    return;
   RX_UART1_TEST[0]=DATA;  //в тестовом режиме - для индикации посл. байта
   RX_UART1_TEST[1]=Time_1s;      //в тестовом режиме - 1 сек индикации    
   if (UART1.RXBeg==0)  //начало
    {if (DATA==0x3A)    //стартовый байт
      {TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //пуск время 140мкс*N
       OCR3A=Time1sUART;    //между байтами ждем не более 1 сек
       UART1.RXBeg=1;
       //RX_UART1[0]=0x3A;
       NMB_UART1=1;
       
      }
    }
   else 
    {switch (NMB_UART1)
     {case 1:  if (DATA==convert_to_ASCII((EAddrNet>>4)&0x0F))  //адрес ст. байт
                break;
               goto exit;
      case 2:  if (DATA==convert_to_ASCII(EAddrNet&0x0F))       //адрес мл. байт
                break;
               goto exit;
      case 3:  //3,5,7,8 байты
      case 5:  //ф-ия 03
      case 7:  //данные 00
      case 8: 
              if (DATA==0x30) 
               break;
              goto exit;
      case 4:  //4,6,8,9 байты 
              if (DATA==0x33) 
               break;
              goto exit;
       
      case 6: if (DATA==0x31) //Длина  сообщения 1
               break;
              goto exit;
              
      case 9:                 //проверка LRC (диапазон 0-F)
      case 10:
              if ((DATA>0x2F)&&(DATA<0x3A)) //0-9
               break;
              if ((DATA>0x40)&&(DATA<0x47)) //A-F
               break;
              goto exit;        

      case 11: if (DATA==0x0D)   //0x0D
                break;
               goto exit;
      case 12: if (DATA==0x0A)   //0x0A
                {UART1.RXCmpl=1;
                 TCNT3=0;  //сброс контроля времени
                 TCCR3B=0; // стоп таймер
                 break;
                }
      default:         
               goto exit;
               
     }
   RX_UART1[NMB_UART1++]=DATA;
     return;         
     
exit: 
       TCCR3B=0;     // стоп таймер
       UART1.RXBeg=0;
       UART1.RXERR=1;
     }
}
/** @fn void USART1_TXC_interrupt(void)
*   @brief Передача по UART в АПК-ДК 

    Отправка данных в UART1 до тех пор, пока их номер NMB_UART_TX1 не превысит  
    значения MAX_NMB_UART1. После чего передача останавливается, и устанавливается 
    флаг завершения передачи UART1.TXCmpl
*/
#pragma vector=USART1_TXC_vect //APK 
 __interrupt void USART1_TXC_interrupt(void)
 {TCNT3=0;             //сброс контроля времени
  if (NMB_UART_TX1<MAX_NMB_UART1)
   UDR1 = TX_UART1[NMB_UART_TX1++];
  else  //передача закончена
   UART1.TXCmpl=1;
 }