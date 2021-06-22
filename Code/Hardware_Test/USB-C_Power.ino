#include <bb_spi_lcd.h>
#include <SparkFun_STUSB4500.h>
#include <Wire.h> 
STUSB4500 usb;
// fonts
#include "Orbitron_Medium_16.h"
#include "Orbitron_Bold_22.h"

#define TFT_CS         4
#define TFT_RST        5 
#define TFT_DC         22
#define TFT_LED        19
#define TFT_MOSI       23
#define TFT_SCK        18

const int Analog_channel_pin= 2;
int ADC_VALUE = 0;

//Leds
 const int Red_LED = 9; 
 const int Green_LED = 10; 

//Buttons
enum {
  BUTT_UP=0,
  BUTT_DOWN,
  BUTT_LEFT,
  BUTT_RIGHT,
  BUTT_ACTION,
  BUTT_POWER,
  BUTT_COUNT
};
const uint8_t ucButtons[] = {25, 34, 32, 33, 35, 26}; // d-pad GPIO numbers

SPILCD lcd; // my display library

int GetButtons(void)
{
static int iOldState;
int iState, iPressed;
int i, j;

  iState = 0;
  for (i=0; i<BUTT_COUNT; i++) {
      if (digitalRead(ucButtons[i]) == HIGH) // active high
         iState |= (1<<i);
  }
  // Test button bits for which ones changed from LOW to HIGH
  iPressed = (iState ^ iOldState) & iState; // tells us which ones just changed to 1
  iOldState = iState;
  iPressed = iPressed | (iState << 8); // prepare combined state (lower 8 = current, upper 8 = changed)
  return iPressed;
} /* GetButtons() */

void ButtonTest(void)
{
int i, iButtons, iOldButtons = -1;
char szTemp[32];
int iExitPattern = (0x100 << BUTT_UP) | (0x100 << BUTT_LEFT);
const char *szButtons = "UDLRAP";

  spilcdWriteString(&lcd, 0,72,(char *)"Press U+L to exit", 0xff1f, 0, FONT_8x8,DRAW_TO_LCD);

  while (1) {
    iButtons = GetButtons();
    if (iButtons != iOldButtons) {
       for (i=0; i<BUTT_COUNT; i++) {
          szTemp[0] = szButtons[i];
          szTemp[1] = 0;
          spilcdWriteString(&lcd, i*24,40,szTemp, (iButtons & (0x100<<i)) ? 0xf800 :0x1f, 0, FONT_16x16,DRAW_TO_LCD);
       }
       iOldButtons = iButtons;
       if ((iButtons & iExitPattern) == iExitPattern)
          return;
    }
   delay(50);
  } // while (1)
 
} /* ButtonTest() */

void setup() {
  for (int i=0; i<sizeof(ucButtons); i++) {
    pinMode(ucButtons[i], INPUT); // set all push buttons to digital in
  }
  pinMode(Red_LED, OUTPUT);
  pinMode(Green_LED, OUTPUT);
  digitalWrite(Red_LED, HIGH); // show the power off
  digitalWrite(Green_LED, LOW);

   Wire.begin(13, 14);
 
 if(!usb.begin())
  {
    Serial.println("Cannot connect to STUSB4500.");
    Serial.println("Is the board connected? Is the device ID correct?");
    while(1);
  }

 Serial.println("Connected to STUSB4500!");
 delay(100);

 float voltage, current;
 byte lowerTolerance, upperTolerance, pdoNumber;

 pdoNumber = usb.getPdoNumber();
 voltage = usb.getVoltage(3);
 current = usb.getCurrent(3);
 lowerTolerance = usb.getLowerVoltageLimit(3);
 upperTolerance = usb.getUpperVoltageLimit(3);
 usb.setPdoNumber(3);

  Serial.begin(115200);
  delay(1000); // give Serial a little time to start
  spilcdInit(&lcd, LCD_ST7735S_B, FLAGS_INVERT | FLAGS_SWAP_RB, 40000000, TFT_CS, TFT_DC, TFT_RST, TFT_LED, -1, TFT_MOSI, TFT_SCK);
  spilcdSetOrientation(&lcd, LCD_ORIENTATION_90);
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 0,0,(char *)"Mike's Power", 0x7e0, 0, FONT_12x16,DRAW_TO_LCD);
  spilcdWriteString(&lcd, 0,16,(char *)" Controller", 0x7e0, 0, FONT_12x16,DRAW_TO_LCD);
  ButtonTest();
}

const char *szVolts[] = {"    ","XXX  ", "5V  ", "9V  ", "12V ", "15V ", "20V ","    "};
const char *szAmps[] = {"    ","0.25A", "0.5A ", "0.75A", "1.0A ", "1.25A", "1.5A ","    "};

