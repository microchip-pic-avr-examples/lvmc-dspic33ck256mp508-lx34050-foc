/*******************************************************************************
* Copyright (c) 2017 released Microchip Technology Inc.  All rights reserved.
*
* SOFTWARE LICENSE AGREEMENT:
* 
* Microchip Technology Incorporated ("Microchip") retains all ownership and
* intellectual property rights in the code accompanying this message and in all
* derivatives hereto.  You may use this code, and any derivatives created by
* any person or entity by or on your behalf, exclusively with Microchip's
* proprietary products.  Your acceptance and/or use of this code constitutes
* agreement to the terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS".  NO
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP'S
* PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE LIABLE,
* WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF
* STATUTORY DUTY),STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE,
* FOR ANY INDIRECT, SPECIAL,PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL
* LOSS, DAMAGE, FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE,
* HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR
* THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW,
* MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS CODE,
* SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP SPECIFICALLY TO
* HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and
* determining its suitability.  Microchip has no obligation to modify, test,
* certify, or support the code.
*
*******************************************************************************/
#include <xc.h>
#include <stdint.h>
#include <libq.h>
#include <math.h>
#include "lx_sensor.h"
#include "userparms.h"


SIN_COS_THETA_T sine;
SIN_COS_THETA_T cosine;
SIN_COS_THETA_T theta;

MOVING_AVG_FILTER_T movingAvgOmega;
MOVING_AVG_FILTER_T movingAvgVelocity;

int16_t i;

int16_t instantTheta;
int16_t prevTheta;
int16_t LX_thetaDiff;
int16_t LX_omega;
int32_t LX_omega_temp;
int16_t LX_estimVelocity;
int16_t LX_filtVelocity;

const int16_t Atan_Table16[257] = {
        0,     41,     82,    123,    163,    204,    245,    286,
      327,    367,    408,    449,    490,    531,    571,    612,
      653,    693,    734,    775,    815,    856,    896,    937,
      977,   1018,   1058,   1099,   1139,   1179,   1220,   1260,
     1300,   1341,   1381,   1421,   1461,   1501,   1541,   1581,
     1621,   1661,   1700,   1740,   1780,   1819,   1859,   1899,
     1938,   1978,   2017,   2056,   2095,   2135,   2174,   2213,
     2252,   2291,   2330,   2369,   2407,   2446,   2485,   2523,
     2562,   2600,   2638,   2677,   2715,   2753,   2791,   2829,
     2867,   2905,   2942,   2980,   3018,   3055,   3093,   3130,
     3167,   3204,   3241,   3278,   3315,   3352,   3389,   3426,
     3462,   3499,   3535,   3571,   3607,   3644,   3680,   3716,
     3751,   3787,   3823,   3858,   3894,   3929,   3965,   4000,
     4035,   4070,   4105,   4140,   4174,   4209,   4244,   4278,
     4312,   4347,   4381,   4415,   4449,   4483,   4516,   4550,
     4583,   4617,   4650,   4683,   4717,   4750,   4783,   4815,
     4848,   4881,   4913,   4946,   4978,   5010,   5042,   5074,
     5106,   5138,   5170,   5201,   5233,   5264,   5296,   5327,
     5358,   5389,   5420,   5450,   5481,   5512,   5542,   5572,
     5603,   5633,   5663,   5693,   5723,   5752,   5782,   5812,
     5841,   5870,   5899,   5929,   5958,   5987,   6015,   6044,
     6073,   6101,   6130,   6158,   6186,   6214,   6242,   6270,
     6298,   6325,   6353,   6381,   6408,   6435,   6462,   6489,
     6516,   6543,   6570,   6597,   6623,   6650,   6676,   6703,
     6729,   6755,   6781,   6807,   6833,   6858,   6884,   6909,
     6935,   6960,   6985,   7010,   7035,   7060,   7085,   7110,
     7135,   7159,   7184,   7208,   7232,   7256,   7281,   7305,
     7328,   7352,   7376,   7400,   7423,   7447,   7470,   7493,
     7516,   7539,   7562,   7585,   7608,   7631,   7654,   7676,
     7699,   7721,   7743,   7766,   7788,   7810,   7832,   7853,
     7875,   7897,   7919,   7940,   7962,   7983,   8004,   8025,
     8047,   8068,   8088,   8109,   8130,   8151,   8171,   8192,
     8192};	
