
/** @file MRO.c 
*   @brief �������� ��������� ���
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


uint8 Period_1s;            /**< ���-�� ������ */
uint8 TimeNoPress;          /**<����� ��������� ������ */
uint8 TimeAddr;             /**<����� �������� ������ */
uint8 ReadAddr;             /**<����������� ����� � ����*/
uint8 LINE1[48],            /**<1 ������ � 4-� ������� ������� ��� ���. ������+ ���������� ������. �� ��������� ������*/
LINE2[48],                  /**<2 ������ � 4-� ������� ������� ��� ���. ������+ ���������� ������. �� ��������� ������*/
ADDLINE[12];/**<1, 2 ������ � 4-� ������� ������� ��� ���. ������+ ���������� ������. �� ��������� ������*/
uint8 IndexResourse [16*8+1];/**<������ ���������� ������� 0...3bit -MRK, 4..7bit- Module in section*/
uint8 IndexSrab [16*8+1];   /**<������ ���������� ������������ 0...3bit -MRK, 4..7bit- Module in section*/
uint8 ReqMRK;               /**<������ ���*/                 
 
uint16 OnD4;        //���������� �����. ������ ��� ��� ����������������
uint8 OnCircMRO;    //���������� �����. ������ ��� ��� ����������������
uint8 NmbFreeMROIn; //��� ������  ����������� ���������� ����� ��� � � � ��, �� ������
uint8 LCOn;         //��� � � �� ��������� ������, =1 -��, =0 -��� ������  
 
//����������
uint16 DisplayFailMRK[2]; /**<[0]-����0(���1) 15...0 ���, [1]-����1(���2) 15...0 ��� */
uint8 DisplayFailMRO;     /**<6 ������/����� ���(0-5)*/ 
/**����� � ������. ��� ���������� ��� ������� �� �������- ��������� �����*/
uint16 TimeFailMRK[32];       
uint16 TimeFailMRO[6];    /**<����� � ������.*/
uint16 TimeRele;          /**<����� � ������.*/
uint8 ReleMode;           /**<����� ������ ����  RelePower (��������� �������)/ RelePowerEnd (������� ������)*/
/**�����*/
uint16 DisplayAlarmMRK[2];    //0��� -��-�0 ...15��� -��-�15. [0]-����0(���1), [1]-����1(���2)
uint8 DisplayAlarmMRO;        /**<�����*/
uint8 DisplayResourceMRK[16]; /**<��������� ������� MRK>80%*/
uint8 NMBResource;
uint16 DisplayNetwMRK;        /**<��� ����� � ���0-15 ������ 10�*/
uint8 TimeNoAnswMRK[16];  /**<����� ������ �����*/    
uint8 TimeSrabMRO[6];     /**<����� ������������ ���*/
extern uint16 NoAnswMRK;  /**<��� ������ ���, ��������*/
extern uint16 NoClearMRK; /**<��� ������� ���, ��������*/

uint8 NMBSrabSection;             /**<� ������������. ����� ������ �������� ����� �  ���� ��������� NMBSrabSection*/
uint8 DisplaySrabSection[16];     /**<8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]*/
uint8 DisplaySrabSectionForAPK[16];
uint16 DisplayFailMRK_copy[2];    /**��� ���������������� ��������� ���� ����� ������:
<0��� -��-�0 ...15��� -��-�15. [0]-����0(���1), [1]-����1(���2)*/
uint8 DisplayFailMRO_copy;        /**<���������������� ��������� ���� ����� ������:*/
uint16 DisplayAlarmMRK_copy[2];   /**<��� ���������������� ��������� ���� ����� ������: 
0��� -��-�0 ...15��� -��-�15. [0]-����0(���1), [1]-����1(���2)*/
uint8 DisplayAlarmMRO_copy;       /**<���������������� ��������� ���� ����� ������:*/
uint8 DisplayResourceMRK_copy[16];/**<���������������� ��������� ���� ����� ������:*/
uint16 DisplayNetwMRK_copy;       /**<���������������� ��������� ���� ����� ������:*/
uint8 DisplaySrabSection_copy[16]; /**<8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]*/

uint16 DisplayAlarmMRK_APK[2];/**<���������������� ��������� ���� ����� ������:*/
uint8 DisplayAlarmMRO_APK;    /**<���������������� ��������� ���� ����� ������:*/
uint8 DisplayResourceMRK_APK[16];/**<���������������� ��������� ���� ����� ������:*/
uint16 DisplayNetwMRK_APK;/**<���������������� ��������� ���� ����� ������:*/
uint8 DisplaySrabSection_APK[16]; /**<���������������� ��������� ���� ����� ������:
8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]*/

/** ������:  ���������, �������� � ����, ���.���� */
uint8 DisplayAllAlarm,  /**< ������:  ���������*/
//APKAllAlarm,            /**< ������:  �������� � ���� */
ReleAlarm;              /**< ���.���� */
uint8 PrepClear,        /**<��� ������� �������� ������*/
PrepTest;               /**<��� ��������� ������*/

extern uint8 Init_Stage; /**<������ ������������� �������*/
extern uint8 DATA_APK[];/**<������ ������ ��� �������� �� APK*/
//uint8 NmbSectionSend[16];  //������ �������� ��� ��������, �� 16 ���� ? (��� ���� � DATA_SEKT)
int8 NmbSendAPK, NmbCurrAPK;          //���������� ������ ��� ��������, ������� ����� ��������

uint8 DATA_SECT_APK[4*16];    //��� 16 ������ - 16 �������� �������� (�� 4 �� �������) 

extern uint8 RX_UART0_TEST[], RX_UART1_TEST[] ;

uint8 Resource80;       /**<��������� >80%*/
uint8 Event;            /**<������� ������������*/

uint8 RunShift;         /**<����� ��� ������� ������*/

struct ConfigMRO CONFIGMRO;/**<������������ ���*/
struct DataMRK DATAMRK;   /**<������ ��� ��������� ��������� ��� = 0xFF*/
struct Button BUTTON;     /**<������*/

__flash uint8 TVersion [2] [12]={"___��-�", "������_01.11"};/**< ����� ��� ���������� */
__flash uint8 TAddress [2] [12]={"A����_", "����._9600"};/**< ����� ��� ���������� */
__flash uint8 TConfig [3] [12]={"��������", "������������", "������_����."};/**< ����� ��� ���������� */
__flash uint8 TVZU [5] [11]={"���-���-�", "���-���-M", "���-���-�", "���-���-TM", "���-���-"};/**< ����� ��� ���������� */
__flash uint8 TSide [2][12]={"�������_", "�����._�����", /*"���._�����_"*/};/**< ����� ��� ���������� ��� � � �*/
__flash uint8 TWidth [5] [11]={"������_","630","660","900", "���"};
__flash uint8 TA630 [18] [10]={
"NA1-NA6","NA7-NA12","NA1-NA9","���", "���","���",          //M1 ��0 ��1 ���0 ���1 ��0 ��1
"NA1-NA6","NA7-NA12","���","���","NA1-NA4","NA5-NA8",     //M2
"���","���","���","���","���","���"};                       //M3
__flash uint8 TA660 [18] [10]={
"NA1-NA8","NA9-NA16", "���","���","NA1-NA6","���",
"���","���","���","���","���","���",
"���","���","���","���","NA1-NA18","���"};
__flash uint8 TA900 [18] [10]={
 "NA1-NA10","NA11-NA20","NA1-NA12", "���","NA1-NA4","NA5-NA8", 
 "NA1-NA10","NA11-NA20","���","���","NA1-NA4", "NA5-NA8",
 "���","���","���","���","���","���"};
 __flash uint8 TVZU_H [3] [12]={"���_�5-�8", "������������", "__���������?"};
__flash uint8 TCircuit [6] [8]={"����_","_��","_���","_��","_��/���", "��"};
__flash uint8 TSearch [4] [12]={"�����_��-�","�������_��-�", "-���", "_....." };/**< ����� ��� ���������� */
__flash uint8 TSection [13] [12]={"������_", "���_���_����", "��_��-�", "������_�", "���.", "���.A", "���", "���_��-�4", "��-�","_-���", "_-����", "���_������_", "���_����_2"};/**< ����� ��� ���������� */
__flash uint8 TRes_Srab [5] [12]={"�����-��_", "������._", "���-��_����.", "������������", "���" };/**< ����� ��� ���������� */
__flash uint8 TResp [2] [12]={"��������","������_" };/**< ����� ��� ���������� */
__flash uint8 TNoContr [6] [12]={"���_��������", "�����_������", "���-��_����.", "���_�����_�","��������_��", "������������" };/**< ����� ��� ���������� */
__flash uint8 TLC_MR [12]={"��-�_����."};/**< ����� ��� ���������� */
__flash uint8 TMod [13]={"������:"};/**< ����� ��� ���������� */
__flash uint8 TVirob [2] [12]={"���������", "�������!"};/**< ����� ��� ���������� */
__flash uint8 TZamen [2] [12]={"��������", "������"};/**< ����� ��� ���������� */
__flash uint8 TClear [9] [12]={"��������?","��������_���", "�������?", "����_������", "��������.", "������", "��������", "��������...", "�����_�����?"};/**< ����� ��� ���������� */
__flash uint8 TTest [18] [12]={"���������?", "Check_of", "___display", "������������", "������������", "`_`_`_`_`_`_", "_`_`_`_`_`_`", "����_������", "����_������", 
"����_������","����_����", "����_��_���", "����_��_����", "����_RS_���", "����_RS_���" ,  "�������_","___����", "_�������"  };/**< ���� ��� ���������� */
__flash uint8 TDig [6] = {"123456"};

//���. ����� ������ 630-900 ��� ��/���/��: 00-���, 01- 0�� ����, 10-  1 ��.����, 11 -��� ����
/*
const uint8 CheckPortW630[3][3]={
3,1,0,                   	  //M1 ��,���,��
3,0,3,                   	  //M2
0,0,0};                  	  //M3
const uint8 CheckPortW660[3][3]={//���. ����� ������ 660 ��� ��/���/��. 00-���, 01- 0�� ����, 10-  1 ��.����, 11 -��� ����
3,0,1,                    	 //M1 ��,���,��
0,0,0,                    	 //M2
0,0,0};                   	 //M3
const uint8 CheckPortW900[3][3]={//���. ����� ������ 900 ��� ��/���/��. 00-���, 01- 0�� ����, 10-  1 ��.����, 11 -��� ����
3,1,3,                    	 //M1 ��,���,��
3,0,3,                    	 //M2
0,0,1};                   	 //M3
*/
//���������� �������� ����� ��� ����������
//M1, 630, ���
//�1, 900, ���

//���������� �������� ����� ��� ����������
//�1, 660, ��
//�3, 900, ��




 const uint8 AllowWidth[3]={//����������� ���� ������ ��� �1/�2/�3
 7,  		//0b00000111,    //M1, bit 2-900 1-660 0-630 
 5,  		//0b00000101,  	 //M2, bit 900 660 630 
 2}; 		//0b00000010     //M3, bit 900 660 630
 const uint8 AllowCircuit[3][3]={//����������� ���� ����� ��� �1/�2/�3 630/660/900
 3, 5, 7, 	//0b00000011,  0b00000101,   0b00000111,       //M1:  630 660 900, bit 2-��/1-���/0-��
 5, 0, 5, 	//0b00000101,  0b00000000,   0b00000101,       //M2:  630 660 900, bit ��/���/��
 1, 4, 0};  //0b00000001,  0b00000100,   0b00000000      //M3:  630 660 900, bit ��/���/��
 
 
extern unsigned char convert_to_ASCII(uint8 HalfByte);
//******************************************************************************
//�������������

/** @fn void initPort(void)
*   @brief ������������� ������ �����/������
*
*/
void initPort(void)
  {DDRB=(1<<DB4)|(1<<DB5)|(1<<DB6)|(1<<DB7) |(1<<RW) ;
   DDRC=(1<<RS)|(1<<E);
   PORTC=(1<<KEY1)|(1<<KEY2)|(1<<KEY3)|(1<<KEY4); //button +5V
   DDRD=(1<<RELE)|(1<<TXD1)|(1<<DR1);  //������� RS485
   DDRE=(1<<TXD0)|(1<<DR0);            //����� RS485
   PORTE=(1<<DIP1)|(1<<DIP2)|(1<<DIP3)|(1<<DIP4)|(1<<DIP5); //dip to +5V
   PORTG=(1<<DIP6);                    //dip to +5V
  }

/** @fn void initTimer(void)
*   @brief ������������� ��������
*
*/
void initTimer(void)
  {OCR1A = 1000; //���� (140 ��)
   StartT1;
//������ �������� ������� ���-�� - 1 ��� ����� ������� � �������� ������� ��������/������   
   TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //���� �����. ���=OCR3A,  7372800/1024=7200Hz=140���
   OCR3A=Time1sUART;    //����� ������� ���� �� ����� 1 ���
   ETIMSK = (1<<OCIE3A);
//2-������������� ������ - �������� 20 ��   
   OCR2  = 144;      //20 ����
   TCCR2 = (1<<CS22)|(1<<CS20)|(1<<WGM21);  //���=OCR2 7372800/1024= 138.9 mks
   TIMSK  = (1<<OCIE0)|(1<<OCIE1A)|(1<<OCIE2);
  }
 
/** @fn void initDATA(void)
*   @brief ������������� ������.
*
*   ����������� ������� � ����������� ������������ ������� � �������, 
*   �������� ������ ����� �� ���� ���������� �����. ����� ����������� ����������� ������ � ��-�. 
*   ��� ������ ���������  ������� ������� (� ������� - ��� ������ 0xFF) ����������� ������ � ��-� ���������� 
*   �-�� ClearDATAMRK()
*   ��� ����������� ������ ������� ��������������� ����� ������, �-�� � ���������� ������������ 
*   ��� ��������������� �������� � ����� ������������.
*/
void initDATA(void) //��������� � CONFIGMRO 1 �� 2� ���������� ECONFIGMRO
  {uint8 i; 
   uint16 Error;
      
   i=0;
   CONFIGMRO=ECONFIGMRO[0].CONFIGMRO; //1-� �����
   if (CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-3)!=0)
    SETB(i,ErrorConfigMRO1);
    
   CONFIGMRO=ECONFIGMRO[1].CONFIGMRO; //2-� �����
   if (CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-3)!=0)
    {SETB(i,ErrorConfigMRO2);
     if CHKB(i,ErrorConfigMRO1)
      CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;  //��������������� 1-� �����
    }
    
   if ((ECONFIGMRO[0].CONFIGMRO.Error==0XFF)||(ECONFIGMRO[1].CONFIGMRO.Error==0XFF)) //���� 1-� ��������� - ������� ������
    {ClearDATAMRK();   //������ ������� ������ ��� � ������
     i=(1<<ErrorConfigMRO1)|(1<<ErrorConfigMRO2);             //Error
    }

   CONFIGMRO.Error=i;                    //��������� ����� ������
//   ECONFIGMRO[0].CONFIGMRO.Error=i; 
//   ECONFIGMRO[1].CONFIGMRO.Error=i;
 
//����������� ������ �� ��K   
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
*   @brief �������� ��������� ���������� �������� �������
*
*/
void ChekBusyDisplay (void)
 {uint16 i=0;
CLRB(PORTC,E); 
SETB(PORTB,RW);   //displ-READ
DDRB_READ; 
  while (i<10000)
   {SETB(PORTC,E);//������ DB7
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
*   @brief ����������� ������������� ������� ��� ������
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
   Write4bitDispl(0x06);//������ 01 I/D S/H
 
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
*   @brief ������������� ������������� �������.
*
*   �������� ������� � ������������� ��� ������������������ ���  ���������� ��������� � ����� �� ����� 
*   � ����������� �� �������� ���������� Init_Stage ����������� 1 �� 4 ������ ������������� �������. 
*   ����� ���������� ����� �� ������ Init_Stage ���������������� ��� ���������� ��������� ������ ��� 
*   ��������� ������ ������������. �.�. �� ������ ������ ������������� ���������� ������������� ����� 20 �� 
*   ��� ���������� ����������� ������ �������, ��� ������ ������� CPU �� �������� ��������� ��������
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
      Write4bitDispl(0x06);//������ 01 I/D S/H
   break;
   }
 Init_Stage++; 
 }
 
