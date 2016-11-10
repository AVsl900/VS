/** @file EEPROM.h
*   @brief EEPROM Header File
*   @date 07.07.2013
*   @version 01.0.00
*   
*   This file contains:

��������� ������ � ������ ������

*/


//************************************************************
//                EEPROM MEGA128


//struct ConfigMRO CONFIGMRO;
//struct DataMRO DATAMRO[16];

__no_init __eeprom struct EConfigMRO         
 {struct ConfigMRO CONFIGMRO;
 }ECONFIGMRO[2]; /**< 2 ����� ������������ CONFIGMRO � ������*/
//extern struct EConfigMRO ;

__no_init __eeprom uint8 EAddrNet @ 0x40;/**< ����� � ���� ������*/
__no_init __eeprom uint8 ESpeedNet @ 0x41;/**< �������� � ���� ������*/

__no_init __eeprom struct EDataMRK          
 {struct DataMRK DATAMRK;
 }EDATAMRK[16];/**< ����������� ������ ��� 16 ��-�*/
//extern struct EDataMRO EDATAMRO[16];
