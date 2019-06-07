/*
 * Button Module
 */
#include "button.h"
#include "system_config.h"


// Button state
volatile bit ButtonIocDetected;
bit ButtonDown;
bit ButtonPrev;
bit ButtonPressedDetect;
bit ButtonReleasedDetect;
uint16_t ButtonDownCount;
uint16_t ButtonDownThreshold;


// API routines
void BUTTON_SetupWake()
{
    // Setup our state based on our IOC status upon wakeup
    ButtonDownCount = 0;
    if (ButtonIocDetected == 1) {
        // Button activity triggered the wakeup - start off assuming the button is
        // pressed
        ButtonDown = 1;
        ButtonPrev = 1;
    } else {
        ButtonDown = 0;
        ButtonPrev = 0;
    }
    // Disable interrupt-on-change for our input (all inputs)
    INTCONbits.IOCIE = 0;
    //BUTTON_IOCP = 0;
    //BUTTON_IOCN = 0;
    IOCAP = 0;
    IOCAN = 0;
    IOCBP = 0;
    IOCBN = 0;
}


void BUTTON_SetupSleep()
{
    // Enable interrupt-on-change for our input
    ButtonDownCount = 0;
    ButtonIocDetected = 0;
    //BUTTON_IOCF = 0;
    IOCAF = 0;
    IOCBF = 0;
    INTCONbits.IOCIE = 1;
    BUTTON_IOCN = 1; // Negative edge transitions can interrupt
}


void BUTTON_Tasks()
{
    static bit curButton;
    static bit keyPressed;
    static bit keyReleased;

    // Flag bits will be set as necessary
    ButtonPressedDetect = 0;
    ButtonReleasedDetect = 0;

    // Sample button value (active-low)
    curButton = ~PORTAbits.RA3;

    // Compute current state
    keyPressed = curButton & ButtonPrev & ~ButtonDown;
    keyReleased = ButtonDown & ~curButton & ~ButtonPrev;
    if ((curButton & ButtonPrev) == 1) {
        ButtonDown = 1;
    }
    if ((curButton | ButtonPrev) == 0) {
        ButtonDown = 0;
    }
    ButtonPrev = curButton;

    if (keyPressed == 1) {
        ButtonDownCount = 0;
    } else if (keyReleased == 1) {
        ButtonReleasedDetect = 1;
        ButtonDownCount = 0;
    } else if (ButtonDown == 1) {
        if (ButtonDownCount < ButtonDownThreshold) {
            if (++ButtonDownCount == ButtonDownThreshold) {
                ButtonPressedDetect = 1;
            }
        }
    }
}


bool BUTTON_PressedDetected()
{
    return (ButtonPressedDetect == 1);
}


bool BUTTON_Released() {
    return (ButtonReleasedDetect == 1);
}
