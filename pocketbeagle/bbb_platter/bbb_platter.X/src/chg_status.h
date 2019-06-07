/*
 * Charge status monitor header
 */
#ifndef CHG_STATUS_H
#define CHG_STATUS_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>


// API
void CHGS_Initialize();
void CHGS_UpdateOutput();
bool CHGS_ChargeStatus();


#endif // CHG_STATUS_H
