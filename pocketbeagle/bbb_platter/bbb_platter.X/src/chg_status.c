/*
 * Charge Status Monitor
 */
#include "chg_status.h"
#include "system_config.h"


// API
void CHGS_Initialize()
{
    // Initialize our IO
    CHARGE_N_TRIS = 1;   // Active low charge sense input
    CHARGE_LED_PIN = 0;  // Active high charge indication output
    CHARGE_LED_TRIS = 0;
}


void CHGS_UpdateOutput()
{
    CHARGE_LED_PIN = ~CHARGE_N_PIN;
}


bool CHGS_ChargeStatus()
{
    return(CHARGE_N_PIN == 0);
}
