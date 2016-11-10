
/** @file UART.c 
*   @brief ��������� ���������� UART
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


struct Uart UART0,  /**<����� ��-�*/
UART1;              /**< ����� � ���-��*/
struct ReadMRK READMRK; /**< ����������� ������ � ��-�*/
struct DataSummMRK DATASUMMMRK[],/**< ����������� ��������� ������ � ��-�*/
DATASUMMMRK_Copy ;/**< ����� ��������� ������ � ��-�*/
/** �������� UART*/
__flash uint16 UBRR[10] ={FOSC/16/2400-1, FOSC/16/4800-1, FOSC/16/9600-1, FOSC/16/14400-1, FOSC/16/19200-1, FOSC/16/28800-1, FOSC/16/38400-1, FOSC/16/57600-1, FOSC/16/115200-1, FOSC/16/1200-1};
/** �������� ��� 1.5 byte*/
__flash uint8 RTU_DELAY[10] ={45, 23, 11, 8, 6,  4,  3, 2, 1, 90}; 
//7.37MHz:1024=138mks. ����� �� ��������� OCR0A
//1200-20ms, 2400-10, 4800-5 ms, 9600-2.5 �����=2.5 ��, 14400- 1.7, 19200 - 1.25, 28800-0.8, 38400 -0.6
/** �������� ��� 2 byte*/  
__flash uint8 RTU_ANSW[10] ={60, 30, 15, 10, 8,  5, 4, 3, 1, 120}; 
/**�-�� ���� ��� ������ DataType: [0]-�����, 1- ����*/
__flash uint8 NmbByteReadMRK[2]= {16, 34};      
/**����� ��� ������ DataType: [0]-�����, 1- ����*/
__flash uint8 BeginAddrMRK[2]= {0x00, 0x20};    
   //� ����� �������� ����� 2+1.5=3.5 �����

/**������� ������� �������� MRK_ECSE*/
__flash uint8 TableNmbSectionMRK_ECSE[2][4]={ //0- ��� ������
1,2,3,4, //�������=1
5,6,7,8};//�������=2
/**������� ������� �������� MRK_ECSM*/
__flash uint8 TableNmbSectionMRK_ECSM[4]={0,1,2,3,}; //��� �1 -1 ������, �2 -2, �3 -3

uint8 NMB_UART_RX0, /**<������� ����� ����� ������*/
NMB_UART_TX0,       /**<������� ����� ����� ��������*/
MAX_NMB_UART0,      /**<���������� ���� ��������*/
NMB_UART1,          /**<������� ����� ����� ������*/
NMB_UART_TX1,       /**<������� ����� ����� ��������*/
MAX_NMB_UART1;      /**<���������� ���� ��������*/

uint8 RX_UART0[48];/**<����� ������ ����� RS485 -MRK*/
uint8 TX_UART0[16];/**<����� �������� ����� RS485 -MRK*/
extern uint8 ReqMRK; //������ ���
/**� MRK ��� 0-Current, 1-Summ*/
uint8 NMB_MRK[2];    //0-Current, 1-Summ
uint16 NoAnswMRK; /**<��� ������ ����� 2-� ��������*/
uint16 NoClearMRK;
extern uint16 DisplayFailMRK[2];       //[0]-����0(���1) 15...0 ���, [1]-����1(���2) 15...0 ��� 

uint8 RX_UART1[48];/**<����� ������ ����� RS485 -���-��*/
uint8 TX_UART1[48];/**<����� �������� ����� RS485 -���-��*/
uint8 DATA_APK[24];/**<������ �� ������� ���-��*/

uint8 RX_UART0_TEST[2], RX_UART1_TEST[2]; 

extern __eeprom uint8 EAddrNet;/**����� ��-� � ���� ���-��*/

/** @fn void InitUSART(void)
*   @brief ������������� USART

    USART0 -����� RS485, USART1 -����� RS485 
*/
void InitUSART(void)
 {//0-����� RS485
  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);       //|(1<<UPM0);
  UBRR0H = (uint8)(UBRR[S9600]>>8);     // Set baud rate
  UBRR0L = (uint8)UBRR[S9600];
  UCSR0B=(1<<TXEN0)|(1<<TXCIE0)|(1<<RXEN0)|(1<<RXCIE0);   //Enable TX+receiver
// Set frame format: 8data, 1stop bit  2stop b:|(1<<USBS)
// UPM1=1, UPM0=0 - parity even, 11 - odd
  
  //1-����� RS485
  UCSR1C = (1<<UCSZ10)|(1<<UCSZ11);       //|(1<<UPM0);
  UBRR1H = (uint8)(UBRR[S9600]>>8);     // Set baud rate
  UBRR1L = (uint8)UBRR[S9600];
  UCSR1B=(1<<TXEN1)|(1<<TXCIE1)|(1<<RXEN1)|(1<<RXCIE1);   //Enable TX+receiver
 } 

