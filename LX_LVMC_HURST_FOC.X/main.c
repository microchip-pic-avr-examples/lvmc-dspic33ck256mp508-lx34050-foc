/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CK256MP508
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB 	          :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <xc.h>
#include <stdbool.h>
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/X2Cscope/X2Cscope.h"
#include "mcc_generated_files/pwm.h"
#include "hal/board_service.h"
#include "control.h"
#include "userparms.h"
#include "lx_sensor.h"

void ChargeBootstrapCapacitors(void);
void InitDutyPWM124Generators(void);
/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    
    InitDutyPWM124Generators();
    
    PG2CONLbits.ON = 1;      // Enable PWM module after initializing generators
    PG4CONLbits.ON = 1;      // Enable PWM module after initializing generators
    PG1CONLbits.ON = 1;      // Enable PWM module after initializing generators
    ChargeBootstrapCapacitors();
    
    BoardServiceInit();
    CORCONbits.SATA = 0;

    ResetParmeters();
    mcappData.runDirection = CW;
    IO_LD11_SetHigh();
    
    while (1)
    {
        // Add your application code
        X2CScope_Communicate();
        BoardService();
        
        if (IsPressed_Button1())
            {
                if(mcappData.runCmd == 0)
                {
                    mcappData.runCmd = 1;
                    IO_LD10_SetHigh();
                    mcappData.state = MCAPP_INIT;
                }
                else
                {
                    mcappData.runCmd = 0;
                    IO_LD10_SetLow();
                }
 
            }
        // Monitoring for Button 2 press in LVMC
        if (IsPressed_Button2() && mcappData.runCmd == 0)
        {
            mcappData.changeDirection = 1;
        }
    }
    return 1; 
}

void ChargeBootstrapCapacitors(void){
    uint16_t i = BOOTSTRAP_CHARGING_COUNTS;
    uint16_t prevStatusCAHALF = 0,currStatusCAHALF = 0;
    uint16_t k = 0;
    
    // Enable PWMs only on PWMxL ,to charge bootstrap capacitors at the beginning
    // Hence PWMxH is over-ridden to "LOW"
    PG4IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM4H,L, if Override is Enabled
    PG2IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM2H,L, if Override is Enabled
    PG1IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM1H,L, if Override is Enabled

    PG4IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM4H
    PG2IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM2H
    PG1IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM1H

    PG4IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM4L
    PG2IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM2L
    PG1IOCONLbits.OVRENL = 1;  // 1 = OVRDAT<0> provides data for output on PWM1L

    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER
    // Initialize the PWM duty cycle for charging
    MOTOR_PHASE_C_DC = LOOPINTCY - (DDEADTIME/2 + 5);
    MOTOR_PHASE_B_DC = LOOPINTCY - (DDEADTIME/2 + 5);
    MOTOR_PHASE_A_DC = LOOPINTCY - (DDEADTIME/2 + 5);
    
    while(i)
    {
        prevStatusCAHALF = currStatusCAHALF;
        currStatusCAHALF = PG1STATbits.CAHALF;
        if (prevStatusCAHALF != currStatusCAHALF)
        {
            if (currStatusCAHALF == 0)
            {
                i--; 
                k++;
                if (i == (BOOTSTRAP_CHARGING_COUNTS - 50))
                {
                    // 0 = PWM generator provides data for PWM1L pin
                    PG1IOCONLbits.OVRENL = 0;
                }
                else if (i == (BOOTSTRAP_CHARGING_COUNTS - 150))
                {
                    // 0 = PWM generator provides data for PWM2L pin
                    PG2IOCONLbits.OVRENL = 0;  
                }
                else if (i == (BOOTSTRAP_CHARGING_COUNTS - 250))
                {
                    // 0 = PWM generator provides data for PWM4L pin
                    PG4IOCONLbits.OVRENL = 0;  
                }
                if (k > 25)
                {
                    if (PG4IOCONLbits.OVRENL == 0)
                    {
                        if (MOTOR_PHASE_C_DC > 2)
                        {
                            MOTOR_PHASE_C_DC -= 2;
                        }
                        else
                        {
                           MOTOR_PHASE_C_DC = 0; 
                        }
                    }
                    if (PG2IOCONLbits.OVRENL == 0)
                    {
                        if (MOTOR_PHASE_B_DC > 2)
                        {
                            MOTOR_PHASE_B_DC -= 2;
                        }
                        else
                        {
                            MOTOR_PHASE_B_DC = 0; 
                        }
                    }
                    if (PG1IOCONLbits.OVRENL == 0)
                    {
                        if (MOTOR_PHASE_A_DC > 2)
                        {
                            MOTOR_PHASE_A_DC -= 2;
                        }
                        else
                        {
                            MOTOR_PHASE_A_DC = 0; 
                        }
                    }
                    k = 0;
                } 
            }
        }
    }
    // PDCx: PWMx GENERATOR DUTY CYCLE REGISTER
    // Initialize the PWM duty cycle for charging
    MOTOR_PHASE_C_DC = 0;
    MOTOR_PHASE_B_DC = 0;
    MOTOR_PHASE_A_DC = 0;

    PG4IOCONLbits.OVRENH = 0;  // 0 = PWM generator provides data for PWM4H pin
    PG2IOCONLbits.OVRENH = 0;  // 0 = PWM generator provides data for PWM2H pin
    PG1IOCONLbits.OVRENH = 0;  // 0 = PWM generator provides data for PWM1H pin
    
}

void InitDutyPWM124Generators(void)
{

// Enable PWMs only on PWMxL ,to charge bootstrap capacitors initially.
    // Hence PWMxH is over-ridden to "LOW"
   
    PG4IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM4H,L, if Override is Enabled
    PG2IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM2H,L, if Override is Enabled
    PG1IOCONLbits.OVRDAT = 0;  // 0b00 = State for PWM1H,L, if Override is Enabled

    PG4IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM4H
    PG2IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM2H
    PG1IOCONLbits.OVRENH = 1;  // 1 = OVRDAT<1> provides data for output on PWM1H

    PG4IOCONLbits.OVRENL = 0;  // 0 = PWM generator provides data for PWM4L pin
    PG2IOCONLbits.OVRENL = 0;  // 0 = PWM generator provides data for PWM2L pin
    PG1IOCONLbits.OVRENL = 0;  // 0 = PWM generator provides data for PWM1L pin

    /* Set PWM Duty Cycles */
    PG4DC = 0;
    PG2DC = 0;      
    PG1DC = 0;

}
/**
 End of File
*/

