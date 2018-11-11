/*
 * test Lepton 3.5 - for teensy 3.2 w/ LEP (96 MHz w/ Faster + LTO optimizations)
 *   - A combination of lep_test5 & lep_test9 ported to Adafruit LCD w/ LCD CS on 10 and DC on 9
 *   - Enable AGC and display 8-bit output displayed through a color map
 *   - Allow user to change emissivity
 *   - Output display on ILI9341 320x240 pixel LCD display
 *   - Display current color map, spot temp, emissivity % and battery voltage
 *   - Rocker selection of color map or emissivity percent
 *   - Uses Lepton VSYNC output
 *   - Uses hardware platform with modified Sparkfun LiPo charger + power button + rocker button
 * 
 * Gets about 4.4 Hz refresh rate
 * 
 * Operation
 *   - Press and hold power switch to startup (release when you see the display clear as teensy code is running)
 *   - Press power switch again to power down
 *   - Rocker switch downward press selects between color map mode or emissivity mode
 *   - Rocker swtich left/right selects
 *     - between color maps in color map mode
 *     - between emissivity values (5% increments) in emissivity mode (0-100%)
 *     
 * Connections
 *   D0 - Active Low Right Switch Input
 *   D1 - Active Low Left Switch Input
 *   D2 - Lepton nCS output
 *   D3 - Lepton VSYNC input
 *   D4 - SD Card (on LCD) nCS output (unused)
 *   D7 - Active High Power Enable/Hold Output (LCD Backlight enable)
 *   D8 - Active Low Push Switch Input 
 *   D9 - LCD DC control signal output
 *   D10- LCD nCS output
 *   D11 - SPI MOSI (to LCD)
 *   D12 - SPI MISO (to Lepton)
 *   D13 - SPI SCK output (to Lepton and LCD)
 *   A0 - Touch Screen Y- input (unused)
 *   A1 - Touch Screen X- input (unused)
 *   A2 - Touch Screen Y+ input (unused)
 *   A3 - Touch Screen X+ input (unused)
 *   A4 - I2C SDA0 (to Lepton) with external 4.7 k pull-up to 3V3
 *   A5 - I2C SCL0 (to Lepton) with external 4.7 k pull-up to 3V3
 *   A6 - Power Button SNS input
 *   A7 - Battery SNS input
 *   
 * Software released "as-is" for instructional use.  No warranty as to correctness or fitness for any application.
 * 
 * Written (or, more accurately, hacked together) by Dan Julio
 * 
 */
#include <SPI.h>
#include <LeptonSDKEmb32OEM.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "colormaps.h"

#define LEP_MAX_FRAME_DELAY_USEC 9450

#define LEP_WIDTH      160
#define LEP_HEIGHT     120
#define LEP_NUM_PIXELS (LEP_WIDTH*LEP_HEIGHT)
#define LEP_PKT_LENGTH 164

// Rocker Buttons
#define L_B_MASK 0x01
#define P_B_MASK 0x02
#define R_B_MASK 0x04

//Lepton frame error return
enum LeptonReadError {
  NONE, DISCARD, SEGMENT_ERROR, ROW_ERROR, SEGMENT_INVALID
};

// IO Pins
const int pin_lepton_cs = 2;
const int pin_lepton_vsync = 3;
const int pin_tft_dc = 9;
const int pin_tft_cs = 10;

LeptonSDKEmb32OEM lep;

Adafruit_ILI9341 tft = Adafruit_ILI9341(pin_tft_cs, pin_tft_dc);

LEP_CAMERA_PORT_DESC_T portDesc;
LEP_CAMERA_PORT_DESC_T_PTR portDescP = &portDesc;

//Array to store one Lepton packet
static uint8_t lepPacket[LEP_PKT_LENGTH];

// Lepton Frame buffer (8-bit for AGC values)
static uint8_t lepBuffer[LEP_WIDTH*LEP_HEIGHT];

volatile bool dispBufferValid = false;

volatile int curSegment = 1;
volatile bool validSegmentRegion = false;

// GUI
#define NUM_GUI_CONTROLS 2
#define GUI_LUT 0
#define GUI_EM 1
int curGuiSelector = GUI_LUT;

// NUM_COLORMAPS includes grayscale as index 0
#define NUM_COLORMAPS 4
static uint16_t colorMap[256];
int colorMapSelector = 0;

// RAD Flux Linear Parameters
//  Default (read)
//    0x2000
//    0x734B
//    0x2000
//    0x734B
//    0x2000
//    0x734B
//    0x0000
//    0x734B
#define EMISSIVITY_STEP 5
LEP_RAD_FLUX_LINEAR_PARAMS_T radFluxParms;
uint16_t curEmissivityInt = 100;            // 0 - 100 %


