/*
 * test Lepton 3.5 - for teensy 3.2 w/ LEP (96 MHz w/ Faster + LTO optimizations)
 *   - Display 16-bit temperature values, linearly scaled to 8-bits and processed through a color map
 *   - Output display on ILI9341 320x240 pixel LCD display
 *   - Display current color map, temperature range and battery voltage
 *   - Display temperature at center of display 
 *   - Rocker selection of color map or temperature range
 *   - Uses Lepton VSYNC output
 *   - Uses hardware platform with modified Sparkfun LiPo charger + power button + rocker button
 * 
 * Gets about 4.4 Hz refresh rate
 * 
 * Operation
 *   - Press and hold power switch to startup (release when you see the display clear as teensy code is running)
 *   - Press power switch again to power down
 *   - Rocker switch downward press selects between color map mode or temperature range mode
 *   - Rocker swtich left/right selects
 *     - between color maps in color map mode
 *     - between temperature ranges in temperature range mode
 *   - Temperature ranges
 *     1. Lepton frame min/max
 *     2. 15C to 40C
 *     3. 0C to 100C
 *     4. -10C to 10C
 *     5. 100C to 300C
 *     Software will adjust the range if the lepton frame exceeds selected range
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

// IO Pins
const int pin_lepton_cs = 2;
const int pin_lepton_vsync = 3;
const int pin_tft_dc = 6;
const int pin_tft_cs = 5;

LeptonSDKEmb32OEM lep;

Adafruit_ILI9341 tft = Adafruit_ILI9341(pin_tft_cs, pin_tft_dc);

LEP_CAMERA_PORT_DESC_T portDesc;
LEP_CAMERA_PORT_DESC_T_PTR portDescP = &portDesc;

//Array to store one Lepton packet
static uint8_t lepPacket[LEP_PKT_LENGTH];

// Lepton Frame buffer (16-bit values)
static uint16_t lepBuffer[LEP_NUM_PIXELS];

volatile bool dispBufferValid = false;

volatile int curSegment = 1;
volatile bool validSegmentRegion = false;
volatile uint16_t minLepVal = 0xFFFF;
volatile uint16_t maxLepVal = 0x0;

uint32_t prevFrameUsec = 0;

// GUI
#define NUM_GUI_CONTROLS 2
#define GUI_LUT 0
#define GUI_TEMP 1
int curGuiSelector = GUI_LUT;

// NUM_COLORMAPS includes grayscale as index 0
#define NUM_COLORMAPS 4
static uint16_t colorMap[256];
int colorMapSelector = 0;

// Temperature ranges (temp range 0 is camera min/max)
#define NUM_TEMP_RANGES 5
// Fixed ranges (although camera extremes can exceed
// LepVal = TempC + 273.16 * 100
const uint16_t tempRanges[NUM_TEMP_RANGES-1][2] = {
  15 * 100 + 27316, 40 * 100 + 27316,
  0 * 100 + 27316, 100 * 100 + 27316,
  -10 * 100 + 27316, 10 * 100 + 27316,
  100 * 100 + 27316, 300 * 100 + 27316
};
uint16_t curMinLepRange;
uint16_t curMaxLepRange;
uint16_t curCentLepVal;
int curRangeIndex = 0;


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
  } else {
    
    if (lep.LEP_SetRadTLinearAutoResolution(portDescP, LEP_RAD_ENABLE) != LEP_OK) {
      tft.println("Set RAD Linear AutoResolution Enable failed");
      success = false;
    } else {
      LEP_RAD_ENABLE_E radAutoResEnable;
      if (lep.LEP_GetRadTLinearAutoResolution(portDescP, &radAutoResEnable) != LEP_OK) {
        tft.println("Get RAD Linear AutoResolution Enable failed");
        success = false;
      } else {
        tft.printf("RAD Linear AutoResolution Enable = %d\n", (int) radAutoResEnable);
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
    } else if (curGuiSelector == GUI_TEMP) {
      if (--curRangeIndex == -1) curRangeIndex = NUM_TEMP_RANGES-1;
    }
  } else if (RockerShortPress(R_B_MASK)) {
    if (curGuiSelector == GUI_LUT) {
      if (++colorMapSelector == NUM_COLORMAPS) colorMapSelector = 0;
      initColorMap();
    } else if (curGuiSelector == GUI_TEMP) {
      if (++curRangeIndex == NUM_TEMP_RANGES) curRangeIndex = 0;
    } 
  } else if (dispBufferValid) {
    //Serial.print(millis());
    AnalyzeLepData();
    //dispColorImage(80, 60, LEP_WIDTH, LEP_HEIGHT);
    dispFullColorImage();
    dispStatusLine();
    //dumpLeptonImage();
    //Serial.printf("Min: %d  Max %d\n", minLepVal, maxLepVal);
    //WaitForChar();
    dispBufferValid = false;
    //Serial.print(" ");
    //Serial.println(millis());
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
                prevFrameUsec = micros();  // Record end-time of this frame
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
  uint8_t* lepPopPtr = &lepPacket[4];
  uint16_t* acqPushPtr = &lepBuffer[((curSegment-1) * 30 * LEP_WIDTH) + (line * (LEP_WIDTH/2))];
  uint16_t t;

  while (lepPopPtr <= &lepPacket[163]) {
    t = *lepPopPtr++ << 8;
    t |= *lepPopPtr++;
    *acqPushPtr++ = t;
  }
}


uint32_t AbsDiff32u(uint32_t n1, uint32_t n2) {
  if (n2 >= n1) {
    return (n2-n1);
  } else {
    return (n2-n1+0xFFFFFFFF);
  }
}


void AnalyzeLepData()
{
  uint16_t* ptr = &lepBuffer[0];
  uint16_t t;

  // Find the minimum and maximum temps in the lepton data
  minLepVal = 0xFFFF;
  maxLepVal = 0;
  while (ptr < &lepBuffer[LEP_NUM_PIXELS]) {
    t = *ptr++;
    if (t < minLepVal) minLepVal = t;
    if (t > maxLepVal) maxLepVal = t;
  }

  // Determine display min/max based on selected temperature range and camera data
  if (curRangeIndex == 0) {
    // Use camera ranges
    curMinLepRange = minLepVal;
    curMaxLepRange = maxLepVal;
  } else {
    // Use preset range - but exceed it if camera does
    curMinLepRange = tempRanges[curRangeIndex-1][0];
    curMaxLepRange = tempRanges[curRangeIndex-1][1];
    if (minLepVal < curMinLepRange) curMinLepRange = minLepVal;
    if (maxLepVal > curMaxLepRange) curMaxLepRange = maxLepVal;
  }

  // Finally get the center temp value (before we may overwrite the lep buffer for video)
  curCentLepVal = lepBuffer[LEP_NUM_PIXELS/2 + (LEP_WIDTH/2)];
}


// Fast routine to display 1:1 pixel image at specified display location
void dispColorImage(int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint16_t diff = curMaxLepRange - curMinLepRange;
  uint16_t* ptr = &lepBuffer[0];
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  tft.setAddrWindow(x, y, x+w-1, y+h-1);
  digitalWrite(pin_tft_dc, HIGH);
  digitalWrite(pin_tft_cs, LOW);
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      SPI.transfer16(colorMap[(*ptr++ - curMinLepRange) * 255 / diff]);
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
  uint16_t diff = curMaxLepRange - curMinLepRange;
  uint16_t* ptr;

  // Convert the raw lep data into scaled values once so we don't have to repeat the calcs
  ptr = &lepBuffer[0];
  while (ptr < &lepBuffer[LEP_NUM_PIXELS]) {
    pixel = (*ptr - curMinLepRange) * 255 / diff;
    *ptr++ = pixel;
  }

  // Copy the data to the frame buffer expanding via the colormap (skip the first 10 lines of data)
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

  // Draw an inverted pixel at the center
  pixel = ~colorMap[lepBuffer[LEP_NUM_PIXELS/2 + (LEP_WIDTH/2)]];
  tft.fillRect(159, 119, 2, 2, pixel);
}


void dispStatusLine() {
  static int prevGuiSelector = -1;
  static int prevColorMapSelector = -1;
  static int prevTmin = 999;
  static int prevTcur = 999;
  static int prevTmax = 999;
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

  // Temps
  if (prevGuiSelector != curGuiSelector) {
    tft.fillRect(94, 5, 5, 8, ILI9341_BLACK);
    tft.setCursor(94, 5);
    if (curGuiSelector == GUI_TEMP) {
      tft.print(">");
    }
  }
  t = round(curMinLepRange / 100.0 - 273.16);
  if (prevTmin != t) {
    tft.fillRect(100, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(100, 5);
    tft.print(t);
    prevTmin = t;
  }
  t = round(curCentLepVal / 100.0 - 273.16);
  if (prevTcur != t) {
    tft.fillRect(140, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(140, 5);
    tft.print(t);
    prevTcur = t;
  }
  t = round(curMaxLepRange / 100.0 - 273.16);
  if (prevTmax != t) {
    tft.fillRect(180, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(180, 5);
    tft.print(t);
    prevTmax = t;
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


void dumpLeptonImage() {
  int x, y;
  uint16_t* ptr = &lepBuffer[0];

  for (x=0; x<LEP_WIDTH; x++) {
    for (y=0; y<LEP_HEIGHT; y++) {
      Serial.printf("%4x ", *ptr++);
    }
    Serial.println();
    Serial.println();
  }
}


void dumpLeptonPacket() {
  int i;

  for (i=0; i<LEP_PKT_LENGTH; i++) {
    Serial.printf("%2x ", lepPacket[i]);
    if ((i == 3) || (i == 43) || (i == 83) || (i == 123) || (i == 163)) {
      Serial.println();
    }
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