/** @fn void Find_MRK(void)
*   @brief ����� �� UART ���� �������������� ��-� (��� ���������������� �������), ���������������� ������ ���

    �� ����� ������ 16 ��-� ������ �� (����� ����������) ������������������.
    ����������� ������ ������� ������ CONFIGMRO.NmbSectionMRK[]: ��� ���������� 
    ���-���-� � ���-���-� ��� 4-� ��-� � ������ ����������� ������ ������ �� �������. 
    ����� ����������� ������ ������ ��� 6 ������ ��-�. 

    ���������� ��������  2 ������� ��� ������� ��-�, ���������� ��-� �������� �� ������������. 
    ���������� ��-� ������������ � CONFIGMRO.OnMRK ��� ������������. 
    ����� ���������� ����. �������: 
    - ����������� �����. 
    - ����������� ������  TX_UART0[] ��� 1-�� ������� ������� ����� ��� 16 ������� 
      ��-�, ����� ��������������� ������ �-�� ��� �������� ����� � �� ����� 0 (��� ������ � ������� 0)
    - ��������� ����� ������ RX_UART0[], ��������������� ������� �������� ������ CNT1 
      � ����������� ��������. ������� ����� �������� ��� ��������� ������� �� ��������.
    - ���������� �����,  ��������������� ������� �������� ������. ������� ����� ������ 
      ��� ��������� ������� �� �����
    - ����������� ������������ ���������� ������ � ������ ������ RX_UART0[]. 
      ���� ������ ���������� - ��������������� ��� ������������� ��-�.
    - ����������� �����, ����������� ����� 100 �� ��� ������ ����. ��-�
    - ����� ����������� ��� 16 ��-�, ����� ���� ����������� 2-� ������� ������ �� ���������� ��-�. 

*/
void Find_MRK(void) //����� ��� 0-15 2 ����. ��������� 100%. ����� 32*16=0.5�
 {uint8 i, Nmb;
  uint16 CRC_TX;
/** - �������� �������������� ��� � CONFIGMRO.NmbSectionMRK  */
  for(i=0;i<16;i++)                       //�������� �������������� ��� 
    CONFIGMRO.NmbSectionMRK[i]=0;
  
  if (CONFIGMRO.Type==VZU_M) //��� VZU_M c����� 1 ��� �1, 2 ��� �2,3 ��� �3
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
  else                         //VZU_E -��� 4 ������
    for(i=0;i<4;i++)
     {Nmb=TableNmbSectionMRK_ECSE[(CONFIGMRO.Side-1)][i];
      CONFIGMRO.NmbSectionMRK[Nmb]=Nmb;     //����� ��� ������ = ������ ������
      if (i<3)                              //MRO: 6 ������ - 3 ������
       {CONFIGMRO.NmbSectionMRO[2*i]=Nmb;
        CONFIGMRO.NmbSectionMRO[2*i+1]=Nmb;
       }
     }
  
  CONFIGMRO.OnMRK=0;
  CLRB(UCSR0B,RXEN0);          //����. ��������
//   - ����������� ������  TX_UART0[] ��� 1-�� ������� ������� ����� ��� 16 ������� 
//    ��-�, ����� ��������������� ������ �-�� ��� �������� ����� � �� ����� 0 (��� ������ � ������� 0)
  for (i=0;i<2;i++)//2 ������ � ����������� ������
   {TX_UART0[1]=0x03;
    TX_UART0[2]=0x00;
    TX_UART0[3]=BeginAddrMRK[0];      //����� �������� (������)
    TX_UART0[4]=0x00;
    TX_UART0[5]=0x01;              //1 �����

// - ���� � �1 ����� ��� �-�� ���� � ������ ������, �.�. ����� ��� ������ = ������ ������    
    for (NMB_MRK[0]=0;NMB_MRK[0]<16;NMB_MRK[0]++)//��� � �������� �� 1 �� 16
     {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])  //���������� �� ����������
       continue;
      if (CONFIGMRO.NmbSectionMRK[NMB_MRK[0]]==0) //0 ������ � 0 ��� ���, �� ���� 
       continue;//����� ��� ������ = ������ ������
      TX_UART0[0]=NMB_MRK[0];
      CRC_TX=CRC16(&TX_UART0[0],6) ;    
      TX_UART0[6]=(uint8)CRC_TX;
      TX_UART0[7]=(uint8)(CRC_TX>>8);
      MAX_NMB_UART0=8;
      CLR_BUF(&RX_UART0[0],8);
      NMB_UART_TX0=1;

// - ���� �1. �������� ������� �������� =OCR1A*139���      
      OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[0]*4; //���� 2*14*4=56 ����(112 ��)
      StartT1; 
      TCNT1=0;//����� ������ �������� ������

      UART0.TXBeg=1;               //�������� �������� (+�������� 3 ����)
      UART0.TXCmpl=0;
      UART0.RX_TIME=0;
// - �������� ����� ������� �������� 3 ����� � ����� ��������*/
      TCNT0=0;                      //������ ������� ����� �������
      StartT0;                      //���. �0, :1024
      OCR0=(RTU_DELAY[S9600]*2);    //3 ����� 
//  - ���� ����� �������� � �������� �����*/    
      while((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) //���� ����� �������� � �������� �����
       __watchdog_reset();
      UART0.RX_TIME=0;              //������ �������� ������
      UART0.RXCmpl=0;
      SETB(UCSR0B,RXEN0);           //���. ��������
      CLR_BUF(&RX_UART0[0],16);     //������ �� �������� ��������� �����, �����. �������� �����
      UART0.RXBeg=0;
      NMB_UART_RX0=0;
      TCNT1=0;                      //����� ������ �������� ������
//  - ���� ����� ������ � �������� ����� */      
      while ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) //���� ����� ������ � �������� �����
       __watchdog_reset();
      if (UART0.RXCmpl==1)       //��������� �����
// - ����������� ������������ ���������� ������ � ������ ������ RX_UART0[].         
       {if ((RX_UART0[0]==NMB_MRK[0])&&(RX_UART0[1]==0x03)&&(RX_UART0[2]==2))
         {if (CRC16(&RX_UART0[0],7)==0) //�������� �����
           {SETB(CONFIGMRO.OnMRK,NMB_MRK[0]); //�������� ��������� ���
// - ���� ��� ����, ���� ���- ����������� (����� ��� ������ = ������ ������) 
//���� ���������� ������� ������ ��� ��������� ��� � � ���, ���� ���� - � ��� �� ���������  
/*            for (i=0;i<6;i++)//3*2=6 ������ ���            
             {if (CONFIGMRO.NmbSectionMRO[i]==CONFIGMRO.NmbSectionMRK[NMB_MRK[0]])                          //
               CLRB(CONFIGMRO.OnCircMRO,i);  //����. ���� ��� � ��������� �������
             }
*/
           }
         }
// - ����������� �����, ����������� ����� 100 �� ��� ������ ����. ��-�/      
        CLRB(UCSR0B,RXEN0);      //����. ��������
        UART0.RXCmpl=0;
        while (UART0.RX_TIME==0) //���� �������� 100�� ��� ����. �������
         __watchdog_reset();
       }
     }
   }
 }

