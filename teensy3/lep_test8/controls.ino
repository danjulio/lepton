/*
 * Camera controls
 *   D0 - Active Low Right Switch Input
 *   D1 - Active Low Left Switch Input
 *   D8 - Active Low Push Switch Input
 *   D7 - Active High Power Enable/Hold Output
 *   A6 - Power Button SNS input
 *   A7 - Battery SNS input
 *   
 *   ADC Voltage input calculations
 *     - External resistor divider networks are 35.7 kohm over 11.5 kohm => 0.2436
 *     - Using internal Vref = 1.2V => count / 1023 * 1.2 * 1/0.2436
 */

/*
 * Constants
 */
#define CONTROL_EVAL_MSEC 25
#define CONTROL_NUM_BATT_AVG 8

// Voltage above which we decide that the power button is pressed (and supplying a higher voltage to 
// the Enable signal into to boost converter than we do from our output)
#define POWER_BUT_THRESH 3.2

// Critical battery voltage
#define CRIT_BATT_VOLTS 3.3

// Critical battery detection duration (eval periods)
#define CRIT_BATT_TO (1000/CONTROL_EVAL_MSEC)

// Power-down reasons
#define PD_USER_BUTTON 0
#define PD_CRIT_BATT   1

/*
 * IO Pins
 */
const int but_right_input = 0;
const int but_left_input = 1;
const int but_push_input = 8;
const int hold_output = 7;
const int but_sns_ain = A6;
const int batt_sns_ain = A7;

/*
 * Variables
 */
uint16_t battAvgArray[CONTROL_NUM_BATT_AVG];
int battPushIndex;
float battVolts;
int battCritTimer;

bool prevPowerButton;
bool powerButtonDown;
bool powerDownFlag;
float buttVolts;

// Long press timeout is specified in evaluation units
#define BUTT_LP_TO (2000/CONTROL_EVAL_MSEC)

#define NUM_BUTTONS 3
uint8_t prevRockerButton;
uint8_t rockerButtonPressed;
uint8_t rockerButtonReleased;
uint8_t rockerButtonDown;
uint8_t rockerButtonLongPress;
uint8_t rockerButtonTimers[NUM_BUTTONS];

unsigned long prevControlEvalT;

void InitControls()
{
  int i;
  
  // Immediately drive power output enable
  pinMode(hold_output, OUTPUT);
  digitalWrite(hold_output, HIGH);

  // Setup other inputs
  pinMode(but_left_input, INPUT_PULLUP);
  pinMode(but_right_input, INPUT_PULLUP);
  pinMode(but_push_input, INPUT_PULLUP);

  // Setup to use internal 1.2V Vref
  analogReference(INTERNAL);

  // Initialize the battery array
  for (i=0; i<CONTROL_NUM_BATT_AVG; i++) {
    battAvgArray[i] = analogRead(batt_sns_ain);
  }

  // Initialize other variables
  prevPowerButton = true;  // Starts off pressed
  powerButtonDown = true;
  powerDownFlag = false;
  battPushIndex = 0;
  battVolts = Adc2Voltage(battAvgArray[0]);
  battCritTimer = CRIT_BATT_TO;
  prevRockerButton = 0;
  rockerButtonPressed = 0;
  rockerButtonReleased = 0;
  rockerButtonDown = 0;
  rockerButtonLongPress = 0;
  for (i=0; i<NUM_BUTTONS; i++) {
    rockerButtonTimers[i] = BUTT_LP_TO;
  }

  prevControlEvalT = millis();
}

void EvalControls()
{
  uint32_t curT = millis();
  
  if (AbsDiff32u(curT, prevControlEvalT) > CONTROL_EVAL_MSEC) {
    prevControlEvalT = curT;

    EvalPowerButton();
    EvalBattery();
    EvalRocker();
  }
}


float GetBattVolts()
{
  return battVolts;
}


bool PowerDownDetected()
{
  return (powerDownFlag);
}


