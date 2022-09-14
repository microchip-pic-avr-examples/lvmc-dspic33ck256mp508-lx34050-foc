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
#include <stdbool.h>
#include <libq.h>
  
#include "motor_control_noinline.h"

#include "userparms.h"
#include "control.h"  

#include "mcc_generated_files/pwm.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/X2Cscope/X2Cscope.h"

#include "hal/board_service.h"
#include "hal/measure.h"
#include "lx_sensor.h"
#include "mcc_generated_files/pin_manager.h"


#define ADCBUF_INV_A_IPHASE1   (int16_t)(-ADCBUF0)
#define ADCBUF_INV_A_IPHASE2   (int16_t)(-ADCBUF1)

#define ADCBUF_SPEED_REF_A      ADCBUF11

volatile UGF_T uGF;

CTRL_PARM_T ctrlParm;
MOTOR_STARTUP_DATA_T motorStartUpData;
MCAPP_DATA_T mcappData;

volatile int16_t thetaElectrical = 0,thetaElectricalOpenLoop = 0;
uint16_t pwmPeriod;

MC_ALPHABETA_T valphabeta,ialphabeta;
MC_SINCOS_T sincosTheta;
MC_DQ_T vdq,idq;
MC_DUTYCYCLEOUT_T pwmDutycycle;
MC_ABC_T   vabc,iabc;

MC_PIPARMIN_T piInputIq;
MC_PIPARMOUT_T piOutputIq;
MC_PIPARMIN_T piInputId;
MC_PIPARMOUT_T piOutputId;
MC_PIPARMIN_T piInputOmega;
MC_PIPARMOUT_T piOutputOmega;

MCAPP_MEASURE_T measureInputs;

int16_t adcBuffer;
int16_t measuredSpeed;
int16_t measuredAngle;
int16_t offsetTransOpenToClose;

/** Definitions */
/** Open loop angle scaling Constant - This corresponds to 1024(2^10)
   Scaling down motorStartUpData.startupRamp to thetaElectricalOpenLoop   */
#define STARTUPRAMP_THETA_OPENLOOP_SCALER       10 
/* Fraction of dc link voltage(expressed as a squared amplitude) to set 
 * the limit for current controllers PI Output */
#define MAX_VOLTAGE_VECTOR                      0.98

void InitControlParameters(void);
void DoControl(void);
void CalculateParkAngle(void);
void ResetParmeters(void);
void MCAPP_StateMachine(void);

// *****************************************************************************
/* Function:
    ResetParmeters()

  Summary:
    This routine resets all the parameters required for Motor through Inv-A

  Description:
    Reinitializes the duty cycle,resets all the counters when restarting motor

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void ResetParmeters(void)
{
    /* Make sure ADC does not generate interrupt while initializing parameters*/
	ADC1_IndividualChannelInterruptDisable(CH_AN1_IB);

    /* Re initialize the duty cycle to minimum value */
    PWM_DutyCycleSet(PWM_GENERATOR_1,MIN_DUTY);
    PWM_DutyCycleSet(PWM_GENERATOR_2,MIN_DUTY);
    PWM_DutyCycleSet(PWM_GENERATOR_4,MIN_DUTY);
    
    DisablePWMOutputsInverterA();
         
    /* Set the reference speed value to 0 */
    ctrlParm.qVelRef = 0;
    /* Restart in open loop */
    uGF.bits.OpenLoop = 1;
    /* Change mode */
    uGF.bits.ChangeMode = 1;
    
    if(mcappData.runDirection == CW){
        offsetTransOpenToClose = INITOFFSET_TRANS_OPEN_CLSD;
    }
    else
    {
        offsetTransOpenToClose = -INITOFFSET_TRANS_OPEN_CLSD;
    }
    
    /* Initialize PI control parameters */
    InitControlParameters();        

    /* Initialize LX parameters */
    LX_Params_Init();

    /* Initialize measurement parameters */
    MCAPP_MeasureCurrentInit(&measureInputs);
    /* Initialize moving average parameters */
    MovingAvgInit();

    /* Enable ADC interrupt */
    ADC1_IndividualChannelInterruptFlagClear(CH_AN1_IB);
    ADC1_IndividualChannelInterruptEnable(CH_AN1_IB);
}