/** @fn void DisplayFunction(void)
*   @brief ������������� �������, ������� � ����� ������ �-�� 
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
//���������
 
/** @fn void Display(void)
*   @brief ������ ���������� �� �������. 
*
*   ��� ��������� ������� (REGIME.Mode) � ���������� (REGIME.SubMode) ��������� �  ������ ��-� ��������������
*   �������� ���������� � ������� � ����� �� ������ ���������������� ������� - ���������������� ��-�.
*/
void Display (void) //���������
  {uint8 i, n, m;
  
/** �� 1-� ����� �����������:
        - ��������� ������ ��������� ������� OnDisplay(1,0,0) ���� ����� ������ �� "��������"
        - ��������� ���������� REGIME.DisplDelay, �������������� ����� ���������
*/  
   if (REGIME.Mode!=Ish)
    OnDisplay(1,0,0);         //DISPL ON
 
    
   if (REGIME.DisplDelay!=0)  
    {if ((CHKB(PINC,KEY1))&&(CHKB(PINC,KEY2))&&(CHKB(PINC,KEY3))&&(CHKB(PINC,KEY4)))//��� ������� ������ - ����� �� ���������
      REGIME.DisplDelay--;
    }
  
/** �� 2-� ����� �������������� ������ � ������� ������� ������ ��-� (����� ���������������� � �������� ������������):
      ���������� �-�� �����������:
        - ����� ��������� ������� ����� 80%
        - �������� ������� ���������� ������  (�������� ����������) CheckTimeFail() ��� 6 �������� ������ ��- � � 2 ������ ��-�
        - ���� ��������� ������������ ������ (���������� ����REGIME.PermitAlarm) �� ���������� �-�� ����������� 
          ���� ����� ������ ��� �� ���������������� ������������ ��������� � ������� � ���-��.
          - ��������� ������ ��� ���������������� ��������� ���� ����� ������
          - �������� ���������� � ��-� � CheckFail_MRO(), ����� ��������� ���������, ����������� ������ ���� ������
          - �������� ���������� � ��-� � CheckFail_MRK(), ����� ��������� ���������, ����������� ������ ���� ������%
          - �������� ������������ 
          - �������� ������ � ��-� (���������� ����� ������������� �������)
          - �������� ������ � ��-� (���������� ����� ������������� �������) 
          - �������� ��������� ����� 80%
          - �������� ���������� ����� � ��-�   
*/   
   if (REGIME.Mode>Check)       //��������- ������ � ������� ������� ��� ������ > Check)
    {Resource80=PollingResource();         //����� ���������
     CheckTimeFail();           //�������� ������� ���������� ������ 
//�� ��������� �� ����� 3-� ���. ��������� � ����� ������� ������ 5 ���     
     if ( (REGIME.DisplDelay==0)&&(REGIME.PermitAlarm==1) )  
      {DataDisplOrder();        //������ ��� ���������������� ��������� ���� ����� ������:
       CheckFail_MRO();         //��������  ���������� - ��������� �� ��������� ��� ����� 
       CheckFail_MRK();
       CheckSrab();             //��������  ������������
       CheckAlarm_MRO();        //��������  ������� MRO
       CheckAlarm_MRK();        //��������  ������� MRK 
       CheckResource();         //�������� ��������� ����� 80%/ 
       CheckNetw();
      }
    }
   
/** - ������� ������� ������ LINE1, LINE2 �� �-�� ������ ���������� � ������� ��� ��������� 1-�� � 2-�� ������
*/    
   for (i=0;i<48;i++)//������� ������:
    {LINE1[i]=0x20;
     LINE2[i]=0x20;
    }
   for (i=0;i<12;i++)//������� ������:
    ADDLINE[i]=0x20;

/** �� 3-� ����� � ����������� �� ������ ������/��������� (��������� REGIME.Mode) �����������
*/
   switch (REGIME.Mode)
    {
/** - ����� ��������� Plug
    1. � ������� ����������� �������� ������� � ����� ������ �� � �-��
    2. ������������ ����� ��������� ������ ����������
    3. �  initDATA() ����������� ������� � ����������� ������������ ������� � �������
      ��� ����������� ������ ������� ��������������� ����� ������, �-�� � ���������� 
      ������������ ��� ��������������� �������� � ����� ������������
    4. ���������� ��������� ����� ������/��������� 
*/    
    case Plug: //���������
         LINE_Version();
         if (REGIME.DisplDelay==0)   
          {REGIME.DisplDelay=Time_2s;
           REGIME.Mode=AddressM;
           initDATA();
          }
     break;
/** - ����� ��������� ������ AddressM
    1. � ������� ����������� ������� ����� ������� � �������� ������ 
    2. ��������������� ����� ���������
    3. ���������� ��������� ����� ������/���������
*/    
     case AddressM:   //�����
         LINE_Address();
         if (REGIME.DisplDelay==0)   
          {REGIME.DisplDelay=Time_5s;
           REGIME.Mode=Config0;
          }
     break;
/** - ����� �������� ������������ Config0
    1. � ������� ����������� "�������� ������������" 
    2. ���� ��������� ������� -������, �.�. ���������������� �� ���� - ��������� ������� ������
       ��� �������� � ����. ���� ���������������� Config
    3. ���� ��-� ����� ��� ���������������, �� ��������� ������� �������� ���������� 
       ����� ������ "��������" � ����������� �������
*/      
     case Config0: //1� ������������. ��� �� ���������������, ������ �� ����������
          CREATE_LINE1(&TConfig[0][0]); //"��������"
          CREATE_LINE2(&TConfig[1][0]); //"������������"
          DisplayAllAlarm=0;
          CONFIGMRO.Side=M1;
          CONFIGMRO.Width=W630;
          for (i=0;i<4;i++)
           CONFIGMRO.Circuit[i]=LC;
          if (CONFIGMRO.Error!=0)   //1-� ������������, ���� ������� ������ up/dwn
           break;
          else if (REGIME.DisplDelay==0)  //�������� ������������ - �������   
           {REGIME.Mode=Ish;
            OnDisplay(0,0,0);//DISPL ON/OFF  -����.
           }
     break;
/** - ����� ������������ Config
    1. � ����������� �� �������� REGIME.SubMode � ������� ����������� ������� 
       ���� ��������� "���-���-�" / "���-���-�" / -� / -�� ��� ����� ������� ������ / ��� �: ������...���� 
    2. ����������� ����� ��-�
    3. ����������� SaveNmbSection() � �-�� � ������ ������������ 2 ����� ������������ ��-�, 
       ���������� ��������������� ������ ������ ��� ��������� ��-�
    4. �� ������� ������������ ��� ��������� ������ ��-�
*/      
     case Config: //������������
            REGIME.BegConfig=0;   //���������� � �������� ������������ �������� ����� ���
            REGIME.ErrorConfig=0; //���������� � �������� ������ ������������ 
            
            if (REGIME.SubMode<=VZU_TM)     //���������� ��� ���� ���
             {if (REGIME.Flash==0)
               CREATE_LINE1(&TVZU[4][0]);
              else
               CREATE_LINE1(&TVZU[REGIME.SubMode][0]);  //���-���-�...���-���-TM
              CopyDataLine(2);//�������� 2 � ������ ������� ������ �� LINE
             }
            else if (REGIME.SubMode==Side)//������� ��� -� � ���-�� ������� ��� � (������)
             {
//  ver 1.11
              if (CONFIGMRO.Type==VZU_E)
               {CREATE_LINE1(&TVZU[0][0]);      //���-���-�
                if (REGIME.Flash==0)            //����� ������
                 CHAR_ADDLINE(CONFIGMRO.Side);       //hex to text �� 99 -> ADDLINE
                CREATE_LINE2(&TSide[0][0]);     //�������_
               }
              else                              //��� -�
                {if (REGIME.Flash==0)           //����� ������
                  CHAR_ADDLINE(CONFIGMRO.Side); //hex to text �� 99 -> ADDLINE 1, 2, 3
                 CREATE_LINE1(&TVZU[1][0]);     //���-���-M
                 CREATE_LINE2(&TSide[1][0]);    //�����._�����
                }
/* ver 1.11  -����� �������� �� � ������. TSide ���._�����_        
CREATE_LINE1(&TVZU[CONFIGMRO.Type][0]);      //���-���-� / M
if (REGIME.Flash==0)           //����� ������
 CHAR_ADDLINE(CONFIGMRO.Side); //hex to text �� 99 -> ADDLINE 1, 2, 3
CREATE_LINE2(&TSide[CONFIGMRO.Type][0]);     //�������_ /���._�����_
 */                 
             }
            else if (REGIME.SubMode==Width)         //������ ��� �
             {CHAR_ADDLINE(CONFIGMRO.Side);         //hex to text �� 99 -> ADDLINE 1, 2, 3
              CREATE_LINE1(&TVZU[1][0]);            //���-���-M
              
              i=CREATE_TEXT(&TWidth[0][0],&LINE2[0]);//������_
              if (REGIME.Flash==0) //����� 0.5 ���, �� ����� 0.2 ���
               CREATE_TEXT(&TWidth[CONFIGMRO.Width][0],&LINE2[i]);    //630-900
              CopyDataLine(2);               
             }
            else if (REGIME.SubMode==Circuit)         //������ ��� �
             {CHAR_ADDLINE(CONFIGMRO.Side);         //hex to text �� 99 -> ADDLINE 1, 2, 3
              CREATE_LINE1(&TVZU[1][0]);            //���-���-M
              
              i=CREATE_TEXT(&TCircuit[0][0],&LINE2[0]);//����_
              if (REGIME.Flash==0) //����� 0.5 ���, �� ����� 0.2 ���
               CREATE_TEXT(&TCircuit[CONFIGMRO.Circuit[0]][0],&LINE2[i]);    //�� ��
              CopyDataLine(2);  //2 ������             
             } 
             
            else if (REGIME.SubMode==Search_MRK)//����� ���
             LINE_Searsh_MRK(); //������ ������� � Check
            else //������ �� ����������, ����� SubMode �� ���������� 
             { CREATE_LINE1(&TConfig[0][0]); //"��������", "������������"
               CREATE_LINE2(&TConfig[1][0]);
               REGIME.DisplDelay=Time_3s; //
             }
     break;
/** - ����� �������� ������ ��� Check
    ������������ ������ ������������ ������, ��������������� ������ ������������ � ���� ��-�
    ������������� ������ ��, ����������� ����������� ������ ��� � ���
    1. ��� ����� � � ��: ����� - 8 ������. ������� ��� ��������� ��� �������������� �� �����. ������� 1-8      
�� ���������� � ������� ������� �������������� ����� ��� �� �-�� 0. ����� ������ (�� ������� ���) ��������� (��� ������)
��� �����. � ����� ��� (��). ���� ��. ��� > ���������� ������ - "������ ����." ��� � ����� ���-�� ���� ��� D4, ��� �� �� �����������
    2. ��� ����� � � � ���� ������ �� ��� ��
*/     
     case Check:
     
     n=REGIME.NmbSection;
     uint8 i, k;
     
     Begin_Check();
     
    if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
     {i=CREATE_TEXT(&TSection[0][0],&LINE1[0]);
      if (REGIME.Flash==0)
       CHAR_LINE(n, &LINE1[i++]);   //������ 1(������)
      else
       LINE1[i++]='_';      
      if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� ��� ��� ����� ������ 
       {  if ( CONFIGMRO.OnCircMRO==0 )     //��� ����� ��� -���������
           CREATE_TEXT(&TSection[11][0],&LINE2[0]);  //��� ������ -�� ������                   
          else
           {if (CHKB(LCOn,n-1)==1)      //������ ��� ��� 8 ������- ���� �� =1 ��� ��� ������ =0 
             {if (REGIME.Flash==0)
               {CREATE_TEXT(&TCircuit[5][0],&LINE2[0]);        //��
                //LINE2[2]=ConvHex(NmbFreeMROIn);           //��� ���� �-�� ����� ������� ��� ������ ������ - ������.
               }
              DISPL_MRO_IN();               //��������� ���������� ������ ��� �� 2� ������
             }
            else
             {if (REGIME.Flash==0)            
               CREATE_TEXT(&TSection[11][0],&LINE2[0]);        //���_������, ������
             }
           }
       }
      else //���� ��� ��� ����� ������ 
       {CLRB(LCOn,n-1);      //��������� ��� ������ ������ ������ � ������� ���
        CREATE_TEXT(&TCircuit[3][0],&LINE1[i]); //��
       
        if(CONFIGMRO.Type==VZU_H)
         {//�������� ������ 0,1 ��K1  ��� 1� ������,��K2 ��� 2-��, ��K3  ��� 3-��
          if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -��������� 
           REGIME.ErrorConfig=1;
          else if(CHKB(DisplayFailMRK[1],n)==1)//D4 MRK -��������� 
           {CLRB(OnD4,n);   //��������� ���� D4 ��� ����������� �������� �-�� ���������
            CREATE_TEXT(&TVZU_H[0][0],&LINE2[0]); //��� �5...�8
           }
         } 
        else if(CONFIGMRO.Type==VZU_TM)    
         {if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -��������� 
           REGIME.ErrorConfig=1;
         }
       }

/*     if ( (REGIME.EndCheck==1)&&(REGIME.ErrorConfig==0) )    //��������� �������� 
       {CREATE_TEXT(&TVZU_H[1][0],&LINE1[0]);    //������������
        CREATE_TEXT(&TVZU_H[2][0],&LINE2[0]);    //_���������?
       } */
       
      CopyDataLine(1);
     }
    else //��� - �� VZU_H
     {
      i=CREATE_TEXT(&TSection[0][0],&LINE1[0]);
      if (REGIME.Flash==0)
       CHAR_LINE(n, &LINE1[i]);   //������ 1 - ��
      i++; 
       
     if (CONFIGMRO.Type==VZU_M)  
      {CREATE_TEXT(&TCircuit[CONFIGMRO.Circuit[0]][0],&LINE1[i]);    //��� � -�� [0] �� ��� ��
       CopyDataLine(1);
      }      
     else  //VZU_E
      {if (n>4) //���� ����� ���� � CONFIGMRO.Circuit[0-3] ��� 1-4 ��� 5-8 ������
        k=n-5;
       else
        k=n-1;
      
       if (CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0)//���� ��� - ���: ��/���
        {if ( ((CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==1))||(k==3) ) //���� ��� ����� ��� �� ����������, ��� 4 ��� 8 ������
          {CopyDataLine(1); //��/��� - � ���. ������ �� ����������
           CLRB(OnCircMRO,k*2);
           CLRB(OnCircMRO,k*2+1);//� ��������� �� �� ��������
          }
         else
          {CREATE_TEXT(&TCircuit[4][0],&LINE1[i]); //��� � ��/���
           RunDataLine(0, 3); //���. ������
          }
        }         
       else//���� ��� - ����: ��
        {CREATE_TEXT(&TCircuit[3][0],&LINE1[i]); //�� - �� ������
         CopyDataLine(1);
        }         
      }
  
//�������� ������, 2� ������
     if (CONFIGMRO.Type==VZU_M)
      {if (CONFIGMRO.Circuit[0]==LC)
        {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� ��� ��� ����� ������ 
          {//�������� ������ ��� 0,1 ��� 1� ������, 2,3 ��� 2-��, 4,5 ��� 3-��
           if (CHKB(PINA,(n-1)*2)==1) //0���� =1, ����� ������ �����. � 1 � ��� � 0
            REGIME.ErrorConfig=1;
           else if (CHKB(PINA,(n-1)*2+1)==1)//0���� =0, 1�� =1
            {CLRB(OnCircMRO,(n-1)*2+1);   //��������� ���� ��� ����������� �������� �-�� ���������
             i=CREATE_TEXT(&TWidth[4][0],&LINE2[0]); //���
             k=6*(CONFIGMRO.Side-1)+1; //������� � TA630-900: �1- ��0 ��1 ���0 ���1 ��0 ��1, �2- ��0 ��1 ���0 ���1 ��0 ��1, 
             if (CONFIGMRO.Width==W630)
              CREATE_TEXT(&TA630[k][0],&LINE2[i]); //M1 630 ��1: N�7-N�12. M2: N�13-N�24...
             else if (CONFIGMRO.Width==W660)
              CREATE_TEXT(&TA660[k][0],&LINE2[i]); //M1 660 ��1: N�9-N�16
             else
              CREATE_TEXT(&TA900[k][0],&LINE2[i]); //M1 900 ��1: N�11-N�20
             ChangeNmb(n,LINE2);  //������ ������� N �� Nmb 
            }
           else if (CHKB(PINA,(n-1)*2+1)==0)//0���� =0, 1�� =0//0���� =0, 1�� =0
            SETB(OnCircMRO,(n-1)*2+1);   //�������� ���� ��� ����������� �������� �-�� �������
          }
         else
           REGIME.ErrorConfig=1; //2 ������ ����������� ������ ������. ����� ���� �������
        }
       else if  (CONFIGMRO.Circuit[0]==LCS)
        {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� ��� ��� ����� ������ 
          {//�������� ������ ��� 0,1 ��� 1� ������, 2,3 ��� 2-��, 4,5 ��� 3-��
           if (CHKB(PINA,(n-1)*2)==1) //0���� =1
            REGIME.ErrorConfig=1;
           }
          else
           REGIME.ErrorConfig=1; //2 ������ ����������� ������ ������. ����� ���� �������
        }
       else //RC
        {ApplyMaskFailMRK();   //��������� ����� ������������� � ���������� ������ ���
         if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� ��� ��� ����� ������ 
          REGIME.ErrorConfig=1; //2 ������ ����������� ������ ������. ����� ���� �������
         else
          {//�������� ������ 0,1 ��K1  ��� 1� ������,��K2 ��� 2-��, ��K3  ��� 3-��
            if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -��������� 
             REGIME.ErrorConfig=1;
            else if(CHKB(DisplayFailMRK[1],n)==1)//D4 MRK -��������� 
             {CLRB(OnD4,n);   //��������� ���� D4 ��� ����������� �������� �-�� ���������
              i=CREATE_TEXT(&TWidth[4][0],&LINE2[0]); //���
              uint8 k=6*(CONFIGMRO.Side-1)+5; //������� � TA630-900: �1- ��0 ��1 ���0 ���1 ��0 ��1, �2- ��0 ��1 ���0 ���1 ��0 ��1,
              if (CONFIGMRO.Width==W630)
               CREATE_TEXT(&TA630[k][0],&LINE2[i]); 
              else if (CONFIGMRO.Width==W660)
               CREATE_TEXT(&TA660[k][0],&LINE2[i]); 
              else
               CREATE_TEXT(&TA900[k][0],&LINE2[i]);  //M1 900 ��1: N�5-N�8. M2: N�9-N�16...
              ChangeNmb(n,LINE2);  //������ ������� N �� Nmb
            }
           else if(CHKB(DisplayFailMRK[1],n)==0)//D4 MRK -��������� 
            SETB(OnD4,n);
          }  
        }
      }
     else if (CONFIGMRO.Type==VZU_E)//VZU_E
      {if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� ��� ��� ����� ������ 
        {if (k==3) //4 ��� 8 ������
          {CHAR_ADDLINE(n); 
           CREATE_LINE2(&TSection[11][0]);   //��� ������
           break;
          }
         //����������� ����� ��� ��� ������. ������
          if ( (CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==0) )//0���� =1, 1��=0
           REGIME.ErrorConfig=1;
          else if ( (CHKB(PINA,k*2)==1)&&(CHKB(PINA,k*2+1)==1) )//0���� =1, 1�� =1
           CREATE_TEXT(&TSection[11][0],LINE2);   //��� ������
          else if ( (CHKB(PINA,k*2)==0)&&(CHKB(PINA,k*2+1)==1) )//0���� =0, 1�� =1 
           {CLRB(OnCircMRO,k*2+1);   //��������� ���� ��� ����������� �������� �-�� ���������. ����� ���� �� ������ �� ���������, �.�. ��� - �� ���������
            CREATE_TEXT(&TSection[12][0],LINE2);   //��� ���� 2
           }
          else if ( (CHKB(PINA,k*2)==0)&&(CHKB(PINA,k*2+1)==0) )//0���� =0, 1�� =0
           SETB(OnCircMRO,k*2+1);
        }
       else     //����������� ����� ���
        { if (CHKB(DisplayFailMRK[0],n)==1)   //D3 MRK -���������
           REGIME.ErrorConfig=1;
          else if (CHKB(DisplayFailMRK[1],n)==1)
           {CLRB(OnD4,n);   //��������� ���� D4 ��� ����������� �������� �-�� ���������
            CREATE_TEXT(&TSection[12][0],LINE2);   //��� ���� 2
           }
          else if  (CHKB(DisplayFailMRK[1],n)==0)
           SETB(OnD4,n);
        } 
      }
    }//��� - �� VZU_H


      if (REGIME.ErrorConfig==1)
        {CREATE_TEXT(&TConfig[2][0],LINE2);//������_����.
        }
      CopyDataLine(2);
     break;
     
     
/** - ����� �������� Ish. 
     ����������� �������
*/     
     case Ish:    //��������
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
 CopyDataLine(1);//�������� � ������ ������� ������ �� LINE
 CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
 OnDisplay(1,0,0);         //DISPL ON
 
 if (REGIME.DisplDelay==0)
  {REGIME.SubMode=NoDispl;
   OnDisplay(0,0,0);//DISPL ON/OFF  -����.
  }
 }
       
else 
*/
       OnDisplay(0,0,0);//DISPL ON/OFF  -����.
     break;
     
/**- ����� ��������� Inform.
      - ����� ��������� ������� � ������������ ��� ���������� ������ ������ � ������. 
      ���� ��� �� ������ ������������� ��-� � ������� �������� "�������� �� ������������"
      �� ������� ������������, � ����������� �� �������� REGIME.SubMode:
    1.Modul_cect  -����� ������,  ������ ������ "�������"
    2.Modul_mod -����� ������, ������ ������ "�������"
    3.Modul_inf - ��������� ������ ������: "������ ��%. ������. ���"". ��� ����������� ������ - "�������� �� ������������"
      - ����� ��������� ������ ������ � ������ ��� �������������� �� �������� � �������� 
      �������� ������� � ������������. � ����������� �� �������� REGIME.SubMode ������������:
    1.Resource_res  -  ����� ��������� � ����������� �� �������,  ������ ������ "������"
    2.Resource_srab - ����� ��������� � ����������� �� ������������, ������ ������ "���-��_����"
    3.Resource_inf_res - ��������������� �������� ������� � ��������� ������ � ������
    4.Resource_inf_srab - ��������������� �������� ������������ � ��������� ������ � ������
*/     
     case Inform: //��������

      if (CONFIGMRO.OnMRK==0)  //�� ��������� �� ���� ���
         {CREATE_LINE1(&TNoContr[4][0]); //��������
          CREATE_LINE2(&TNoContr[5][0]); //�� ������������
          if (REGIME.DisplDelay==0)
           REGIME.Mode=Ish; //�� ��������� ��������� ��������� � Ish
          return;
         }
      
      n=FindNmbMRK(); //N ��������� ��� �� N �������� ������ REGIME.NmbSection
      if (REGIME.NmbModule>7)
       REGIME.NmbModule=7;
      m=REGIME.NmbModule+1; //��� ���������- +1, �.�. ��� 0 ������
      
      switch (REGIME.SubMode)
           {case Modul_cect:     //������ �������
            default:
             if (REGIME.Flash==0) //����� 0.5 ���, �� ����� 0.2 ���
              ADDLINE[0]=0;
             else
              CHAR_ADDLINE(REGIME.NmbSection);//hex to text �� 99 -> ADDLINE
             CREATE_LINE1(&TSection[0][0]);   //������� 
             CHAR_ADDLINE(m);                 //hex to text �� 99 -> ADDLINE
             CREATE_LINE2(&TSection[3][0]);   //�������
            break;
            case Modul_mod:                   //������ �������
             CHAR_ADDLINE(REGIME.NmbSection); //hex to text �� 99 -> ADDLINE
             CREATE_LINE1(&TSection[0][0]);   //������� 
             
             if (REGIME.Flash==0)   //����� 0.5 ���, �� ����� 0.2 ���
              ADDLINE[0]=0;
             else
              CHAR_ADDLINE(m);                 //hex to text �� 99 -> ADDLINE
             CREATE_LINE2(&TSection[3][0]);   //�������
            break;
            
            case Modul_inf:                 //������ ��%. ������. ���". 
             if (CHKB(CONFIGMRO.OnMRK,n)==0)  //��� - �� ���������
              {CREATE_LINE1(&TNoContr[4][0]); //��������
               CREATE_LINE2(&TNoContr[5][0]); //�� ������������
               if (REGIME.DisplDelay==0)
                REGIME.SubMode=Modul_cect; //�� ��������� ��������� ��������� � Modul_cect
               return;
              }
             ADDLINE[CHAR_ADDLINE(EDATAMRK[n].DATAMRK.Resource[m-1])]='%';//��� ��������� ���- +1, ������� ���� -1
             CREATE_LINE1(&TRes_Srab[0][0]);   //������
             
             INT_ADDLINE(EDATAMRK[n].DATAMRK.SrabMod[m-1]);  //��� ��������� ���- +1, ������� ���� -1      
             CREATE_LINE2(&TRes_Srab[1][0]);   //������
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
 CopyDataLine(1);//�������� � ������ ������� ������ �� LINE
 CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
break; 
*/
        
            case Resource_res:   //������ ������
             if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
              CREATE_LINE1(&TRes_Srab[0][0]);   //������
             else
              CopyDataLine(1);//�������� � ������ ������� ������ �� LINE
             CREATE_LINE2(&TRes_Srab[2][0]);   //���-��_����
            break;
            
            case Resource_srab:  //������ ���-��_����
             CREATE_LINE1(&TRes_Srab[0][0]);   //������
             if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
              CREATE_LINE2(&TRes_Srab[2][0]);   //���-��_����
             else
              CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
            break;
//������ ��%. ���.z ��� ��            
            case Resource_inf_res:
             //if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
             ADDLINE[CHAR_ADDLINE(EDATAMRK[n].DATAMRK.Resource[m-1])]='%';//��� ��������� ���- +1, ������� ���� -1 
             CREATE_LINE1(&TRes_Srab[0][0]);                   //������
             
             i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
             LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������;
             LINE2[i++]=' ';                                      //
             i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);
             LINE2[i]=ConvHex(m);       //N ������
             CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
            break;
            
            case Resource_inf_srab:
             //if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
             if (Event==0)
              {CREATE_LINE1(&TRes_Srab[3][0]);  //������������
               CREATE_LINE2(&TRes_Srab[4][0]);  //���
              }
             else
              {
             INT_ADDLINE(EDATAMRK[n].DATAMRK.SrabMod[m-1]); //��� ��������� ���- +1, ������� ���� -1 
             CREATE_LINE1(&TRes_Srab[1][0]);                   //���� 
             
             i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
             LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������;
             LINE2[i++]=' ';                                      //
             i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);
             LINE2[i]=ConvHex(m);       //N ������
             CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
             }
            break;
            }
     break;
     