bool RockerShortPress(uint8_t mask)
{
  bool b;

  b = ((rockerButtonReleased & mask) == mask) && ((rockerButtonLongPress & mask) == 0x00);
  rockerButtonReleased &= ~mask; // clear state after consumption
  return (b);
}


bool RockerLongPress(uint8_t mask)
{
  return ((rockerButtonLongPress & mask) == 0x00);
}


void PowerDown()
{
  ControlPowerDown(PD_USER_BUTTON);
}


void ControlPowerDown(int pdReason)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.setCursor(100, 100);
  if (pdReason == PD_USER_BUTTON) {
    tft.print("Power Down!");
  } else if (pdReason == PD_CRIT_BATT) {
    tft.print("Batt Critical!");
    delay(1000);
  }
  digitalWrite(hold_output, LOW);
  while (1) {};  // Spin until power goes away when the user releases the button
}


void EvalPowerButton()
{
  bool curButton;
  int butAdc;

  // Get button value - we detect it pressed if the voltage is higher indicating power through button
  // instead of hold output
  butAdc = analogRead(but_sns_ain);
  curButton = (Adc2Voltage(butAdc) > POWER_BUT_THRESH);

  // Evalute logic
  powerDownFlag = !powerButtonDown & curButton & prevPowerButton;
  powerButtonDown = curButton & prevPowerButton;
  prevPowerButton = curButton;
}


void EvalBattery()
{
  int i;
  int sum;
  
  battAvgArray[battPushIndex++] = analogRead(batt_sns_ain);
  if (battPushIndex == CONTROL_NUM_BATT_AVG) battPushIndex = 0;

  // Compute an average
  sum = 0;
  for (i=0; i<CONTROL_NUM_BATT_AVG; i++) {
    sum += battAvgArray[i];
  }
  // Round up if necessary
  if ((sum & (CONTROL_NUM_BATT_AVG/2)) != 0) {
    sum += CONTROL_NUM_BATT_AVG/2;
  }
  sum /= CONTROL_NUM_BATT_AVG;

  // Compute the battery voltage
  battVolts = Adc2Voltage(sum);

  if (battVolts < CRIT_BATT_VOLTS) {
    if (--battCritTimer == 0) {
      ControlPowerDown(PD_CRIT_BATT);
    }
  } else {
    // Hold timer reset
    battCritTimer = CRIT_BATT_TO;
  }
}


void EvalRocker()
{
  uint8_t curButton;
  uint8_t buttMask;
  int i;
  
  // Set Eval state - will be changed if necessary
  rockerButtonPressed = 0;
  rockerButtonReleased = 0;
  
  // Get the active high current state
  curButton = 0;
  if (digitalRead(but_left_input) == LOW) {
    curButton |= L_B_MASK;
  }
  if (digitalRead(but_push_input) == LOW) {
    curButton |= P_B_MASK;
  }
  if (digitalRead(but_right_input) == LOW) {
    curButton |= R_B_MASK;
  }
  
  // Compute the button state
  rockerButtonPressed = curButton & prevRockerButton & ~rockerButtonDown;
  rockerButtonReleased = rockerButtonDown & ~curButton & ~prevRockerButton;
  rockerButtonDown |= curButton & prevRockerButton;        // Set bits when both cur & prev are set
  rockerButtonDown &= ~(~curButton & ~prevRockerButton);   // Clear bits when both cur & prev are clear
  prevRockerButton = curButton;

  // Evaluate the long-press detection
  buttMask = 0x01;
  for (i=0; i<NUM_BUTTONS; i++) {
    if (rockerButtonPressed & buttMask) {
      rockerButtonTimers[i] = BUTT_LP_TO;
      rockerButtonLongPress &= ~buttMask;
    } else if (rockerButtonDown & buttMask) {
      if (rockerButtonTimers[i] == 0) {
        rockerButtonLongPress |= buttMask;
      } else {
        --rockerButtonTimers[i];
      }
    }
    buttMask = buttMask << 1;
  }
}


float Adc2Voltage(int adcVal)
{
  return ((float) adcVal * (1.2 * (1.0/0.2436)) / 1023.0);
}

