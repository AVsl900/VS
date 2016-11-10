/** @file UART.h
*   @brief UART Header File
*   @date 07.07.2013
*   @version 01.0.00
*   
*   This file contains:
*   - Definitions
*   - enum
*   - struct
    - ��������� �-��
*   .
*/


#ifndef UART_H
#define UART_H

#include "stdtypes.h"

#define FOSC 7372800 //Clock

#define Time1sUART      FOSC/1024    //1s
#define TimeStartUART1  FOSC/204800  //5 ��


#define ClearAddrH 0x01
#define ClearAddrL 0x10

#define StartT0   TCCR0 = (1<<CS02)|(1<<CS01)|(1<<CS00)|(1 << WGM01) // OCR0*139���
#define StopT0    TCCR0 = 0

#define StartT1   TCCR1B = (1<<CS10)|(1<<CS12)|(1<<WGM12); // OCR1A*139���
#define StopT1    TCCR1B=0;

enum AnswCheckDataMRK { NoCorr=3, NoData=5, HaveData=7, NoSummCorr=9, HaveSummData=11};

enum UBRRSPEED {S2400, S4800, S9600, S14400, S19200, S28800, S38400, S57600, S115200, S1200};   

enum UartRegime {OffUart, SendTX, AnswRX, SendTXClear, AnswRXClear /*GetSummStart, GetSummTX, GetSummRX*/}; 
enum UartType {Current, Summ}; 

#define NmbByteClearMRK 8

#define ShiftAPK        3 //����� ������ ��� ������������ ������� � ���-��

struct Uart
{unsigned RXBeg:1;   //����� ������
 unsigned RXCmpl:1;  //������� ������
 unsigned RXERR:1;   //... � ������
 unsigned TXBeg:1;   //����� ��������
 unsigned TXExec:1;  //���� ��������
 unsigned TXCmpl:1;  //�������� ���������
 unsigned RX_TIME:1; // ��������� ����� �������� �����
 unsigned Regime:3;  //����/�����/��������
 unsigned DataType:1;  //0 -�������, 1 -��������� ������
};
extern struct Uart UART0, UART1;


void InitUSART(void);
void Link_UART_MRO(void);
void Link_UART_APK(void);
void Find_MRK(void);
void Poling_MRK(void); //������ / ������ ������ �� ���� ���
void SendPoling_MRK (void);
void Clear_MRK(void); //�������� ���������� ���� � ��� ���� ��� !=0
void SendClear_MRK (void);

uint8 CheckDataMRK(void);        //����������� �������/��������� ������
//uint16 CRC(uint8 *buff,uint8 size); //max 256 ����
uint16 CRC16(uint8 *buff,uint16 size);
//__eeprom uint16 CRC16E(__eeprom uint8 *buff,uint16 size);
void CLR_BUF(uint8 *Data, uint8 NMB);

void CONV_ASCII_UN(uint8 *sourse,uint8 *dest,uint8 len);
void CONV_ASCII(uint8 *sourse,uint8 *dest,uint8 len);
uint8 LRC(uint8* data , uint16 len);
#endif