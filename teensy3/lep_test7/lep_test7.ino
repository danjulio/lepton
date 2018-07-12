/*
 * test Lepton 3.5 - for teensy 3.2 w/ LEP (96 Mhz w/ Faster + LTO optimizations)
 *   - Attempt to enable AGC output with RGB output through built-in LUTs
 *   - Output display on ILI9341 320x240 pixel LCD display
 *   - Display currently selected Lepton LUT and battery voltage on display
 *   - Uses Lepton VSYNC output
 *   - Uses hardware platform with modified Sparkfun LiPo charger + power button + rocker button
 * 
 * Gets about 4.4 Hz refresh rate
 * 
 * Note: this sketch skips the first 10 lines of data from the Lepton because of memory limitations in the
 * Teensy 3.2.  That omission works out ok because we draw some text in those positions anyway.
 *  
 * Operation
 *   - Press and hold power switch to startup (release when you see the display clear as teensy code is running)
 *   - Press power switch again to power down
 *   - Rocker switch downward press unused
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

#define LEP_MAX_FRAME_DELAY_USEC 9450

#define LEP_WIDTH      160
#define LEP_HEIGHT     120
#define LEP_NUM_PIXELS (LEP_WIDTH*LEP_HEIGHT)
#define LEP_PKT_LENGTH 244

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

// Lepton Frame buffer (8-bit for RGB values)
// Note: This is minus the first 20 packets (10 lines)
//       - we only accumulate after seeing a valid line number
//         (this lets us fit in Teensy 3.2 RAM)
static uint8_t lepBuffer[3*LEP_WIDTH*(LEP_HEIGHT-10)];

volatile bool dispBufferValid = false;

volatile int curSegment = 1;
volatile bool validSegmentRegion = false;

// NUM_COLORMAPS does not include user LUT
#define NUM_COLORMAPS 8
int colorMapSelector = 1;  // Fusion is default

uint32_t prevFrameUsec = 0;


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

  delay(2000);
  
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_CYAN);
  tft.println("Open LEP");
  if (lep.LEP_OpenPort(0, LEP_CCI_TWI, 100, portDescP) != LEP_OK) {
    tft.println("Open failed");
    success = false;
  } else {
    // Configure the Lepton's operating state
/*
    if (lep.LEP_SetVidFocusCalcEnableState(portDescP, LEP_VID_FOCUS_CALC_DISABLE) != LEP_OK) {
      tft.println("Set Focus Calc disable failed");
      success = false;
    } else {
      LEP_VID_FOCUS_CALC_ENABLE_E focusEnable;
      if (lep.LEP_GetVidFocusCalcEnableState(portDescP, &focusEnable) != LEP_OK) {
        tft.println("Set Focus Calc disable failed");
        success = false;
      } else {
        tft.printf("Focus enable = %d\n", (int) focusEnable);
      }
    }
    
    if (lep.LEP_SetAgcPolicy(portDescP, LEP_AGC_LINEAR) != LEP_OK) {
      tft.println("Set AGC Policy failed");
      success = false;
    } else {
      LEP_AGC_POLICY_E agcPolicy;
      if (lep.LEP_GetAgcPolicy(portDescP, &agcPolicy) != LEP_OK) {
        tft.println("Get AGC Policy failed");
        success = false;
      } else {
        tft.printf("AGC policy = %d\n", (int) agcPolicy);
      }
    }

    if (lep.LEP_SetAgcHeqScaleFactor(portDescP, LEP_AGC_SCALE_TO_8_BITS) != LEP_OK) {
      tft.println("Set AGC Scale Factor failed");
      success = false;
    } else {
      LEP_AGC_HEQ_SCALE_FACTOR_E heqScaleFactor;
      success = false;
      if (lep.LEP_GetAgcHeqScaleFactor(portDescP, &heqScaleFactor) != LEP_OK) {
        tft.println("Get AGC Scale Factor failed");
      } else {
        tft.printf("AGC Scale Factor = %d\n", (int) heqScaleFactor);
      }
    }
*/
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

/*
    if (lep.LEP_SetVidVideoOutputFormat(portDescP, LEP_VID_VIDEO_OUTPUT_FORMAT_RGB888) != LEP_OK) {
      tft.println("Set AGC Video Format failed");
      success = false;
    } else {
      LEP_VID_VIDEO_OUTPUT_FORMAT_E videoFormat;
      if (lep.LEP_GetVidVideoOutputFormat(portDescP, &videoFormat) != LEP_OK) {
        tft.println("Get Video Format failed");
        success = false;
      } else {
        tft.printf("Video Format = %d\n", (int) videoFormat);
      }
    }
*/
    if (lep.LEP_SetOemVideoOutputFormat(portDescP, LEP_VIDEO_OUTPUT_FORMAT_RGB888) != LEP_OK) {
      tft.println("Set OEM Video Format failed");
      success = false;
    } else {
      LEP_OEM_VIDEO_OUTPUT_FORMAT_E videoFormat;
      if (lep.LEP_GetOemVideoOutputFormat(portDescP, &videoFormat) != LEP_OK) {
        tft.println("Get Video Format failed");
        success = false;
      } else {
        tft.printf("Video Format = %d\n", (int) videoFormat);
      }
    }

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

    SetLepLUT();  // Set initial colormap
  }

  if (!success) {};  // Spin forever on init failure

  delay(5000);

  tft.fillScreen(ILI9341_BLACK);
  
  // Enable vsync interrupts
  EnableImageAcquisition();
}