/** - ����� ��������� ������������  Response
      ��� ������������ �� ������� ������������ "�������� ���", ��� � - ����� ������, � - ����� ������. 
      �� ��������� ����������� ��� ���������  ������� - ������� � �������� �����
*/     
     case Response://������������
//  ��� ������������ (+1): �� 3-4 ��� "�������� ���. �����". ���� ���������  - �� 3 ��� ��� ������� ������
//���� �������� ��� ���� �� 180 ���  
   
      if (REGIME.DisplDelay==0)
       REGIME.Mode=Ish; //�� ��������� ��������� ��������� � �������� �����
//"�������� ���. �x��" -xx section, y-module -����� � � ��, ��� ��� - ������ ����� ������           
      CREATE_LINE1(&TResp[0][0]);   //��������
      
      i=CREATE_TEXT(&TResp[1][0],&LINE2[0]);     //���.
      i+=CHAR_LINE(REGIME.NmbSection, &LINE2[i] ); //����� ������
      if ( (CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M) )
       {LINE2[i++]='A';
        LINE2[i++]=ConvHex(REGIME.NmbModule+1);//����� ������
       }  
      CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
     break;
/**- ����� ������ Alarm.
      �� ������� ������������, � ����������� �� �������� REGIME.SubMode:
      1.NoContr  -��� ��������  "���_��������" � ��������� ������ ������ � ������,  ��� ���������� ���� �������� ����������� ��-� ���� ��-�
      2.Breakdown - �����, "����� ������" � ��������� ������� ������ � ������, ������ ���� �������� ����������� ��-� ���� ��-� ����� 60 ���. ��� ��������� ������������ ��-� ���������� �������� ������� ������� � ��������� (�.�. � ������ ��������� ����� ��������������� ���������� ��������� �������������� �������)
      3.EndResource - ��������� �������, ������� "���������. �������" � "�������� ������" � ��������� ������� ������ � ������
      4.NoNetw -��� �����, ������� "��� �����" � ��������� ������� ������
*/     
     case Alarm:  //���������
// ��� ��������. ��� ���-���-� ������� "��� ��������. ���.�� �����", 
//��� ���-���-� ������� "��� ��������. ������ �". ���� ������������� -������� ��������
      if (REGIME.DisplDelay==0)
       REGIME.Mode=Ish; //�� ��������� ��������� ��������� � �������� �����
      switch (REGIME.SubMode)
           {case NoContr:      //��� ��������
            default://���������� ���� �������� ����������� ��-� ���� ��-�
            
            if (CONFIGMRO.Type==VZU_E)
             {if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
                CREATE_LINE1(&TNoContr[0][0]);   //���_��������
              else
                CopyDataLine(1);//���� ������
              if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
               LINE2_SEC_R(); //2 ������:��� NmbSection ��� NmbModule
              else
               CopyDataLine(2);//���� ������
             }
            else if (CONFIGMRO.Type==VZU_M)
             {CREATE_TEXT(&TNoContr[0][0],&LINE1[0]);   //���_��������
              goto   Type_VZU_M;
             }
            else    //� � ��
             {if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
               CREATE_LINE1(&TNoContr[0][0]);   //���_��������
              else
                CopyDataLine(1);//���� ������ 
              i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
              LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������; //2 ������:��� NmbSection ��� NmbModule
              CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
             }
                          
            break;
            
            case Breakdown:      //�����
//���������� ���� �������� ����������� ��-� ���� ��-� ����� 60 ���            
            
            if (CONFIGMRO.Type==VZU_E)
             {CREATE_LINE1(&TNoContr[1][0]);   //�����_������
              LINE2_SEC_R(); //2 ������:��� NmbSection ��� NmbModule
             }
            else if (CONFIGMRO.Type==VZU_M)//"����� ������. ������ �" 2."��� �������: ���� - ��zz".
             {CREATE_TEXT(&TNoContr[1][0],&LINE1[0]);      //1� ����� -�����_������
Type_VZU_M:             
              CREATE_TEXT(&TMod[0],&LINE1[12]);            //2� ����� ���_�������:
              i=CREATE_TEXT(&TSection[0][0],&LINE2[0]);    //1� ����� -������
              CHAR_LINE(REGIME.NmbSection,&LINE2[i]);      //N �������� ������ hex to text �� 99 -> LINE
              
              uint8 k;    //������ 
              k = 6*(CONFIGMRO.Side-1);          //������ �1, �2, �3
              k +=  (CONFIGMRO.Circuit[0]-1)*2;  //��������� ��� ����
              if (REGIME.NmbModule==2)
               k += 1 ;                               //��������� ����� ������, ���� =2, +1

              i=12;
//���������� ������:
              if (CONFIGMRO.Width==W630)
               i+=CREATE_TEXT(&TA630[k][0],&LINE2[i]); //M1 630 ��1: N�7-N�12. M2: N�13-N�24...
              else if (CONFIGMRO.Width==W660)
               i+=CREATE_TEXT(&TA660[k][0],&LINE2[i]); //M1 660 ��1: N�9-N�16
              else
               i+=CREATE_TEXT(&TA900[k][0],&LINE2[i]); //M1 900 ��1: N�11-N�20
 
              LINE2[i++]=0;
              ChangeNmb(REGIME.NmbSection,&LINE2[12]);
      
              
              RunString(12,2); //������� ������/ ����� 12, 2 ������
              SetAddrDisplay(0x00);         //1 ������ 
              CopyDataDisplay(&LINE1[REGIME.NmbLine1]); //2 ������ � 1-��
              SetAddrDisplay(0x40);         //2 ������ 
              CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //2 ������ � 2-��
             }
            else        //��� � � ��: "����� ������. ������ �"
             {CREATE_LINE1(&TNoContr[1][0]);   //�����_������
              i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
              LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������; //2 ������:��� NmbSection ��� NmbModule
              CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
             }             
            break;
            
             
            case EndResource:  //���������. �������
//��������� ����� 80% ������� ��-�. ������� 1."���������. �������!" 2."��������. ������ ����" 
             CREATE_TEXT(&TVirob[0][0],&LINE1[0]);           //1� ����� -���������
             CREATE_TEXT(&TZamen[0][0],&LINE1[12]);          //2� ����� ��������:
             CREATE_TEXT(&TVirob[1][0],&LINE2[0]);           //1� ����� -�������
             i=12;
             i+=CREATE_TEXT(&TZamen[1][0],&LINE2[12]);       //2� ����� ������:
             i+=CHAR_LINE(REGIME.NmbSection,&LINE2[i]); //hex to text �� 99 -> LINE
             if ((CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M))
              {LINE2[i++]='A';      //��������. ������ ����
               i+=CHAR_LINE((REGIME.NmbModule+1),&LINE2[i]); //hex to text �� 99 -> LINE
              }
             LINE2[i++]=0;
             
             RunString(12,2); //������� ������/ ����� 12, 2 ������
             SetAddrDisplay(0x00);         //1 ������ 
             CopyDataDisplay(&LINE1[REGIME.NmbLine1]); //2 ������ � 1-��
             SetAddrDisplay(0x40);         //2 ������ 
             CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //2 ������ � 2-��
            break;
            
            case NoNetw:   //�����(��� �����)
//���������� ����� � �����-���� ������� ��-�. ������� "����� ������. ��-���"
             CREATE_LINE1(&TNoContr[3][0]);     //��� �����            
             ADDLINE[CHAR_ADDLINE(REGIME.NmbSection)]; //����� ��-�
             CREATE_LINE2(&TLC_MR[0]);     //��-�
            break;
           }
     break;
     
