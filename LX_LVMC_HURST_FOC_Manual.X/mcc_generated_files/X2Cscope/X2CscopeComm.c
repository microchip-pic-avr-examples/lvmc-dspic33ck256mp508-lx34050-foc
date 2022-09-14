/* ************************************************************************** */
/** X2CComm.c

  @Company
    Microchip Technology

  @Summary
    Implements the X2C Lin protocol connection with MCC Peripheral drivers.

 */
/* ************************************************************************** */
#include <xc.h>
#include "X2CscopeComm.h"
#include "../uart1.h"

/** 
  @brief
    Puts the data to the hardware layer. (UART)
   @param[in] serial Serial interface object. (Not used)
   @param[in] data Data to send 
 */
void sendSerial(uint8_t data)
{

#if defined (__dsPIC33E__)
  U1TXREG = data;   
  
#elif defined (__dsPIC33C__)
  U1TXREG = data;   
  
#elif defined (__PIC32M__)
  U1TXREG = data; 
  
#else
  #error "Please check device family or implement own interface."
#endif

}

/** 
  @brief
   Get serial data from hardware. Reset the hardware in case of error. (UART2)
  @param[in] serial Serial interface object. (Not used)
  @return
    Return with the received data
 */
uint8_t receiveSerial()
{
#if defined (__dsPIC33E__)
    if (U1STA & 0x0E) {
        U1STAbits.OERR = 0; /* reset error */
        return (0);
    }
    return U1RXREG; 

#elif defined (__dsPIC33C__)
    if ((U1STAbits.OERR == 1))
    {
        U1STAbits.OERR = 0;
        return(0);
    }
    return U1RXREG;

#elif defined (__PIC32M__)
    if (U1STA & 0x0E) {
        U1STAbits.OERR = 0; /* reset error */
        return (0);
    }
    return U1RXREG;
    
#else
  #error "Please check device family or implement own interface."
#endif

}

/** 
  @brief  Check data availability (UART).
  @param[in] serial Serial interface object. (Not used)
  @return
    True -> Serial data ready to read.
    False -> No data.
 */
uint8_t isReceiveDataAvailable()
{
#if defined (__dsPIC33E__)
    return (U1STAbits.URXDA == 1);

#elif defined (__dsPIC33C__)
    return (U1STAHbits.URXBE == 0);
  
#elif defined (__PIC32M__)
    return (U1STAbits.URXDA == 1);
  
#else
  #error "Please check device family or implement own interface."
#endif


}

/** 
  @brief
   Check output buffer. (UART)
  @param[in] serial Serial interface object. (Not used)
  @return    
    True -> Transmit buffer is not full, at least one more character can be written.
    False -> Transmit buffer is full.
 */
uint8_t isSendReady()
{
#if defined (__dsPIC33E__)
    return (U1STAbits.UTXBF == 0); //TX Buffer full

#elif defined (__dsPIC33C__)
    return (U1STAHbits.UTXBF == 0);//TX Buffer full

#elif defined (__PIC32M__)
    return (U1STAbits.UTXBF == 0); //TX Buffer full
  
#else
  #error "Please check device family or implement own interface."
#endif

}
/* *****************************************************************************
 End of File
 */