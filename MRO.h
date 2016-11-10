/** @file MRO.h
*   @brief MRO Header File
*   @date 07.07.2013
*   @version 01.0.00
*   
*   This file contains:
    - �������
*   - Definitions
*   - enum
*   - struct
    - ��������� �-��
*   .
*/

#ifndef MRK_H
#define MRK_H

#include "stdtypes.h"
/*! 
   ������� �������� 
*/
#define SETB(x,y) (x |= (1<<y)) //(1<<y1)+(1<<y2)+(1<<y3)) /* Set bit y in byte x*/
//#define SETB(x,y,y1) (x |= (1<<y)+(1<<y1))
#define CLRB(x,y) (x &= (~(1<<y))) /* Clear bit y in byte x*/
#define EORB(x,y) (x ^= (1<<y)) /* EOR bit y in byte x*/
//#define CHKB(x,y) (x & (1<<y)) /* Check bit y in byte x*/
#define CHKB(x,y) ((x>>y) & 1) // Check bit y in byte x 
//((x>>y) & 1) ������ �� 3 ���� ��� (x & (1<<y))


#define _nop __asm ("nop")
#define _cli __asm ("cli")
#define _sei __asm ("sei")
#define _wdr __asm ("wdr")

#define EMPT  0x20  //����� �� ����������
//#define DIG_CHAR(x) (x+0x30)    //����� �� ����������


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
#define   DB4         0 //DB0-�� �����
#define   DB5         1
#define   DB6         2
#define   DB7         3 //DB3
#define   DDRB_WRITE (DDRB|=0x0F)  //
#define   DDRB_READ  (DDRB&=0xF0)

//� ����� ����� ���� RW PORTB_4 ������ PORTB_6
//#define   RW          6 //������ �����
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
*   ����� � 20 �� ����������
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

//����� � 100 �� ����������
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


//��� ������ ������������
//#define MaxSection   4
#define MaxModule    8
#define MaxSecMod    16*8 //4 ������ �� 8 �������
#define MaxFail      8 //4 ������ �� 8 �������


//DisplayAllAlarm, APKAllAlarm, ReleAlarm
#define FailMRK      6     //��������� - � ������� ������� ����
#define FailMRO      5
#define AlarmMRK     4
#define AlarmMRO     3     //=1 ���� DisplayAlarmMRK_copy!=0 DisplayAlarmMRO_copy
#define ResourceMRK  2
#define NetwMRK      1     //=1 ���� NetwMRK_copy!=0
#define SrabSection  0     //=1 ���� ���� ���� ��� � DisplaySrabSection_copy[16]!=0



enum Mode{Plug, AddressM, Config0, Config, Check, Ish, Inform, Response, Alarm, Clear, Test};
//��������� ����� ������������ �������� �������� �������� ������������ ��������� �������� ����
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
  uint8 ClearMode;      //��� ��������
  uint8 NmbLine1;       //������� ������ ��� 1-� ������
  uint8 NmbLine2;       //������� ������ ��� 2-� ������
  uint8 NmbSection;     //�������� ������
  uint8 NmbModule;      //��������� ������ � ������ NmbCheckSection
  uint8 NmbRes;
  uint8 NmbSrab;        //����� ����. ������� ��� ��������.
  uint8 FlashDelay;     //�������� �� �������
  uint8 DisplDelay;     //�������� �� ��������� (����. ��� ���������� ������� - �� ������ �� 3 ���)
  uint8 RunStringDelay; //������ �������� �� ������� ������
  uint8 RunStringNext;   //���� �������� �� ������� ������
//�����
  unsigned BegConfig:1;      //��������� ����������������
  unsigned Flash:1;          //�������
  unsigned RunFlash:1;       //��� ��������� � ���������� ���� ��� ���. ������
  unsigned PermitAlarm:1;    //���������� ��������� ������
  unsigned ErrorConfig:1;
  unsigned EndCheck:1;       //��������� ������ - ���������, ����� ��������
  //unsigned APKCompl:1;       //�������� ������ ������ �� ��� - ���������
 };
extern struct Regime REGIME;



struct Button
 {uint16 TimeUp;
  uint16 TimeDwn;
  uint16 TimeChoice;
  uint16 TimeEsc;
  uint16 TimeUpDw;
  uint8 Press; //����� ������� ������� ����� �� ������
  uint8 Up;  //0-����, 1- ������ ������
  uint8 Dwn;
  uint8 Choice;
  uint8 Esc;
  uint8 UpDwn;
 };
extern struct Button BUTTON;

