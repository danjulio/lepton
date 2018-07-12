/*
 * LeptonSDKEmb32OEM on Teensy demo
 * 
 * Connect Teensy I2C signals to the Lepton I2C along with a pair of 4.7 kohm pull-up resistors.
 * 
 * All references are to FLIR's Document Number: 110-0144-04 Rev 200 "Software Interface Description Document (IDD)"
 */
#include <LeptonSDKEmb32OEM.h>


//
// Instance the device
//
LeptonSDKEmb32OEM lep;


//
// Global variables
//
LEP_CAMERA_PORT_DESC_T portDesc;
LEP_CAMERA_PORT_DESC_T_PTR portDescP = &portDesc;
bool lepConnected = false;


//
// Arduino environment entry points
//
void setup() {
  LEP_OEM_GPIO_MODE_E gpioMode;

  Serial.begin(115200);

  // Wait for the lepton to become ready
  delay(1000);

  // Attempt to connect to the Lepton
  if (lep.LEP_OpenPort(0, LEP_CCI_TWI, 100, portDescP) != LEP_OK) {
    Serial.println("Open failed");
  } else {
    lepConnected = true;

    // Display the Lepton Model
    DisplayLeptonModel();

    // Enable VSYNC on the GPIO3 output (you should be able to see this using an oscilloscope)
    // (IDD section 4.6.15 OEM GPIO Mode Select)
    if (lep.LEP_GetOemGpioMode(portDescP, &gpioMode) == LEP_OK) {
      Serial.printf("GPIO mode = %d\n", (int) gpioMode);
    }
    if (lep.LEP_SetOemGpioMode(portDescP, LEP_OEM_GPIO_MODE_VSYNC) != LEP_OK) {
      Serial.println("Set GPIO failed");
    } 
    if (lep.LEP_GetOemGpioMode(portDescP, &gpioMode) == LEP_OK) {
      Serial.printf("GPIO mode = %d\n", (int) gpioMode);
    }
  }

}


void loop() {
  LEP_SYS_FPA_TEMPERATURE_KELVIN_T fpaKx100;
  float fpaC;
  
  // Display the Lepton's FPA temperature once per second (IDD section 4.4.17)
  if (lepConnected) {
    if (lep.LEP_GetSysFpaTemperatureKelvin(portDescP, &fpaKx100) == LEP_OK) {
      fpaC = (fpaKx100 - 27315) / 100.0;  // Convert to C
      
      Serial.print("FPA temp = ");
      Serial.print(fpaKx100);
      Serial.print("  (");
      Serial.print(fpaC);
      Serial.println("C)");
    }
  }

  delay(1000);
}


//
// Subroutines
//
void DisplayLeptonModel() {
  LEP_CHAR8 oemPartString[32];
  
  // Get the OEM FLIR Systems Part Number (IDD section 4.6.4)
  if (lep.LEP_GetOemFlirPartNumber(portDescP, (LEP_OEM_PART_NUMBER_T_PTR) oemPartString) == LEP_OK) {
    Serial.printf("Part number: %s  ==> ", oemPartString);

    // Part numbers from FLIR Lepton Engineering Datasheet, section 1.5 Key Specifications
    if (strstr(oemPartString, "500-0659-01") != NULL) {
      Serial.println("Lepton2 Shuttered");
    }

    else if (strstr(oemPartString, "500-0763-01") != NULL) {
      Serial.println("Lepton2.5 Shuttered (Radiometric)");
    }

    else if (strstr(oemPartString, "500-0726-01") != NULL) {
      Serial.println("Lepton3 Shuttered");
    }

    else if (strstr(oemPartString, "500-0771-01") != NULL) {
      Serial.println("Lepton3.5 Shuttered (Radiometric)");
    }

    else {
      Serial.println("Unknown");
    }
  }
}