void setup() {
  bool success = true;
  
  // Initialize our controls first to keep power on
  InitControls();
  
  Serial.begin(115200);
  
  pinMode(pin_lepton_cs, OUTPUT);
  digitalWrite(pin_lepton_cs, HIGH);
  pinMode(pin_lepton_vsync, INPUT);
  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  initColorMap();

  delay(2000);

  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_CYAN);
  tft.println("lep_test10");
  if (lep.LEP_OpenPort(0, LEP_CCI_TWI, 100, portDescP) != LEP_OK) {
    tft.println("LEP Open failed");
  } else {   
    if (lep.LEP_GetRadFluxLinearParams(portDescP, &radFluxParms) != LEP_OK) {
      tft.println("Get RAD Flux Linear Parameters failed");
      success = false;
    } else {
      tft.println("Got RAD Flux Linear Parameters");
    }

    if (lep.LEP_SetAgcEnableState(portDescP, LEP_AGC_ENABLE) != LEP_OK) {
      tft.println("Set AGC failed");
      success = false;
    } else {
      LEP_AGC_ENABLE_E agcMode;
      if (lep.LEP_GetAgcEnableState(portDescP, &agcMode) != LEP_OK) {
        tft.println("Get AGC failed");
        success = false;
      } else {
        tft.printf("AGC mode = %d\n", (int) agcMode);
      }
    }

    if (lep.LEP_SetAgcCalcEnableState(portDescP, LEP_AGC_ENABLE) != LEP_OK) {
      tft.println("Set AGC Calc Enable failed");
      success = false;
    } else {
      LEP_AGC_ENABLE_E heqCalcEnable;
      if (lep.LEP_GetAgcCalcEnableState(portDescP, &heqCalcEnable) != LEP_OK) {
        tft.println("Get AGC Calc Enable failed");
        success = false;
      } else {
        tft.printf("AGC Calc Enable = %d\n", (int) heqCalcEnable);
      }
    }

    if (lep.LEP_SetRadEnableState(portDescP, LEP_RAD_DISABLE) != LEP_OK) {
      tft.println("Set RAD Disable failed");
      success = false;
    } else {
      LEP_RAD_ENABLE_E radEnable;
      if (lep.LEP_GetRadEnableState(portDescP, &radEnable) != LEP_OK) {
        tft.println("Get RAD Enable failed");
        success = false;
      } else {
        tft.printf("RAD Enable = %d\n", (int) radEnable);
      }
    }
    
    if (lep.LEP_SetOemGpioMode(portDescP, LEP_OEM_GPIO_MODE_VSYNC) != LEP_OK) {
      tft.println("Set GPIO failed");
    } else {
      LEP_OEM_GPIO_MODE_E gpioMode;
      if (lep.LEP_GetOemGpioMode(portDescP, &gpioMode) != LEP_OK) {
        tft.println("Get GPIO failed");
      } else {
        tft.printf("GPIO mode = %d\n", (int) gpioMode);
      }
    }
  }

  if (!success) {};  // Spin forever on init failure
  
  delay(2000);

  tft.fillScreen(ILI9341_BLACK);
  
  // Enable vsync interrupts
  EnableImageAcquisition();
}


void loop() {
  EvalControls();

  if (PowerDownDetected()) {
    PowerDown();
  } else if (RockerShortPress(P_B_MASK)) {
    if (++curGuiSelector == NUM_GUI_CONTROLS) curGuiSelector = 0;
  } else if (RockerShortPress(L_B_MASK)) {
    if (curGuiSelector == GUI_LUT) {
      if (--colorMapSelector == -1) colorMapSelector = NUM_COLORMAPS-1; 
      initColorMap();
    } else if (curGuiSelector == GUI_EM) {
      if (curEmissivityInt >= EMISSIVITY_STEP) {
        curEmissivityInt -= EMISSIVITY_STEP;
        UpdateEmissivity();
      }
    }
  } else if (RockerShortPress(R_B_MASK)) {
    if (curGuiSelector == GUI_LUT) {
      if (++colorMapSelector == NUM_COLORMAPS) colorMapSelector = 0;
      initColorMap();
    } else if (curGuiSelector == GUI_EM) {
      if (curEmissivityInt <= (100 - EMISSIVITY_STEP)) {
        curEmissivityInt += EMISSIVITY_STEP;
        UpdateEmissivity();
      }
    } 
  } else if (dispBufferValid) {
    //dispColorImage(80, 60, LEP_WIDTH, LEP_HEIGHT);
    dispFullColorImage();
    dispStatusLine();
    dispBufferValid = false;
    EnableImageAcquisition();
  }
}


void EnableImageAcquisition() {
  attachInterrupt(pin_lepton_vsync, vsyncHandler, RISING);
  
  // Configure the SPI library to be able to run in the ISR
  SPI.usingInterrupt(pin_lepton_vsync);
}