// *****************************************************************************
/* Function:
    DoControl()

  Summary:
    Executes one PI iteration for each of the three loops Id,Iq,Speed

  Description:
    This routine executes one PI iteration for each of the three loops
    Id,Iq,Speed

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void DoControl( void )
{
    /* Temporary variables for sqrt calculation of q reference */
    volatile int16_t temp_qref_pow_q15;
    
    if(uGF.bits.OpenLoop)
    {
        /* OPENLOOP:  force rotating angle,Vd and Vq */
        if(uGF.bits.ChangeMode)
        {
            /* Just changed to open loop */
            uGF.bits.ChangeMode = 0;

            /* Synchronize angles */
            /* VqRef & VdRef not used */
            ctrlParm.qVqRef = 0;
            ctrlParm.qVdRef = 0;

            /* Reinitialize variables for initial speed ramp */
            motorStartUpData.startupLock = 0;
            motorStartUpData.startupRamp = 0;
        }

        /* PI control for D */
        piInputId.inMeasure = idq.d;
        piInputId.inReference  = ctrlParm.qVdRef;
        MC_ControllerPIUpdate_Assembly(piInputId.inReference,
                                       piInputId.inMeasure,
                                       &piInputId.piState,
                                       &piOutputId.out);
        vdq.d = piOutputId.out;
         /* Dynamic d-q adjustment
         with d component priority 
         vq=sqrt (vs^2 - vd^2) 
        limit vq maximum to the one resulting from the calculation above */
        temp_qref_pow_q15 = (int16_t)(__builtin_mulss(piOutputId.out ,
                                                      piOutputId.out) >> 15);
        temp_qref_pow_q15 = Q15(MAX_VOLTAGE_VECTOR) - temp_qref_pow_q15;
        piInputIq.piState.outMax = _Q15sqrt (temp_qref_pow_q15);
        piInputIq.piState.outMin = - piInputIq.piState.outMax;    
        
        /* PI control for Q */
        /* Speed reference */
        if(mcappData.runDirection == CW)
        {
            ctrlParm.qVelRef = Q_CURRENT_REF_OPENLOOP;
        }
        else
        {
            ctrlParm.qVelRef = -Q_CURRENT_REF_OPENLOOP;                           
        }
            /* q current reference is equal to the velocity reference 
         while d current reference is equal to 0
        for maximum startup torque, set the q current to maximum acceptable 
        value represents the maximum peak value */
        ctrlParm.qVqRef = ctrlParm.qVelRef;
        piInputIq.inMeasure = idq.q;
        piInputIq.inReference = ctrlParm.qVqRef;
        MC_ControllerPIUpdate_Assembly(piInputIq.inReference,
                                       piInputIq.inMeasure,
                                       &piInputIq.piState,
                                       &piOutputIq.out);
        vdq.q = piOutputIq.out;
        
    }
    else
    /* Closed Loop Vector Control */
    {                
        if(mcappData.runDirection == CW)
        {
            ctrlParm.targetSpeed = (__builtin_mulss(measureInputs.potValue,
                    MAXIMUMSPEED_ELECTR-ENDSPEED_ELECTR)>>15)+
                    ENDSPEED_ELECTR; 

            if  (ctrlParm.speedRampCount < SPEEDREFRAMP_COUNT)
            {
                ctrlParm.speedRampCount++; 
            }
            else
            {
                /* Ramp generator to limit the change of the speed reference
                  the rate of change is defined by CtrlParm.qRefRamp */            
                ctrlParm.qDiff = ctrlParm.qVelRef - ctrlParm.targetSpeed;

                /* Speed Ref Ramp */
                if (ctrlParm.qDiff < 0)
                {
                    /* Set this cycle reference as the sum of
                    previously calculated one plus the reference ramp value */
                    ctrlParm.qVelRef = ctrlParm.qVelRef+ctrlParm.qRefRamp;
                }
                else
                {
                    /* Same as above for speed decrease */
                    ctrlParm.qVelRef = ctrlParm.qVelRef-ctrlParm.qRefRamp;
                }
                /* If difference less than half of ref ramp, set reference
                directly from the pot */
                if (_Q15abs(ctrlParm.qDiff) < (ctrlParm.qRefRamp << 1))
                {
                    ctrlParm.qVelRef = ctrlParm.targetSpeed;
                }
                ctrlParm.speedRampCount = 0;
            }  
        }
        else
        {          
            ctrlParm.targetSpeed = -(__builtin_mulss(measureInputs.potValue,  
                    MAXIMUMSPEED_ELECTR-ENDSPEED_ELECTR)>>15) -
                    ENDSPEED_ELECTR; 

            if(ctrlParm.speedRampCount < SPEEDREFRAMP_COUNT)
            {
                ctrlParm.speedRampCount++; 
            }
            else
            {
                ctrlParm.qDiff = ctrlParm.qVelRef - ctrlParm.targetSpeed;          

                if (ctrlParm.qDiff > 0)                                           
                {
                    /* Set this cycle reference as the sum of
                    previously calculated one plus the reference ramp value */
                    ctrlParm.qVelRef = ctrlParm.qVelRef-ctrlParm.qRefRamp;
                }
                else
                {
                    /* Same as above for speed decrease */
                    ctrlParm.qVelRef = ctrlParm.qVelRef+ctrlParm.qRefRamp;
                }
                /* If difference less than half of ref ramp, set reference
                directly from the pot */
                if (_Q15abs(ctrlParm.qDiff) < (ctrlParm.qRefRamp << 1))
                {
                    ctrlParm.qVelRef = ctrlParm.targetSpeed;
                } 
                ctrlParm.speedRampCount = 0;
            }
        } 
    }


        if (uGF.bits.ChangeMode)
        {
            /* Just changed from open loop */
            uGF.bits.ChangeMode = 0;
            piInputOmega.piState.integrator = (int32_t)ctrlParm.qVqRef << 13;
            
            if(mcappData.runDirection == CW)
            {
                ctrlParm.qVelRef = ENDSPEED_ELECTR;
            }
            else
            {
                ctrlParm.qVelRef = -ENDSPEED_ELECTR;                              
            }
        }
              
        /* Execute the velocity control loop */
        piInputOmega.inMeasure = measuredSpeed;
       
        piInputOmega.inReference = ctrlParm.qVelRef;
        MC_ControllerPIUpdate_Assembly(piInputOmega.inReference,
                                      piInputOmega.inMeasure,
                                      &piInputOmega.piState,
                                      &piOutputOmega.out);
        ctrlParm.qVqRef = piOutputOmega.out;                 
    
        ctrlParm.qVdRef = 0;
        /* PI control for D */
        piInputId.inMeasure = idq.d;
        piInputId.inReference  = ctrlParm.qVdRef;
        MC_ControllerPIUpdate_Assembly(piInputId.inReference,
                                       piInputId.inMeasure,
                                       &piInputId.piState,
                                       &piOutputId.out);
        vdq.d    = piOutputId.out;

        /* Dynamic d-q adjustment
         with d component priority 
         vq=sqrt (vs^2 - vd^2) 
        limit vq maximum to the one resulting from the calculation above */
        temp_qref_pow_q15 = (int16_t)(__builtin_mulss(piOutputId.out ,
                                                      piOutputId.out) >> 15);
        temp_qref_pow_q15 = Q15(MAX_VOLTAGE_VECTOR) - temp_qref_pow_q15;
        piInputIq.piState.outMax = _Q15sqrt (temp_qref_pow_q15);
        piInputIq.piState.outMin = - piInputIq.piState.outMax;
        /* PI control for Q */
        piInputIq.inMeasure  = idq.q;
        piInputIq.inReference  = ctrlParm.qVqRef;
        MC_ControllerPIUpdate_Assembly(piInputIq.inReference,
                                       piInputIq.inMeasure,
                                       &piInputIq.piState,
                                       &piOutputIq.out);
        vdq.q = piOutputIq.out;
}
      