void loop() {
  EvalControls();
  
  if (PowerDownDetected()) {
    PowerDown();
  } else if (RockerShortPress(L_B_MASK)) {
    if (--colorMapSelector == -1) colorMapSelector = NUM_COLORMAPS-1;
    SetLepLUT();
  } else if (RockerShortPress(R_B_MASK)) {
    if (++colorMapSelector == NUM_COLORMAPS) colorMapSelector = 0;
    SetLepLUT();
  }  else if (dispBufferValid) {
    //dispColorImage(80, 60, LEP_WIDTH, (LEP_HEIGHT-10));
    dispFullColorImage();
    dispStatusLine();
    //dumpLeptonImage();
    //WaitForChar();
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

  startUsec = micros();
  prevLine = 255;

  // Skip this if the time between it and when we last released CS is less than 185 mSec
  // to properly resync
  if (AbsDiff32u(startUsec, prevFrameUsec) < 185000) {
    return;
  }
  
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
              validSegmentRegion = true;
            }
          } else if ((segment < 2) || (segment > 4)) {
            // Hold/Reset in starting position (always collecting in segment 1 buffer locations)
            validSegmentRegion = false;  // In case it was set
            curSegment = 1;
          }
        }
 
        // Copy the data to the lepton frame buffer if it's valid (and past segment 1/line 20)
        if (validSegmentRegion && (line <= 59)) {
          CopyPacketToBuffer(line);
        }
               
        if (line == 59) {
          // Saw a complete segment, move to next segment or complete frame aquisition if possible
          if (validSegmentRegion) {
            if (curSegment < 4) {
              // Setup to get next segment
              curSegment++;
            } else {
              // Got frame, see if we can display the buffer
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
  uint8_t* lepPopPtr = &lepPacket[4];  // Start with first R value
  uint8_t* acqPushPtr = &lepBuffer[((curSegment-1) * 60 * 240) + (line * 240) - (20*240)];  // Subtract missing first 10 lines

  while (lepPopPtr <= &lepPacket[243]) {
    *acqPushPtr++ = *lepPopPtr++;
  }
}


uint32_t AbsDiff32u(uint32_t n1, uint32_t n2) {
  if (n2 >= n1) {
    return (n2-n1);
  } else {
    return (n2-n1+0xFFFFFFFF);
  }
}


// Fast routine to display 1:1 pixel image at specified display location
void dispColorImage(int16_t x, int16_t y, int16_t w, int16_t h)
{
  uint8_t r, g, b;
  uint16_t pixel;
  uint8_t* ptr = &lepBuffer[0];
  
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  tft.setAddrWindow(x, y, x+w-1, y+h-1);
  digitalWrite(pin_tft_dc, HIGH);
  digitalWrite(pin_tft_cs, LOW);
  for(y=h; y>0; y--) {
    for(x=w; x>0; x--) {
      r = *ptr++ >> 3;
      g = *ptr++ >> 2;
      b = *ptr++ >> 3;
      pixel = (r << 11) | (g << 5) | b;
      SPI.transfer16(pixel);
    }
  }
  SPI.endTransaction();
  digitalWrite(pin_tft_cs, HIGH);
}


// Fast routine to display pixel-doubled image to fill LCD
void dispFullColorImage()
{
  int16_t x, y;
  uint8_t r, g, b;
  uint16_t pixel;
  uint8_t* ptr;

  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  tft.setAddrWindow(0, 20, 319, 239);
  digitalWrite(pin_tft_dc, HIGH);
  digitalWrite(pin_tft_cs, LOW);
  for(y=0; y<2*(LEP_HEIGHT-10); y=y+2) {
    // Line 1
    ptr = &lepBuffer[y/2*3*LEP_WIDTH];
    for(x=0; x<LEP_WIDTH; x++) {
      r = *ptr++ >> 3;
      g = *ptr++ >> 2;
      b = *ptr++ >> 3;
      pixel = (r << 11) | (g << 5) | b;
      SPI.transfer16(pixel);
      SPI.transfer16(pixel);
    }
    // Line 2
    ptr = &lepBuffer[y/2*3*LEP_WIDTH];
    for(x=0; x<LEP_WIDTH; x++) {
      r = *ptr++ >> 3;
      g = *ptr++ >> 2;
      b = *ptr++ >> 3;
      pixel = (r << 11) | (g << 5) | b;
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
      case 0: tft.print("Wheel 6"); break;
      case 1: tft.print("Fusion"); break;
      case 2: tft.print("Rainbow"); break;
      case 3: tft.print("Glowbow"); break;
      case 4: tft.print("Sepia"); break;
      case 5: tft.print("Color"); break;
      case 6: tft.print("Ice/Fire"); break;
      case 7: tft.print("Rain"); break;
      default: tft.print("User"); break;
    }
    prevColorMapSelector = colorMapSelector;
  }
  if (abs(prevBattVolts - GetBattVolts()) > 0.005) {
    tft.fillRect(280, 5, 30, 8, ILI9341_BLACK);
    tft.setCursor(280, 5);
    tft.print(GetBattVolts());
    tft.print("v");
    prevBattVolts = GetBattVolts();
  }
}


void dumpLeptonImage() {
  int x, y;
  uint8_t* ptr = &lepBuffer[0];

  for (x=0; x<LEP_WIDTH; x++) {
    for (y=0; y<(LEP_HEIGHT-10); y++) {
      Serial.printf("%2x ", *ptr++);
    }
    Serial.println();
    Serial.println();
  }
}


void dumpLeptonPacket() {
  int i;

  for (i=0; i<164; i++) {
    Serial.printf("%2x ", lepPacket[i]);
    if ((i == 3) || (i == 43) || (i == 83) || (i == 123) || (i == 163)) {
      Serial.println();
    }
  }
}


void SetLepLUT() {
  (void) lep.LEP_SetVidPcolorLut(portDescP, (LEP_PCOLOR_LUT_E) colorMapSelector);
}

