#include <FastLED.h>

#define BLYNK_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(A0, A1); // RX, TX

#define BLYNK_PRINT DebugSerial
#include <BlynkSimpleSerialBLE.h>

char auth[] = "f795e337afbc4d0cba9ffbb03ef44668";

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN1    5
#define DATA_PIN2    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

#define PIN_8 13
#define PIN_7 12
#define PIN_6 11
#define PIN_5 10
#define PIN_4 A4
#define PIN_3 A5
#define PIN_2 9
#define PIN_1 8

#define READ_PIN 7
#define READ_SELECT_A 2 
#define READ_SELECT_B 3
#define READ_SELECT_C 4

#define COLOR_RED 255
#define COLOR_BLUE 128
#define COLOR_GREEN 100

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  300

#define ANIMATION_DURATION 200


void animate_lines(boolean diagonal);
void animate_random();
void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN1,COLOR_ORDER>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN2,COLOR_ORDER>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  DebugSerial.begin(9600);
  DebugSerial.println("Waiting for connections...");
  
  Serial.begin(9600);  
  Serial.println("--- Start Serial Monitor SEND_RCVE ---");

  pinMode(PIN_8, OUTPUT);
  pinMode(PIN_7, OUTPUT);
  pinMode(PIN_6, OUTPUT);
  pinMode(PIN_5, OUTPUT);
  pinMode(PIN_4, OUTPUT);
  pinMode(PIN_3, OUTPUT);
  pinMode(PIN_2, OUTPUT);
  pinMode(PIN_1, OUTPUT);

  pinMode(READ_SELECT_A, OUTPUT);
  pinMode(READ_SELECT_B, OUTPUT);
  pinMode(READ_SELECT_C, OUTPUT);

  pinMode(READ_PIN, INPUT_PULLUP);

  randomSeed(42);   
  
  Serial.println("--- Start Blynk ---");
  Blynk.begin(DebugSerial, auth);
  Serial.println("--- Started Blynk ---");
}

bool animate = true;

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int pos = 0;

int loaded = 0;
int state = 0;
boolean sensorTestInit = false;

void setLed(String name, int color, int fadeAmt);
void setSquareColor(String locX, int locY, int color, int fadeAmt);
void drawTeams(boolean showArea);

BLYNK_WRITE(V1) // Enable SensorTest
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println(pinValue);
  DebugSerial.println(pinValue);
  loaded = 0;
  if(pinValue) {
    state = 1;
  } else {
    state = 0;
  }
}

void loop()
{
  if(animate && loaded < ANIMATION_DURATION){
    if(loaded < ANIMATION_DURATION/2)  animate_lines(true);
    else animate_random();
    loaded ++;
    delay(10);
  }
  else {
    switch(state){
      case 1:
        sensorTestInit = false;
        sensorTest(); break;
      default:
        play_game();
    }
    
    //
  }
  
  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  
  // periodic updates
  EVERY_N_MILLISECONDS( 20 ) { 
    gHue++; 
  }

  Blynk.run();
}

int pieceMoved = 0;

void play_game(){
  fadeToBlackBy( leds1, NUM_LEDS, 255);
  fadeToBlackBy( leds2, NUM_LEDS, 255);

  drawTeams(false);

  for(int i=1;i<=8;i++){
    demuxChessBoardColumnReader(i);
  }
  
}

void sensorTest() {
  int sensorArray[8][8];
  if(!sensorTestInit){
   for(int i=0;i<8;i++){
      for(int j=0;j<8;j++){
        setSquareColor('A'+i, j+1, COLOR_RED, 0);
      }
    }
    sensorTestInit = true;
  } else {
    for(int x=1;x<=8;x++) {
      demuxChessBoardColumnReader(x);
    }
  }
}