/**- ����� �������� Clear.
      �� ������� ������������, � ����������� �� �������� REGIME.SubMode:
      1.Prepare - ���������� � ��������. � ����������� �� �������� REGIME.ClearMode 
      � ������� �������� ������ �� �������� ������������  "������� ��������.?" ��� 
      �� �������� ����� ������ "������� ���� ������?"
      2.Execute - ���������� ��������. � ����������� �� REGIME.ClearMode � ������� �������� 
      "��������... ��������" ��� "��������... ���� ������"
      3.ExecuteEnd - ���������� ��������. � ����������� �� REGIME.ClearMode � ������� �������� 
      "������ ��������" ��� "��������" � ����� ����� ���������� ������ � ������� ������  
*/     
     case Clear:  //��������
      switch (REGIME.SubMode)
           {case Prepare://"��������? ���.�� �����".
            default:
             if (REGIME.ClearMode==ClearMRK)
              {CREATE_LINE1(&TClear[2][0]);     //�������
               if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
                CREATE_LINE2(&TClear[4][0]);     //��������.?
               else
                CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
              }
             else if (REGIME.ClearMode==ClearEEPROM)
              {CREATE_LINE1(&TClear[2][0]);     //�������
               if (REGIME.Flash!=0) //����� 0.5 ���, �� ����� 0.2 ���
                CREATE_LINE2(&TClear[3][0]);     //����_������?
               else
                CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
              }
             else
              {CREATE_LINE1(&TClear[8][0]);     //�����_�����?
               LINE2_SEC_M(); //2 ������:��� NmbSection ��� NmbModule
              }
            break;
            
            case Execute://���������� -
              CREATE_LINE1(&TClear[7][0]);     //��������...
              if (REGIME.ClearMode==ClearMRK)
               CREATE_LINE2(&TClear[4][0]);     //��������
              else if (REGIME.ClearMode==ClearEEPROM)
               CREATE_LINE2(&TClear[3][0]);     //����_������
              else 
                LINE2_SEC_M();                   //��� NmbSection ��� NmbModule
             
              if (REGIME.DisplDelay==0)
               {REGIME.SubMode=ExecuteEnd;
                REGIME.DisplDelay=Time_2s;     //�������� �� ���������
               }
            break;
            
            case ExecuteEnd://"�������� 
              if ((REGIME.ClearMode==ClearMRK)||(REGIME.ClearMode==ClearEEPROM))
                {if (REGIME.DisplDelay!=0)
                  {CREATE_LINE1(&TClear[5][0]);     //������
                   CREATE_LINE2(&TClear[6][0]);     //��������
                  }
                 else
                  REGIME.Mode=Plug;
                }
               else               
                {if (REGIME.DisplDelay!=0)
                  {CREATE_LINE1(&TClear[1][0]);     //��������
                   LINE2_SEC_M();                   //��� NmbSection ��� NmbModule
                   }
                 else
                  REGIME.Mode=Ish;
                }
            break;
           }
     break;
     
 /**- ����� ������������ Test.
      �� ������� ������������, � ����������� �� �������� REGIME.SubMode:
1.�������� �������. ������� �� 4 ������: 
�	���� ����������� ������, �� ������ � ������� 2 ������ ���������� ������� �Check of display�
�	���� ��������, �� ������ � ������� 2 ������ ���������� ������� ��������� �� 12 ��������� � ��������� ����
�	���� �������� �������� ���������, � ������� 2 ������ ����� ��� ������� �������� ���������
�	���� �������� ������ ���������, � ������� 2 ������ ����� ��� ������� ������ ���������
2.�������� ������� ���������� (������). �� ������ � ������� ������ ������������ ������� ����� ������.
3.�������� ������. �� ������ � ������� ������ ���������� ������� ����� ������. �� ������ ������ ������������ ������ ����������� ������: �1 2 3 4 5 6�, 
���� ���� ������� � ��� ����� �� ������������
4.�������� �������� ������. �� ������ � ������� ������ ������������ ������� ����� ������. � ������ ������ ������ �����, ��������: ������ 32�. 
5.�������� ���� ��. �� ������ � ������� ������ ������������ ������� ����� ����. ��� ��������� ������ �Up� ���  �Dwn� ���� �� ���������� � �� ������ ������ 
���������� ������� ����� �� ���. ���� ������ �� ������ - ���� �� ���������, � �� ���������� ���������� ����� �� ����
6.�������� ���������� RS-485 ����� ��� � ��� ��. �� ������ � ������� ������ ���������� ������� ����� RS ��λ. ���� �� RS-485 � ��� ������� ����, 
�� �� ����� ��������� �� ������ ������, ��������: �������� 23�. ����� ������ ��������� ����� ��� ������� ������������������ ������, � ������ ������ � 0�24. 
7.�������� ���������� RS-485 ����� ��� � ���. �� ������ � ������� ������ ���������� ������� ����� RS ��ʻ. ���� � ��� ������� ����, 
�� �� ����� ��������� �� ������ ������, ��������: �������� 23�. ����� ������ ��������� ����� ��� ������� ������������������ ������, � ������ ������ � 0�24. 
*/     
     case Test:  //Test
      switch (REGIME.SubMode)
       {case TestPrep:
        default:
         CREATE_LINE1(&TTest[0][0]);
         CopyDataLine(2);//�������� 2 � ������ ������� ������ �� LINE
         REGIME.DisplDelay=Time_8s;
        break; //TestPrep
        
        case TestDispl:
          if (REGIME.DisplDelay>Time_6s)   //�� ����� �������� - ��������� ������ ����� ����������
           {CREATE_LINE1(&TTest[1][0]);
            CREATE_LINE2(&TTest[2][0]);
           }
          else if (REGIME.DisplDelay>Time_4s)
           {CREATE_LINE1(&TTest[3][0]); //"������������"
            CREATE_LINE2(&TTest[4][0]); //������������
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
        CREATE_LINE1(&TTest[7][0]);    //����_������
         
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
        SetAddrDisplay(0x40);           //2 ������
        CopyDataExDisplay(&LINE2[0]);   //����������� ������ �� ������ � Display ��� �����������
        break;  //TestButt

        case TestIn:
        CREATE_LINE1(&TTest[8][0]);    //����_������
        DISPL_MRO_IN();               //��������� ���������� ������ ��� �� 2� ������
        CopyDataLine(2);              //�������� � ������ ������� ������ �� LINE 
        break; //TestIn
        
        case TestAddress:
        CREATE_LINE1(&TTest[9][0]);    //����_������
        
        i=CREATE_TEXT(&TAddress[0][0],&LINE2[0]); //"A����_", 
        CHAR_LINE(EAddrNet, &LINE2[i++]);
        CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
        break;  //TestAddress
        
        case TestRelay:
        CREATE_LINE1(&TTest[10][0]);    //����_����
        if ((BUTTON.TimeUp!=0)||(BUTTON.TimeDwn!=0))
          CREATE_LINE2(&TTest[11][0]);    //����_��_���
        else
          CREATE_LINE2(&TTest[12][0]);    //����_��_����
        break;  //TestAddress
        
        case TestRS_MRO:
        CREATE_LINE1(&TTest[13][0]);    //����_RS_���
        i=CREATE_TEXT(&TTest[15][0],&LINE2[0]);  //� �������� ������ - ��� ��������� ����. �����
        if (RX_UART1_TEST[1]!=0)
         {LINE2[i++]=convert_to_ASCII(RX_UART1_TEST[0]>>4);
          LINE2[i++]=convert_to_ASCII(RX_UART1_TEST[0]&0x0F);
          if  (RX_UART1_TEST[1]==Time_1s)
            {SETB(PORTD,DR1);                   //������ ������ �� 1
             UDR1=(RX_UART1_TEST[0]+1);         //����������� �������� 1 �����
             UART1.TXExec=1;                    //���� ��������, �� ���������
            }
          else
           {UART1.TXExec=0;
            CLRB(PORTD,DR1);                    //���������� ������
           }
          RX_UART1_TEST[1]--;
         }
        CopyDataLine(2);                //�������� � ������ ������� ������ �� LINE
        break;  //TestAddress
        
        case TestRS_MRK:
        CREATE_LINE1(&TTest[14][0]);    //����_RS_���
        i=CREATE_TEXT(&TTest[15][0],&LINE2[0]);  //� �������� ������ - ��� ��������� ����. �����
        if (RX_UART0_TEST[1]!=0)
         {LINE2[i++]=convert_to_ASCII(RX_UART0_TEST[0]>>4);
          LINE2[i++]=convert_to_ASCII(RX_UART0_TEST[0]&0x0F);
          if  (RX_UART0_TEST[1]==Time_1s)
           {SETB(PORTE,DR0);            //���. ��������
            UDR0=(RX_UART0_TEST[0]+1);       //����������� �������� 1 ����� 
            } 
         else
          CLRB(PORTE,DR0);        //���. ��������
         RX_UART0_TEST[1]--;
        }         
        CopyDataLine(2);                //����������� � ������ �������
        REGIME.DisplDelay=Time_6s;
        break;  //TestAddress
        
        case TestEnd:
        CREATE_LINE1(&TTest[16][0]);    //����_RS_���
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
*   @brief ��������������� ������������ ������������ ������ ��� � ��� ��� ���� ����� ���  
*/
 void Begin_Check(void)
 {uint8 i, k, Type=CONFIGMRO.Type;
  if  (REGIME.BegConfig==0)  
   {REGIME.BegConfig=1;
    CONFIGMRO.OnCircMRO=0x3F;   //�� ��������� -�������� 6 ������ ���
    CONFIGMRO.OnD3=0xFFFF;      //�������� ����� D3 ���
    CONFIGMRO.OnD4=0xFFFF;      //�������� ����� D4 ���
    LCOn=0x00;                  //������ ��� ��� 8 ������- ���� �� =1 ��� ��� ������ =0. ��������� ��� ������ � ����� ��, ����� �� ���� �������� ��������� ������ ��� - ��������

  if ( (Type==VZU_H)||(Type==VZU_TM) ) 
   {for (i=0,k=0;i<6;i++)       //�������� ������ ���, �� ��������������� - ���������//3*2=6 ������ ���            
     {if (CHKB(PINA,i)==1)      //0���� =1, ����� ������ �����. � 1 � ��� � 0                          //
       {CLRB(CONFIGMRO.OnCircMRO,i);  //����. ���� ���
        k++;                           //���-�� ���. ������ ���
       }
     }
//���� ���. ��������� ����� ����� ��� ��� ������ ������
   for (NmbFreeMROIn=1;NmbFreeMROIn<7;NmbFreeMROIn++) //����  NmbFreeMROIn=7 -�� ��� ���������, ������ ���� ��� ���� �� 1 ��� �� 8 
     {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
       break;
     }
   if ( (NmbFreeMROIn>=7)&&((CONFIGMRO.OnMRK&0xFF)==0) )  
     REGIME.ErrorConfig=1; //������ ���� ��� ���� �� 1 ��� �� 8 � ����������� ��������� �����
  }
 else if (Type==VZU_M)   //M, 
  {if (CONFIGMRO.Side==M1)  //M1
    CONFIGMRO.OnCircMRO=0x03;//��� VZU_M1 - �������� 2 ������ ���
   else if (CONFIGMRO.Side==M2)
    CONFIGMRO.OnCircMRO=0x0F;//��� VZU_M1 - �������� 4 ������ ���
            
   if ( ((CONFIGMRO.Side==M1)&&(CONFIGMRO.Circuit[0]==LCS))&&((CONFIGMRO.Width==W630)||(CONFIGMRO.Width==W900)) )
     CLRB(CONFIGMRO.OnCircMRO,1); //���������� �������� ����� ��� ���������� ��� M1, 630, ���. �1, 900, ���
 
   if ( ((CONFIGMRO.Circuit[0]==RC)&&(CONFIGMRO.Width==W660))
        &&((CONFIGMRO.Side==M1)||(CONFIGMRO.Side==M3)) )
     CONFIGMRO.OnD4=0; //��������� ��� D4 ��� ���������� �1+�3, RC, W660
  }
  
//��� � � �: ���� ���������� ������� ������ ��� ��������� ��� � � ���, ���� ���� - � ��� �� ��������� 
 if ( (Type==VZU_M)||(Type==VZU_E) )
  {k=FindMinNmbSection();
   for (i=0;i<3;i++)//3*2=6 ������ ���            
    {if (CHKB(CONFIGMRO.OnMRK,k+i)==1)  //���� ��� ��� ����� ������                           //
       {CLRB(CONFIGMRO.OnCircMRO,i*2);  //����. ���� ��� � ��������� �������
        CLRB(CONFIGMRO.OnCircMRO,i*2+1);  //����. ���� ��� � ��������� �������
       }
    }
  }
  
    OnCircMRO=CONFIGMRO.OnCircMRO;     //��� ����������������
    OnD4=CONFIGMRO.OnD4;      
   }
 }
  
  
/** @fn void LINE_Version(void)
*   @brief � ������� ����������� �������� ������� � ����� ������ �� � �-�� 
*/
 void LINE_Version(void) 
 {ADDLINE[0]=0;
  CREATE_LINE1(&TVersion[0][0]); //"��-�", "������_01.0"
  CREATE_LINE2(&TVersion[1][0]);
 }
 
/** @fn void LINE_Address(void)
*   @brief � ������� ����������� ������� ����� ������� � �������� ������  
*/
 void LINE_Address(void)   
 {uint8 i;
  i=CREATE_TEXT(&TAddress[0][0],&LINE1[0]); //"A����_", "����._9600"
  CHAR_LINE(EAddrNet, &LINE1[i]);
  CopyDataLine(1);//�������� � ������ ������� ������ �� LINE
  
  CREATE_LINE2(&TAddress[1][0]);
 }

 
/** @fn void LINE_Searsh_MRK(void)
*   @brief ������������ ������ �� ������� ������ "�����_��-�" � �������� Find_MRK(), 
    
    Find_MRK �������� ������ � ���� RS-485  �� �������� �� 1 �� 16,  ���� �����, 
    � ������� ��� ���������� � ������������ ��-� CONFIGMRO.OnMRK ��� ��������� �� ����� ��-�. 
*/
void LINE_Searsh_MRK(void) //����� ���
  {uint8 i, n, NMB_MRK;
   if  ((CONFIGMRO.Type==VZU_E)||(CONFIGMRO.Type==VZU_M))
    i=1; //����.4 ���
   else if  ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
    i=2; //����.8 ���
  else  
    i=4; //����.16 ���
  
   if (RunString(12,i)!=0) //"������� ������" (4 ������ �� 16 �������) � ����� = 12
    {REGIME.Mode=Check;//������� � ����. ������-Check ����� ������������ ������
     REGIME.NmbSection=FindMinNmbSection();
    }

   if (REGIME.ClearMode==0)   //��������� - 1 ���
    {REGIME.ClearMode=1;
     CREATE_LINE1(&TSearch[0][0]);   //�����_��-�
     CREATE_LINE2(&TSearch[3][0]);  //...
   
     Find_MRK();        //����� ��� 2 ����. ��������� 100%. ����� 32*16=0.5� 
     //SaveNmbSection();  //��������� � ������ ������ ������ ��� ���0����3, ���
     REGIME.NmbLine2=0;
    }
   else
    {CREATE_LINE1(&TSearch[1][0]);   //�������_��-�
     if (CONFIGMRO.OnMRK==0) //��-� �� �������
      {CREATE_LINE2(&TSearch[2][0]); //"-���"
      }
     else
      {for(i=0,n=0; i<16; i++) //n- ����� ������� � ������2
         {if CHKB(CONFIGMRO.OnMRK,i)
           {if (i==0)
             NMB_MRK=0x16;  //��� 0�� ���� OnMRK ����� =16
            else 
             NMB_MRK=HEX_DEC(i);
            LINE2[n++]=ConvHex(NMB_MRK>>4);
            LINE2[n++]=ConvHex(NMB_MRK);
            LINE2[n++]=',';
           }
         }
      if (n<12) //���� ������ 4� ���, ��������� ������ 1� ������
        {for(i=0,n=12; i<12; ) //�������� 1� ������ �� 2 ������ ��� ��������� ���. ������
           LINE2[n++]=LINE2[i++];
        }

       SetAddrDisplay(0x40);         //2 ������ 
       CopyDataDisplay(&LINE2[REGIME.NmbLine2]); //���. ������: 4 �����
      }
    }
 }

/** @fn void LINE2_SEC_R(void)
*   @brief ��������� �� 2 ������ N �������� ������ � ���
*/
void LINE2_SEC_R(void) //2 ������:��� NmbSection ��� NmbModule
  {uint8 i;
   i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
   LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������;
   LINE2[i++]=' ';                                      //
   i+=CREATE_TEXT(&TSection[6][0],&LINE2[i]);      //���
   LINE2[i]=ConvHex(REGIME.NmbModule);    //��� =1 ��� 2        
   CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
  }

/** @fn void LINE2_SEC_M(void)
*   @brief ��������� �� 2 ������ N �������� ������ � ����� ������
*/
void LINE2_SEC_M(void) //2 ������:��� NmbSection ��� NmbModule
  {uint8 i;
   i=CREATE_TEXT(&TSection[4][0],&LINE2[0]);            //���
   LINE2[i++]=ConvHex(REGIME.NmbSection);     //N �������� ������;
   LINE2[i++]=' ';                                      //
   i+=CREATE_TEXT(&TSection[5][0],&LINE2[i]);      //���
   LINE2[i]=ConvHex(REGIME.NmbModule+1);//����� ������, ��� ��������� +1             
   CopyDataLine(2);//�������� � ������ ������� ������ �� LINE
  }

/** @fn void CopyDataLine(uint8 Line)
*   @brief �������� � 1 ��� 2 ������ ������� ������ �� LINE1 ��� LINE2
    @param[in] Line
      - 0: ����������� � 1 ������
      - ~0: ����������� � 2 ������
*/
void CopyDataLine(uint8 Line)
 {if (Line==1)
   {SetAddrDisplay(0x00);         //1 ������ 
    CopyDataDisplay(&LINE1[0]); //
   }
  else
   {SetAddrDisplay(0x40);         //2 ������ 
    CopyDataDisplay(&LINE2[0]); //
   }
 }

 /** @fn void RunDataLine(uint8 Line, uint8 Shift)
*   @brief ������� ������. �������� � 1 ��� 2 ������ ������� ������ �� LINE1 ��� LINE2, � ������������ ������� Shift ������ 0.5 ���
    @param[in] Line
      - 0: ����������� � 1 ������
      - ~0: ����������� � 2 ������
    @param[in] Shift - ������������ �������� ������  
*/
void RunDataLine(uint8 Line, uint8 Shift)
 { 
   if (REGIME.RunFlash!=REGIME.Flash)   //������ 0.3, 0.2 ���
    {REGIME.RunFlash=REGIME.Flash;
     if  (REGIME.Flash==1)              //������ 0.3+0.2=0.5 ���
      {RunShift++;
       if (RunShift>Shift)
        RunShift=0;
       }
     }
  if (Line==0)
   {SetAddrDisplay(0x00);         //1 ������ 
    CopyDataDisplay(&LINE1[RunShift]); 
   }
  else
   {SetAddrDisplay(0x40);         //2 ������ 
    CopyDataDisplay(&LINE2[RunShift]); //
   }
 }
 
 
/** @fn void FlashDisplay(void)
*   @brief �������� ��������� ���������� ��������� �� ������� �������� ��������

    ������������� ��������� ����� REGIME.Flash � 1 �� �������� 0.3 ��� � ����� �� 0.2 ��� 
    -��� ������������ �������� �������� �� �������
*/
void FlashDisplay(void)//����� 0.5 ���, �� ����� 0.5 ���
 {
  if (REGIME.FlashDelay!=0)//�������� �� �������
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
*   @brief "������� ������" 
    @param[in] Shift ��� ������� ������ � ��������
    @param[in] Nmb ��� ������� ������ � �������
    \return     
             - 0 ������� ������ �� ���������
             - 1 ������� ������ ���������
*/
uint8 RunString(uint8 Shift, uint8 Nmb) //REGIME.NmbLine1,2 - ������� ������ �� �����- Shift, ���-�� ������� Nmb
 {if (REGIME.RunStringDelay!=0)
   REGIME.RunStringDelay--;
  else   
   {REGIME.RunStringDelay=REGIME.RunStringNext;     //"������� ������" (4 ������ �� 16 �������) � ����� = 12
    REGIME.NmbLine1+=Shift;            //������ 1 �������, ����. ���������� 
    REGIME.NmbLine2+=Shift;            //������ 1 �������, ����. ���������� 
    if (REGIME.NmbLine1>=(Shift*Nmb))  //�� 2 ������ � ������ �� 12 ����
     {REGIME.NmbLine1=0;
      REGIME.NmbLine2=0;
      return 1;                        //end
     }
   }
  return 0;
 }
/*
uint8 Delta (uint8 Data_summ, uint8 Data) //������������� �������
 {if (Data_summ>=Data)
   return Data_summ-Data;
  else
   return  (256-Data)+Data_summ;
 }
*/

/** @fn uint8 PollingResource(void)
*   @brief ����� ��������� >80% 
    \return 
                - 0 ��� ��������� >80%
                - 1 ���� ��������� >80%
*/
uint8 PollingResource(void)       //����� ��������� >80%
 {uint8 i,n,RES80=0;
  for(i=0;i<16;i++)              //�� 16 ���
   {DisplayResourceMRK[i]=0;
    if CHKB(CONFIGMRO.OnMRK,i)   //��� ���������� ���
     {for(n=0;n<8;n++)           //�� 8 �������
       if (EDATAMRK[i].DATAMRK.Resource[n]>80) 
        {SETB(DisplayResourceMRK[i],n);
         RES80=1;
        }
     }
   }
  return RES80;
 } 
 
/** @fn void CheckTimeFail(void)
*   @brief �������� ������� (60�) ���������� ������ � ���������� ����� 
    
    �������� ������� ���������� ������  (�������� ����������) ��� 6 �������� ������ 
    ��- � � 2 ������ ��-� � ���������� ������ ������� �  ������� DisplayAlarmMRK[],
    �������� ������� ���������� c���� � ��-� � ���������� ������ ���������� c���� 
    � DisplayNetwMRK
*/
void CheckTimeFail(void) //�������� ������� (60�) ���������� ������ � ���������� �����.
 {uint8 i;
  DisplayAlarmMRK[0]=0;
  DisplayAlarmMRK[1]=0;
  DisplayAlarmMRO=0;
  DisplayNetwMRK=0;
 
  for(i=0;i<16;i++)
     {if (CHKB(CONFIGMRO.OnMRK,i)==0)
       continue;
//������ MRK 

      ApplyMaskFailMRK();   //��������� ����� ������������� � ���������� ������ ���
    
      if (CHKB(DisplayFailMRK[0],i)==0)
       TimeFailMRK[i]=0;
      else
       {if (TimeFailMRK[i]>Time_60s)
         SETB(DisplayAlarmMRK[0],i);//�����
        else 
         TimeFailMRK[i]++;
       }
      if (CHKB(DisplayFailMRK[1],i)==0)
       TimeFailMRK[i+16]=0;
      else
       {if (TimeFailMRK[i+16]>Time_60s)
         SETB(DisplayAlarmMRK[1],i);//�����
        else
         TimeFailMRK[i+16]++;
       }
//����� ���������� ����� � ���       
      if (CHKB(NoAnswMRK,i)==0)
       TimeNoAnswMRK[i]=0;
      else      
       {if (TimeNoAnswMRK[i]>Time_10s)
         {SETB(DisplayNetwMRK,i);
 //���������� ��������� ������ ��� ��� � ����������� ����� (����� ��������� �������) 
          CLRB(DisplayFailMRK[0],i);      //���������� ���
          CLRB(DisplayFailMRK[1],i);
          CLRB(DisplayFailMRK_copy[0],i); //���������� ���
          CLRB(DisplayFailMRK_copy[1],i);
         }
        else
         TimeNoAnswMRK[i]++;
       }
     }
//������ MR�    
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
*   @brief ��������  ���������� 16 ���  �� 2 ����. 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    �����, ��������� �������, ����� ���/��� �����, ������������
*/

void CheckFail_MRK(void) //��������  ���������� 16 ���  �� 2 ����
  {uint8 i;
 //0��� -��-�0 ...15��� -��-�15. [0]-����0(���1), [1]-����1(���2)
   if ((DisplayFailMRK_copy[0]==0)&&(DisplayFailMRK_copy[1]==0))
    {CLRB(DisplayAllAlarm,FailMRK); 
      return;
    }
    
   for(i=0;i<16;i++)               //16 ��� 
    {if CHKB(DisplayFailMRK_copy[0],i)
      {CLRB(DisplayFailMRK_copy[0],i); //���������������, ���������
       REGIME.NmbModule=1;             //����� ���� ��� ���������� ��-�
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //����� ������ ��� ���������� ��-�
       REGIME.Mode=Alarm;              //������
       REGIME.SubMode=NoContr;         //�����
       TimeSetDispl();
       return;
      }
     if CHKB(DisplayFailMRK_copy[1],i)
      {CLRB(DisplayFailMRK_copy[1],i);
       REGIME.NmbModule=2;             //����� ���� ��� ���������� ��-�
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //����� ������ ��� ���������� ��-�
       REGIME.Mode=Alarm;              //������
       REGIME.SubMode=NoContr;         //�����
       TimeSetDispl();
       return;
      }
    }
  }
 
/** @fn void CheckFail_MRO(void)
*   @brief ��������  ���������� MRO. 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    �����, ��������� �������, ����� ���/��� �����, ������������
*/
void CheckFail_MRO(void) //��������  ���������� MRO
   {uint8 i;
//��������� ����� ��������� ���������� MRK   
    i=DisplayAllAlarm&(0xFF<<FailMRO);//����� �� ������� ����������
    if ((i>(1<<FailMRO))||CHKB(DisplayAllAlarm,FailMRO==0))
     return;    
    if (DisplayFailMRO_copy==0) 
     {CLRB(DisplayAllAlarm,FailMRO); 
      return;
     }
    
    for(i=0;i<6;i++) //max 6 �������� ���� 1-�� NMBFail=0
     if CHKB(DisplayFailMRO_copy,i)
      {CLRB(DisplayFailMRO_copy,i);
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRO[i]; //� ������- �� 4 ���� � ContrCircMRO 
       REGIME.NmbModule=(i&1)+1;      //����� ���� ��� ���������� ��-�
       REGIME.Mode=Alarm;             //������
       REGIME.SubMode=NoContr;        //��� ��������
       TimeSetDispl();
       return;
      }
    
    }
 
/** @fn void CheckAlarm_MRK(void)
*   @brief ��������  ������� MRK. 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    ��������� �������, ����� ���/��� �����, ������������
*/

void CheckAlarm_MRK(void) //��������  ������� MRK
  {uint8 i;
   i=DisplayAllAlarm&(0xFF<<AlarmMRK);//����� �� ������� ����������
   if ((i>(1<<AlarmMRK))||CHKB(DisplayAllAlarm,AlarmMRK==0)) //��������� ����� ��������� ����������
    return;

   for(i=0;i<16;i++)               //��� 16 ��� �� 2 ����� �� ������
    {if CHKB(DisplayAlarmMRK_copy[0],i)
      {CLRB(DisplayAlarmMRK_copy[0],i);
       REGIME.NmbModule=1; //����� ���� ��� ���������� ��-�
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //����� ������ ��� ���������� ��-�
       REGIME.Mode=Alarm;             //������
       REGIME.SubMode=Breakdown;      //�����
       TimeSetDisplLn();
       return;
      }
     else if CHKB(DisplayAlarmMRK_copy[1],i)
      {CLRB(DisplayAlarmMRK_copy[1],i);
       REGIME.NmbModule=2; //����� ���� ��� ���������� ��-�
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //����� ������ ��� ���������� ��-�
       REGIME.Mode=Alarm;             //������
       REGIME.SubMode=Breakdown;      //�����
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
*   @brief ��������  ������� MR�. 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    ��������� �������, ����� ���/��� �����, ������������
*/
void CheckAlarm_MRO(void) //��������  �������
   {uint8 i;
    i=DisplayAllAlarm&(0xFF<<AlarmMRO);//����� �� ������� ����������
    if ((i>(1<<AlarmMRO))||CHKB(DisplayAllAlarm,AlarmMRO==0)) //��������� ����� ��������� ����������
     return;
    if (DisplayAlarmMRO_copy==0)
     {CLRB(DisplayAllAlarm,AlarmMRO);
      return;
     }

    for(i=0;i<6;i++) //max 6 �������� ���� 1-�� NMBFail=0
     if CHKB(DisplayAlarmMRO_copy,i)
      {CLRB(DisplayAlarmMRO_copy,i);
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRO[i]; //� ������- �� 4 ���� � ContrCircMRO
       REGIME.NmbModule=(i&1)+1; //����� ���� ��� ���������� ��-�
       REGIME.Mode=Alarm;             //������
       REGIME.SubMode=Breakdown;        //��� ��������
       TimeSetDisplLn();
       return;
      }
    }

/** @fn void CheckResource(void)
*   @brief �������� ��������� ����� 80%. 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    ����� ���/��� �����, ������������
*/
void CheckResource(void) //�������� ��������� ����� 80%
  {uint8 i, MRK, Module;
   i=DisplayAllAlarm&(0xFF<<ResourceMRK);//����� �� ������� ����������
   if ((i>(1<<ResourceMRK))||CHKB(DisplayAllAlarm,ResourceMRK==0)) //��������� ����� ��������� ����������, �������, 
    return; 
   
   for (i=0;i<16;i++)   //�������� �� ���� 16
    {MRK=DisplayResourceMRK_copy[i];
     if ((MRK!=0)&&(CHKB(CONFIGMRO.OnMRK,i)))
      goto NextCheck;
    }
   CLRB(DisplayAllAlarm,ResourceMRK);
   return;

NextCheck:
   for ( ;NMBResource<MaxSecMod; NMBResource++) //��������� �� ������� ���� ��-� 
    {i=NMBResource>>3;                 //����� ��-� �� ���� ������� (16 ��-� * 8 �������)
     if CHKB(CONFIGMRO.OnMRK,i)        //��������� ����������� ��-� 
      {MRK=DisplayResourceMRK_copy[i]; //��������� 8 ������� � ����� ��-�
       Module=NMBResource-i*8;         //����� ������ �� NMBResource
       if CHKB(MRK,Module)             //���� ������ c ���������� >80%
        {if (TimeNoPress>0)      //���� ������ ����� ������- (������ ����) �� ��������� �� ���������, ���� �� ����, ���� ������ �� ����� ��������
          CLRB(DisplayResourceMRK_copy[i],Module);
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //�� � ��-� ���������� � ������
         REGIME.NmbModule=Module;
         REGIME.Mode=Alarm;            //��������� � �����, ����. ����. ������ �� ���� 3 ���
         REGIME.SubMode=EndResource;   //���������. �������
         TimeSetDispl();
         return;                       //���� ����� �����
        }
      }
     else
      NMBResource|=7; //��������� ��� (����� NMBResource++ ������ 8)
    }
  }
 
/** @fn void CheckSrab(void)
*   @brief �������� ������������ 
    
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm.
    �����������: 
    ����� ���/��� �����
*/
void CheckSrab(void) //��������  ������������
 {uint8 i, MRK, Module;
//��������� ����� ��������� ����������, �������, ���������, ��� � ����, 
  i=DisplayAllAlarm&(0xFF<<SrabSection);//����� �� ������� ����������
  if ((i>(1<<SrabSection))||CHKB(DisplayAllAlarm,SrabSection==0))
   return; // 
  
  for (i=0;i<16;i++)   //�������� �� ���� 16
    {MRK=DisplaySrabSection_copy[i];
     if ((MRK!=0)&&(CHKB(CONFIGMRO.OnMRK,i)))
      goto NextCheck;
    }
   CLRB(DisplayAllAlarm,SrabSection);
   return;

NextCheck:
//����� ��������. ����������  DisplaySrabSection
//��������� 4 ������ �� 8 �������
  for (;NMBSrabSection<MaxSecMod; NMBSrabSection++ ) //��������� �� ���� ������� ���� ��-� ����� �����������
   { i=NMBSrabSection>>3;                 //����� ��-� �� ���� ������� (16 ��-� * 8 �������)
     if CHKB(CONFIGMRO.OnMRK,i)           //��������� ����������� ��-� 
      {MRK=DisplaySrabSection_copy[i];    //��������� 8 ������� � ����� ��-�
       Module=NMBSrabSection-i*8;         //����� ������ �� NMBSrabSection
       if CHKB(MRK,Module)                //���� ����������� ������
        {CLRB(DisplaySrabSection_copy[i],Module);//���������� ��� ��� �������� ���������������� ��������� 
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //�� � ��-� ���������� � ������
         REGIME.NmbModule=Module;
         REGIME.Mode=Response;            //��������� � ������������, ����. ����. ������ �� ���� 3 ���
         TimeSetDispl();
         return;                          //���� ����� �����
        }
      }
     else
      NMBSrabSection|=7; //��������� ��� (����� NMBResource++ ������ 8)
    }
  }
 
/** @fn void CheckNetw(void)
*   @brief �������� ���������� ����� � �����-���� ������� ��-� 
   
    ���� ���� ������ ��� ������- ���������, ����� ���� � _copy, 
    ���� ��� ������ ������� ���� ���������������- ����� ���� � DisplayAllAlarm
  
*/
void CheckNetw(void) //���������� ����� � �����-���� ������� ��-�.
  {uint8 i;
   i=DisplayAllAlarm&(0xFF<<NetwMRK);//����� �� ������� ����������
   if ((i>(1<<NetwMRK))||CHKB(DisplayAllAlarm,SrabSection==0)) //��������� ����� ��������� ����������, �������, ���������,  
    return; 
   
   for (i=0;i<16; i++) //��������� �� ���� ������� ���� ��-� 
    {if CHKB(DisplayNetwMRK_copy,i)            //���� ����������� ������
      {CLRB(DisplayNetwMRK_copy,i);       //���������� ��� ��� �������� ���������������� ��������� 
       REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[i]; //�� � ��-� ���������� � ������
       REGIME.Mode=Alarm;          //��������� � �����, ����. ����. ������ �� ���� 3 ���
       REGIME.SubMode=NoNetw;      //���������. �������
       TimeSetDispl();
       break;                      //���� ����� �����
      }
    }
   
   if (DisplayNetwMRK_copy==0)
    CLRB(DisplayAllAlarm,NetwMRK);
  }
  
/** @fn void TimeSetDispl(void)
*   @brief ����������� ����� ��������� ��� ������� ������ 

*/
//�������� ����� � 2 ���� �� ��������� � TimeSetDisplLn
void TimeSetDispl (void)
 {REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;
  REGIME.RunStringDelay=Time_1s;
  REGIME.RunStringNext=Time_1s;
  REGIME.DisplDelay=Time_2s;     //�������� �� ���������
 }

/** @fn void TimeSetDisplLn(void)
*   @brief ���������� ����� ��������� ��� ������� ������ 

*/
void TimeSetDisplLn (void) //������� ��������� - 3 ���
 {REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;
  REGIME.RunStringDelay=Time_2s;
  REGIME.RunStringNext=Time_2s;
  REGIME.DisplDelay=Time_4s;     //�������� �� ���������
 }

/** @fn void DataDisplOrder(void)
*   @brief ������ ��� ���������������� ��������� ���� ����� ������ 

*/
void DataDisplOrder(void) //������ ��� ���������������� ��������� ���� ����� ������:
  {uint8 i; //����� ��������� � _copy ����� ���� (������������������ ��������)
   if (DisplayAllAlarm!=0) //���� ��������� ���� ���������
    return;
   //uint16 OnMRK=CONFIGMRO.OnMRK;
//����� ��������� ���� ������ �� 3 ��� -����� ����    
   DisplayAlarmMRK_copy[0]=DisplayAlarmMRK[0];  //��� 1
   DisplayAlarmMRK_copy[1]=DisplayAlarmMRK[1];  //��� 2
   if ((DisplayAlarmMRK_copy[0]!=0)||(DisplayAlarmMRK_copy[1]!=0))
    SETB(DisplayAllAlarm,AlarmMRK);
   DisplayAlarmMRO_copy=(DisplayAlarmMRO & CONFIGMRO.OnCircMRO);
   if (DisplayAlarmMRO_copy!=0)
    SETB(DisplayAllAlarm,AlarmMRO);
//[0] -���1, [1] - ���2 ��� 16 ��ʹ 15...0 

   ApplyMaskFailMRK(); //��������� ����� ������������� � ���������� ������ ��� 
   DisplayFailMRK_copy[0]=DisplayFailMRK[0];
   DisplayFailMRK_copy[0]&=(~DisplayAlarmMRK_copy[0]);//����� �������� ���������� DisplayFail � ����� DisplayAlarm: ���������� ������������ � �� ������������
   DisplayFailMRK_copy[1]=DisplayFailMRK[1];
   DisplayFailMRK_copy[1]&=(~DisplayAlarmMRK_copy[1]);//����� �������� DisplayFail � DisplayAlarm: ���������� ������������
   if ((DisplayFailMRK_copy[0]!=0)||((DisplayFailMRK_copy[1]!=0)))
    SETB(DisplayAllAlarm,FailMRK);
    
   DisplayFailMRO_copy=(DisplayFailMRO & CONFIGMRO.OnCircMRO);
   DisplayFailMRO_copy&=~DisplayAlarmMRO_copy;//����� �������� DisplayFail � DisplayAlarm: ���������� ������������
   if (DisplayFailMRO_copy!=0)
    SETB(DisplayAllAlarm,FailMRO);
   
   DisplayNetwMRK_copy=(DisplayNetwMRK & CONFIGMRO.OnMRK); 
   if (DisplayNetwMRK_copy!=0)      
    SETB(DisplayAllAlarm,NetwMRK);
   
   for(i=0;i<16;i++)
    if CHKB(CONFIGMRO.OnMRK,i)
     {DisplaySrabSection_copy[i]=DisplaySrabSection[i]; //8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]
      DisplaySrabSection[i]=0; //���������� - ��������, �.�. ��� ������� ����������� � ������      
      if (DisplaySrabSection_copy[i]!=0)
       SETB(DisplayAllAlarm,SrabSection);
      DisplayResourceMRK_copy[i]=DisplayResourceMRK[i];
      if (DisplayResourceMRK_copy[i]!=0)
       SETB(DisplayAllAlarm,ResourceMRK);
     }
   
//�������� ������������� FailMRO ��� FailMRK: ��������� ���������
   if (CHKB(DisplayAllAlarm,FailMRO)||CHKB(DisplayAllAlarm,FailMRK))
    DisplayAllAlarm&=(1<<FailMRK)|(1<<FailMRO);

   NMBResource=0;
   NMBSrabSection=0;
   }
 
 /** @fn void ApplyMaskFailMRK(void)
*   @brief ��������� ����� ������������� � ���������� ������ ���

*/
void ApplyMaskFailMRK(void) //
 {DisplayFailMRK[0]&=CONFIGMRO.OnMRK;
  DisplayFailMRK[0]&=CONFIGMRO.OnD3;
  DisplayFailMRK[1]&=CONFIGMRO.OnMRK;
  DisplayFailMRK[1]&=CONFIGMRO.OnD4;
 }
 
 
 
/** @fn void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size)
*   @brief ������ � EEPROM
    @param[in]  __eeprom *pEDATA: ��������� �� EEPROM
    @param[in]  *pDATA ��������� �� ������ ��� ������
    @param[in]  Size ������ ������
      
    ��� ���������� ������� ������ - ����� ������ �� ��� ����������
*/
void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size)
 {uint8 i;
  for (i=0; i<Size; i++)
   {if (*pEDATA!=*pDATA)//��� ���������� ������� - ����� ������ �� ��� ����������
     *pEDATA=*pDATA;
    pEDATA++;
    pDATA++;
   }
 }

/** @fn void ClearDATAMRK(void)
*   @brief ������ ������� ������ ��� � ������

    0-�� ������ ����������  CRC
*/
void ClearDATAMRK(void) //������ ������� ������ ��� � ������
 {uint8 i;
  uint8 EnEEPROM=0xA7;   //���������� ������ � ������
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
*   @brief ������ ������� ������ ������
   

*/
void ClearDATAEEPROM(void) //������ ������� ������ ��� � ������
 {uint8 i;
  uint8 EnEEPROM=0xA5;   //���������� ������ � ������
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
*   @brief ����������� ������ �� ������ � Display � ������������ � ������ OLED �������
    @param[in] *Data ��������� �� ������

*/
void CopyDataDisplay(uint8 *Data) //���������� ������ �� ������ � Display � ������������ � ������ �������
 {uint8 ConvData, n, end=0;
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code.
  for (n=0,end=0;n<12;n++)
   {if (*(Data)==0)
     end=1; //����� ������� ����� ������ - ������ ������ �������� 0x20 
    if (end==0)
     ConvData=ConvFont(*(Data++)); //�������������� � ������ OLED 
    else
     ConvData=0x20;
    
    Write4bitDispl(ConvData>>4);
    Write4bitDispl(ConvData&0x0F);
    
   }
 }
 
/** @fn void CopyDataExDisplay(uint8 *Data)
*   @brief ����������� ������ �� ������ � Display ��� �����������
    @param[in] *Data ��������� �� ������

*/
void CopyDataExDisplay(uint8 *Data) //����������� ������ �� ������ � Display ��� �����������
 {uint8 n;
  SETB(PORTC,RS);//RS H: DATA, L:Instruction code.
  for (n=0;n<12;n++)
   {Write4bitDispl((*Data)>>4);
    Write4bitDispl((*Data)&0x0F);
	Data++;
   }
 } 

/** @fn uint8 CREATE_TEXT(uint8 __flash *pLn, uint8 *pText)
*   @brief ����������� ������ (12 ��������) �� FLASH � ��������� �����
    @param[in]  __flash *pLn ��������� �� ����� � FLASH
    @param[in]  *pText ��������� ���� ����������
    \return  ���������� ������������� ��������

*/ 
uint8 CREATE_TEXT (uint8 __flash *pLn, uint8 *pText) //�� pLn � pText �� 12 ��������
 {uint8 n=0;
  do
   {*pText++ =*pLn++;
    n++;
   }
  while ((*pLn)&&(n<12));
   return n;
 }
 
 /** @fn uint8 InsNmb(uint8 Nmb, uint8 *pText)
*   @brief ������ ������� N �� Nmb. ��� ������ ���� NA1-NA23 -> 1A1-1A23. �������� �� 12 ���������
    @param[in]  Nmb - ���������� ������
    @param[in]  *pText ��������� ���� ����������

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
*   @brief ����������� ������ (12 ��������) �� FLASH � LINE1 � ����� � 1 ������ �������
    @param[in]  __flash *pLn ��������� �� ����� � FLASH
    \return  ���������� ������������� ��������

    �������������  � ����� ������ ���������� ���-�� �� ADDLINE.
    ������: �������, �����(�����) � char ������� �� 0 �������
*/
void CREATE_LINE1 (uint8 __flash *pLn) //������: �������, �����(�����) � char ������� �� 0 �������
 {uint8 n=0, i=0;
  do
   LINE1[n++] =*pLn++;
  while ((*pLn)&&(n<12));
  if (ADDLINE[0]!=0)
   while ((ADDLINE[i])&&(n<12)) 
    LINE1[n++]=ADDLINE[i++];  //�������� ��������. ���-�� �� ����� ������
  LINE1[n] = 0;               //0-����� ������
  CopyDataLine(1);//�������� 2 � ������ ������� ������ �� LINE
 }
 
 
/** @fn uint8 CREATE_LINE2(uint8 __flash *pLn)
*   @brief ����������� ������ (12 ��������) �� FLASH � LINE2 � ����� � 2 ������ �������
    @param[in]  __flash *pLn ��������� �� ����� � FLASH
    \return  ���������� ������������� ��������

    �������������  � ����� ������ ���������� ���-�� �� ADDLINE.
    ������: �������, �����(�����) � char ������� �� 0 �������
*/ 
void CREATE_LINE2 (uint8 __flash *pLn) //������: �������, �����(�����) � char ������� �� 0 �������
 {uint8 n=0, i=0;
  do
   LINE2[n++] =*pLn++;
  while ((*pLn)&&(n<12));
  if (ADDLINE[0]!=0)
   while ((ADDLINE[i])&&(n<12)) 
    LINE2[n++]=ADDLINE[i++];
  LINE2[n] = 0;               //0-����� ������
  CopyDataLine(2);//�������� 2 � ������ ������� ������ �� LINE
 }
 
/** @fn void HEX_DEC(uint8 Data)
*   @brief ����������� �� HEX � DEC ������ �� 99
    @param[in]  Data ������ HEX
    \return  ������ DEC

*/ 
uint8 HEX_DEC (uint8 Data) //hex to dec 99
 {uint8 n=Data/10;
  return Data+n*6;
 } 

/** @fn uint16 INT_DEC(uint16 Data)
*   @brief ����������� �� INT � DEC ������ �� 999
    @param[in]  Data ������ INT
    \return  ������ DEC

*/ 
uint16 INT_DEC (uint16 Data) //hex to dec 999 (380cycles) 
 {uint8 n;
  n=Data/100;
  return (n<<8)+HEX_DEC(Data-n*100);
 } 
 
/** @fn uint8 CHAR_LINE (uint8 Data, uint8 *pLn)
*   @brief ����������� �� hex � ����� ������ ������� �� 99
    @param[in]  Data ������ hex
    @param[in]  *pLn ��������� �� ����� ���� ������

    \return  ���-�� ���������

*/ 
uint8 CHAR_LINE (uint8 Data, uint8 *pLn) //hex to text �� 99 -> LINE
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
    *pLn=0;          //���� ������
    return 2; //����� ����. ����������
   }
  else
   {*pLn++=ConvHex(Data);
    *pLn=0;          //���� ������
    return 1; //����� ����. ����������
   }
 }
 
/** @fn uint8 CHAR_ADDLINE (uint8 Data)
*   @brief ����������� �� hex � ����� ������ ������� �� 99 � ADDLINE
    @param[in]  Data ������ hex

    \return  ���-�� ���������
*/ 
uint8 CHAR_ADDLINE (uint8 Data) //hex to text �� 99 -> ADDLINE
 {return CHAR_LINE(Data, &ADDLINE[0]);
 }
 
/** @fn INT_ADDLINE (uint16 Data)
*   @brief ����������� �� INT � ����� ������ ������� �� 999 � ADDLINE
    @param[in]  Data ������ hex

    \return  ���-�� ���������
*/ 
void INT_ADDLINE (uint16 Data) //hex to text �� 999 -> ADDLINE
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
*   @brief ��������� ���������� ������ ��� �� 2� ������
    ��� ������ Check � TestIn
*/
void DISPL_MRO_IN(void) 
{uint8 i;
 if (REGIME.Mode==Check)
  {for (i=0;i<6;i++)
    {if (CHKB(CONFIGMRO.OnCircMRO,i)!=0)
      LINE2[i+6] =TDig[i];
    }
   if  (REGIME.Flash!=0) 
    LINE2[NmbFreeMROIn+5]='_'; //������ ����� ��� ��
  }
 else //test 
  {for (i=0;i<6;i++)
    {if (CHKB(PINA,i)!=0)
      LINE2[i*2] =TDig[i];
    }
  }
}   
 
/** @fn uint8 FindMinNmbSection(void)
*   @brief ����� ����������� ������ ������ � CONFIGMRO
    \return  ���������� ������ ������
*/ 
uint8 FindMinNmbSection(void) //����� ����������� ������ ������
 {uint8 i, Nmb, NmbSection;
  Nmb=99;    //��� ������ ����������� ������ ������
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
*   @brief ����� ����������� ������ ������ � CONFIGMRO
    \return  ���������� ������ ������
*/ 
uint8 FindMaxNmbSection(void) //����� ����������� ������ ������
 {uint8 i, Nmb, NmbSection;
  Nmb=1;    //��� ������ ����������� ������ ������
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
*   @brief ����� ������ MRK �� � ������
    \return  ����� MRK
*/ 
uint8 FindNmbMRK(void)//����� ������ MRK �� � ������
 {uint8 i;
  for(i=0; i<16; i++) //n- ����� ������� � ������2
   {if (CONFIGMRO.NmbSectionMRK[i]==REGIME.NmbSection)
     return i;
   }
  return 0xFF;       
 }

//**************************************************************************
//             OLED DISPLAY
/** @fn void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink)
*   @brief ���������� ��������
    @param[in]  On ��� �������
    @param[in]  Cursor ��� �������
    @param[in]  Blink ��� �������
*/ 
void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink)//������ 1DCB. DISPLAY D: ON=1/OFF=0. Cursor C: ON=1/OFF. Blinking B: of Cursor
 {
  CLRB(PORTB,RW);   //displ-WRITE
  DDRB_WRITE;
  
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  uint8 Data;
  Write4bitDispl(0x00);
  Data=(1<<3);
  if (On!=0)
   Data|=(1<<2); //���. ���������
  if (Cursor!=0)
   Data|=(1<<1);  //���. ������
  if (Blink!=0)
   Data|=(1<<0);  //���. �������
  Write4bitDispl(Data);

 }

/** @fn void ClearDisplay(void)
*   @brief ������� �������
    
*/ 
void ClearDisplay(void)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;  
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Write4bitDispl(0x00);
  Write4bitDispl(0x01);//Clear
//����������� - ��������  
  Delay(20000);   //20ms
 
 }
/** @fn void HomeDisplay(void)
*   @brief ��������� � 0 ��������� �������
    
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
*   @brief ����� ������� -����������
    
*/ 
void FunkSetDisplay(void)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;
  CLRB(PORTC,RS);//RS H: DATA, L:Instruction code.
  Write4bitDispl(0x02);
  Write4bitDispl(0x02);
  Write4bitDispl(0x0A);
//������ NF00 N: When N=1, the 2-line. When N =0, the 1-line.  F: When F=0,  5 x 8 dot character font
//(FT1, FT0) = (1, 0), the ENGLISH_RUSSIAN CHARACTER FONT TABLE
 }
 
/** @fn void Write4bitDispl(uint8 Data)
*   @brief ������ 4 ��� � �������
    @param[in]  Data ������ ��� ������
*/ 
void Write4bitDispl(uint8 Data)
 {
CLRB(PORTB,RW);   //displ-WRITE
DDRB_WRITE;

  PORTB&=0xF0;
  PORTB|=(Data&0x0F);
  SETB(PORTC,E);
  Delay(5);
  CLRB(PORTC,E); //������ Data
  Delay(10);
 }

/** @fn uint8 Read8bitDispl(void)
*   @brief ������ 8 ��� �� �������
    \return  ����������� ����
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
*   @brief ��������� ���������� ������ ������� 
    @param[in]  Addr �����
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
*   @brief �������� ����������������� �������  
    \return  
                - 1 ���������� �����
                - 0 �� ���������� �����
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
*   @brief ����������� ������� ��� �������   
    @param[in]  Letter ������� ������
    \return  ��� ������� ��� �������  
                
*/
uint8 ConvFont(uint8 Letter)
{if (Letter=='_')
  return 0x20; //������
 else if (Letter=='`')
  return 0xFF;  //����� ��� LED  
 else if (Letter<0xC0)
  return Letter;
 else
  switch (Letter)
   {case '�': return 'A'; 
    case '�': return 0xA0;
    case '�': return 'B';
    case '�': return 0xA1;
    case '�': return 0xE0;
    case '�': return 'E';
    case '�': return 0xA2;
    case '�': return 0xA3;
    case '�': return 0xA4;
    case '�': return 0xA5;
    case '�': return 0xA6;
    case '�': return 'K';
    case '�': return 0xA7;
    case '�': return 'M';
    case '�': return 'H';
    case '�': return 'O';
    case '�': return 0xA8;
    case '�': return 'P';
    case '�': return 'C';
    case '�': return 'T';
    case '�': return 0xA9;
    case '�': return 0xAA;
    case '�': return 'X';
    case '�': return 0xE1;
    case '�': return 0xAB;
    case '�': return 0xAC;
    case '�': return 0xE2;
    case '�': return 0xAD;
    case '�': return 0xAE;
    case '�': return 0xC4;
    case '�': return 0xAF;
    case '�': return 0xB0;
    case '�': return 0xB1;
    
    case '�': return 'a';
    case '�': return 0xB2;
    case '�': return 0xB3;
    case '�': return 0xB4;
    case '�': return 0xE3;
    case '�': return 'e';
    case '�': return 0xB5;
    case '�': return 0xB6;
    case '�': return 0xB7;
    case '�': return 0xB8;
    case '�': return 0xB9;
    case '�': return 0xBA;
    case '�': return 0xBB;
    case '�': return 0xBC;
    case '�': return 0xBD;
    case '�': return 'o';
    case '�': return 0xBE;
    case '�': return 'p';
    case '�': return 'c';
    case '�': return 0xBF;
    case '�': return 'y';
    case '�': return 0xE4;
    case '�': return 'x';
    case '�': return 0xE5;
    case '�': return 0xC0;
    case '�': return 0xC1;
    case '�': return 0xE6;
    case '�': return 0xC2;
    case '�': return 0xC3;
    case '�': return 0xC4;
    case '�': return 0xC5;
    case '�': return 0xC6;
    case '�': return 0xC7;
    default: return 0x20; //������
   }
 }

/** @fn uint8 ConvHex(uint8 Data)
*   @brief ����������� hex 0...F � �����   
    @param[in]  Data hex 0...F
    \return  ��� ������� 
                
*/
uint8 ConvHex(uint8 Data) //hex to text �� 15
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
    default: return 0x20; //������
   }
 }
//****************************************************************************** 
//������������ ������ 
 
/** @fn void Poll_button (void)
*   @brief ����� ������   
    
    ���������� �-��:
  - ������ �������� ������ ��-�, �������������� ����������� ��� ������ ������� 
    ���-�� �� RS-485 CheckAddress(void). ��� ��������� ������ � ������������� ��� 
    �������� ����� 2 ������ - ���������� ������ ReadAddr ������������� ����� ����� � 
    ����� �� ������������ � ������ � EAddrNet
  - �������� ������� �� ������� ������ ��� ���������� ������ � ������� ��-� � �������� ����� ������ Ish CheckTimeNoPress()
  - �������� ������� ������� ������ � ����������� ������ ������ � ��������� BUTTON �������� ������� ������� Check_Button() 
  - �������� ������� �������������� ������� 2-� ������ (UP � DWN) � ����������� � ��������� BUTTON �������� ������� �������
*/ 
void Poll_button (void)
 {CheckAddress();     //������ ������
  CheckTimeNoPress(); //����. ��� ���������� ������� �� ������� (��� ������ > Check2)
    
  Check_Button(KEY1,&BUTTON.TimeChoice, &BUTTON.Choice);
  Check_Button(KEY2,&BUTTON.TimeEsc, &BUTTON.Esc);
  Check_Button(KEY3,&BUTTON.TimeUp, &BUTTON.Up);
  Check_Button(KEY4,&BUTTON.TimeDwn, &BUTTON.Dwn);
  
//�������� 2� ������ >5���
  if ((CHKB(PINC,KEY3)==0)&&(CHKB(PINC,KEY4)==0)) //������ ������
   {BUTTON.TimeUp=0;
    BUTTON.TimeDwn=0;//���� ������ ��� ������ ,�� ����������� - ����������
    TimeNoPress=0; //����� �� �������
    BUTTON.TimeUpDw+=1;
    
    if (BUTTON.TimeUpDw>=Delay_6s) //������������- �� ���������� ��������� 10s
     {BUTTON.Press=Push;
      BUTTON.UpDwn=Push_6s;
      BUTTON.TimeUpDw=0;
     }
    }
   else
    BUTTON.TimeUpDw=0;  
 }

/** @fn void CheckTimeNoPress (void)
*   @brief ����. ��� ���������� ������� �� ������� ��� ������ > Check2  
    
    �������� ������� �� ������� ������ ��� ���������� ������ � ������� ��-� � �������� ����� ������ Ish
*/
void CheckTimeNoPress(void) //����. ��� ���������� ������� �� �������
 {Period_1s++;
  if ((Period_1s>Delay_1s)&&(REGIME.Mode>Check)&&(REGIME.Mode!=Alarm)&&(REGIME.Mode!=Config0)) //������ 1s
   {Period_1s=0;
    TimeNoPress++;            //����� �� �������
    if (TimeNoPress>=0xFF)
     TimeNoPress=6;           //
    
    if ((REGIME.Mode==Alarm)&&(REGIME.SubMode==EndResource))
     {if (TimeNoPress>2)        //5s �������� ��� ����. ��������� ������ ����� ������� ������ ������ ��� �������� ����� ������� 
       REGIME.PermitAlarm=1;    //���������� ��������� ������ 
     }
    else
     REGIME.PermitAlarm=1;
     
    if (REGIME.Mode==Test)
     return;
    
    if ((TimeNoPress==15)&&(REGIME.Mode!=Alarm))      //15s
     REGIME.Mode=Ish;         //����� 15 ��� � ��� � 256 ��� ���� �������
   }
 }

/** @fn void CheckAddress (void)
*   @brief ������ �������� ������  ��-� 
    
    ������ �������� ������ ��-�, �������������� ����������� ��� ������ ������� ���-�� �� RS-485. 
    ��� ��������� ������ � ������������� ��� �������� ����� 2 ������ - ���������� ������ ReadAddr 
    ������������� ����� ����� � ����� �� ������������ � ������ � EAddrNet
*/
void CheckAddress(void) //������ ������
 {uint8 NewAddr;
  NewAddr=(PINE>>3);
  NewAddr|=((PING&1)<<5);
  if (NewAddr==EAddrNet)//����� �� ���������
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
*   @brief �������� ������� ������� ������ � ����������� ������ ������ � ��������� 
    BUTTON �������� ������� �������.

    @param[in]  Key ������� ������ ��� ������
    @param[in]  *Time ����� �������
    @param[in] *Button �������� ������

    \return  *Button=*Time
*/
void Check_Button(uint8 Key, uint16 *Time, uint8 *Button )
 {if (CHKB(PINC,Key)==0)   //������ ������
   {TimeNoPress=0;         //����� �� �������
    REGIME.PermitAlarm=0;  //������ ��������� ��������� ������
    if  (*Time<Delay_15s)
     (*Time)+=1;
//������� 2 ���. �������� ��� ������� ������ �� ���������� ����� �������
    if (*Time==Delay_1p5s)//����� ������ � =2��� ,������� ��� ��� ������� 2�  
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
  else if (*Button==Off)   //������ ������ - ���������
   {
//������� Push �������� ��� ���������� ������     
    if ((*Time>Delay_01s)&&(*Time<Delay_1p5s))   //���� ������ 2>t>0.1 ���, ����� ���������
     {*Button=Push;
      BUTTON.Press=Push;
     }
    *Time=0;
   }
 }

/** @fn void Poll_Regime(void)
*   @brief �������� �� ������� ������

    ��� ��������� ������� ��������� ��-� (���������� REGIME.Mode)  � ���������� (���������� REGIME.SubMode) 
    ������ ��-� ����������� ����� ������� ������ (���������  BUTTON) � ��������������:
  - ������� � ������ ������ ������
  - ��������� ���������� ������ ������, ������� � �.�.
  - ���������� �-�� ���������� ����������������
  - ��������� ������ �������� �������� ��������� ������� ��� ���������� ������������ � ������, ���� ��������  ������ � ������������� ������� ����������� ������ � ��-� � ������ ClearDATAMRK() ��� ClearDATAEEPROM() - ������� ������������ ��-� � ������ � �������� ������ ��� � ������.
  - ������� ��������� BUTTON ����� �� ���������� ��������� 
*/
 
void Poll_Regime (void) //�������� �� ������� ������
 {if (BUTTON.Press==Off)
   return;
  uint8 n, DATA;
  uint8 EnEEPROM=0;   //���������� ������ � ������
  switch (REGIME.Mode)
   {case Plug:    //��������� ������ ��������
/*��� ����������� ��������: 1.��� ��������� ������� ������ �����>5���. 
2.������� ������� ��� ��� ���� ������. 3. ������ UpDwn> 2 ���.  */  
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
     if ( ((CONFIGMRO.Error!=0)&&((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push)||(BUTTON.UpDwn>=Push)) )   //1-� ������������, ���� ������� ������ up/dwn
          ||(BUTTON.UpDwn>=Push_6s) )
      {REGIME.Mode=Config;
       REGIME.EndCheck=0;
       REGIME.SubMode=VZU_E;
      }
     break;
    
    case Config:         //������������
     n=REGIME.NmbSection;
        switch (REGIME.SubMode)
         {case VZU_E:
          case VZU_M:
          case VZU_H:
          case VZU_TM:
               if (BUTTON.Up>=Push)
                {if(REGIME.SubMode<VZU_TM) //���������  �  ��� �-"Up ��� Dwn". 
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
                 else                   //��� � � ��
                  PrepSearsh();     //����� ��� � ������� � Check
                }
               else if (BUTTON.Esc>=Push)
                REGIME.Mode=Plug;
          break;
          
          case Side:    //������� ��� -�, ���-�� �����(������) ��� -�
               if (BUTTON.Up>=Push)
                ChoiceSideUp();     //��������� � CONFIGMRO Side
               else if (BUTTON.Dwn>=Push)
                ChoiceSideDwn();    //��������� � CONFIGMRO Side
               else if (BUTTON.Choice>=Push_2s)
                {if (CONFIGMRO.Type==VZU_E)
                  PrepSearsh(); //���������� � ������ ���
                 else
                  {REGIME.SubMode=Width;
                   if (CHKB(AllowWidth[CONFIGMRO.Side-1],0)==0) //�������� �����������
                    {CONFIGMRO.Width=2;
                     if (CHKB(AllowWidth[CONFIGMRO.Side-1],1)==0)
                      CONFIGMRO.Width=3;
                    }
                  }
                }
               else if (BUTTON.Esc>=Push_2s)
                {REGIME.Mode=Config;
                 REGIME.SubMode=CONFIGMRO.Type; //������������ ���: VZU_E/VZU_M
                 CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;  //�������������� ������ �� ������              
                }
           break;
           
           case Width:      //��� -�
              if (BUTTON.Up>=Push)
                CONFIGMRO.Width=ChoiceUp(CONFIGMRO.Width, AllowWidth[CONFIGMRO.Side-1]);     //��������� � CONFIGMRO Width ��� ����� ���������� ���� �
               else if (BUTTON.Dwn>=Push)
                CONFIGMRO.Width=ChoiceDwn(CONFIGMRO.Width, AllowWidth[CONFIGMRO.Side-1]);     //��������� � CONFIGMRO Width ��� ����� ���������� ���� �
               else if (BUTTON.Choice>=Push_2s)
                 {REGIME.SubMode=Circuit;
                  if (CHKB(AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1],0)==0) //�������� �����������
                   {CONFIGMRO.Circuit[0]=LCS;
                    if (CHKB(AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1],1)==0) 
                     CONFIGMRO.Circuit[0]=RC;
                   }
                 }
               else if (BUTTON.Esc>=Push_2s)
                REGIME.SubMode=Side;
           break;
           
           case Circuit:    //��� -�    //��������� � CONFIGMRO Width ��� ����� ���������� ���� � � ������ Width
               if (BUTTON.Up>=Push)
                CONFIGMRO.Circuit[0]=ChoiceUp(CONFIGMRO.Circuit[0], AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1]);     //��������� � CONFIGMRO Circuit ��/���/�� ��� ����� ���������� ���� �
               else if (BUTTON.Dwn>=Push)
                CONFIGMRO.Circuit[0]=ChoiceDwn(CONFIGMRO.Circuit[0], AllowCircuit[CONFIGMRO.Side-1][CONFIGMRO.Width-1]);    //��������� � CONFIGMRO Circuit ��/���/�� ��� ����� ���������� ���� �
               else if (BUTTON.Choice>=Push_2s)
                PrepSearsh(); //���������� � ������ ���
               else if (BUTTON.Esc>=Push_2s)
                REGIME.SubMode=Width;
           break;
         }
   break;
   
   case Check: //�������� ��� H, TM, � � �
          n=REGIME.NmbSection;
          if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) ) //��� -H, TM 
           { if ( (BUTTON.Up==Push)||(BUTTON.Dwn==Push) ) 
               {if (CHKB(CONFIGMRO.OnMRK,n)==0)//��� ��� ��� ������ ������
                 {if (CHKB(LCOn,n-1)==0)
                   SETB(LCOn,n-1); //��
                  else
                   CLRB(LCOn,n-1);  //��� ������
                  
                  if (NmbFreeMROIn>6)               //����� 6 ������, ���� ����������� - �����
                   CLRB(LCOn,n-1);  //��� ������
                 }
                else 
                 CLRB(LCOn,n-1);  //���� ��� ��� ������ ������  
               }
            }
          
        if (REGIME.ErrorConfig==1)
         {if (BUTTON.UpDwn>=Push_6s)
           {REGIME.Mode=Plug;
            REGIME.EndCheck=0;
            REGIME.DisplDelay=Time_1s;
            ECONFIGMRO[0].CONFIGMRO.Error=0XFF;//������ ������ �� �������� ����������������, �.�. ���� ������, � ���� �������� ������ ���� ������������
           }
         }
        else if ( ((BUTTON.Choice>=Push_2s)&&(REGIME.NmbSection==FindMaxNmbSection()))
            || ( (BUTTON.Choice>=Push_2s)&&(CONFIGMRO.Type==VZU_M)&&(CONFIGMRO.Side==M2)&&(CONFIGMRO.Width==W630)&&(CONFIGMRO.Circuit[0]==RC) ) )//���������� ������ - � �2-630-�� ���� 1 ������  
          {if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.EndCheck==0) )//��������� ���� �� ��������� ���������  8 ������
             break;
           REGIME.Mode=Ish;
 //��� ��������� ������ ���� �� �������� UP, ��������� ��         
          // if ( (CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM) )
           // if ( (CHKB(CONFIGMRO.Side,REGIME.NmbSection-1)!=0)&&(NmbFreeMROIn<7) ) //���� ������ �� � 6 ������
            // CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //����������� ������ ����� ��� - ����� ������, �-�� �� ������������
            
//���������� ��������� 8 ������ ���� �� ��������� �� ������� ������ UP
            if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.NmbSection==FindMaxNmbSection())&&(CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0) )
             if (NmbFreeMROIn<7) //6 ������
               CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //����������� ������ ����� ��� - ����� ������, �-�� �� ������������

//������������� ��������� �������� ������ -������ ����������� CRC �  ECONFIGMRO
//������������� ������������ ������ ���
           CONFIGMRO.OnCircMRO&=OnCircMRO;
           CONFIGMRO.OnD4&=OnD4;//���������� �������� ������� ��� � ������������ � ��� ��� �������� ���� ���������������� 
           SaveNmbSection();  //��������� � ������ ������ ������ ��� ���0����3, ��� 
           //CONFIGMRO=ECONFIGMRO[0].CONFIGMRO;
           CONFIGMRO.CRC=CRC16(&CONFIGMRO.Type,sizeof(struct ConfigMRO)-5); //������ 
           ECONFIGMRO[0].CONFIGMRO.CRC=CONFIGMRO.CRC;
           ECONFIGMRO[1].CONFIGMRO.CRC=CONFIGMRO.CRC;
          }
      else if ( (CONFIGMRO.Type==VZU_M)&&(CONFIGMRO.Side==M2)&&(CONFIGMRO.Width==W630)&&(CONFIGMRO.Circuit[0]==RC))
       break;//���������� ������ - � �2-630-�� �� 2 ������ �� ����� ���� ���� 1� ������
      else if ( (BUTTON.Up>=Push_2s)&&(REGIME.ErrorConfig!=1) )
          {//��� ��������� ������, ���� ��� ���������, ��� ��������� �������� EndCheck
              if (REGIME.NmbSection==FindMaxNmbSection())
               REGIME.EndCheck=1;
              else 
               REGIME.EndCheck=0;
              
           if ( ((CONFIGMRO.Type==VZU_H)||(CONFIGMRO.Type==VZU_TM))&&(REGIME.NmbSection<=FindMaxNmbSection())&&(CHKB(CONFIGMRO.OnMRK,REGIME.NmbSection)==0) ) //��� -H, TM ��������� �� �� ��� (���� ��� �� ���)
            {if (CHKB(LCOn,REGIME.NmbSection-1)!=0) //���� ������ �� 
              {if (NmbFreeMROIn<7) //6 ������
                {CONFIGMRO.NmbSectionMRO[NmbFreeMROIn-1]=REGIME.NmbSection;   //����������� ������ ����� ��� - ����� ������, �-�� �� ������������
                 for (NmbFreeMROIn++;NmbFreeMROIn<7;NmbFreeMROIn++)//���� ����. ��������� ���� ���
                   {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
                     break;
                   }
                  
                 }
              }
             uint8 NmbInMRO, NmbInMRK;             //����������������� ��������� �����
             for (NmbInMRO=0,n=NmbFreeMROIn-1;n<7;n++)
              {if (CHKB(CONFIGMRO.OnCircMRO,n)!=0)
                NmbInMRO++;
              }
             for (NmbInMRK=0,n=REGIME.NmbSection;n<9;n++) //����� ��� �������� �� ���������� ����� ��� (��� NmbSection>MRK, �� ��� ��� �� ������)
              {if (CHKB(CONFIGMRO.OnMRK,n)!=0)
                NmbInMRK++;
              }
             if ( NmbInMRO>(8-REGIME.NmbSection-NmbInMRK) )
              REGIME.ErrorConfig=1;       //���� ����������������� ��������� ������ < ���������� ������ -������ 
            }

           if (NmbFreeMROIn>6)
             NmbFreeMROIn=6; 
           REGIME.Mode=Check;  //�������� ���� ������ (��� � � �, ��� H, TM -��������� � ���� ������
           if (REGIME.NmbSection<FindMaxNmbSection() ) 
            REGIME.NmbSection++;
          }
      else if ((BUTTON.Esc>=Push_2s)&&(REGIME.ErrorConfig!=1)&&(REGIME.NmbSection!=FindMinNmbSection()))
          {if (REGIME.EndCheck==0)
            REGIME.NmbSection--; //�������� ���� ������
           REGIME.EndCheck=0;
          //��� � � �� ���� �������  NmbFreeMROIn
           if (CHKB(LCOn,REGIME.NmbSection-1)!=0) //���� ������ ��, ��� ����� ��� ���
            for (NmbFreeMROIn--;NmbFreeMROIn!=1;NmbFreeMROIn--)//���� ����. ��������� ���� ���
             {if (CHKB(CONFIGMRO.OnCircMRO,(NmbFreeMROIn-1))!=0)
               break;
             }
          }
   break;
   
   case Ish:        //��������   
       if (BUTTON.Choice==Push)   //����� ������������ ���������
         {REGIME.Mode=Inform;
          REGIME.SubMode=Modul_cect;
         }
       else if (BUTTON.Choice>=Push_2s)//����� ��������� ��������� ������� � ���������� ������������
         {REGIME.Mode=Inform;
          REGIME.SubMode=Resource_res;
         }