// *****************************************************************************
/* Function:
    LX_EstimAngle()

  Summary:
    This routine estimates the rotor position using the sine and cosine signals 
   from the lx sensor.

  Description:
    Calculates the theta from the adjusted sine and cosine signals. 

  Precondition:
    None.

  Parameters:
    None

  Returns:
    the absolute rotor position

  Remarks:
    None.
 */
int16_t LX_EstimAngle(void){
    int16_t iTemp;
    int32_t args;
    int8_t iTan = (int8_t)1;
    int8_t Q13 = (int8_t)1;
    theta.signal = (int16_t)0;
            
#ifdef  SETTING_OFFSET
        sine.offset = sine.x2c; 
        cosine.offset = cosine.x2c; 
        
        if(mcappData.runDirection == CW)
        {
            theta.offset =  theta.x2c; 
        }
        else
        {
            theta.offset =  theta.x2c_CCW;
        }
#else
        sine.offset =    7856; 
        cosine.offset =  7744; 
        
        if(mcappData.runDirection == CW)
        {
            theta.offset = 26213;
        }
        else
        {
            theta.offset = 13106;
        }
#endif 
    
    sine.signal = ADCBUF21;
    cosine.signal = ADCBUF20;
    
    sine.final = (int32_t)sine.signal + (int32_t)sine.offset;
    
    if(sine.final > INT16_MAX){
        sine.final = INT16_MAX;
    }
    else if(sine.final < -INT16_MAX){
        sine.final = -INT16_MAX;
    }
    
    sine.tempVar = (int16_t)sine.final;
    
    cosine.final = (int32_t)cosine.signal +(int32_t)cosine.offset;
    
    if(cosine.final > INT16_MAX){
        cosine.final = INT16_MAX;
    }
    else if(cosine.final < -INT16_MAX){
        cosine.final = -INT16_MAX;
    }
    cosine.tempVar = (int16_t)cosine.final;
      
    if(cosine.tempVar == (int16_t)0x8000)
    {
        cosine.tempVar = (int16_t)0x8001;
    }
    if(sine.tempVar == (int16_t)0x8000)
    {
        sine.tempVar = (int16_t)0x8001;
    }
    
    if((sine.tempVar == 0) && (cosine.tempVar == 0))    
    {
        theta.signal = 0;
    }
    else
    {
        if(sine.tempVar < (int16_t)0)
        {
            sine.tempVar = -sine.tempVar;
            theta.signal = ATAN2_OFFSET_Q34;
            
            if(cosine.tempVar > (int16_t)0)   //4th quadrant
            {
                theta.signal += ATAN2_OFFSET_Q2;
                Q13 = (int8_t)0;
            }
            else                            //3rd quadrant
            {
                cosine.tempVar = -cosine.tempVar;
            }
        }
        else 
        {
            if(cosine.tempVar < (int16_t)0)   //2nd quadrant
            {
                cosine.tempVar = -cosine.tempVar;
                theta.signal = ATAN2_OFFSET_Q2;
                Q13 = (int8_t)0;
            }
        }
        
        //Check region in quadrant
        if(sine.tempVar > cosine.tempVar)
        {
            iTemp = cosine.tempVar;
            cosine.tempVar = sine.tempVar;
            sine.tempVar = iTemp;
            iTan = (int8_t)0;
        }
        
        if(sine.tempVar != cosine.tempVar)
        {
            args = (int32_t)sine.tempVar << 16;
            args = __builtin_divsd(args,cosine.tempVar);
            LOOKUP(Atan_Table16, (uint16_t)args, iTemp);
        }
        else
        {
            iTemp = QUARTER_PI;
        }
        
        //Check if there's a need for cotangent correction
        if(Q13 ^ iTan)  
        {
            theta.signal += ((int16_t)ATAN2_OFFSET_COT - iTemp);
        }
        else
        {
            theta.signal += iTemp;
        }
    }
    
    theta.final = theta.offset + theta.signal;
    return(theta.final);
}
// *****************************************************************************
/* Function:
    LX_EstimSpeed()

  Summary:
    This routine estimates the speed through getting the theta difference in 
    multiple samples and average them to create a smooth speed measurement. 

  Description:
    Calculates the omega from the theta difference taken in successive samples
    multiplied by the normalized PWM frequency and divided by the normalized 
    total distance in one electrical cycle. 

  Precondition:
    None.

  Parameters:
    None

  Returns:
    the filtered estimated speed

  Remarks:
    None.
 */
