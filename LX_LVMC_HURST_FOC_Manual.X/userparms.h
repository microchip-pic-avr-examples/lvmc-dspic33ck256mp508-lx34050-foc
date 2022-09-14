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

#ifndef USERPARAMS_H
#define USERPARAMS_H

#include <xc.h> 
#include <stdbool.h>

/*Uncomment these defines when setting the offsets for the first time of using
 * the LX sensor or every time the target wheel attachment is changed. Uncommenting 
 * these defines means that the open loop drive is being used for configuring the LX
 * parameters. After obtaining the sine and cosine offsets, comment out the
 * OPEN_LOOP_FUNCTIONING, build the code and load to the device. This time only
 * SETTING_OFFSET define is active. Manually set the theta.offset using the excel
 * sheet guide included in this code under docs folder.  After obtaining all 
 * the offsets(sin, cos and theta), comment out the defines to 
 * use the LX sensor for providing the feedback to the system.
 * 
*Functioning Mode. */
//#define OPEN_LOOP_FUNCTIONING
#define SETTING_OFFSET

/*Direction of rotation*/
#define CW      0
#define CCW     1

typedef signed int SFRAC16;

#define Q15(Float_Value)	\
        ((Float_Value < 0.0) ? (SFRAC16)(32768 * (Float_Value) - 0.5) \
        : (SFRAC16)(32767 * (Float_Value) + 0.5))

#define FOSC    200000000       
#define PWMFREQUENCY 10000      

#define N_P             5          // Electrical Pole-Pairs

#define DEADTIMESEC     0.0000005   // Dead time in seconds - 500ns
#define DFCY            FOSC/2      // Instruction cycle frequency (Hz)

#define DDEADTIME       (unsigned int)(DEADTIMESEC*DFCY)	// Dead time in dTcys
#define MIN_DUTY            0
#define LOOPINTCY       ((DFCY/PWMFREQUENCY)-1) // Basic loop period in units of Tcy

#define LOOPTIME_SEC            0.000100 //Specify PWM Period in seconds, (1/ PWMFREQUENCY_HZ) 

// Specify bootstrap charging time in Seconds (mention at least 10mSecs)
#define BOOTSTRAP_CHARGING_TIME_SECS 0.01
  
// Calculate Bootstrap charging time in number of PWM Half Cycles
#define BOOTSTRAP_CHARGING_COUNTS (uint16_t)((BOOTSTRAP_CHARGING_TIME_SECS/LOOPTIME_SEC )* 2)

#define MAX_SPEED           2900                    /*rpm*/
#define NOMINAL_SPEED_RPM   2000 


/* The following value is given in the xls attached file */
#define NORM_CURRENT_CONST     0.000671

/* initial offset added to estimated value, 
 when transitioning from open loop to closed loop 
 the value represents 22.5deg and should satisfy both 
 open loop and closed loop functioning 
 normally this value should not be modified, but in 
 case of fine tuning of the transition, depending on 
 the load or the rotor moment of inertia */
#define INITOFFSET_TRANS_OPEN_CLSD 0x1000

/* current transformation macro, used below */
#define NORM_CURRENT(current_real) (Q15(current_real/NORM_CURRENT_CONST/32768))

/* Open loop startup constants */
/* The following values depends on the PWM frequency,
 lock time is the time needed for motor's poles alignment 
before the open loop speed ramp up */
/* This number is: 10,000 is 1 second. */
#define LOCK_TIME 2000 
/* Open loop speed ramp up end value Value in RPM*/
#define END_SPEED_RPM 300 
/* Open loop acceleration */
#define OPENLOOP_RAMPSPEED_INCREASERATE 80  
/* Open loop q current setup - */
#define Q_CURRENT_REF_OPENLOOP NORM_CURRENT(1.0)

/* Specify Over Current Limit - DC BUS */
#define Q15_OVER_CURRENT_THRESHOLD NORM_CURRENT(3.0)

/* Maximum motor speed converted into electrical speed */
#define MAXIMUMSPEED_ELECTR MAX_SPEED*N_P
/* Nominal motor speed converted into electrical speed */
#define NOMINALSPEED_ELECTR NOMINAL_SPEED_RPM*N_P

/* End speed converted to fit the startup ramp */
#define END_SPEED (END_SPEED_RPM * N_P * LOOPTIME_SEC * 65536 / 60.0)*1024
/* End speed of open loop ramp up converted into electrical speed */
#define ENDSPEED_ELECTR END_SPEED_RPM*N_P
    
/* In case of the potentiometer speed reference, a reference ramp
is needed for assuring the motor can follow the reference imposed /
minimum value accepted */
#define SPEEDREFRAMP   Q15(0.00003)  
/* The Speed Control Loop Executes every  SPEEDREFRAMP_COUNT */
#define SPEEDREFRAMP_COUNT   3  
    
/* PI controllers tuning values - */     
/* D Control Loop Coefficients */
#define D_CURRCNTR_PTERM       Q15(0.05)
#define D_CURRCNTR_ITERM       Q15(0.003)
#define D_CURRCNTR_CTERM       Q15(0.999)
#define D_CURRCNTR_OUTMAX      0x7FFF

/* Q Control Loop Coefficients */
#define Q_CURRCNTR_PTERM       Q15(0.05)
#define Q_CURRCNTR_ITERM       Q15(0.003)
#define Q_CURRCNTR_CTERM       Q15(0.999)
#define Q_CURRCNTR_OUTMAX      0x7FFF

/* Velocity Control Loop Coefficients */
#define SPEEDCNTR_PTERM        Q15(0.05)
#define SPEEDCNTR_ITERM        Q15(0.001)
#define SPEEDCNTR_CTERM        Q15(0.999)
#define SPEEDCNTR_OUTMAX       0x5000

#define MOTOR_PHASE_A_DC        PG1DC
#define MOTOR_PHASE_B_DC        PG2DC
#define MOTOR_PHASE_C_DC        PG4DC

// *****************************************************************************
// Section: Enums, Structures
// *****************************************************************************
typedef struct {
    uint16_t changeDirection;
    uint16_t state;
    uint16_t runCmd;
    bool runDirection;
} MCAPP_DATA_T;

typedef enum {
    MCAPP_INIT = 0,             //Initialize Run time parameters
    MCAPP_CMD_WAIT = 1,         //Wait for Run command
    MCAPP_RUN = 2,              //Run the motor 
    MCAPP_CHANGE_DIRECTION = 3, //Change motor running direction
    MCAPP_STOP = 4,             //Stop the motor
} MCAPP_STATE_T;

extern  MCAPP_DATA_T mcappData;
/*For resetting all the parameters before running the motor.*/
extern void ResetParmeters(void);
    
#endif	/* USERPARAMS_H */

