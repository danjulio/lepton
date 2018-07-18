/*
 * test Lepton 3.5 - for teensy 3.2 w/ LEP (96 MHz w/ Faster + LTO optimizations)
 *   - Designed to compare radiometric output with the various AGC modes
 *   - Output display on ILI9341 320x240 pixel LCD display
 *   - Display currently selected Lepton LUT and battery voltage on display
 *   - Uses Lepton VSYNC output
 *   - Uses hardware platform with modified Sparkfun LiPo charger + power button + rocker button
 * 
 * Operation
 *   - Press and hold power switch to startup (release when you see the display clear as teensy code is running)
 *   - Press power switch again to power down
 *   - Rocker switch downward press toggles between radiometric output and 4 various AGC modes
 *   - Rocker switch left/right selects between built-in LUTs
 *     
 * Connections
 *   D0 - Active Low Right Switch Input
 *   D1 - Active Low Left Switch Input
 *   D2 - Lepton nCS output
 *   D3 - Lepton VSYNC input
 *   D4 - SD Card (on LCD) nCS output (unused)
 *   D5 - LCD nCS output
 *   D6 - LCD DC control signal output
 *   D7 - Active High Power Enable/Hold Output (LCD Backlight enable)
 *   D8 - Active Low Push Switch Input
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


const int pin_lepton_cs = 2;
const int pin_lepton_vsync = 3;
const int pin_tft_dc = 6;
const int pin_tft_cs = 5;

LeptonSDKEmb32OEM lep;

Adafruit_ILI9341 tft = Adafruit_ILI9341(pin_tft_cs, pin_tft_dc);

LEP_CAMERA_PORT_DESC_T portDesc;
LEP_CAMERA_PORT_DESC_T_PTR portDescP = &portDesc;
LEP_CHAR8 oemPartString[32];

//Array to store one Lepton packet
static uint8_t lepPacket[LEP_PKT_LENGTH];

// Lepton Frame buffer sized for 16-bit linear configuration
// First half used for AGC mode
static uint8_t lepBuffer[2*LEP_NUM_PIXELS];

volatile bool dispBufferValid = false;

volatile int curSegment = 1;
volatile bool validSegmentRegion = false;
volatile uint16_t minLepVal = 0xFFFF;
volatile uint16_t maxLepVal = 0x0;

// NUM_COLORMAPS includes grayscale as index 0
#define NUM_COLORMAPS 4
static uint16_t colorMap[256];
int colorMapSelector = 0;

// AGC control
int prevAgcSelection = -1;  // Force initial update
int agcSelector = 0;


void setup() {
  bool success = true;
  
  // Initialize our controls first to keep power on
  InitControls();
  
  Serial.begin(115200);
  
  pinMode(pin_lepton_cs, OUTPUT);
  digitalWrite(pin_lepton_cs, HIGH);
  pinMode(pin_lepton_vsync, INPUT);
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);

  initColorMap();

  delay(2000);
  
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_CYAN);
  tft.println("Open LEP");
  if (lep.LEP_OpenPort(0, LEP_CCI_TWI, 100, portDescP) != LEP_OK) {
    tft.println("Open failed");
    success = false;
  } else {
    if (lep.LEP_SetOemGpioMode(portDescP, LEP_OEM_GPIO_MODE_VSYNC) != LEP_OK) {
      tft.println("Set GPIO failed");
      success = false;
    } else {
      LEP_OEM_GPIO_MODE_E gpioMode;
      if (lep.LEP_GetOemGpioMode(portDescP, &gpioMode) != LEP_OK) {
        tft.println("Get GPIO failed");
        success = false;
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
    if (++agcSelector == 5) agcSelector = 0;
    ConfigureAgcState();
  } else if (RockerShortPress(L_B_MASK)) {
    if (--colorMapSelector == -1) colorMapSelector = NUM_COLORMAPS-1; 
    initColorMap();
  } else if (RockerShortPress(R_B_MASK)) {
    if (++colorMapSelector == NUM_COLORMAPS) colorMapSelector = 0;
    initColorMap();
  } else if (dispBufferValid) {
    if (agcSelector == 0) {
      AnalyzeLepData();
      Scale16bitData();
    }
    //dispColorImage8(80, 60, LEP_WIDTH, LEP_HEIGHT);
    dispFullColorImage8();
    dispStatusLine();
    //WaitForChar();
    dispBufferValid = false;
    EnableImageAcquisition();
  }
}


void ConfigureAgcState() {
  if (agcSelector == 0) {
    // AGC disabled
    (void) lep.LEP_SetAgcEnableState(portDescP, LEP_AGC_DISABLE);
    (void) lep.LEP_SetRadEnableState(portDescP, LEP_RAD_ENABLE);
  } else {
    (void) lep.LEP_SetAgcEnableState(portDescP, LEP_AGC_ENABLE);
    (void) lep.LEP_SetRadEnableState(portDescP, LEP_RAD_DISABLE);
    switch (agcSelector) {
      case 1:
        (void) lep.LEP_SetAgcPolicy(portDescP, LEP_AGC_LINEAR);
        (void) lep.LEP_SetAgcCalcEnableState(portDescP, LEP_AGC_ENABLE);
      case 2:
        (void) lep.LEP_SetAgcPolicy(portDescP, LEP_AGC_HEQ);
        (void) lep.LEP_SetAgcCalcEnableState(portDescP, LEP_AGC_ENABLE);
      case 3:
        (void) lep.LEP_SetAgcPolicy(portDescP, LEP_AGC_LINEAR);
        (void) lep.LEP_SetAgcCalcEnableState(portDescP, LEP_AGC_DISABLE);
      case 4:
        (void) lep.LEP_SetAgcPolicy(portDescP, LEP_AGC_HEQ);
        (void) lep.LEP_SetAgcCalcEnableState(portDescP, LEP_AGC_DISABLE);
    }
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


void WaitForChar() {
  while (!Serial.available()) {};
  while (Serial.available()) {
    (void) Serial.read();
  }
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
          if (agcSelector == 0) {
            // AGC disabled
            CopyPacket16ToBuffer(line);
          } else {
            // AGC enabled
            CopyPacket8ToBuffer(line);
          }
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
            done = true;
          }
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


// Used with AGC disabled and data is 16-bit radiometric
void CopyPacket16ToBuffer(uint8_t line) {
  uint8_t* lepPopPtr = &lepPacket[4];
  uint8_t* acqPushPtr = &lepBuffer[2*(((curSegment-1) * 30 * LEP_WIDTH) + (line * (LEP_WIDTH/2)))];

  while (lepPopPtr <= &lepPacket[163]) {
    *acqPushPtr++ = *lepPopPtr++;
  }
}


void AnalyzeLepData()
{
  uint8_t* ptr = &lepBuffer[0];
  uint16_t t;

  // Find the minimum and maximum temps in the lepton data
  minLepVal = 0xFFFF;
  maxLepVal = 0;
  while (ptr < &lepBuffer[2*LEP_NUM_PIXELS]) {
    t = *ptr++ << 8;
    t |= *ptr++;
    if (t < minLepVal) minLepVal = t;
    if (t > maxLepVal) maxLepVal = t;
  }
}


// Used when AGC enabled
void CopyPacket8ToBuffer(uint8_t line) {
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


// Scale the 16-bit data in the whole buffer and restore it in the first half of the buffer
// so the 8-bit display routines can display it
void Scale16bitData() 
{
  int i;
  uint16_t t;
  uint16_t diff = maxLepVal - minLepVal;
  
  for (i=0; i<LEP_NUM_PIXELS; i++) {
    t = (lepBuffer[i*2] << 8) | lepBuffer[i*2+1];
    lepBuffer[i] = (uint8_t) ((t - minLepVal) * 255 / diff);
  }
}


void dispColorImage8(int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint8_t* ptr = &lepBuffer[0];
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  tft.setAddrWindow(x, y, x+w-1, y+h-1);
  digitalWrite(pin_tft_dc, HIGH);
  digitalWrite(pin_tft_cs, LOW);
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      SPI.transfer16(colorMap[*ptr++]);
    }
  }
  SPI.endTransaction();
  digitalWrite(pin_tft_cs, HIGH);
}


void dispFullColorImage8()
{
  int16_t x, y;
  uint16_t pixel;
  uint8_t* ptr;

  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  tft.setAddrWindow(0, 20, 319, 239);
  digitalWrite(pin_tft_dc, HIGH);
  digitalWrite(pin_tft_cs, LOW);
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
}


void dispStatusLine() {
  static float prevBattVolts = 0;
  static int prevColorMapSelector = -1;

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  if (prevColorMapSelector != colorMapSelector) {
    tft.fillRect(20, 5, 75, 8, ILI9341_BLACK);
    tft.setCursor(20, 5);
    switch (colorMapSelector) {
      case 1: tft.print("Golden"); break;
      case 2: tft.print("Rainbow"); break;
      case 3: tft.print("Iron Black"); break;
      default: tft.print("Grayscale"); break;
    }
    prevColorMapSelector = colorMapSelector;
  }
  if (prevAgcSelection != agcSelector) {
    tft.fillRect(130, 5, 80, 8, ILI9341_BLACK);
    tft.setCursor(130, 5);
    switch (agcSelector) {
      case 0:
        tft.print("AGC disabled");
        break;
      case 1:
        tft.print("AGC linear C");
        break;
      case 2:
        tft.print("AGC HEQ C");
        break;
      case 3:
        tft.print("AGC linear");
        break;
      case 4:
        tft.print("AGC HEQ");
        break;
    }
    prevAgcSelection = agcSelector;
  }
  if (abs(prevBattVolts - GetBattVolts()) > 0.005) {
    tft.fillRect(280, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(280, 5);
    tft.print(GetBattVolts());
    tft.print("v");
    prevBattVolts = GetBattVolts();
  }
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

