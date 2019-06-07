/*
 * Analog Input Module header
 */
#ifndef ADC_H
#define ADC_H


// Reference Index
#define ADC_REF_VDD     0
#define ADC_REF_1       1
#define ADC_REF_2       2
#define ADC_REF_3       3
#define ADC_REF_VDD4FVR 4

// Mask to ensure AIN1,2 reference selections are valid
#define ADC_AIN_REF_MASK 0x03

// Default Reference configuration
#define ADC_DEF_REF_1_CFG ADC_REF_VDD


// API
void ADC_Initialize();

void ADC_SetupWake();

void ADC_SetupSleep();

uint16_t ADC_Read(uint8_t ch);

void ADC_SetRefType(uint8_t r);

uint8_t ADC_GetRefType();

#endif // ADC_H