void DisableImageAcquisition() {
  detachInterrupt(pin_lepton_vsync);
  SPI.notUsingInterrupt(IRQ_PORTA);
}


//
// VSYNC ISR
//   - Attempt to read a complete segment from the Lepton
//   - Data loaded into lepBuffer
//   - dispBufferValid flag set when all 4 segments have been read for a frame
//     - Image acquisition disabled to allow main code time to display (it must re-enable acquisition)
// 
void vsyncHandler() {
  uint32_t startUsec;
  uint8_t line, prevLine;
  uint8_t segment;
  bool done = false;
  bool beforeValidData = true;

  startUsec = micros();
  prevLine = 255;

  while (!done) {
    if (ProcessPacket(&line, &segment)) {
      // Saw a valid packet
      if (line == prevLine) {
        // This is garbage data since line numbers should always increment
        done = true;
      } else {
        // Check for termination or completion conditions
        if (line == 20) {
          // Check segment
          if (!validSegmentRegion) {
            // Look for start of valid segment data
            if (segment == 1) {
              beforeValidData = false;
              validSegmentRegion = true;
            }
          } else if ((segment < 2) || (segment > 4)) {
            // Hold/Reset in starting position (always collecting in segment 1 buffer locations)
            validSegmentRegion = false;  // In case it was set
            curSegment = 1;
          }
        } 
        
        // Copy the data to the lepton frame buffer
        //  - beforeValidData is used to collect data before we know if the current segment (1) is valid
        //  - then we use validSegmentRegion for remaining data once we know we're seeing valid data
        if ((beforeValidData || validSegmentRegion) && (line <= 59)) {
          CopyPacketToBuffer(line);
        }
        
        if (line == 59) {
          // Saw a complete segment, move to next segment or complete frame aquisition if possible
          if (validSegmentRegion) {
            if (curSegment < 4) {
              // Setup to get next segment
              curSegment++;
            } else {
              // Got frame, see if we can flip/flop the frame buffer (other process is done with previous buffer)
              if (!dispBufferValid) {
                // Hand this buffer over to other process
                dispBufferValid = true;
                DisableImageAcquisition();
              }
              // Setup to get the next frame
              curSegment = 1;
              validSegmentRegion = false;
            }
          }
          done = true;
        }
      }
      prevLine = line;
    } else if (AbsDiff32u(startUsec, micros()) > LEP_MAX_FRAME_DELAY_USEC) {
      // Did not see a valid packet within this segment interval
      done = true;
    }
  }
}


//
// This routine attempts to read one RGB packet from the lepton
//   - Return false for discard packets
//   - Return true otherwise
//     - line contains the packet line number for all valid packets
//     - seg contains the packet segment number if the line number is 20
//
bool ProcessPacket(uint8_t* line, uint8_t* seg) {
  bool valid = false;

  *seg = 0;
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE1));

  //Start transfer  - CS LOW
  digitalWriteFast(pin_lepton_cs, LOW);

  SPI.transfer(lepPacket, LEP_PKT_LENGTH);

  //Repeat as long as the frame is not valid, equals sync
  if ((lepPacket[0] & 0x0F) == 0x0F) {
    valid = false;
  } else {
    *line = lepPacket[1];

    // Get segment when possible
    if (*line == 20) {
      *seg = (lepPacket[0] >> 4);
    }

    valid = true;
  }

  //End transfer - CS HIGH
  digitalWriteFast(pin_lepton_cs, HIGH);

  //End SPI Transaction
  SPI.endTransaction();

  return(valid);
}


void CopyPacketToBuffer(uint8_t line) {
  uint8_t* lepPopPtr = &lepPacket[5];  // Only going to copy the low bytes
  uint8_t* acqPushPtr = &lepBuffer[((curSegment-1) * 30 * LEP_WIDTH) + (line * (LEP_WIDTH/2))];

  while (lepPopPtr <= &lepPacket[163]) {
    *acqPushPtr++ = *lepPopPtr;
    lepPopPtr += 2;
  }
}


uint32_t AbsDiff32u(uint32_t n1, uint32_t n2) {
  if (n2 >= n1) {
    return (n2-n1);
  } else {
    return (n2-n1+0xFFFFFFFF);
  }
}


void UpdateEmissivity()
{
  uint32_t newEmissivity = (8192 * curEmissivityInt) / 100;

  radFluxParms.sceneEmissivity = (uint16_t) newEmissivity;

  (void) lep.LEP_SetRadFluxLinearParams(portDescP, radFluxParms);
}


int16_t GetSpotMeter()
{
  LEP_RAD_SPOTMETER_OBJ_KELVIN_T spotVal;

  if (lep.LEP_GetRadSpotmeterObjInKelvinX100(portDescP, &spotVal) == LEP_OK) {
    return (round(spotVal.radSpotmeterValue / 100.0 - 273.16));
  }

  return 0;
}


