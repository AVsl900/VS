/** @file EEPROM.h
*   @brief EEPROM Header File
*   @date 07.07.2013
*   @version 01.0.00
*   
*   This file contains:

Структуры памяти и адреса ЕЕПРОМ

*/


//************************************************************
//                EEPROM MEGA128


//struct ConfigMRO CONFIGMRO;
//struct DataMRO DATAMRO[16];

__no_init __eeprom struct EConfigMRO         
 {struct ConfigMRO CONFIGMRO;
 }ECONFIGMRO[2]; /**< 2 копии конфигурации CONFIGMRO в ЕЕПРОМ*/
//extern struct EConfigMRO ;

__no_init __eeprom uint8 EAddrNet @ 0x40;/**< адрес в сети ЕЕПРОМ*/
__no_init __eeprom uint8 ESpeedNet @ 0x41;/**< скорость в сети ЕЕПРОМ*/

__no_init __eeprom struct EDataMRK          
 {struct DataMRK DATAMRK;
 }EDATAMRK[16];/**< Накопленные архивы для 16 МР-К*/
//extern struct EDataMRO EDATAMRO[16];