void DrawMenu(int iV, int iA, int iMenuPos)
{
uint16_t usFG1 = (iMenuPos==0) ? 0x1f:0x14;
uint16_t usFG2 = (iMenuPos==1) ? 0x1f:0x14;
uint16_t dim;
 spilcdFill(&lcd, 0, DRAW_TO_LCD);
#define MAX_POS 6
  dim = (usFG1 * 3)>>2; // dim smaller text
  if (szVolts[iV-1][0] != ' ') // some strange drawing error on the first line
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Medium_16, 0, 23, (char *)szVolts[iV-1], dim, 0, 1, DRAW_TO_LCD); //24
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Bold_22, 0, 45, (char *)szVolts[iV], usFG1, 0, 1, DRAW_TO_LCD);   //50
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Medium_16, 0, 65, (char *)szVolts[iV+1], dim, 0, 1, DRAW_TO_LCD); //74
  dim = (usFG2 * 3)>>2; // dim smaller text
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Medium_16, 70, 23, (char *)szAmps[iA-1], dim, 0, 1, DRAW_TO_LCD); //35
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Bold_22, 70, 45, (char *)szAmps[iA], usFG2, 0, 1, DRAW_TO_LCD);   //50
  spilcdWriteStringCustom(&lcd, (GFXfont *)&Orbitron_Medium_16, 70, 65, (char *)szAmps[iA+1], dim, 0, 1, DRAW_TO_LCD); //74
} /* DrawMenu() */
#define MAX_MENU 1

void SetPower(int iP, int iV, int iA)
{
  const float fVolts[] = {0.0f,5.0f,5.0f,9.0f,12.0f,15.0f,20.0f};
  const float fAmps[] = {0.0f,0.25f,0.5f,0.75f,1.0f,1.25f,1.5f};
  digitalWrite(Red_LED, !iP); // show the power on or off
  digitalWrite(Green_LED, iP);

  
  if (iP) {
    usb.softReset();
    usb.setVoltage(3,fVolts[iV]);
    usb.setCurrent(3,fAmps[iA]);
    usb.setLowerVoltageLimit(3,20);
    usb.setUpperVoltageLimit(3,20);
    usb.write();
  } else { // turn power off
    usb.softReset();   //Enables Output
    usb.setPdoNumber(3);    
    usb.write();
  }

} /* SetPower() */
void loop() {
  
  char szTemp[32];  //NEW

  
  int iVolts = 1, iAmps = 1, iMenuPos = 0;
  int iButtons, bChanged = 1;
  int iPower = 0;
  spilcdFill(&lcd, 0, DRAW_TO_LCD);
  spilcdWriteString(&lcd, 0,0,(char *)"Power Controller", 0x7e0, 0, FONT_12x16,DRAW_TO_LCD);
  while (1) {
    iButtons = GetButtons();
    if (iButtons & 0xff00) { // a button was pressed
      if ((iButtons & (0x1<<BUTT_RIGHT)) && iMenuPos < MAX_MENU) {
        iMenuPos++; bChanged = 1;
      }
      if (iButtons & (0x1<<BUTT_UP)) {
        if (iMenuPos == 0 && iVolts > 1) {
          iVolts--; bChanged = 1;
        } else if (iMenuPos == 1 && iAmps > 1) {
          iAmps--; bChanged = 1;
        }
      }
      if (iButtons & (0x1<<BUTT_DOWN)) {
        if (iMenuPos == 0 && iVolts < MAX_POS) {
          iVolts++; bChanged = 1;
        } else if (iMenuPos == 1 && iAmps < MAX_POS) {
          iAmps++; bChanged = 1;
        }
      }
      if ((iButtons & (0x1<<BUTT_LEFT)) && iMenuPos > 0) {
        iMenuPos--; bChanged = 1;
      }
      if (bChanged) {
        bChanged = 0;
        if (iPower) {
          iPower = 0; // turn off power if settings are being changed
          SetPower(iPower, iVolts, iAmps);
        }
        DrawMenu(iVolts, iAmps, iMenuPos);
      } else if (iButtons & (0x1 << BUTT_POWER)) {
        iPower = !iPower;
        SetPower(iPower, iVolts, iAmps);
      }
    } else {
      delay(20); // waste a little time
      spilcdWriteString(&lcd, 0,0,(char *)" Voltage  Current", 0x7e0, 0, FONT_8x8,DRAW_TO_LCD); //Top
      
      spilcdWriteString(&lcd, 0,72,(char *)" Current Draw=", 0x7e0, 0, FONT_8x8,DRAW_TO_LCD);    //Bottom
      ADC_VALUE = analogRead(Analog_channel_pin);
      sprintf(szTemp, "%d", ADC_VALUE);
      spilcdWriteString(&lcd, 120,72,szTemp, 0x7e0,0, FONT_8x8,DRAW_TO_LCD);
    }
  }; // while (1)
} /* loop() */