// *****************************************************************************
/* Function:
   _ADCAN1Interrupt()

  Summary:
   _ADCAN1Interrupt() ISR routine

  Description:
    Does speed calculation and executes the vector update loop
    The ADC sample and conversion is triggered by the PWM period.
    The speed calculation assumes a fixed time interval between calculations.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void __attribute__((__interrupt__,no_auto_psv)) _ADCAN1Interrupt()
{
    measureInputs.current.Ia = ADCBUF_INV_A_IPHASE1;
    measureInputs.current.Ib = ADCBUF_INV_A_IPHASE2; 
    
    measureInputs.potValue = (int16_t)(ADCBUF_SPEED_REF_A>>1);

    measuredAngle = (int16_t)LX_EstimAngle();
    measuredSpeed = (int16_t)LX_EstimSpeed();
        
    MCAPP_StateMachine();
    
    BoardServiceStepIsr(); 
    X2CScope_Update();
    adcBuffer = ADCBUF_INV_A_IPHASE2;
    ADC1_IndividualChannelInterruptFlagClear(CH_AN1_IB);   
}

// *****************************************************************************
/* Function:
    CalculateParkAngle ()

  Summary:
    Function calculates the angle for open loop control

  Description:
    Generate the start sine waves feeding the motor terminals
    Open loop control, forcing the motor to align and to start speeding up.
 
  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void CalculateParkAngle(void)
{
    //For CCW
    int16_t lockTime = -LOCK_TIME;
    int32_t endSpeed = -END_SPEED;
    int16_t rampIncRate = OPENLOOP_RAMPSPEED_INCREASERATE;
    
    
    if(mcappData.runDirection == CW)
    {
        /* if open loop */
        if (uGF.bits.OpenLoop)
        {
            /* begin with the lock sequence, for field alignment */
            if (motorStartUpData.startupLock < LOCK_TIME)
            {
                motorStartUpData.startupLock += 1;
            }
            /* Then ramp up till the end speed */
            else if (motorStartUpData.startupRamp < END_SPEED)
            {
                motorStartUpData.startupRamp += OPENLOOP_RAMPSPEED_INCREASERATE;
            }
            /* Switch to closed loop */
            else 
            {
                #ifndef OPEN_LOOP_FUNCTIONING
                    uGF.bits.ChangeMode = 1;
                    uGF.bits.OpenLoop = 0;
                #endif
            }
            /* The angle set depends on startup ramp */
            thetaElectricalOpenLoop += (int16_t)(motorStartUpData.startupRamp >> 
                                                STARTUPRAMP_THETA_OPENLOOP_SCALER);
        }
        /* Switched to closed loop */
        else 
        {
            /* In closed loop slowly decrease the offset add to the estimated angle */
            if (offsetTransOpenToClose > 0)
            {
                offsetTransOpenToClose--;
            }
        }
    }
    else
    {
       if (uGF.bits.OpenLoop)
        {
            /* begin with the lock sequence, for field alignment */
            if (motorStartUpData.startupLock > lockTime)
            {
                motorStartUpData.startupLock -= 1;
            }
            /* Then ramp up till the end speed */
            else if (motorStartUpData.startupRamp > endSpeed)
            {
                motorStartUpData.startupRamp -= rampIncRate;
            }
            /* Switch to closed loop */
            else 
            {
                #ifndef OPEN_LOOP_FUNCTIONING
                    uGF.bits.ChangeMode = 1;
                    uGF.bits.OpenLoop = 0;
                #endif
            }
            
            /* The angle set depends on startup ramp */
            thetaElectricalOpenLoop -= (int16_t)(motorStartUpData.startupRamp >>    
                                                STARTUPRAMP_THETA_OPENLOOP_SCALER);
        }
        /* Switched to closed loop */
        else 
        {
            /* In closed loop slowly decrease the offset add to the estimated angle */
            if (offsetTransOpenToClose < 0)
            {
                offsetTransOpenToClose++;
            }
        }
    }        
}
/******************************************************************************
 * Description: The MCAPP_StateMachine describes the code flow and controls the
 *              BLDC motor based on its state. Thus enabling smooth control of motor
 *              and is easily understood by the user.
 *****************************************************************************/