//���� ������ Error
#define Srab               0 //>3 ������������ ������������ ������� ������
#define NoClear            1 //���������� ������ ����� ������ �� 0 ������
#define ErrorCRC           2 //������ ������� ������ (�� ���������� CRC)
#define ErrorRead          3 //������ ������ �� ������� ������ , ����. CRC. ������������ ����� ��������� ������
#define ErrorConfigMRO1    4 //������ ������������ MRO1(�� ������ CRC ������������)
#define ErrorConfigMRO2    5 //������ ������������ MRO2(�� ������ CRC ������������)
#define ErrorDATAMRO1      6 //������ ����������� ������������ MRO(�� ������ CRC ������������)
#define ErrorDATAMRO2      7

#define ErrorNoClear       10
//#define ErrorConfigMRK     6 //������ ������������ MRK(�� ������ CRC ������������)
//#define NoConfigMRO        7 //��� ������������ MRO


struct  ConfigMRO
{uint8  Type;              //������������ ��-�: ������ ������������ VZU_E/VZU_M / ������������  
 uint8  Side;               //������� ��� ���-���-� (1���2). ���������� �����(������) ��� ���-���-� (1, 2, 3). ��� � � ��: ������ ��� ��� 8 ������- ���� �� =1 ��� ��� ������ =0
 uint8  Width;              //������ 630, 660, 900 ��� ���-���-�:
 uint8  Circuit[4];        //���� ��, ���, �� ��� ���-���-� �� 0, ��� � �� 0-3 ��� 1-4 ��� 5-8 ������
 uint16 OnMRK;             //������������ ��-�, �� 16 ������� (� 0 �� 15) - (��� ������ �� ����� �����������) 0- ����, 1- ���
 uint16 OnD3;              //�������� ����� D3 0- ����, 1- ���
 uint16 OnD4;              //�������� ����� D4 0- ����, 1- ���
 uint8  NmbSectionMRK[16]; //����� ������ ��� ���0����15
 uint8  NmbSectionMRO[6];  //����� ������ ��� 6 ����� �������� ����������� 
 uint8  OnCircMRO;         //������������ ���� �������� ����������� ��-�. 0- ����, 1- ���
 uint16 CRC;
 uint8  Error;              //����� ������
 uint16 ErrorDataMRK;      //����� ������ ������ MRK (����������� ������������) 
};
extern struct ConfigMRO CONFIGMRO;

struct  DataMRK   //����������� ������ �
{uint8 Resource[8];   //��������� �������, %, 0 ���� - ������0
 uint16 SrabMod[8];  //������������ ������� ������: 0-1 ���� - ������0 � 6-7 ���� - ������7 
 uint8 FlagDamage; //����� ������������� ������ ������� ������ 
 uint8 Error;
 uint16 CRC;
};
extern struct DataMRK DATAMRK;

struct  DataSummMRK   //����������� (������������) ������ �� ���� ���
{uint8 Resource[8];   //��������� �������, %, 0 ���� - ������0
 uint16 SrabMod[8];  //������������ ������� ������: 0-1 ���� - ������0 � 6-7 ���� - ������7 
 uint16 CRC;
 uint8 ADC[8];
};
extern struct DataSummMRK DATASUMMMRK[16], DATASUMMMRK_Copy;

struct  ReadMRK   //����������� ������ � MRK
{uint8 Resource[8];   //��������� �������, %, 0 ���� - ������0
 uint16 SrabMod;  //������������ ������� ������: 0,1 ��� - ������0  14,15 ��� - ������7 
 uint16 FlagDamage; //����� ������������� ������ ������� ������ 
 uint16 CRC;
 uint16 Error;
 uint8 NMB_MRK;
};
extern struct ReadMRK READMRK;

/*
struct DataMRO //����������� ������ �� ������������ � ���
{uint8 Flag;         //�� ���-��
 uint16 SrabMod[6];  //������������ ������� ������: 0-1 ���� - ������0 � 6-7 ���� - ������7
 uint16 CRC;
};
extern struct DataMRO DATAMRO;
*/