// Fast routine to display 1:1 pixel image at specified display location
void dispColorImage(int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint8_t* ptr = &lepBuffer[0];
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(pin_tft_cs, LOW);
  digitalWrite(pin_tft_dc, LOW);
  tft.setAddrWindow(x, y, w, h);
  digitalWrite(pin_tft_dc, HIGH);
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      SPI.transfer16(colorMap[*ptr++]);
    }
  }
  SPI.endTransaction();
  digitalWrite(pin_tft_cs, HIGH);
}


// Fast routine to display pixel-doubled image to fill LCD
void dispFullColorImage()
{
  int16_t x, y;
  uint16_t pixel;
  uint8_t* ptr;

  // Copy the data to the frame buffer expanding via the colormap (skip the first 10 lines of data)
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  digitalWrite(pin_tft_cs, LOW);
  digitalWrite(pin_tft_dc, LOW);
  tft.setAddrWindow(0, 20, 320, 240);
  digitalWrite(pin_tft_dc, HIGH);
  for(y=20; y<2*LEP_HEIGHT; y=y+2) {
    // Line 1
    ptr = &lepBuffer[y/2*LEP_WIDTH];
    for(x=0; x<LEP_WIDTH; x++) {
      pixel = colorMap[*ptr++];
      SPI.transfer16(pixel);
      SPI.transfer16(pixel);
    }
    // Line 2
    ptr = &lepBuffer[y/2*LEP_WIDTH];
    for(x=0; x<LEP_WIDTH; x++) {
      pixel = colorMap[*ptr++];
      SPI.transfer16(pixel);
      SPI.transfer16(pixel);
    }
  }
  SPI.endTransaction();
  digitalWrite(pin_tft_cs, HIGH);

  // Draw an inverted pixel at the center
  pixel = ~colorMap[lepBuffer[LEP_NUM_PIXELS/2 + (LEP_WIDTH/2)]];
  tft.fillRect(159, 119, 2, 2, pixel);
}


void dispStatusLine() {
  static int prevGuiSelector = -1;
  static int prevColorMapSelector = -1;
  static int prevTcur = 999;
  static uint8_t prevEmissivity = 255;
  static float prevBattVolts = 0;
  int t;

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);

  
  // Colormap
  if (prevGuiSelector != curGuiSelector) {
    tft.fillRect(4, 5, 5, 8, ILI9341_BLACK);
    tft.setCursor(4, 5);
    if (curGuiSelector == GUI_LUT) {
      tft.print(">");
    }
  }
  if (prevColorMapSelector != colorMapSelector) {
    tft.fillRect(10, 5, 75, 8, ILI9341_BLACK);
    tft.setCursor(10, 5);
    switch (colorMapSelector) {
      case 1: tft.print("Golden"); break;
      case 2: tft.print("Rainbow"); break;
      case 3: tft.print("Iron Black"); break;
      default: tft.print("Grayscale"); break;
    }
    prevColorMapSelector = colorMapSelector;
  }

  // Temp
  t = GetSpotMeter();
  if (prevTcur != t) {
    tft.fillRect(160, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(160, 5);
    tft.print(t);
    prevTcur = t;
  }

  // Emissivity
  if (prevGuiSelector != curGuiSelector) {
    tft.fillRect(214, 5, 5, 8, ILI9341_BLACK);
    tft.setCursor(214, 5);
    if (curGuiSelector == GUI_EM) {
      tft.print(">");
    }
  }
  if (prevEmissivity != curEmissivityInt) {
    tft.fillRect(220, 5, 40, 8, ILI9341_BLACK);
    tft.setCursor(220, 5);
    tft.print(curEmissivityInt);
    tft.print("%");
    prevEmissivity = curEmissivityInt;
  }
  
  // Battery
  if (abs(prevBattVolts - GetBattVolts()) > 0.005) {
    tft.fillRect(280, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(280, 5);
    tft.print(GetBattVolts());
    tft.print("v");
    prevBattVolts = GetBattVolts();
  }

  prevGuiSelector = curGuiSelector;
}


void initColorMap() {
  uint8_t r, g, b;
  uint8_t* ptr;
  int i;

  switch (colorMapSelector) {
    case 1: ptr = (uint8_t*) colormap_golden; break;
    case 2: ptr = (uint8_t*) colormap_rainbow; break;
    case 3: ptr = (uint8_t*) colormap_ironblack; break;
    default: ptr = (uint8_t*) colormap_grayscale; break;
  }
  
  for (i=0; i<256; i++) {
    r = *ptr++ >> 3;
    g = *ptr++ >> 2;
    b = *ptr++ >> 3;
    colorMap[i] = (r << 11) | (g << 5) | b;
  }
}