//����� ����� ������
/** @fn void Poling_MRK(void)
*   @brief �����  ��-� � ��-� 

    �-�� ���������� ������������ ��-� (��������������� ������������� ����� � CONFIGMRO.OnMRK). 
    ��� ������ ������ ������������ 3-�, ��� ������ - 6-�   �-�� MODBUS, ��. �������� ������ ��-� - ��-�
    ��� ��-� � ������� READMRK.NMB_MRK ������������ 3-�� �-�� (��. ����. �������� ������ ��-�)
  - ������� ������ - ����� ��������� �������, ������������ ������ � ������������� ������ �������
  - ��������� ������ - ����������� ������� ��������� ������� � ���������� ������������
    ����������� ������ �������/��������� ������ - ���� ��������� ����� ���� �������: 
    ������� ������ ������������ ��� ���� ������������ ��-� (����� ������������� ��-� - � NMB_MRK[0]), 
    ����� ���� ������������ ��������� ������ ��� ������ ��-� (����� ������������� ��-� - � NMB_MRK[1]) � �.�.
    ��� ����������� ��������� �������� ���������� ������������ � ��������� ������� ���������� �������� 
    ������/�������� �� ��-� ������� ������. � ������ ����� ��������� �� ��-�, ��-� - ����������� ������ �� �������. 
    �� ������� � ��-� ��-� ������ ����������� ������� ������. ���� � ����������� ������� ������ ������������ ����� 
    ������, ���������� ������������ (�� 3 ��. �� ������ ������) � ��������� ������� (�� 99% �� ������ ������)  
    ��  ��-� �������� � ��-� ��� ��������� ������ ������ �� �������� ��-� �������� �������� ������� ������. 
    � �������� ���� ��������� ������������ CRC (������ 0�06) ���� ������� ������, �-�� ���������� �������� 6-� �-�� 
    � ������ � ������� 0�110, ��. ����. �������� ������ ��-�.

    �-�� ����������� ���� / �����: 
  - 1. ������������ ��� ��-� � ���� RS-485 (CONFIGMRO.OnMRK = 0), ��� ������� ����� ������� ������� / ��������� ������ ��� �������� ��-�.
  - 2. �� ����� ���� ������� ��� ������ ��-� (����. ����� ���������� - ��������� �������), �� �� ���������� �-�� SendPoling_MRK().

    <b> ���� ��� ������ �� �-��, ��� �������� ��� ������� ����� ������ ��� ��������: </b>
      - 1.����� ������ AnswRX. ����������� ��������� ������� ������ (UART0.RX_TIME) � ���� ��� �������, �� ������� �� ��� ����� (UART0.RXCmpl).

    ���� ����� ������ ���������, � ���������� ������ ��� - ��������������� ���� ������ ������ NoAnswMRK ��� ������������� ��-�, 
    ���������� ��������� ������ (�� 5 ������� � ������ ������ ������� ������). �� ���������� 5 ������� ������ ������� ������ ��-�,
    �-�� �� ��������, ���������� ��������� ��-� ��� ������.

    ���� ����� ��������, �.�. �� ��������� ������� ������ (UART0.RX_TIME) ���� �������  ���������� ������ (UART0.RXCmpl), ����������� 
    ������������ �������� �� ��-� ������, ������� �������� CRC. ���� ������ ��������� ��� ������ ���� ������ CheckDataMRK:
          - HaveSummData - ����� ��������� ������ ����������� �  ��-�. ���������� �������������� � ������ DATASUMMMRK[] ��-� 
          - HaveData - ����� ������� ������. ����������� ���������� �������� ��������, � ���� ��� ���� ���������, �� ����� ����� �� 
    ���������� ����� ������ �-�� AddDataMRK() ���������� ����������� � ������ DATAMRK[]. ���� �������� �� ������, ���������� 
    ������� �������  SendClear_MRK(). ��� �������������� ���������� ��������� ���������� ����� � ��� �� ������ � ������ ��-�, 
    � ������ ���� �� ����� ���� �������� �� ���� ��������� ������� ��������� ������ ��-�.

    ����� ����������� ������ � ���������� ��-�. 
    ����� ������ ������� ������ ��� ����  ��-� ���������� ������ ��������� ������ ��� ������ ��-�.
	
      - 2.����� �������� SendTX. ����������� ��������� ������� �������� (UART0.RX_TIME) � �������� ���������� �������� (UART0. TXCmpl). 
    ��� ���������� ������ �� ���� ������� -������� � ����� ������ � ��������� ������� CNT1 �� �������� ������� ������ 

*/
void Poling_MRK(void) //������ / ������ ������ �� ���� ��� ��� ������� ��� ��������� ������
 {uint8 i;
  
  if ((CONFIGMRO.OnMRK==0)||(UART0.Regime==SendTXClear)||(UART0.Regime==AnswRXClear))
   return;
  if ((UART0.Regime==SendTX)||(UART0.Regime==AnswRX))//�������� UART0 ���� �� ��� ����.
   ;//��, 
  else //������ � ������ ���� ��� ����
   {UART0.DataType=Current;
    SendPoling_MRK();   //��������
   }
 
//����� ������  
  if (UART0.Regime==AnswRX)      //��� ����� ������, ��������� ��� ���������
   {//if ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) 
    if (UART0.RX_TIME==0)
     return;                     //���� ����� ������ � �������� �����
    if (UART0.RXCmpl==1)         //����� ��������, ������ ������
     {if ((RX_UART0[0]==NMB_MRK[UART0.DataType])&&(RX_UART0[1]==0x03)&&(RX_UART0[2]==NmbByteReadMRK[UART0.DataType])
         &&(CRC16(&RX_UART0[0],NmbByteReadMRK[UART0.DataType]+5)==0))  //�������� �����
       {CLRB(NoAnswMRK,NMB_MRK[UART0.DataType]);
        
        switch (CheckDataMRK())        //�������� �������/��������� ������
         {case HaveSummData:
if (NMB_MRK[1]<16) //����� �� ����������� �����
             DATASUMMMRK[NMB_MRK[1]]=DATASUMMMRK_Copy;//����������� ��������� ������
          case NoData:
Next_MRK:   
            ReqMRK=0;              //��� ������ ��� �������� ������� ��������� ��������
            NMB_MRK[0]++;
            for(i=0 ; i<16; NMB_MRK[0]++,i++) //����� ����. ���
             {if (NMB_MRK[0]>15)      //������� ���� ������, ��������� ������ ��� ��������
               {NMB_MRK[0]=0;          
//��� � ���� ������ ���� ��� - ����������� ��������� ��� ������ ���               
                NMB_MRK[1]++;
                if (NMB_MRK[1]>15)      //������� ���� ������, ��������� ������ ��� ��������
                 NMB_MRK[1]=0;
                for(i=0 ; i<16; NMB_MRK[1]++,i++)
                 {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[1])
                   {UART0.DataType=Summ;    //�����
                    SendPoling_MRK();      //�������� 
                    return;
                   }
                 }
               }
              if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
               break;              //����� ���. ���
             }
            
           if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
            {UART0.DataType=Current;    //�����
             SendPoling_MRK();      //��������
            }
           return;//case NoData

          case HaveData:
            if (CHKB(NoClearMRK,READMRK.NMB_MRK)==0)//���������� �������� - �������
             AddDataMRK();         //���������� ������ � ������ DATAMRK �� ���������� �� RS485
            ReqMRK=0;
            SendClear_MRK();      //������� ������� �������
          return;//case HaveData
          case NoCorr:
            goto Next_Req;
         }
       }
      else              //����� ��������, ��������� ������
       goto Next_Req;
     }
    else                //��� ������
Next_Req:       
     {if CHKB(CONFIGMRO.OnMRK,NMB_MRK[0])
       {SETB(NoAnswMRK,NMB_MRK[0]);//������ ������
        ReqMRK++;
        if (ReqMRK<6)     //��������� ������
         {UART0.DataType=Current;    //�����
          SendPoling_MRK();//��������
         }
       }
      goto Next_MRK;
     }
   }
