/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LX_SENSOR_H
#define	LX_SENSOR_H

#include <xc.h> // include processor files - each processor file is guarded.  

/* Quadrant Offset Macros */
#define ATAN2_OFFSET_Q2		0x4000
#define ATAN2_OFFSET_Q34	0x8000
#define ATAN2_OFFSET_COT	0x3FFF

#define QUARTER_PI			0x2000		/* 45? */

/*One electrical rotation is equal to 65535, divided by 5 equals 13107 */
#define ONE_ROTATION_NORMALIZED     (int16_t)13107 
/*The PWM Frequency is 10kHz. Divided by 5 to match the one electrical rotation*/
#define PWM_NORMALIZED       (int16_t)2000   

/*These indexes are for the moving averages filter.*/
#define     INDEX_VALUE      128
#define     INDEX_SHIFTER    7

//********************************ATAN FUNC and Table**************************
#define LOOKUP(Table, Index, RetVal)                                                  \
                    do{																	\
						int16_t tbl;														\
						int32_t tmp;														\
						(RetVal)  = (Table)[(uint8_t)((uint16_t)(Index) >> 8)];				\
						tbl       = (Table)[(uint8_t)((uint16_t)(Index) >> 8) + 1];			\
						tmp       = __builtin_mulss((tbl-(RetVal)),((Index) & 0x00FF));	\
						(RetVal) += ((int16_t)(tmp >> 8));								\
					}while(0)

extern const int16_t Atan_Table16[257];
//*****************************************************************************

// *****************************************************************************
// Section: Structures
// *****************************************************************************
typedef struct{
    int16_t signal;
    int16_t offset;
    int16_t final;
    int16_t tempVar;
    int16_t x2c;
    int16_t x2c_CCW;
}SIN_COS_THETA_T;

typedef struct{
    int16_t index;
    int32_t sum;
    int16_t avg;
    int16_t buffer[INDEX_VALUE];
}MOVING_AVG_FILTER_T;

extern MOVING_AVG_FILTER_T movingAvg;
extern SIN_COS_THETA_T sinCosTheta;

extern int16_t LX_filtVelocity;

extern int16_t LX_EstimAngle(void);
extern int16_t LX_EstimSpeed(void);

extern void MovingAvgInit(void);
extern void LX_Params_Init(void);

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* LX_SENSOR_H */