/*
 typedef volatile  struct  DataMRO
{uint8 FlagDamage; //����� ������������� ������ ������� ������ 
 uint16 SrabMod[8];  //������������ ������� ������: 0,1 ���� - ������0 � 14,15 ���� - ������7 
 uint8 Resource[8];   //��������� �������, %, 0 ���� - ������0
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
void CopyDataLine(uint8 Line);//�������� � 1 ��� 2 ������ ������� ������ �� LINE1 ��� LINE2
void RunDataLine(uint8 Line, uint8 Shift); //������� ������ � ������������ ������� Shift ������ 0.5 ���. �������� � 1 ��� 2 ������ ������� ������ �� LINE1 ��� LINE2
void FlashDisplay(void);//����� 0.5 ���, �� ����� 0.2 ���
uint8 RunString(uint8 Shift, uint8 Nmb); //REGIME.NmbLine1,2 - ������� ������ �� �����- Shift, ���-�� ������� Nmb
void Delay(uint16 n);


void OnDisplay(uint8 On, uint8 Cursor, uint8 Blink);//������ 1DCB. DISPLAY D: ON=1/OFF=0. Cursor C: ON=1/OFF. Blinking B: of Cursor
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
void CopyDataDisplay(uint8 *Data); //���������� ������ �� ������ � Display � ������������ � ������ �������
void CopyDataExDisplay(uint8 *Data);//����������� ������ �� ������ � Display ��� �����������
uint8 CREATE_TEXT (uint8 __flash *pLn, uint8 *pText);
void ChangeNmb (uint8 Nmb, uint8 *pText);//������ ������� N �� Nmb
void CREATE_LINE1 (uint8 __flash *pLn); //������: �������, ����� � hex ������� 
void CREATE_LINE2 (uint8 __flash *pLn); //������: �������, ����� � hex ������� 
uint8 CHAR_ADDLINE (uint8 Data);//hex to text �� 99 -> ADDLINE
void INT_ADDLINE (uint16 Data); //hex to text �� 999 -> ADDLINE
void DISPL_MRO_IN(void);

void Begin_Check(void);
void LINE_Version(void);
void LINE_Address(void);
void LINE_Searsh_MRK(void);
uint8 FindMinNmbSection(void); //����� ����������� ������ ������, ���-� � REGIME.NmbSection
uint8 FindMaxNmbSection(void); //����� ����������� ������ ������, ���-� � REGIME.NmbSection


uint8 FindNmbMRK(void);//����� ������ MRK �� � ������
void ClearDATAMRK(void);//������ ������� ������ ��� � ������
void LINE2_SEC_R(void); //2 ������:��� NmbSection ��� NmbModule
void LINE2_SEC_M(void); //2 ������:��� NmbSection ��� NmbModule

void Rele(void);  //���������� ����

void ApplyMaskFailMRK(void);//��������� ����� ������������� � ���������� ������ ���
void CheckTimeFail(void); //�������� ������� ���������� ������. ����>60 ��� - �����
void CheckFail_MRK(void); //��������  ���������� 32 ��� 
void CheckFail_MRO(void); //��������  ���������� 6 ���
void CheckAlarm_MRK(void); //��������  ������� 
void CheckAlarm_MRO(void); //��������  ������� 
void CheckResource(void); //�������� ��������� ����� 80%
void CheckSrab(void); //��������  ������������
void CheckNetw(void); //���������� ����� � �����-���� ������� ��-�.
void TimeSetDispl (void);    //��� ���. ������
void TimeSetDisplLn (void); //� ���. �������

void DataDisplOrder(void); //������ ��� ���������������� ��������� ���� ����� ������:
uint8 HEX_DEC (uint8 Data); //hex to dec 99
uint8 CHAR_LINE (uint8 Data, uint8 *pLn); //hex to text �� 99 -> LINE
uint8 PollingResource(void); //����� ���������

uint8 Delta (uint8 Data1, uint8 Data2); //������������� �������

void Poll_Regime (void); //�������� �� ������� ������
void CheckTimeNoPress(void); //����. ��� ���������� ������� �� �������
void Check_Button(uint8 Key, uint16 *Time, uint8 *Button );
void CheckAddress(void); //������ ������
void Poll_button (void);
void Poll_Srab (void);  //����� ������������ �� 6 ������. ���� ����� 60���- �����
void PrepSearsh(void); //���������� � ������ ���

void ChoiceSideDwn(void);//������ � CONFIGMRO Side
void ChoiceSideUp(void);//������ � CONFIGMRO Side
uint8 ChoiceDwn(uint8 Data, uint8 Allow);//������ � Data ������ 1-3 ��� ������� Dwn � ��������� ���������� ����� � Allow
uint8 ChoiceUp(uint8 Data, uint8 Allow);//������ � Data ������ 1-3 ��� ������� Up � ��������� ���������� ����� � Allow 

void SaveNmbSection(void);//��������� � ������ ������ ������ ��� ���0����3
void SORT_Res(void); //���������� �� �������
uint8 SORT_Srab(void);//���������� �� ������������
void AddDataMRK(void); //���������� ������ � ������ DATAMRK �� ���������� �� RS485
 
void DataAPKOrder(void); //������ ��� ���������������� �������� � ���� ���� ����� ������:
void DataAlarmAPK(void); //������������ ������ ������ ��� �������� �� APK
//void CheckAPKAllAlarm(void);     //�������� ������� ������ 

void WriteEEPROM(uint8 __eeprom *pEDATA, uint8 *pDATA, uint8 Size);
#endif