//����� ��������   
  else if (UART0.Regime==SendTX) 
   {if ((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) 
     return;            //���� ����� �������� � �������� �����
//�������� ���������, ����� + �������� ������� ������
    TCNT1=0;            //����� ������ �������� ������
    UART0.RXBeg=0;   
    NMB_UART_RX0=0;
//������ �������� ������      
    UART0.RX_TIME=0;             
    //OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[UART0.DataType]*3; //���� 2*14*2=112���� 
OCR1A = RTU_ANSW[S9600]*NmbByteReadMRK[UART0.DataType]*8; //���� 2*14*15=420���� (0.4 ���) 
//���� ���������� ����, �������� ������  
    UART0.Regime=AnswRX;         //
    SETB(UCSR0B,RXEN0);          //���. ��������
   }
 }


/** @fn void Clear_MRK(void)
*   @brief C������� � ��-� ���������� ���������� (���� ��� ����)

    �-�� ������������ �������� � ����� ��-� �� ������ �� ��������  ������� ������ ��� ��-� � ������� READMRK.NMB_MRK. 
    ����������� ��������� �� ��� ��-� � ���� RS-485 (�� ������ CONFIGMRO.OnMRK), ������� �� ����� ������/�������� ������� ������. 
    ���� ���� �� ������� ����������� - �-�� �����������. ���� ��� ������ �� �-��, ��� �������� ��� ������� ����� ������/�������� ��������:
  - ����� ������ AnswRXClear. ����������� ��������� ������� ������ (UART0.RX_TIME) � ���� ��� �������, �� ������� �� ��� ����� (UART0.RXCmpl). 
    ���� ����� ������ ���������, � ���������� ������ ��� - ��������������� ���� ������ ������ � �������������� ������� � ����� ������  
    ������� ������, � ������� �������������� ���������� �������� ���-��. ���� ����� ��������, ����������� ������������ �������� ������, 
    ������� �������� CRC. ���� ������ ���������, ������������ ���� ������ ������ � �������������� ������� � ����� ������  ������� ������.
  - ����� �������� SendTXClear. ����������� ��������� ������� �������� (UART0.RX_TIME) � �������� �� ���� �������� (UART0. TXCmpl). 
    � ������ ���������� ������ �� ���� ������� - ��������� �������� � ��������� � ����� ������ � ��������� ������� CNT1 �� �������� ������� ������.

 */
void Clear_MRK(void) //�������� ���������� ���� � ��� ���� ��� !=0
 {if ((CONFIGMRO.OnMRK==0)||(UART0.Regime==SendTX)||(UART0.Regime==AnswRX))
   return;
  
//�������� ������� �������, �� 2� ���
  if (UART0.Regime==AnswRXClear)    //��� ����� ������, ��������� ��� ���������
   {//if ((UART0.RXCmpl==0)&&(UART0.RX_TIME==0)) 
    if (UART0.RX_TIME==0)
     return;
    if (UART0.RXCmpl==1)            //���� ������, ������ ������
     {if ((RX_UART0[0]==READMRK.NMB_MRK)&&(RX_UART0[1]==0x06)
          &&(RX_UART0[2]==ClearAddrH)&&(RX_UART0[3]==ClearAddrL)
          &&(CRC16(&RX_UART0[0],NmbByteClearMRK)==0))  //�������� �����, �.� ������
       {CLRB(NoAnswMRK,READMRK.NMB_MRK);
        CLRB(NoClearMRK,READMRK.NMB_MRK); //�������� ��������
        ReqMRK=0;
       }
      else //������, ����� ������
       {SETB(NoAnswMRK,READMRK.NMB_MRK);   //��� ������ 
        SETB(NoClearMRK,READMRK.NMB_MRK);  //������ ��������
       }
     }
    else //��������� ����� �������� ������ � ��� -20��
     {SETB(NoAnswMRK,READMRK.NMB_MRK);   //��� ������ 
      SETB(NoClearMRK,READMRK.NMB_MRK);  //������ ��������
     }
    
    UART0.Regime=SendTX;
    UART0.DataType=Current;
    SendPoling_MRK();           //������� � ����� ������ ���, 
   }
//����� ��������     
  else if (UART0.Regime==SendTXClear) 
   {if ((UART0.TXCmpl==0)&&(UART0.RX_TIME==0)) 
     return;            //���� ����� �������� � �������� �����
//�������� ���������, ����� + �������� ������� ������
    TCNT1=0;            //����� ������ �������� ������
    UART0.RXBeg=0;   
    NMB_UART_RX0=0;
//������ �������� ������      
    UART0.RX_TIME=0;
    //OCR1A = RTU_ANSW[S9600]*NmbByteClearMRK*3; //���� 8*2*3= (��� 45) ����
OCR1A = RTU_ANSW[S9600]*NmbByteClearMRK*10; //���� 2*8*15=240����  (240 ��)   
    UART0.Regime=AnswRXClear;        
    SETB(UCSR0B,RXEN0);          //���. ��������
   }
 }

