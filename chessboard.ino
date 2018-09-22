#include <FastLED.h>

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

#define PIN_A 10 
#define PIN_B 11

#define PIN_1 13
#define PIN_4 12

#define READ_PIN 7
#define READ_SELECT_A 2 
#define READ_SELECT_B 3
#define READ_SELECT_C 4


#define WRITE_PIN 8
#define WRITE_SELECT_A 9 
#define WRITE_SELECT_B 10
#define WRITE_SELECT_C 11

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


  Serial.begin(115200);  
  Serial.println("--- Start Serial Monitor SEND_RCVE ---");

  pinMode(PIN_1, OUTPUT);
  pinMode(PIN_4, OUTPUT);

  pinMode(READ_SELECT_A, OUTPUT);
  pinMode(READ_SELECT_B, OUTPUT);
  pinMode(READ_SELECT_C, OUTPUT);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  pinMode(READ_PIN, INPUT_PULLUP);

  randomSeed(42);   

}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int pos = 0;

int loaded = 0;

void setLed(String name, int color);
void setSquareColor(String loc, int color);

void loop()
{
  if(loaded < ANIMATION_DURATION){
    if(loaded < ANIMATION_DURATION/2)  animate_lines(true);
    else animate_random();
    loaded ++;
    delay(10);
  }
  else {
    play_game();
  }
  
  //Serial.println("Location : " + String(randomX)+String(randomY));
  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { 
    gHue++; 
  }
}

boolean pieceMoved = false;

void play_game(){
  fadeToBlackBy( leds1, NUM_LEDS, 255);
  fadeToBlackBy( leds2, NUM_LEDS, 255);

  for (int i=0;i<8;i++){
    setSquareColor('A'+i, 1, 128);
    if(!pieceMoved || i!=1) setSquareColor('A'+i, 2, 128);

    setSquareColor('A'+i, 7, 255);
    setSquareColor('A'+i, 8, 255);
  }
   
  //chessBoardRowReader(1);
  //chessBoardRowReader(4);

  demuxChessBoardColumnReader(1);
  demuxChessBoardColumnReader(4);
  
}

void demuxChessBoardColumnReader(int row) {
  int A, B, C = 0;
  int totalChannels = 8;

  int newRow = row-1;
  A = bitRead(newRow,0); //Take first bit from binary value of i channel.
  B = bitRead(newRow,1); //Take second bit from binary value of i channel.
  C = bitRead(newRow,2); //Take third bit from value of i channel.

  digitalWrite(PIN_1, (1==row?HIGH:LOW) );
  digitalWrite(PIN_4, (4==row?HIGH:LOW) );
  
  Serial.println("Wrote True at "+String(C)+","+String(B)+","+String(A)+".");
  delay(10);
  for(int i=0; i<totalChannels; i++){
    A = bitRead(i,0); //Take first bit from binary value of i channel.
    B = bitRead(i,1); //Take second bit from binary value of i channel.
    C = bitRead(i,2); //Take third bit from value of i channel.

    digitalWrite(READ_SELECT_A, A);
    digitalWrite(READ_SELECT_B, B);
    digitalWrite(READ_SELECT_C, C);

    Serial.println("Read value at "+String(C)+","+String(B)+","+String(A)+": " + String(digitalRead(READ_PIN)));
    if(digitalRead(READ_PIN)==HIGH){
        setSquareColor('A'+i, row, 100);
    }
  }
  delay(100);
}


void chessBoardRowReader(int row){
  
  digitalWrite(PIN_1, (1==row?HIGH:LOW) );
  digitalWrite(PIN_4, (4==row?HIGH:LOW) );
  
  Serial.println("Row : " + String(row) + ", Values : [ A : " + String(analogRead(PIN_A)) + "(" + String(digitalRead(PIN_A)==HIGH) +"), B : "+ String(analogRead(PIN_B)) + "(" + String(digitalRead(PIN_B)==HIGH) +")]\n");
  if(digitalRead(PIN_A)==HIGH){
      setSquareColor('A', row, 100);
  }
  if(digitalRead(PIN_B)==HIGH){
      pieceMoved = true;
      //Serial.println("move: B-"+String(2)+" to B-"+row);
      setSquareColor('B', row, row==4?128:100);
  } else {
    pieceMoved = false;
  }
  delay(100);
}

void animate_random(){
  char randomX = char(random(0,8)+'A');
  int randomY = random(0,8)+1;
  uint8_t color = random(0,225);

  fadeToBlackBy( leds1, NUM_LEDS, 20);
  fadeToBlackBy( leds2, NUM_LEDS, 20);
  
  setSquareColor(randomX, randomY, color);
}

void animate_lines(boolean diagonal){
  fadeToBlackBy( leds1, NUM_LEDS, 150);
  fadeToBlackBy( leds2, NUM_LEDS, 150);

  int incr=1;
  for(int i=0;i<8;i++)
  {
    setSquareColor('A',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('B',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('C',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('D',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('E',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('F',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('G',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
    setSquareColor('H',(pos+incr)%8+1,gHue);diagonal?incr++:incr;
  }
  pos++;
  if(pos>8) pos = 0;
}

void setSquareColor(char locX, int locY, int color) {
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
  setLed(String(side)+String(ledId1), color);

  switch(ledId1){
    case 0: 
    case 29:
    case 30:
    case 59:
      break;
    default:
      int ledId2 = flip?ledId1+1:ledId1-1;
      setLed(String(side)+String(ledId2), color);
  }
  
}

void setLed(String name, int color) {
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
        break;
      case 'R':
        leds2[i] = CHSV( color, 255, value); 
        break;
    }
    
}
