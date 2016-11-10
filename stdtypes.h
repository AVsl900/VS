/** @file STDTYPES.h
*   @brief Типы данных
*   @date 07.07.2013
*   @version 01.0.00
*/

#ifndef STDTYPES_H__
#define STDTYPES_H__

/* USER CODE BEGIN (0) */
/* USER CODE END */
/************************************************************/
/* Type Definitions                                         */
/************************************************************/
#ifndef _UINT64_DECLARED
typedef unsigned long long uint64;
#define	_UINT64_DECLARED
#endif

//vsl
#ifndef _UINT32_DECLARED
typedef unsigned long int uint32;
#define	_UINT32_DECLARED
#endif

#ifndef _UINT16_DECLARED
typedef unsigned short uint16;
#define	_UINT16_DECLARED
#endif

#ifndef _UINT8_DECLARED
typedef unsigned char uint8;
#define	_UINT8_DECLARED
#endif

#ifndef _BOOLEAN_DECLARED
typedef unsigned char boolean;
typedef unsigned char boolean_t;
#define	_BOOLEAN_DECLARED
#endif

#ifndef _SINT64_DECLARED
typedef signed long long sint64;
#define	_SINT64_DECLARED
#endif

//vsl
#ifndef _SINT32_DECLARED
typedef signed long int sint32;
#define	_SINT32_DECLARED
#endif

#ifndef _SINT16_DECLARED
typedef signed short sint16;
#define	_SINT16_DECLARED
#endif

#ifndef _SINT8_DECLARED
typedef signed char sint8;
#define	_SINT8_DECLARED
#endif

#ifndef _FLOAT32_DECLARED
typedef float float32;
#define	_FLOAT32_DECLARED
#endif

#ifndef _FLOAT64_DECLARED
typedef double float64;
#define	_FLOAT64_DECLARED
#endif

typedef uint8 Std_ReturnType;

typedef struct
{
    uint16 vendorID;
    uint16 moduleID;
    uint8  instanceID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

/*****************************************************************************/
/* SYMBOL DEFINITIONS                                                        */
/*****************************************************************************/
#ifndef STATUSTYPEDEFINED
  #define STATUSTYPEDEFINED
  #define E_OK     0x00

  typedef unsigned char StatusType;
#endif

#define E_NOT_OK   0x01

#define STD_ON     0x01
#define STD_OFF    0x00


/************************************************************/
/* Global Definitions                                       */
/************************************************************/
/** @def NULL
*   @brief NULL definition
*/
#ifndef NULL
    #define NULL ((void *) 0U)
#endif

/** @def TRUE
*   @brief definition for TRUE
*/
#ifndef TRUE
    #define TRUE (boolean)1U
#endif

/** @def FALSE
*   @brief BOOLEAN definition for FALSE
*/
#ifndef FALSE
    #define FALSE (boolean)0U
#endif

/* USER CODE BEGIN (1) */
/* USER CODE END */

#endif /* __HAL_STDTYPES_H__ */