/** @fn void SendPoling_MRK(void)
*   @brief ���������� �������� ��� ������ ��-� � �� �����  

*/
void SendPoling_MRK (void)
 {  uint16 CRC_TX;
    CLRB(UCSR0B,RXEN0);          //����. ��������
    TX_UART0[0]=NMB_MRK[UART0.DataType];
    TX_UART0[1]=0x03;
    TX_UART0[2]=0x00;
    TX_UART0[3]=BeginAddrMRK[UART0.DataType];      //����� ��������
    TX_UART0[4]=0x00;
    TX_UART0[5]=NmbByteReadMRK[UART0.DataType]/2;     //�-�� ���� ������
    CRC_TX=CRC16(&TX_UART0[0],6) ;
    TX_UART0[6]=(uint8)CRC_TX;
    TX_UART0[7]=(uint8)(CRC_TX>>8);
    MAX_NMB_UART0=8;
    UART0.Regime=SendTX;
    
    UART0.TXBeg=1;               //�������� �������� (+�������� 3 ����)
    UART0.TXCmpl=0;
    UART0.RX_TIME=0;
//�������� ��  RTU_DELAY  
    TCNT0=0;                     //������ ������� ����� �������
    StartT0; //���. �0, :1024
    OCR0=RTU_DELAY[S9600]*2;     //3 �����
//������� ������� �����    
    NMB_UART_TX0=1;
    NMB_UART_RX0=0;
    CLR_BUF(&RX_UART0[0],16);
 }

/** @fn void SendClear_MRK(void)
*   @brief ���������� �������� ��� �������� ��-� � �� �����  

*/
void SendClear_MRK (void)
 {  uint16 CRC_TX;
    CLRB(UCSR0B,RXEN0);          //����. ��������
    TX_UART0[0]=READMRK.NMB_MRK;
    TX_UART0[1]=0x06;
    TX_UART0[2]=ClearAddrH;
    TX_UART0[3]=ClearAddrL;      //����� ��������
    TX_UART0[4]=(uint8)(READMRK.CRC>>8);
    TX_UART0[5]=(uint8)READMRK.CRC;
    CRC_TX=CRC16(&TX_UART0[0],6) ;
    TX_UART0[6]=(uint8)CRC_TX;
    TX_UART0[7]=(uint8)(CRC_TX>>8);
    MAX_NMB_UART0=8;
    UART0.Regime=SendTXClear;
    
    
    UART0.TXBeg=1;               //�������� �������� (+�������� 3 ����)
    UART0.TXCmpl=0;
    UART0.RX_TIME=0;
//�������� ��  RTU_DELAY 
    TCNT0=0;                     //������ ������� ����� �������
    StartT0; //���. �0, :1024
    OCR0=RTU_DELAY[S9600]*2;     //3*2 �����
//������� ������� �����    
    NMB_UART_TX0=1;
    NMB_UART_RX0=0;
    CLR_BUF(&RX_UART0[0],16);
 }

