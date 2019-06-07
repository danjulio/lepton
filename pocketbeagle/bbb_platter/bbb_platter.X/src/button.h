/*
 * Button Module header
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

// Externs
extern volatile bit ButtonIocDetected;
extern bit ButtonDown;
extern uint16_t ButtonDownThreshold;


// API
void BUTTON_SetupWake();

void BUTTON_SetupSleep();

void BUTTON_Tasks();

bool BUTTON_PressedDetected();

bool BUTTON_Released();

// API Macros
#define BUTTON_SetIocDetected() (ButtonIocDetected = 1)
#define BUTTON_Down() (ButtonDown == 1)
#define BUTTON_SetPressEvalCount(c) (ButtonDownThreshold = c)


#endif // BUTTON_H
