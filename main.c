// MRK

/** @file main.c 
*   @brief ������� �-��
*   @date 07.07.2014
*   @version 01.00.00
*
*   ���� ��������
  - ����������, ����� reset: 
	  - ������������� watchdog,
	  - ������������� ������ �����/������ initPort();
	  - ������������� �������� initTimer();  
	  - ������������� UART InitUSART(); 
	  - ������������� ������� InitDisplay();
  - �������� ����, ����������
	 - ���������� �����������, ������������, ������ 20 ��:
	    - ����� ������ Poll_button();
	    - ������� �� ������� ������ Poll_Regime();
	    - ����� �������� ������ (����������) Poll_Srab();
	    - �������� ������� � ������������� ��� ������������������ ���  ���������� ��������� � ����� �� ����� InitDisplay2(): � �������� ������ (��������� ���������) - ������ 2 ���, � ��������� ������� ( ��������� ��������) - ������ 20 ���.
	 - ���������� �����������, ������������ ������ 100 ��:
	    - �������� ��������� ���������� ��������� �� ������� �������� �������� FlashDisplay();
	    - ������ ���������� �� ������� Display();
	    - ���������� ���� Rele();
	    - �����  ��-� � ���-�� Link_UART_APK();
	    - �����  ��-� � ��-� Poling_MRK();
	    - ������ ������ ���������� �� ��-�, ������ �� � ������ AddDataMRK()
	    - ������ � ��-� �� �������� ���������� ���������� Clear_MRK();
  - ���������� ���������� ������� ������ 20 �� TIMER2_COMP_vec(void) �� �������� ���������������� ���������� Period_20ms.

\note
��������� �� �����:
1.+ � �� �������, ��������, ����������
2. � �� ����� 33, ������ ���� 12-22
3. ����������
4. ���� �� ���������
5. + ����. �� ������/������ ����������- ��� �������� ��� ��������� - ����

\par
\note 
� ����� ����� ���� RW PORTB_4 ������ PORTB_6*/

/*  
���. ������ �� ������ � ������ - �������� EnEEPROM
*/

//UART �������� ������� �� �����, �.�. ����� RE ��������!!!!!!
/*
//test
1.�������� �����- ��� ��������� ASCII.
//CONV_ASCII (&DATA_APK[0],&TX_UART1[1],20);
2. � ����. 16 ������ -���������� ��� ��� ��� 1 � 2
3. ��� ��������� ������������/������� ���/���: ��. up- �������� A�� - 8 ������
4. ��� �������� ������� �� ��� - �� �����. ������ 
5. � ���. ������ ��. up- �������� 8 ���� ������ ������ ���
*/


//#define DEBUG_AVR_STUDIO_4   1   //������� � ��� ������

#ifndef ENABLE_BIT_DEFINITIONS
#define ENABLE_BIT_DEFINITIONS
#endif

#include "inavr.h"
#include "ioavr.h"
#include "MRO.h"
#include "UART.h"

uint8 Period_display, /**< ����� ��������� */
Period_20ms, /**< ����� � 20 �� ���������� */
Period_Test, /**< ����� �������� ������� \note ������ 2 ���*/
Period_Reset, /**< ����� ������ ������� \note ������ 20 ���*/
Init_Stage;/**< */
struct Regime REGIME;/**< ����� ��-�*/
extern uint8 LINE1[], LINE2[];
extern uint8 PrepClear, PrepTest;
extern uint16 TimeRele;
extern uint8 ReleMode;
uint8 Period_butt; /**< ����� ������ ������*/