/** @fn void CheckDataMRK(void)
*   @brief ����������� � ��-� ������� � ��������� ������ ���������� �� ��-�

    ������,  ���������� �� ��-� (������ RX_UART0[]) ����������� �� ������������. 
    � ������ ������������� �������� �RC �-�� ���������� ������ NoCorr. ���� ������ ����������:
  - � ������ ������ ������� ������ Current - ������ RX_UART0[] ������������� � ��������� READMRK 
    (��� ���������� ������ � ������ ��-�). ����������� ������������� ���������� ����� ������ � 
    ���������� � ������: ���� ������� ������������ ��� ������� ��������� ������� �-�� ���������� 
    HaveData (���� ������ ��� ������������), � ��������� ������ - NoData (��� ����� ������). 
    <b>�������������: </b> � ������ �� 16 ��-� ���� 2 ����� �������� ������, ��������������� 1-�� � 2-�� ���� �������. ��� ��������� ������ � ������� ������ � ���-��, ��� ������� ����� ������ (�������� ����������) ��������������� ����� ������ � ����� �� 16 ����� (��� ������ �� 16 ��-�) DisplayFailMRK[0] ��� 1-�� ���� � DisplayFailMRK[1] ������� ����. 
  - � ������ ������ ��������� ������ - ������ RX_UART0[] ������������� � ��������� DATASUMMMRK. 
    �-�� ���������� HaveSummData.

*/
uint8 CheckDataMRK(void)        //����������� �������/��������� ������
 {uint8 i;
  if (UART0.DataType==Current)//����������� ������� ������
   {if (CRC16(&RX_UART0[3],sizeof(struct ReadMRK)-3)!=0)//������. �����
     return NoCorr;
    READMRK.NMB_MRK=NMB_MRK[0];  //��������� ����� READMRK
    for(i=0; i<8; i++)
     READMRK.Resource[i]=RX_UART0[i+3];       //������ ������� � 3 �����
    
    READMRK.SrabMod=(RX_UART0[8+3]<<8)+RX_UART0[9+3];
    READMRK.FlagDamage=(RX_UART0[10+3]<<8)+RX_UART0[11+3]; 
//����������- ����������, ������� �� ����������� � ���������� ������� ���������
  if CHKB(READMRK.FlagDamage,0)
   SETB(DisplayFailMRK[0],READMRK.NMB_MRK);
  else
   CLRB(DisplayFailMRK[0],READMRK.NMB_MRK);
  if CHKB(READMRK.FlagDamage,1)
   SETB(DisplayFailMRK[1],READMRK.NMB_MRK);
  else
   CLRB(DisplayFailMRK[1],READMRK.NMB_MRK);
      
    READMRK.CRC=(RX_UART0[12+3]<<8)+RX_UART0[13+3];   //������������������ - ��������?
    READMRK.Error=(RX_UART0[14+3]<<8)+RX_UART0[15+3];
   
    for (i=0;i<8;i++)                    //���� ���� 1 ������ ��� ������������?
     {if (READMRK.Resource[i]!=0)
       {i=0xff;
        break;
       }
     }
    
    if ((READMRK.SrabMod==0)&&(i!=0xff))//&&(READMRK.FlagDamage==0)) //���� ���� 1 ������ ��� ������������?
     return NoData; //��� ����� ������, FlagDamage- �������� � ������ �� ������������
    else
     return HaveData; //���� ������ ��� ������������
   }
  else                        //����������� ��������� ������
   {NMB_MRK[0]=0;             //����� ������ ����� ,������ �������, ����� � 0 ������
    UART0.DataType=Current;   //��������� ����� ���������, ��������� � �������
    if (CRC16(&RX_UART0[3],8+16+2)!=0)
     return NoData;           //����� ��������, ���������� ����� �������
//������ �����������?
    
    for(i=0; i<8; i++)        //��������� �����
     {DATASUMMMRK_Copy.Resource[i]=RX_UART0[i+3];       //������ ������� � 3 �����
      DATASUMMMRK_Copy.SrabMod[i]=(RX_UART0[2*i+8+3])*256+RX_UART0[2*i+9+3];
      DATASUMMMRK_Copy.ADC[i]=RX_UART0[i+8+16+2+3];
     }
/*
    for(i=0; i<8; i++)        //��������� �����
     DATASUMMMRK_Copy.Resource[i]=RX_UART0[i+3];//������ ������� � 3 �����
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
*   @brief ����������� � ��-� ������� � ��������� ������ ���������� �� ��-�

3 ���� ��������: 
1.������������ (������ 0�00...0�10) ����� 0, ���-�� 32����
2.����������� ������. ������ 0�20 �����, ������ (34)26 ����. ������ 0�20, 0�36, 0�58, 0�7�...  ,0�21E-0�23F
3.����������� ������ �����������. ������- 0�240 �����, ������ 28 ����. ������ 0�240,  0�25�, 0�278,  0�294... , 0x3�4...0�3FF

*/ 

void Link_UART_MRO(void) //����� RS485 ��� ������ MR�
 {
 }

/** @fn void Link_UART_APK(void)
*   @brief �����  ��-� � ���-�� 

    �-�� ��������� ����� �� ����������� ������, ��. �������� ������ ��-�  �  
    ������� ������������������� �������������� �������� ��� ��

    <b> �-�� ���������: </b> 
  - ����� ���������� �������� UART1.TXCmpl ��� ���������� ������� ������ UART1.RX_TIME. 
    � ������ ������� ������ -�� ������ �������� ������� � ����� �����, ���������� ��������� ���������� ������ 
    UART1, ��������������� ������ �������� ������� TCNT3 � ����� �� �-�� (�������� ��������� ��� ������ ������)
  - ����� ���������� ��������� ������ UART1.RXCmpl ��� ����������� �������� UART1.TXExec. ��� ������ ������ 
    -����� �� �-�� (���� ��������� ������ ��� ��������)
    ���� ����� �������� ������ ��� ������ �� �-��, ��� �������� ��������� ������ ���������� �� APK-��. 
    ������ ������������� �� ������� ASCII �-��� CONV_ASCII_UN() � ����������� ������������ LRC � ������� 
    �-�� LRC(). ��� ������� ������ - ��������������� ���� ������,  �������� ����� ����� � ������� �� ���� �-��. 
    ���� ����� ������� ��  ���-�� ��� ����������, ��������� ������ ��� �������� � ���� ���� ����� ������ 
    � ������� DataAPKOrder() ������������ ������� �������� ������ (� ����� ������ ����� �� ���������� ��� 
    ������ � ���� ��������� ��� ���������� ������ ����� ��� ��� ������ ������������ �����). ����� ����������� 
    ����������� ������ �-�� DataAlarmAPK() �������� ��������� ������ � ���-��. ������ �� ������� ������������� 
    � ������� DATA_APK[].
    �� ������� DATA_APK[] ������ �������������� � ������ ASCII �-�� CONV_ASCII() � ������ TX_UART1[], 
    ��������������� ���������� ���� �� �������� MAX_NMB_UART1, ��������������� ������� ������� �������� 
    �������� � TCNT3 � ����������� ��������.
*/

void Link_UART_APK(void)
 {//���� ���� ������ - ������� 
   
  
  if ((UART1.TXCmpl==1)||(UART1.RX_TIME==1))//||(UART1.RXERR==1))  //��������� �������� ��� ��������� ����� ��� �������� �����
   {UART1.TXCmpl=0;
    UART1.TXExec=0;
    UART1.TXBeg=0;
    UART1.RXBeg=0;
    UART1.RXCmpl=0;
    UART1.RX_TIME=0;
    UART1.RXERR=0;
    NMB_UART1=0;

    CLRB(PORTD,DR1);    //���������� ������
    TCNT3=0;            //����� �������� �������
    TCCR3B=0;           //���� ������
    return;
   }
  if ((UART1.RXCmpl==0)||(UART1.TXExec==1)) //�� ������� ������� ��� ���� ��������
   return;
//������� �������, ���������
  UART1.RXCmpl=0;
  CONV_ASCII_UN(&RX_UART1[1],&RX_UART1[1],5); //����������� ASCII
  if   (RX_UART1[5]!=LRC(&RX_UART1[1], 4)) //��������� LRC
   {UART1.RXERR=1;      //������, ���������� �����
    UART1.RXBeg=0;
    UART1.RX_TIME=0;
    CLRB(PORTD,DR1);    //���������� ������
    TCNT3=0;            //����� �������� �������
    TCCR3B=0;           // ���� ������
    return;
   }
//��������� ����� 
  SETB(PORTD,DR1);      //������ ������ �� 1
  DataAlarmAPK();       //������������ ������ ������ ��� �������� �� APK
  DATA_APK[0]=EAddrNet;  //addr0
  DATA_APK[1]=0X03;     //3 �������
  DATA_APK[2]=0X20;     //������ 
//DataAPKOrder();       //������ ��� ���������������� �������� � ���� ���� ����� ������:

  DATA_APK[19]=LRC(&DATA_APK[0], 19);

//��������������� ������ � ASCII
  CONV_ASCII (&DATA_APK[0],&TX_UART1[1],20);//� 40 ����
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
   REGIME.DisplDelay=Time_2s;     //�������� �� ���������
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
  UART1.TXBeg=1;  //����� ��������
  UART1.TXExec=1; //���� ��������
//������ ������� �� ��������, ��� ���������� - �������������� ������� �� �����  
  TCNT3=0;
  TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //���� �����. 
  OCR3A=TimeStartUART1;    
 }

/** @fn uint8 convert_ASCII_byte(uint8 ASCII_byte)
*   @brief convert_ASCII_byte 
    \param[in] ASCII_byte ���� � ������� ASCII (0-9, A-F)
    \return  
        ����������� ��������
*/
uint8 convert_ASCII_byte(uint8 ASCII_byte)
{
uint8 Temp;
  if ( (ASCII_byte>0x2F) && (ASCII_byte<0x3A) ) Temp = ASCII_byte & 0x0F; // �������� ������� �� 0 �� 9
  else {
        Temp = ASCII_byte &  0xDF;	                // ������� � ������� ������� ����� �� A-F
	if ( (Temp>0x40) && (Temp<0x47) ) Temp -= 0x37; // �������� ������� �� CHAR � A �� F
	else  Temp = 0x0F; // ������ �� � ���������
  }
return(Temp);	
}

/** @fn void CONV_ASCII_UN(uint8 *sourse,uint8 *dest,uint8 len)
*   @brief �������� � ASCII �� ������ *sourse � *dest ������ len
    \param[in]  *sourse �� ������
    \param[in]  *dest  � �����
    \param[in]  len  ������
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
*   @brief �������� �������� � ASCII 
    \param[in]  HalfByte
    \return  ASCII ����          
*/
uint8 convert_to_ASCII(uint8 HalfByte)
{HalfByte&=0X0F;
 if (HalfByte<0X0A)
  return HalfByte+0X30;
 else 
  return HalfByte+0X37;
}

/** @fn void CONV_ASCII(uint8 *sourse,uint8 *dest,uint8 len)
*   @brief �������� � ASCII �� ������ *sourse � *dest ������ len
    \param[in]  *sourse ��������� ��� �����������
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
*   @brief ������ LRC
    \param[in]  *data ��������� ��� �����������
    \param[in]           len ����� 
    \return LRC
*/
uint8 LRC(uint8* data , uint16 len)
 {uint8 LRC=0;
  while( len-- )
   LRC+=*(data++);
 return  0xFF - LRC + 1;
 }

/** @fn CLR_BUF(uint8 *Data, uint8 NMB)
*   @brief ������� ������ � ������
    \param[in]  *Data �����
    \param[in]   NMB ����� 
*/
 void CLR_BUF(uint8 *Data, uint8 NMB)
 {while (NMB--)
   *(Data+NMB)=0;
 }


/** 
������� ������� CRC
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
������� ������� CRC
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
*   @brief ������ CRC16 � ������
    \param[in]  *buff �����
               size �����
    \return �RC
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
*   @brief ������ �������� ������� ����� ������� -���
    
������ ���������������: 
  - � ������ ������ ������ (����� ��������� ������� ����� � ������������ ����� 
������ ������ UART0.RXBeg), ���������� ������� ���������� ��������������� � ���������� 
������� �������� ������ ���������� ����� (2 ���������), �.�. ����������� ����� ������ � 
��������������� ���� UART0.RXCmpl ����������������� � ����� ������
  - � ������ ������ �������� (UART0.TXBeg==1) ������ ���������� ������������ ��� ������������ 
�������� ����� ������ ������ � ������� ��������. ����� ����� �������� ��������, � UDR0 
���������� 1-� ���� 
*/
#pragma vector=TIMER0_COMP_vect //������ �������� ������� ����� ������� -���
__interrupt void TIMER0_COMP_vec(void)

{StopT0; //stop timer
 if (UART0.RXBeg==1) 
  {UART0.RXBeg=0; 
   UART0.RXCmpl=1;// �������� ������ 1.5 ����� -����� ������
  }
//����� ��������� TXBeg=1, �������� 2����� � �������� �������� (� ����� �������� ����� 2+1.5=3.5 �����) 
 else if (UART0.TXBeg==1)   
  {UART0.TXBeg=0;
   SETB(PORTE,DR0);        //���. ��������
   UDR0=TX_UART0[0];       //���������� �� RS485
  }
}

/** @fn void TIMER1_COMPA_vec(void)
*   @brief ������ �������� ������� ������ (������) � ��-� 
    
  �� ���������� ��������������� ���� ���������� ������� UART0.RX_TIME
*/
#pragma vector=TIMER1_COMPA_vect //�������� ������� ������ -���
__interrupt void TIMER1_COMPA_vec(void)
 {//StopT1;         //���� �������
  UART0.RX_TIME=1; //����� ���������
 }

/** @fn void USART0_RXC_interrupt(void)
*   @brief ����� �� UART � ���
    
    ������������ ������ �������� ������� ������ ����� TIMER0.
    ���� ������ ������ ���� - ��������������� ���� ������ ������ UART0.RXBeg, 
    ����������� TIMER0 � ��������� �������� ������ ���������� ����� (����� ������ 2 ����).
    �������� ����� (�� 48 ���� � ������� ������) - ������������ � ������ RX_UART0[]  
*/
#pragma vector=USART0_RXC_vect //����� ���
 __interrupt void USART0_RXC_interrupt(void) 
{uint8 DATA=UDR0;
    if (RX_UART0_TEST[1]!=(Time_1s-1))
    {RX_UART0_TEST[0]=DATA;  //� �������� ������ - ��� ��������� ����. �����
     RX_UART0_TEST[1]=Time_1s;     //� �������� ������ - 1 ��� ���������
    }
   TCNT0=0; //����� ������� �������� ������� ����� �������
//������ ������, ��������� �������� ������� ����� �������
   if (UART0.RXBeg==0)
    {UART0.RXBeg=1;
     OCR0=(RTU_DELAY[S9600]*2);
     StartT0; //���. �0
    }
   if (NMB_UART_RX0<48) //����� �� ��������� ������
    *(&RX_UART0[NMB_UART_RX0++])=DATA ;
}
/** @fn void USART0_TXC_interrupt(void)
*   @brief �������� �� UART � ��� 

    �������� ������ � UART0 �� ��� ���, ���� �� ����� NMB_UART_TX0 �� ��������  
    �������� MAX_NMB_UART0. ����� ���� �������� ���������������, � UART0.TXCmpl 
    ������������ ���� ���������� �������� 
*/

#pragma vector=USART0_TXC_vect //����� ��� 
 __interrupt void USART0_TXC_interrupt(void)
 {if (NMB_UART_TX0<MAX_NMB_UART0)
   UDR0 = TX_UART0[NMB_UART_TX0++];
  else               //���� ��������
   {CLRB(PORTE,DR0); //����. ��������
    UART0.TXCmpl=1;  //�������� ���������
   }
 }

// UART1 APK
/** @fn void TIMER3_COMPA_vec(void)
*   @brief ������ �������� ������� ������ � ���-��  

    - � ������ ������ ������ (����� ��������� ������� ����� � ������������ ����� 
      ������ ������ UART1.RXBeg), ���������� ������� ���������� ��������������� 
      � ���������� ������� �������� ������ ���������� ����� (1 ���), �.�. ����������� 
      ����� ������ � ��������������� ���� UART0.RX_TIME  ����������������� � ����� ������
    - � ������ ������ �������� (UART1.TXBeg==1) ������ ���������� ������������ ��� 
      ������������ �������� ����� ������ ������ � ������� ��������. ����� �������� ������ 
      ���������� �������� � ���������� 1-� ���� 0x3A � UDR1. ��������������� ����� ������� 
      1 ���. � CNT3 �� �������� ������� ��������.

*/
#pragma vector=TIMER3_COMPA_vect //������ �������� ������� ���-�� - 1 ��� ����� �������
__interrupt void TIMER3_COMPA_vec(void)
 {//
  if (UART1.RXBeg==1) 
  {UART1.RX_TIME=1;
  }
//����� ��������� TXBeg=1, �������� 2����� � �������� �������� (� ����� �������� ����� 2+1.5=3.5 �����) 
 else if (UART1.TXBeg==1)   
  {UART1.TXBeg=0;
   UDR1=0x3A;              //���������� �� RS485
   OCR3A=Time1sUART;        //1 ��� �� ��������, ��� ���������� - �������������� ������� �� ����� 
  }
 }

/** @fn void USART1_RXC_interrupt(void)
*   @brief ����� �� UART � ���-��

      ��� ����� �  ���-�� ������������ ASCII MODBUS � ������� ���������� ���������� 
      ������� ����� ��������� ������� 1 ���. ��� ���������� ������� ��������� � ������� 
      �� ������ ������ � ���������� ������������� ������ �������� ���� �������, � ��� ��� 
      �������������� ������������ ���� ������ ������ UART1.RXBeg,  ��������������� ���� 
      ������ UART1.RXERR � ����� ���������� ������. 
  - ������������ ������� �������� ������� ����� ������� CNT3
  - �.�. �������� UART ��� ����� ������ �� ����� �� ��������� ������������ ������. 
    ��� ���������� ������� �� ����� �� ����� ��������  (UART1.TXExec=1) ���������� ����� �� ���������� 
  - ����������� ������� �� ������ ������ (UART1.RXBeg). ���� ��� - ���������  ��������� ���� 
    (0x3A), ��������������� ���� ������ ������ UART1.RXBeg  � ����������� ������ �������� 
    ������� ����� ������� CNT3.
  - ���� ������� ������ ������, ����������� ������������ �������� ����: ������ � ����, ����� 
    �-��  MODBUS � �.�. ����� �������������� ����������� ����� LRC � ��� ��������� ������� -����� 0x0D � 0x0A. 
  - ����� ��������� ���� ��������� ������� ��������������� ���� ���������� ������ UART1.RXCmpl,
    ������ CNT3 - ���������������.
  - ��� ���������� ������ ������������ � ������ RX_UART1[].

*/
#pragma vector=USART1_RXC_vect //APK
 __interrupt void USART1_RXC_interrupt(void) 
{unsigned char DATA=UDR1;
   TCNT3=0;             //����� �������� �������
   if (UART1.TXExec==1) //���� ��������, �� ���������
    return;
   RX_UART1_TEST[0]=DATA;  //� �������� ������ - ��� ��������� ����. �����
   RX_UART1_TEST[1]=Time_1s;      //� �������� ������ - 1 ��� ���������    
   if (UART1.RXBeg==0)  //������
    {if (DATA==0x3A)    //��������� ����
      {TCCR3B = (1<<CS32)|(1<<CS30)|(1<<WGM21); //���� ����� 140���*N
       OCR3A=Time1sUART;    //����� ������� ���� �� ����� 1 ���
       UART1.RXBeg=1;
       //RX_UART1[0]=0x3A;
       NMB_UART1=1;
       
      }
    }
   else 
    {switch (NMB_UART1)
     {case 1:  if (DATA==convert_to_ASCII((EAddrNet>>4)&0x0F))  //����� ��. ����
                break;
               goto exit;
      case 2:  if (DATA==convert_to_ASCII(EAddrNet&0x0F))       //����� ��. ����
                break;
               goto exit;
      case 3:  //3,5,7,8 �����
      case 5:  //�-�� 03
      case 7:  //������ 00
      case 8: 
              if (DATA==0x30) 
               break;
              goto exit;
      case 4:  //4,6,8,9 ����� 
              if (DATA==0x33) 
               break;
              goto exit;
       
      case 6: if (DATA==0x31) //�����  ��������� 1
               break;
              goto exit;
              
      case 9:                 //�������� LRC (�������� 0-F)
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
                 TCNT3=0;  //����� �������� �������
                 TCCR3B=0; // ���� ������
                 break;
                }
      default:         
               goto exit;
               
     }
   RX_UART1[NMB_UART1++]=DATA;
     return;         
     
exit: 
       TCCR3B=0;     // ���� ������
       UART1.RXBeg=0;
       UART1.RXERR=1;
     }
}
/** @fn void USART1_TXC_interrupt(void)
*   @brief �������� �� UART � ���-�� 

    �������� ������ � UART1 �� ��� ���, ���� �� ����� NMB_UART_TX1 �� ��������  
    �������� MAX_NMB_UART1. ����� ���� �������� ���������������, � ��������������� 
    ���� ���������� �������� UART1.TXCmpl
*/
#pragma vector=USART1_TXC_vect //APK 
 __interrupt void USART1_TXC_interrupt(void)
 {TCNT3=0;             //����� �������� �������
  if (NMB_UART_TX1<MAX_NMB_UART1)
   UDR1 = TX_UART1[NMB_UART_TX1++];
  else  //�������� ���������
   UART1.TXCmpl=1;
 }