int16_t LX_EstimSpeed(void){
    instantTheta = theta.final;
    
    if(mcappData.runDirection == CW)
    {
        if(instantTheta>prevTheta)
        {
            LX_thetaDiff = instantTheta-prevTheta;
        }
        else
        {
            LX_thetaDiff = ((int16_t)32768+instantTheta) + ((int16_t)32767 - prevTheta);
        }

        movingAvgOmega.sum += LX_thetaDiff;

        if(movingAvgOmega.index < INDEX_VALUE)
        {
            movingAvgOmega.index++;
        }
        else
        {
           movingAvgOmega.avg =  movingAvgOmega.sum >> INDEX_SHIFTER;
           movingAvgOmega.sum = 0;
           movingAvgOmega.index = 0;
        }    

        LX_omega_temp = (int32_t)(__builtin_mulss(movingAvgOmega.avg,PWM_NORMALIZED));
        if(LX_omega_temp != 0){
            LX_omega = (int16_t)(__builtin_divud((unsigned long)LX_omega_temp,ONE_ROTATION_NORMALIZED));
        }

        LX_estimVelocity = (int16_t)(__builtin_mulss(LX_omega,(int16_t)60));


        movingAvgVelocity.sum += LX_estimVelocity;

        if(movingAvgVelocity.index < INDEX_VALUE)
        {
            movingAvgVelocity.index++;
        }
        else
        {
           movingAvgVelocity.avg =  movingAvgVelocity.sum >> INDEX_SHIFTER;
           movingAvgVelocity.sum = 0;
           movingAvgVelocity.index = 0;
        }
    }
    else
    {
        if(instantTheta<prevTheta)
        {
            LX_thetaDiff = instantTheta-prevTheta;
        }
        else if(prevTheta<instantTheta)
        {
            LX_thetaDiff = (instantTheta - (int16_t)32767) - ((int16_t)32768 + prevTheta);
        }

        movingAvgOmega.sum += LX_thetaDiff;

        if(movingAvgOmega.index < INDEX_VALUE)
        {
            movingAvgOmega.index++;
        }
        else
        {
           movingAvgOmega.avg =  movingAvgOmega.sum >> INDEX_SHIFTER;
           movingAvgOmega.sum = 0;
           movingAvgOmega.index = 0;
        }    

        LX_omega_temp = (int32_t)(__builtin_mulss(movingAvgOmega.avg,PWM_NORMALIZED));
        if(LX_omega_temp != 0){
            LX_omega = (int16_t)(__builtin_divsd((signed long)LX_omega_temp,ONE_ROTATION_NORMALIZED));
        }

        LX_estimVelocity = (int16_t)(__builtin_mulss(LX_omega,(int16_t)60));


        movingAvgVelocity.sum += LX_estimVelocity;

        if(movingAvgVelocity.index < INDEX_VALUE)
        {
            movingAvgVelocity.index++;
        }
        else
        {
           movingAvgVelocity.avg =  movingAvgVelocity.sum >> INDEX_SHIFTER;
           movingAvgVelocity.sum = 0;
           movingAvgVelocity.index = 0;
        }
    }
      
    LX_filtVelocity = movingAvgVelocity.avg;
    
    prevTheta = instantTheta;
    return((int16_t)LX_filtVelocity);
}

// *****************************************************************************
/* Function:
    MovingAvgInit()

  Summary:
    Function for initializing the moving average parameters

  Description:
    Initialize the moving average variables for omega and velocity.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void MovingAvgInit(void){
    movingAvgOmega.avg = 0;
    
    for(i=1; i<INDEX_VALUE;i++){
        movingAvgOmega.buffer[i] = 0;
    }
    
    movingAvgOmega.index = 0;
    movingAvgOmega.sum = 0;
    
    movingAvgVelocity.avg = 0;
    
    for(i=1; i<INDEX_VALUE;i++){
        movingAvgVelocity.buffer[i] = 0;
    }
    
    movingAvgVelocity.index = 0;
    movingAvgVelocity.sum = 0;
}
// *****************************************************************************
/* Function:
    LX_Params_Init()

  Summary:
    Function initializes LX parameters

  Description:
    Initialize LX parameters: sine, cosine, theta etc.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void LX_Params_Init(void)
{
    sine.final = 0;
    cosine.final = 0;
    theta.final = 0;
    
    instantTheta = 0;
    prevTheta = 0;
    LX_thetaDiff = 0;
    LX_omega = 0;
    LX_omega_temp = 0;
    LX_estimVelocity = 0;
    LX_filtVelocity = 0;
}
/*
 End of File
 */