/*       
//test
else if (BUTTON.Up>=Push)
 {REGIME.SubMode=DisplAPK;
  REGIME.DisplDelay=Time_2s;     //�������� �� ���������
 }
*/
       if ((REGIME.Mode==Inform)&&(CONFIGMRO.OnMRK==0))  //�� ��������� �� ���� ���
        REGIME.DisplDelay=Time_2s;     //�������� 2 ��� ���� ������ �� �����
        REGIME.NmbSection=FindMinNmbSection();  //���������� ����� ������
        REGIME.NmbModule=0;                     //���������� ����� ������
   break;
   
   case Inform:   //��������
      switch (REGIME.SubMode)
       {case Modul_cect:                           //����� ������
         if (BUTTON.Up>=Push)
          {n=FindMaxNmbSection();
           if (REGIME.NmbSection<n)
            REGIME.NmbSection++;
           else
            REGIME.NmbSection=FindMinNmbSection();  //���������� ����� ������
          }
         else if (BUTTON.Dwn>=Push) //������ ���������� � 1
          {n=FindMinNmbSection();  //���������� ����� ������
           if (REGIME.NmbSection>n)
            REGIME.NmbSection--;
           else
            REGIME.NmbSection=FindMaxNmbSection();  //���������� ����� ������
          }
         else if (BUTTON.Choice>=Push)    //��������� ��������
          REGIME.SubMode=Modul_mod;
         else if (BUTTON.Esc>=Push)    //�����
          REGIME.Mode=Ish;
        break; 
        
        case Modul_mod:                   //����� ������ � ������ NmbSection
         if (BUTTON.Up>=Push)
          {if (REGIME.NmbModule<(MaxModule-1))//���� 8 �������
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
         else if (BUTTON.Choice>=Push)    //��������� ��������
          {REGIME.SubMode=Modul_inf;
           REGIME.DisplDelay=Time_2s;     //�������� 2 ��� ���� ������ �� �����
          }
         else if (BUTTON.Esc>=Push_2s)    //�����
          REGIME.Mode=Ish;
         else if (BUTTON.Esc>=Push)       //������� � ����� � ������ � ������
          REGIME.SubMode=Modul_cect;
        break;
        
        case Modul_inf:                   //�������� ������
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
        
case Modul_ADC:                   //�������� ���
if (BUTTON.Esc>=Push)
 REGIME.SubMode=Modul_cect;
break;        
*/        
        case Resource_res:                //����� ������������/������
         if ((BUTTON.Up>=Push)||(BUTTON.Dwn>=Push))
          REGIME.SubMode=Resource_srab;
         else if (BUTTON.Choice>=Push)
          {REGIME.SubMode=Resource_inf_res;
           REGIME.NmbRes=0;  //����� � ����. �������������
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
           REGIME.NmbSrab=0;  //����� � ����. ��������.
           goto Srabat;
          }
         else if (BUTTON.Esc>=Push)
          REGIME.Mode=Ish;
        break;
      
        case Resource_inf_res:
         if (BUTTON.Esc==Push)
          {REGIME.SubMode=Resource_inf_srab;
           REGIME.NmbSrab=0;  //����� � ����. �������������
           goto Srabat;
          }
         else if (BUTTON.Esc>=Push_2s)
          REGIME.Mode=Ish;
         else if ((BUTTON.Dwn>=Push)&&(REGIME.NmbRes<MaxSecMod))
           REGIME.NmbRes++;
         else if ((BUTTON.Up>=Push)&&(REGIME.NmbRes!=0))
          REGIME.NmbRes--;
         /* ������- �� �������
         else if (BUTTON.UpDwn>=Push_2s)
          {REGIME.Mode=Clear;
           REGIME.SubMode=Prepare;
           REGIME.ClearMode=ClearRes;
           //REGIME.ClearMode=ClearResSrab;//������� ���
           REGIME.DisplDelay=Time_3s;
          }
         */
Res:         
         SORT_Res(); //���������� �� �������,
         DATA=IndexResourse[REGIME.NmbRes];//0...3bit -MRK, 4..7bit- Module in section
         if ((DATA==0xFF)&&(REGIME.NmbRes!=0)) //F ������, F ����� - ��� ������, �.�. ����� ������
          {REGIME.NmbRes--;
           DATA=IndexResourse[REGIME.NmbRes];//0...3bit -MRK, 4..7bit- Module in section
          }
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[(DATA>>4)];
         REGIME.NmbModule=DATA&0X0F;     //Module in section
        break;
        
        case Resource_inf_srab:
         if (BUTTON.Esc==Push)
          {REGIME.SubMode=Resource_inf_res;
           REGIME.NmbRes=0;  //����� � ����. �������������
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
         if (SORT_Srab()==0) //���������� �� �������������
          {Event=0;                 //��� ������������
           break;
          }
         DATA=IndexSrab[REGIME.NmbSrab];//0...3bit -MRK, 4..7bit- Module in section
         if ((DATA==0xFF)&&(REGIME.NmbSrab!=0)) //F ������, F ����� - ��� ������, �.�. ����� ������
          {REGIME.NmbSrab--;
           DATA=IndexSrab[REGIME.NmbSrab];//0...3bit -MRK, 4..7bit- Module in section
          }
         REGIME.NmbSection=CONFIGMRO.NmbSectionMRK[(DATA>>4)];
         REGIME.NmbModule=DATA&0X0F;     //Module in section
        break;
       }
   break;
   
   case Response: //������������
   break;
   
   case Alarm:
      if (REGIME.SubMode==EndResource)//������ >80%
       {if (BUTTON.Choice>=Push_10s)
         {REGIME.Mode=Clear;
          REGIME.SubMode=Prepare;
          REGIME.ClearMode=ClearRes;
          REGIME.DisplDelay=Time_3s;
         }
        break;
       }
      else if (BUTTON.Choice==Push)   //����� ������������ ���������
       {REGIME.Mode=Inform;
        REGIME.SubMode=Modul_cect;
       }
      else if (BUTTON.Choice>=Push_2s)//����� ��������� ��������� ������� � ���������� ������������
       {REGIME.Mode=Inform;
        REGIME.SubMode=Resource_res;
       }
      n=FindMinNmbSection();  //���������� ����� ������
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
//�.�. ������� ������� �������� ����������, ������ ��������� ���� �����,           
           {CREATE_LINE1(&TClear[7][0]);     //��������...
            CREATE_LINE2(&TClear[4][0]);     //��������
            ClearDATAMRK();                //������ ������� ������ ��� � ������
           }
          else if (REGIME.ClearMode==ClearEEPROM)
           {CREATE_LINE1(&TClear[7][0]);     //��������...
            CREATE_LINE2(&TClear[3][0]);     //����_������
            ClearDATAEEPROM();                //������ ������� ������������ ������ +������ ���
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
              if (EnEEPROM==0xB6) //���. ������ �� ������ � ������
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
              if (EnEEPROM==0xB7) //���. ������ �� ������ � ������
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
         REGIME.DisplDelay=Time_2s;     //�������� �� ���������
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
*   @brief ������ � CONFIGMRO ������ ������� ��� ������� DWN 
    
*/
void ChoiceSideDwn(void)//������ � CONFIGMRO Side
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
*   @brief ������ � CONFIGMRO ������ ������� ��� ������� UP 
    
*/
void ChoiceSideUp(void)//������ � CONFIGMRO Side
 {switch (CONFIGMRO.Side)
   {case 1:
     CONFIGMRO.Side=2;
    break;
    case 2:
     if (CONFIGMRO.Type==VZU_E)
      break;    //��� � -�� 2�
     CONFIGMRO.Side=3;
    break; 
   }
 }

  /** @fn void ChoiceUp(void)
*   @brief ������ � Data ������ 1-3 ��� ������� UP � ��������� ���������� ����� � Allow 
    ������. ������� - �� 7
*/
 uint8 ChoiceUp(uint8 Data, uint8 Allow)//�������� ������������ ��� ������� UP � ��������� ���������� ����� � Allow 
{switch (Data)
    {case 0:
     case 1:
      if (CHKB(Allow,1)==1) //�������� ������� �� 2
       return 2;
      else if (CHKB(Allow,2)==1)
       return 3;
      break;
      case 2:
       if (CHKB(Allow,2)==1) //�������� ������� �� 3
        return 3;
      break;
     default:
     return Data;
    }
 return Data;
} 

  /** @fn void ChoiceDwn(void)
*   @brief ������ � Data ������ 1-3 ��� ������� Dwn � ��������� ���������� ����� � Allow 
    ������. ������� - �� 7
*/
 uint8 ChoiceDwn(uint8 Data, uint8 Allow)//�������� ������������ ��� ������� UP 
{switch (Data)
    {case 0:
     case 3:
      if (CHKB(Allow,1)==1) //�������� ������� �� 2
       return 2;
      else if (CHKB(Allow,0)==1)//�������� ������� �� 1
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
*   @brief ��������� � ������ ������ ������ ��� ���0����3 
    
*/
void SaveNmbSection(void)  //��������� � ������ ������ ������ ��� ���0����3
 {
//������ � ������
  CONFIGMRO.Error=0;
  CONFIGMRO.CRC=0; //������ � ������ - ����� ������������ �������� 
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
*   @brief ���������� � ������ ��� 
    
*/
void PrepSearsh(void) //���������� � ������ ���
 {REGIME.SubMode=Search_MRK; //����� ���� ���������-������� �� ����� ���
  REGIME.ClearMode=0; //����� �������� ����������� ������ ��� (�����������) 
  ReqMRK=0;
  REGIME.RunStringDelay=Time_4s;//���. ���������
  REGIME.RunStringNext=Time_4s;//���. ���������
  REGIME.NmbLine1=0;
  REGIME.NmbLine2=0;         //������� ������ ��� 2-� ������ -�������
//  REGIME.NmbSection=FindMinNmbSection(); //����� ����������� ������ ������
 }

/** @fn void SORT_Res(void)
*   @brief ���������� �� ������� 
    
*/
void SORT_Res(void) //���������� �� �������
//47000cycle for 32 data!=0 (6.4 ms for 7.3MHz
 {uint8 i, n, ni, nimax; //������, ������
  uint8 Data_curr, Data_next;
  uint8 IndexResourse0 [16*8]; //0...3bit -MRK, 4..7bit- Module in section
 
//��������� ������� �������� �� 0 � ��� ���������� MRK

  for(i=0, ni=0; i<16; i++) //1800 cycle for 32 data!=0
   {if CHKB(CONFIGMRO.OnMRK,i)
     {for(n=0; n<8; n++)
       {if (EDATAMRK[i].DATAMRK.Resource[n]!=0)
         IndexResourse0[ni++]=(i<<4)+n;//����� ��� + ������ � ���
       }
     }
   }
  nimax=ni;   //� ���������� �������
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
    IndexResourse0[m]=0xFF;   //������ �����������
   }
  IndexResourse[i]=0xFF; //����. ������
 }

/** @fn void SORT_Srab(void)
*   @brief ���������� �� ������������ 
    
*/
uint8 SORT_Srab(void) //���������� �� ������������
//47200cycle for 32 data!=0 (6.4 ms for 7.3MHz
 {uint8 i, n, m, ni, nimax, full=0; //������, ������
  uint16 Data_curr, Data_next;
  uint8 IndexSrab0 [16*8];    //0...4 Section, 4..6 Module in sect
//��������� ������� �������� �� 0 � ��� ���������� MRK
  for(i=0, ni=0; i<16; i++)//2000 cycle for 32 data!=0
   {if CHKB(CONFIGMRO.OnMRK,i)
     {for(n=0; n<8; n++)
       {if (EDATAMRK[i].DATAMRK.SrabMod[n]!=0)
         {IndexSrab0[ni++]=(i<<4)+n;//����� ��� + ������ � ���
          full=1;//���� �� 0 ������������
         }
       }
     }
   }
  nimax=ni;   //� ���������� �������
 
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
    IndexSrab0[m]=0xFF;   //������ �����������
   }
  IndexSrab[i]=0xFF; //����. ������
  return full;
 }
//****************************************************************************** 
/** @fn void Poll_Srab(void)
*   @brief ����� ������������ �� 6 ������. ���� ����� 60���- ����� 
    
*/
void Poll_Srab (void)  //����� ������������ �� 6 ������. ���� ����� 60���- �����
 {uint8 i; 
/*
  if ((Period_20ms==Period_Srab))//||(REGIME.Mode<=Check)) //������ 20ms � ������ � ������� �������
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
    
    if (TimeSrabMRO[i]>Delay_05s)   //>0.5 ���
     SETB(DisplayFailMRO,i);
    else if (TimeSrabMRO[i]==0) 
     CLRB(DisplayFailMRO,i);
   }
 }



//****************************************************************************** 



/** @fn void AddDataMRK(void)
*   @brief ���������� ������ � ������ DATAMRK �� ���������� �� RS485 
    
    �� ������ ��-� ��� ������� ��-� ���������� ����������� ������ � ��������� DATAMRK. ����� � ��� ����������� �������� �� ��-�  (��������� READMRK): 
  - ����� ���������� (���������� ������) FlagDamage.
  - ���������� ������������ (��� 8 �������). ���� ��� �� �������, �� ����������� � ������ DATAMRK.SrabMod[], �� ���������� �������������� ��������� 999. ������������� ��������������� ����� ��� ��������� ������ ������ �� ������� DisplaySrabSection � �������� � ���-�� DisplaySrabSectionForAPK
  - ������������� ������ (��� 8 �������). ��� ����������� � ������ DATAMRK.Resource[] � ����� ����������� 99%.
    ��� ����������� ������ �������������� CRC, ����� ���� ��������� DATAMRK  ��� ����� ��-� ���������� � ������ EDATAMRK[].

*/
 void AddDataMRK(void) //���������� ������ � ������ DATAMRK �� ���������� �� RS485
 {uint8 i, NMB=READMRK.NMB_MRK;
  uint8 SrabMRK;
  uint8 EnEEPROM=0x95;
  DATAMRK=EDATAMRK[NMB].DATAMRK;
  DATAMRK.FlagDamage=READMRK.FlagDamage;
//�������� ��������� ������?  
  DATAMRK.Error=READMRK.Error;
  if CHKB(NoClearMRK,NMB)
   SETB(DATAMRK.Error,ErrorNoClear); //������ ��������
  
//������������- �����������, ������� ����������� 
  for (i=0;i<8;i++)
   {SrabMRK=(READMRK.SrabMod)>>(i*2);//�� 2 ���� �� ������
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
*   @brief ������ ��� ���������������� �������� � ���� ���� ����� ������ 
    
    ���������� ������� �������� ������ (� ����� ������ ����� �� ���������� ��� ������ 
    � ���� ��������� ��� ���������� ������ ����� ��� ��� ������ ������������ �����). 
*/
void DataAPKOrder(void) //������ ��� ���������������� �������� � ���� ���� ����� ������:
  {uint8 i; //����� ��������� � _copy ����� ���� (������������������ ��������)
   
//DisplayFailMRK_APK[0]=DisplayFailMRK[0] & CONFIGMRO.OnMRK;   //������ ��, �-�� �������� � OnMRK;
//DisplayFailMRK_APK[1]=DisplayFailMRK[1] & CONFIGMRO.OnMRK;
//DisplayFailMRO_APK=(DisplayFailMRO & CONFIGMRO.OnCircMRO);
//����. ����� DisplayFailMRK_APK - �� ����������, ������ ������   
DisplayAlarmMRK_APK[0]=DisplayAlarmMRK[0] & CONFIGMRO.OnMRK;
DisplayAlarmMRK_APK[1]=DisplayAlarmMRK[1] & CONFIGMRO.OnMRK;
DisplayAlarmMRO_APK=(DisplayAlarmMRO & CONFIGMRO.OnCircMRO);
   
   DisplayNetwMRK_APK=(DisplayNetwMRK & CONFIGMRO.OnMRK);
   for(i=0;i<16;i++)
    if CHKB(CONFIGMRO.OnMRK,i)
     {DisplaySrabSection_APK[i]=DisplaySrabSectionForAPK[i]; //8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]
DisplaySrabSectionForAPK[i]=0;      //���������� - ��������, �.�. ��� ������� ����������� � ������
      DisplayResourceMRK_APK[i]=DisplayResourceMRK[i];
     }
    else
     {DisplaySrabSection_APK[i]=0; //8 ������� � 16 �������: � ������ ������������, ���� ���=1: ��������� �� 3 ��� "�������� ������... ������[16].������[���]
      DisplayResourceMRK_APK[i]=0;
     }
  }
   
/** @fn void DataAlarmAPK(void)
*   @brief ������������ ������ ������ ��� �������� �� APK 
    1. ���� ��� ������ ��� ��� ���� ����� �������� (NmbSendAPK==0) -��������� ����� �������. ��������� �����. ������ ��� �������� NmbSendAPK.
    ��������� DATA_SE�T_APK[4][16], ��� 16 �������� ��� �������� 4 ������ �� 1 �������, �� 4 ����� �� ������ � ������   
    2. ��������� �������� � ��� DATA_APK[4*4=16], ����������� ���� �� ����������� DATA_SE�T_APK[4][16], �� ��� ��� ���� �� ����������� ��� ������ NmbSendAPK
    
*/
void DataAlarmAPK(void)     //������������ ������ ������ ��� �������� �� APK
 {uint8 i, n, Alarm;               //���������� ������ ������ ��� ���� ������
 
//��������� ����� ������  
  if (NmbSendAPK<0) //�������� ������ ������ �� ��� - ���������
   {NmbSendAPK=0;
    NmbCurrAPK=0;
    DataAPKOrder();     //������������ ����� ������
   
    for (i=0; i<(16*4); i++) //����� ���-�� ������������ ������� 
       DATA_SECT_APK[i]=0;     //�������������� ������� ���� ������ ��� ��������

//����. ����� DisplayFailMRK_APK - �� ����������, ������ ������      
//�������� ������ 16 ������ �����. � ��� � ���������� �� 16 �������� DATA_SE�T_APK[4][16] ���� ���� ������
    for (i=0; i<16; i++)                        //��� 16 ������, ��������� DATA_SE�T_APK
     {Alarm=0;
      if CHKB(CONFIGMRO.OnMRK,i)                //�������� ��� ���. ��-�
       {if (DisplaySrabSection_APK[i]!=0)       //��������� ������������
         {DATA_SECT_APK[(NmbSendAPK*4)+1]=DisplaySrabSection_APK[i]; //����� ������������ ������� ������ ������ ������ 
          Alarm++;
         }
        if (DisplayResourceMRK_APK[i]!=0)       //��������� ��������� �������
         {DATA_SECT_APK[(NmbSendAPK*4)+2]=DisplayResourceMRK_APK[i]; //����� ��������� ����� 80% �������
          Alarm++;
         }
//���������� ������ ������ ��� ���� ���������� ���
        if CHKB(DisplayAlarmMRK_APK[0],i)  //�� 2 ����� �� ��� [0]-1���, [1]-2���
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],0);
          Alarm++;
         }
        if CHKB(DisplayAlarmMRK_APK[1],i)
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],1);
          Alarm++;
         }
        if CHKB(DisplayNetwMRK_APK,i) //��� ����
         {SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],7);
          Alarm++;
         }
        if (Alarm!=0)
         {DATA_SECT_APK[(NmbSendAPK*4)+0]=CONFIGMRO.NmbSectionMRK[i];//���������� ����� ������
          NmbSendAPK++;                                      //���� ���� ������, ��������� � ���� �������
         }
       }
//������� ���� ��� ����������
   DisplayResourceMRK_APK[i]=0;
   DisplaySrabSection_APK[i]=0; 
   CLRB(DisplayNetwMRK_APK,i);        
   CLRB(DisplayAlarmMRK_APK[0],i);
   CLRB(DisplayAlarmMRK_APK[1],i);            
     }
//��� 6 ������ ���, �� �� ����� 16 ��������, ��������� DATA_SE�T_APK      
//6 ������ ��� ����� ���� ��� �� 1 �� 6 ������ 
    for (n=0; ((n<6)&&(NmbSendAPK<16)); )   
     {if (CHKB(CONFIGMRO.OnCircMRO,n))                      //�������� ��� ���. ��-O
       {if (CHKB(DisplayAlarmMRO_APK,n))                    //���� ���������� � ���?
         {i=CONFIGMRO.NmbSectionMRO[n];
          DATA_SECT_APK[(NmbSendAPK*4)+0]=CONFIGMRO.NmbSectionMRO[n]; //����� �����
          DATA_SECT_APK[(NmbSendAPK*4)+1]=0;                //����� ������. ��� ��� ���
          DATA_SECT_APK[(NmbSendAPK*4)+2]=0;                //����� ��������� ����� 80% �������. ��� ��� ���
          SETB(DATA_SECT_APK[(NmbSendAPK*4)+3],n);          //��������� ���� ��� ������ ��� ������� ������ ������ , ���� ��� ����          
         }
        n++;
        if (i!=CONFIGMRO.NmbSectionMRO[n])      //���� ��������� ����� ������  
          NmbSendAPK++;                         //����� ������ �������� ���
       }
      else
       n++;      
     }
   DisplayAlarmMRO_APK=0;  //������� ���� ��� ���������� � ���
   }
  else //�������� �� 4 ������� (�� �������� 16)
   {if (NmbSendAPK>16)
     NmbSendAPK=0;      //������ �� ������ ��� �� 16 ��������
    for (i=0; i<16; i++) //������� ������ ��������
      DATA_APK[i+ShiftAPK]=0;
      
    for (i=0; (i<4)&&(NmbSendAPK<0); i++) //����������� ������ ��������
     {for (n=0; n<4; n++)
       {DATA_APK[(i*4)+n+ShiftAPK]=DATA_SECT_APK[(NmbCurrAPK*4)+n];
       }
      NmbSendAPK--;
      NmbCurrAPK++;
     }
   }  