void MCAPP_StateMachine(void)
{
     switch (mcappData.state){
         
        case MCAPP_INIT:
            ResetParmeters();

            if (MCAPP_MeasureCurrentOffsetStatus(&measureInputs) == 0)
            {
                MCAPP_MeasureCurrentOffset(&measureInputs);
            }
            
            mcappData.state = MCAPP_CMD_WAIT;
            break;

        case MCAPP_CMD_WAIT:

            if (mcappData.runCmd == 1) {
                EnablePWMOutputsInverterA();
                mcappData.state = MCAPP_RUN;
            }

            if (mcappData.changeDirection == 1) {
                mcappData.state = MCAPP_CHANGE_DIRECTION;
            }
            break;

        case MCAPP_RUN:
            MCAPP_MeasureCurrentCalibrate(&measureInputs);
            iabc.a = measureInputs.current.Ia;
            iabc.b = measureInputs.current.Ib;

            /* Calculate qId,qIq from qSin,qCos,qIa,qIb */
            MC_TransformClarke_Assembly(&iabc,&ialphabeta);
            MC_TransformPark_Assembly(&ialphabeta,&sincosTheta,&idq);
            
            /* Calculate control values */
            DoControl();
            /* Calculate qAngle */
            CalculateParkAngle();
            /* if open loop */
            if (uGF.bits.OpenLoop == 1)
            {
                /* the angle is given by park parameter */               
                if(mcappData.runDirection == CW)
                {
                    thetaElectrical = thetaElectricalOpenLoop;
                }
                else
                {
                   thetaElectrical = -thetaElectricalOpenLoop; 
                }
            }
            else
            {
                /* if closed loop, angle generated by resolver */
                thetaElectrical = measuredAngle + offsetTransOpenToClose;
                         
            }
            
            MC_CalculateSineCosine_Assembly_Ram(thetaElectrical,&sincosTheta);
            MC_TransformParkInverse_Assembly(&vdq,&sincosTheta,&valphabeta);

            MC_TransformClarkeInverseSwappedInput_Assembly(&valphabeta,&vabc);
                
            MC_CalculateSpaceVectorPhaseShifted_Assembly(&vabc,pwmPeriod,
                                                        &pwmDutycycle);
            PWMDutyCycleSet(&pwmDutycycle);
            
            
            if (mcappData.runCmd == 0) {
                mcappData.state = MCAPP_STOP;
            }
            break;

        case MCAPP_CHANGE_DIRECTION:

            if (mcappData.changeDirection == 1) {
                if (mcappData.runDirection == CW)
                {
                    mcappData.runDirection = CCW;
                    IO_LD11_SetLow();
                } 
                else
                {
                    mcappData.runDirection = CW;
                    IO_LD11_SetHigh();
                }
                mcappData.changeDirection = 0;
            }

            mcappData.state = MCAPP_CMD_WAIT;
            
            break;

        case MCAPP_STOP:

            pwmDutycycle.dutycycle3 = MIN_DUTY;
            pwmDutycycle.dutycycle2 = MIN_DUTY;
            pwmDutycycle.dutycycle1 = MIN_DUTY;

            PWMDutyCycleSet(&pwmDutycycle);
            
            mcappData.state = MCAPP_INIT;
            
            break;
    }
}
// *****************************************************************************
/* Function:
    InitControlParameters()

  Summary:
    Function initializes control parameters

  Description:
    Initialize control parameters: PI coefficients, scaling constants etc.

  Precondition:
    None.

  Parameters:
    None

  Returns:
    None.

  Remarks:
    None.
 */
