#include "FastLED.h"

#define NUM_LEDS 64
#define DATA_PIN 3
#define DATA_PIN2 5
#define DATA_PIN3 6
#define DATA_PIN4 7

#define JOYSTICKX A0
#define JOYSTICKY A1

/*class SingleScreen
  {
  int screenPos;
  public : int ref [1];
  public : int references [7];

  public:
  SingleScreen(int pos)
  {
    screenPos = pos;
  }

  void instantiate()
  {
    if(screenPos == 1)
    {

        for(int i = 7; i > 0; i--)
        {
          references[i] = ref[0] = i;
          references[i] = ref[1] = 0;
        }
    }
  }
  };*/

CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

int timer;
int ding;
int x;
int y;
int prevX, prevY;
int height;
int width;
boolean vdown;
boolean hleft;

float grav;
float realY;
float realX;
float yVec;
float xVec;
float spd;
float maxHspeed = 1;
float maxVspeed = 1;
float friction = 0.01;
float deacc = 200;

//Determines what screen is drawn on
int currentScreen = 1;

//For Joystick
int range = 12;
int threshold = range / 4;
int center = range / 2;


void lightPixels(int xx, int yy, int prevX, int prevY) { //Denne funktion tager 2 ints, som agerer koordinater i beregningerne. Hver anden vertikale linje
  //skal "vendes om", da boardet læser fra venstre mod højre på hveranden linje.

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
    leds2[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
    leds3[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
    leds4[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
  }


  //SCREEN 1
  if (currentScreen == 1) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds2[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
      leds3[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
      leds4[i] = CRGB(0, 0, 10); //Farv alle ikke-relevante pixels sorte
    }
    if (yy % 2 == 0) { //hvis du er på en ulige linje, så byt x-aksen om
      leds[(yy * 8) + xx] = CRGB(20, 0, 0);
    }
    else {
      leds[(yy * 8) + (7 - xx)] = CRGB(20, 0, 0);
    }
  }

  //SCREEN 2
  if (currentScreen == 2) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 10);
      leds3[i] = CRGB(0, 0, 10);
      leds4[i] = CRGB(0, 0, 10);
    }
    if (yy % 2 == 0) {
      leds2[(yy * 8) + xx] = CRGB(20, 0, 0);
    }
    else {
      leds2[(yy * 8) + (7 - xx)] = CRGB(20, 0, 0);
    }
  }

  //SCREEN 3
  if (currentScreen == 3) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds2[i] = CRGB(0, 0, 10);
      leds[i] = CRGB(0, 0, 10);
      leds4[i] = CRGB(0, 0, 10);
    }
    if (yy % 2 == 0) {
      leds3[(yy * 8) + xx] = CRGB(20, 0, 0);
    }
    else {
      leds3[(yy * 8) + (7 - xx)] = CRGB(20, 0, 0);
    }
  }

  //SCREEN 4
  if (currentScreen == 4) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds2[i] = CRGB(0, 0, 10);
      leds3[i] = CRGB(0, 0, 10);
      leds[i] = CRGB(0, 0, 10);
    }
    if (yy % 2 == 0) {
      leds4[(yy * 8) + xx] = CRGB(20, 0, 0);
    }
    else {
      leds4[(yy * 8) + (7 - xx)] = CRGB(20, 0, 0);
    }
  }
}

int readAxis(int thisAxis) {
  // read the analog input:
  int reading = analogRead(thisAxis);

  // map the reading from the analog input range to the output range:
  reading = map(reading, 0, 1023, 0, range);

  // if the output reading is outside from the rest position threshold, use it:
  int distance = reading - center;

  if (abs(distance) < threshold) {
    distance = 0;
  }

  // return the distance for this axis:
  return distance;
}

void setup() {
  height = 7; //mængden af horisontale pixels
  width = 7; //mængden af vertikale pixels
  x = 3; //x-pixel, fra venstre
  y = 2; //y-pixel, fra toppen
  prevX = 0;
  prevY = 0;

  //grav = 0.05; //tyngdekraft
  realX = 3; //den "ægte" x. x og y rundes ned eller op til det nærmeste heltal fra realX og realY for at fortælle, hvilken pixel, der skal lyses op
  realY = 2; //den "ægte" y.
  yVec = 0; //den vertikale vector
  xVec = 0; //den horisontale vector

  //SingleScreen ss(1);
  //ss.instantiate();

  Serial.begin(9600);
  // put your setup code here, to run once:
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN2>(leds2, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN3>(leds3, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN4>(leds4, NUM_LEDS);

  timer = ding + 30;
}

void loop() {
  lightPixels(x, y, prevX, prevY);

  float xReading = -readAxis(A0);
  float yReading = readAxis(A1);

  //Serial.println(xReading);
  //HORISONTAL MOVEMENT
  xVec += xReading / deacc;
  realX += xVec;
  if (realX >= width)
  {
    if (currentScreen == 4)
    {
      realX = width;
      xVec = -xVec;
    }
    else
    {
      currentScreen += 1;
      realX = 1;
      x = 1;
      Serial.print("Current Screen: ");
      Serial.println(currentScreen);
      delay(200);
    }
  }

  if (realX <= 0)
  {
    if (currentScreen == 1)
    {
      realX = 0;
      xVec = -xVec;
    }
    else
    {
      currentScreen -= 1;
      realX = 6;
      x = 6;
      Serial.print("Current Screen: ");
      Serial.println(currentScreen);
      delay(200);
    }
  }
  Serial.println(realX);

  //VERTICAL MOVEMENt
  yVec += yReading / deacc;
  realY += yVec;

  if (realY >= height)
  {
    realY = height;
    yVec = -yVec;
  }
  if (realY <= 0)
  {
    realY = 0;
    yVec = -yVec;
  }

  x = round(realX);
  y = ceil(realY);

  // FRICTION
  if (xVec > 0.05) {
    xVec -= friction;
  }
  if (xVec < -0.05) {
    xVec += friction;
  }
  if (yVec > 0.05) {
    yVec -= friction;
  }
  if (yVec < -0.05) {
    yVec += friction;
  }
  FastLED.show();
  delay(20);
}




