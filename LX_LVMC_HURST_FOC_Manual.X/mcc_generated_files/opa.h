
/**
  OPA Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    opa.h

  @Summary
    This is the generated header file for the OPA driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for OPA.
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

#ifndef _OPA_H
#define _OPA_H

/**
  Section: Included Files
*/

#include <xc.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: OPA APIs
*/


/**
  @Function
    OPA_Initialize

  @Description
    This routine configures the OPA
    specific control registers
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA_Initialize();
    </code>
*/

void OPA_Initialize (void);

/**
  @Function
    OPA_Enable

  @Description
    Enables OPAMP modules if their respective enable bits are also asserted    
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA_Enable();
    </code>
*/

inline static void OPA_Enable(void)
{
    AMPCON1Lbits.AMPON = 1; //Enable OPA;
}

/**
  @Function
    OPA_Disable

  @Description
    Disables all OPAMP modules 
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA_Disable();
    </code>
*/

inline static void OPA_Disable(void)
{
    AMPCON1Lbits.AMPON = 0; //Disable OPA;
}


/**
  @Function
    OPA1_Enable

  @Description
    Enables OPAMP 1 if the AMPON bit is also asserted
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA1_Enable();
    </code>
*/

inline static void OPA1_Enable(void)
{
    AMPCON1Lbits.AMPEN1 = 1; //Enable OPA1;
}

/**
  @Function
    OPA1_Disable

  @Description
    Disables OPAMP 1
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA1_Disable();
    </code>
*/

inline static void OPA1_Disable(void)
{
    AMPCON1Lbits.AMPEN1 = 0; //Disable OPA1;
}


/**
  @Function
    OPA2_Enable

  @Description
    Enables OPAMP 2 if the AMPON bit is also asserted
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA2_Enable();
    </code>
*/

inline static void OPA2_Enable(void)
{
    AMPCON1Lbits.AMPEN2 = 1; //Enable OPA2;
}

/**
  @Function
    OPA2_Disable

  @Description
    Disables OPAMP 2
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA2_Disable();
    </code>
*/

inline static void OPA2_Disable(void)
{
    AMPCON1Lbits.AMPEN2 = 0; //Disable OPA2;
}


/**
  @Function
    OPA3_Enable

  @Description
    Enables OPAMP 3 if the AMPON bit is also asserted
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA3_Enable();
    </code>
*/

inline static void OPA3_Enable(void)
{
    AMPCON1Lbits.AMPEN3 = 1; //Enable OPA3;
}

/**
  @Function
    OPA3_Disable

  @Description
    Disables OPAMP 3
	
  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Example
    <code>
    OPA3_Disable();
    </code>
*/

inline static void OPA3_Disable(void)
{
    AMPCON1Lbits.AMPEN3 = 0; //Disable OPA3;
}


#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif //_OPA_H
    
/**
 End of File
*/
