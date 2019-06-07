/*
 * Analog Input Module
 */
#include "system.h"
#include "system_config.h"
#include "adc.h"
#include "eeprom.h"



// Variables
uint8_t ADC_A1ref;


// Internal function forward references
void ADC_SetRef(uint8_t r);


void ADC_Initialize()
{
    // Configure our IO as analog input
    AN1_TRIS = 1;
    AN1_ANSEL = 1;
    BATT_TRIS = 1;
    BATT_ANSEL = 1;
}


void ADC_SetupWake()
{
    // Enable the FVR
    FVRCONbits.FVREN = 1;

    // Configure the ADC
    ADCON0bits.ADON = 1;
    ADCON0bits.CHS = AN1_CH;  // default
    ADCON1bits.ADFM = 1;  // right justified: 8 LSB in ADRESL
    ADCON1bits.ADCS = 0x03;  // Internal RC Oscillator (1-6 uSec/bit)
    ADCON1bits.ADPREF = 0x00;  // Vref+ = VDD
    ADCON2 = 0x00;  // no auto-conversion

    // Initialize variables
    ADC_A1ref = EEP_GetArefCfg() & ADC_AIN_REF_MASK;
}


void ADC_SetupSleep()
{
    // Disable the FVR and ADC
    FVRCONbits.FVREN = 0;
    ADCON0bits.ADON = 0;
}


uint16_t ADC_Read(uint8_t ch)
{
    switch (ch) {
        case 0:     // Direct battery voltage
            ADC_SetRef(ADC_REF_1);
            ADCON0bits.CHS = BATT_CH;
            break;
        case 1:     // Channel 1
            ADC_SetRef(ADC_A1ref);
            ADCON0bits.CHS = AN1_CH;
            break;
        default:    // Indirect battery voltage - measure our FVR against VDD
            ADC_SetRef(ADC_REF_VDD4FVR);  // VREF connected to VDD
            ADCON0bits.CHS = 0x1F;        // FVR Input to ADC
            break;
    }

    // Short delay to allow the input capacitor to charge
    // 10 uSec @ 83.3 nSec/cycle = 120 cycles
    _delay(120);

    // Read the value
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO_nDONE);     // Wait for conversion
    return((ADRESH << 8) | ADRESL);
}


void ADC_SetRefType(uint8_t r)
{
    ADC_A1ref = r & 0x03;
}


uint8_t ADC_GetRefType() {
    return ADC_A1ref;    
}



// Internal Functions
void ADC_SetRef(uint8_t r)
{
    uint8_t FVRCON_OrVal;
    uint8_t ADCON1_OrVal;

    switch (r) {
        case ADC_REF_VDD:
            FVRCON_OrVal = 0x00;   // ADC FVR Off
            ADCON1_OrVal = 0x00;   // ADC VREF = VDD
            break;
        case ADC_REF_1:
            FVRCON_OrVal = 0x01;   // ADC FVR = 1.024v
            ADCON1_OrVal = 0x03;   // ADC VREF = FVR
            break;
        case ADC_REF_2:
            FVRCON_OrVal = 0x02;   // ADC FVR = 2.048v
            ADCON1_OrVal = 0x03;   // ADC VREF = FVR
            break;
        case ADC_REF_3:
            FVRCON_OrVal = 0x03;   // ADC FVR = 4.096v
            ADCON1_OrVal = 0x03;   // ADC VREF = FVR
            break;
        case ADC_REF_VDD4FVR:
            FVRCON_OrVal = 0x01;   // ADC FVR = 1.024v
            ADCON1_OrVal = 0x00;   // ADC VREF = VDD
            break;
    }
    FVRCON = (FVRCON & 0xFC) | FVRCON_OrVal;
    ADCON1 = (ADCON1 & 0xFC) | ADCON1_OrVal;
}