//�������� ������� ������ ��� ������� ��������� ������� �������� 0xFF
  if (PrepClear==0xBB)
   {PrepClear=0;
    for (i=0; i<16; i++) //������� ������ ��������
     DATA_APK[i+ShiftAPK]=0;
    DATA_APK[15+ShiftAPK]=0xFF;
   } 
 }

/** @fn void Delay(uint16 n)
*   @brief �������� � ���
    \param[in] n �������� � ���
    
*/
 void Delay(uint16 n) //0.5 mks
 {for(;n;n--);
 }

/** @fn Rele(void)
*   @brief ���������� ����
    � �������� ������ - ����������� � ������� �����
*/
void Rele(void)  
 {if (TimeRele!=0)
   {TimeRele--;
    return;
   }
  if ((REGIME.Mode==Test)&&(REGIME.SubMode==TestRelay))
   {if ((BUTTON.TimeUp!=0)||(BUTTON.TimeDwn!=0))
     SETB(PORTD,RELE);      //���
    else
     CLRB(PORTD,RELE);     //����
    return; 
   }

  switch (ReleMode)
   {case RelePower:
     ReleMode=RelePowerEnd;
     TimeRele=Time_2s;
     SETB(PORTD,RELE);      //���
    break;
      
   case RelePowerEnd:
   default:
     if ((DisplayAlarmMRK[0]!=0)||(DisplayAlarmMRK[1]!=0)||(DisplayAlarmMRO!=0)
      ||(DisplayNetwMRK!=0)||(Resource80!=0))
      {CLRB(PORTD,RELE);     //����
      }
     else
      SETB(PORTD,RELE);      //���
     if CHKB(ReleAlarm,SrabSection)
      {CLRB(ReleAlarm,SrabSection); 
       TimeRele=Time_180s; //��� �� 180 ���
       CLRB(PORTD,RELE);     //����
      }
    break;
   }
 }
 

 