int main( void )
{
// #ifndef DEBUG_AVR_STUDIO_4
  /** - ����������, ����� reset:
        - ������������� watchdog,
        - ������������� ������ �����/������ initPort();
        - ������������� �������� initTimer();  
        - ������������� UART InitUSART(); 
        - ������������� ������� InitDisplay();
  */
__disable_interrupt();
 __watchdog_reset();// ������������� watchdog  
WDTCR|=(1<<WDE)|(1<<WDCE);
_nop;
WDTCR=(1<<WDP0)|(1<<WDP1)|(1<<WDP2)|(1<<WDE); //1.9 ���

__enable_interrupt();

 initPort();  //������������� ������ �����/������ 
 initTimer(); //������������� �������� 
 InitUSART(); //������������� UART 
 CLRB(PORTD,RELE);
//��� ������� �������� ������ ��������� ������� �����, ����� �� ������ ���� �������
 PrepClear=0xBB;

 while (CHKB(PINC,KEY1)==0)
  {PrepClear++;
   if (PrepClear==0xA5)
    break;
  }
 //��� ����� � �������� ����� ��������� ������� �����
 PrepTest=0xDD; 
 while (CHKB(PINC,KEY2)==0)
  {PrepTest++;
   if ((PrepTest==0xAA)||(PrepClear==0xA5))
    break;
  }
 
 Delay(60000);  //60 ms �������� ����� �������������� LED 
 Delay(60000);  //60 ms
 __watchdog_reset();
 Delay(60000);  //60 ms �������� ����� �������������� LED 
 Delay(60000);  //60 ms
 __watchdog_reset(); 

 InitDisplay();  //������������� �������
 REGIME.Mode=Plug;
 REGIME.DisplDelay=Time_2s;
 ReleMode=RelePower;
 TimeRele=Time_2s-(300/100); //������������ 2c ��� ��������� - 0.3s �� �������� �������

 TIFR=0XFF;    //�����  ������ ���������� ���� ��������, ��� ������ - �0 ����������

 TestDisplay();
Init_Stage=5;
 while(1)
  {__watchdog_reset();
   __enable_interrupt();
/** - �������� ����, ����������
      - ���������� �����������, ������������, ������ 20 ��:
        - ����� ������ Poll_button();
        - ������� �� ������� ������ Poll_Regime();
        - ����� �������� ������ (����������) Poll_Srab();
        - �������� ������� � ������������� ��� ������������������ ���  ���������� ��������� � ����� �� ����� InitDisplay2(): � �������� ������ (��������� ���������) - ������ 2 ���, � ��������� ������� ( ��������� ��������) - ������ 20 ���.
      - ���������� �����������, ������������ ������ 100 ��:
        - �������� ��������� ���������� ��������� �� ������� �������� �������� FlashDisplay();
        - ������ ���������� �� ������� Display();
        - ���������� ���� Rele();
        - �����  ��-� � ���-�� Link_UART_APK();
        - �����  ��-� � ��-� Poling_MRK();
        - ������ ������ ���������� �� ��-�, ������ �� � ������ AddDataMRK()
        - ������ � ��-� �� �������� ���������� ���������� Clear_MRK();
   */  
   if (Period_butt!=Period_20ms)
    {Period_butt=Period_20ms;
     Period_display++;
     Period_Test++;
     Poll_button();  // - ����� ������ 
     Poll_Srab();    // - ����� ������������ �� 6 ������
     Poll_Regime();  // - �������� �� ������� ������
     
     
    if (Period_Test>Delay_2s)
     {Period_Test=0;
      if (REGIME.Mode==Ish) //� ���. ���������- �������������� ������� ������ 2 ���
       Init_Stage=0;  //���� InitDisplay
       //InitDisplay();
      else   //� ���������- �������������� ������� ������ 20 ���, ��������� - ������ 2 ���
       {Period_Reset++;
        if (Period_Reset>10) //������ 20 ���
         {Period_Reset=0;
          Init_Stage=0;  //���� InitDisplay
         }
        else if (TestDisplay()==0)
         Init_Stage=0;  //���� InitDisplay
       }
      }
     InitDisplay2(); //������ 20 �� ���� ��� ���� InitDisplay

    }
//������ 100 ��
   if (Period_display>=Delay_01s)
    {Period_display=0;
     FlashDisplay();              //REGIME.Flash -����� 0.5 ���, �� ����� 0.2 ���
     if (Init_Stage>4) //�� ����� �������������- �� ������
      Display();   //���������
     Rele();       //���������� ����
    }

   if (REGIME.Mode>=Check)//||(REGIME.Mode==Check2)&&(CONFIGMRO.Circuit[0]==RC)&&((CONFIGMRO.Type==VZU_M)))
    {//Link_UART_MRO(); //��������. ���������� �� 485
     Link_UART_APK();//485-���
     Poling_MRK();   //������/������ ������ �� ���� ���
     Clear_MRK();    //�������� ���������� ���� � ��� ���� ��� !=0
    }
  }
}



//**************************   interrupt *****************************
/** - ���������� ���������� ������� ������ 20 �� TIMER2_COMP_vec(void) �� �������� ���������������� ���������� Period_20ms.*/
#pragma vector=TIMER2_COMP_vect
__interrupt void TIMER2_COMP_vec(void) //0,02 sec
 {Period_20ms++;
 }