int* demuxChessBoardColumnReader(int row) {
  int A, B, C = 0;
  int totalChannels = 8;
  int output[8]; 

  digitalWrite(PIN_8, (8==row?HIGH:LOW) );
  digitalWrite(PIN_7, (7==row?HIGH:LOW) );
  digitalWrite(PIN_6, (6==row?HIGH:LOW) );
  digitalWrite(PIN_5, (5==row?HIGH:LOW) );
  digitalWrite(PIN_4, (4==row?HIGH:LOW) );
  digitalWrite(PIN_3, (3==row?HIGH:LOW) );
  digitalWrite(PIN_2, (2==row?HIGH:LOW) );
  digitalWrite(PIN_1, (1==row?HIGH:LOW) );
  
  //Serial.println("Wrote True at "+String(C)+","+String(B)+","+String(A)+".");
  //delay(10);
  for(int i=0; i<totalChannels; i++){
    A = bitRead(i,0); //Take first bit from binary value of i channel.
    B = bitRead(i,1); //Take second bit from binary value of i channel.
    C = bitRead(i,2); //Take third bit from value of i channel.

    digitalWrite(READ_SELECT_A, A);
    digitalWrite(READ_SELECT_B, B);
    digitalWrite(READ_SELECT_C, C);

    //Serial.println("Reading value at "+String(C)+","+String(B)+","+String(A)+": " + String(digitalRead(READ_PIN)));
    
    pieceMoved = 0;
    output[i] = 0;
    if(digitalRead(READ_PIN)==HIGH){
        setSquareColor('A'+i, row, COLOR_GREEN, 0);
        output[i] = 1;
        //Serial.println("Read value at "+String(C)+","+String(B)+","+String(A)+": " + String(digitalRead(READ_PIN)));
    }
  }
  delay(10);
  return output;
}

void drawTeams(boolean showArea){
  for (int i=0;i<8;i++){
    setSquareColor('A'+i, 1, 128, 0);
    setSquareColor('A'+i, 2, 128, 0);
    if(showArea) {
      setSquareColor('A'+i, 3, 128, 0);
      setSquareColor('A'+i, 4, 128, 0);
      setSquareColor('A'+i, 5, 255, 0);
      setSquareColor('A'+i, 6, 255, 0);
    }
    setSquareColor('A'+i, 7, 255, 0);
    setSquareColor('A'+i, 8, 255, 0);
  }
}

void animate_random(){
  char randomX = char(random(0,8)+'A');
  int randomY = random(0,8)+1;
  uint8_t color = random(0,225);

  fadeToBlackBy( leds1, NUM_LEDS, 20);
  fadeToBlackBy( leds2, NUM_LEDS, 20);
  
  setSquareColor(randomX, randomY, color, 0);
}

void animate_lines(boolean diagonal){
  fadeToBlackBy( leds1, NUM_LEDS, 150);
  fadeToBlackBy( leds2, NUM_LEDS, 150);

  int incr=1;
  for(int i=0;i<8;i++)
  {
    setSquareColor('A',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('B',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('C',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('D',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('E',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('F',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('G',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
    setSquareColor('H',(pos+incr)%8+1,gHue,0);diagonal?incr++:incr;
  }
  pos++;
  if(pos>8) pos = 0;
}

void setSquareColor(char locX, int locY, int color, int fadeAmt) {
  // L : ABCD
  // R : EFGH
  //Serial.println("Location:" + String(locX)+String(locY));
  char side = ' ';
  int offset = 0;
  boolean flip = false;
  switch(locX){
    case 'A':
      offset = 45; side = 'L'; flip = true;  break;
    case 'B':
      offset = 30; side = 'L'; flip = false; break;      
    case 'C':
      offset = 15; side = 'L'; flip = true; break;
    case 'D':
      offset = 0; side = 'L'; flip = false; break;
    case 'E':
      offset = 0; side = 'R'; flip = false; break;
    case 'F':
      offset = 15; side = 'R'; flip = true; break;
    case 'G':
      offset = 30; side = 'R'; flip = false; break;
    case 'H':
      offset = 45; side = 'R'; flip = true; break;
    break;    
  }
  int ledId1 = offset+2*(flip?8-locY:locY-1);
  setLed(String(side)+String(ledId1), color, fadeAmt);

  switch(ledId1){
    case 0: 
    case 29:
    case 30:
    case 59:
      break;
    default:
      int ledId2 = flip?ledId1+1:ledId1-1;
      setLed(String(side)+String(ledId2), color, fadeAmt);
  }
  
}

void setLed(String name, int color, int fadeAmt) {
    char ledSet = name.startsWith("L")?'L':'R';
    int i = name.substring(1).toInt();
    int value = 255;
    switch(i){
      case 0:
      case 29:
      case 30:
      case 59:
        value = 255;
        break;
      default:
        value = 200;
        break;
    }
    switch(ledSet){
      case 'L':
        leds1[i] = CHSV( color, 255, value);
        leds1[i].fadeLightBy( fadeAmt );
        break;
      case 'R':
        leds2[i] = CHSV( color, 255, value); 
        leds1[i].fadeLightBy( fadeAmt );
        break;
    }
    
}