void InitControlParameters(void)
{ 
    ctrlParm.qRefRamp = SPEEDREFRAMP;
    ctrlParm.speedRampCount = SPEEDREFRAMP_COUNT;
    /* Set PWM period to Loop Time */
    pwmPeriod = LOOPINTCY;
 
    /* PI - Id Current Control */
    piInputId.piState.kp = D_CURRCNTR_PTERM;
    piInputId.piState.ki = D_CURRCNTR_ITERM;
    piInputId.piState.kc = D_CURRCNTR_CTERM;
    piInputId.piState.outMax = D_CURRCNTR_OUTMAX;
    piInputId.piState.outMin = -piInputId.piState.outMax;
    piInputId.piState.integrator = 0;
    piOutputId.out = 0;

    /* PI - Iq Current Control */
    piInputIq.piState.kp = Q_CURRCNTR_PTERM;
    piInputIq.piState.ki = Q_CURRCNTR_ITERM;
    piInputIq.piState.kc = Q_CURRCNTR_CTERM;
    piInputIq.piState.outMax = Q_CURRCNTR_OUTMAX;
    piInputIq.piState.outMin = -piInputIq.piState.outMax;
    piInputIq.piState.integrator = 0;
    piOutputIq.out = 0;

    /* PI - Speed Control */
    piInputOmega.piState.kp = SPEEDCNTR_PTERM;
    piInputOmega.piState.ki = SPEEDCNTR_ITERM;
    piInputOmega.piState.kc = SPEEDCNTR_CTERM;
    piInputOmega.piState.outMax = SPEEDCNTR_OUTMAX;
    piInputOmega.piState.outMin = -piInputOmega.piState.outMax;
    piInputOmega.piState.integrator = 0;
    piOutputOmega.out = 0;
}
/*
 End of file
 */