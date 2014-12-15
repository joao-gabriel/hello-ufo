#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define SHIP_W 8
#define SHIP_H 8

static unsigned char PROGMEM ship_bmp[] =
{ B00011000, 
  B00100100,
  B01000010, 
  B01011010,
  B10111101, 
  B10011001,
  B10111101, 
  B11100111 };
  
static unsigned char PROGMEM ufo_bmp[] =
{ B00000000, 
  B00000000,
  B00111100, 
  B01011010,
  B11100111, 
  B01011010,
  B00111100, 
  B00000000 };
  
static unsigned char PROGMEM shot_bmp[] =
{ B0110, 
  B0110,
  B0110, 
  B0110,
  B1001 };  
  

// Pinos dos botoes  
const int leftBtnPin = 2;
const int rightBtnPin = 8;
const int midBtnPin = 9;

// Variaveis do jogo
int ship_x = 10;

int shot_y = 0;
int shot_x = 0;

int ufo_x = 0;
int ufo_y = 0;
int ufo_dir = 1;
float ufo_vel = 1;
int ufo_y_delay = 8;
int change_dir = 100;

int lives = 3;
int score = 0;

// Reset
void(* resetFunc) (void) = 0;

void setup()   {
  
  pinMode(leftBtnPin, INPUT);
  pinMode(rightBtnPin, INPUT);
  pinMode(midBtnPin, INPUT);
  
  Serial.begin(9600);
  display.begin();
  display.setContrast(55);

  // text display 
  display.setTextSize(1);
  display.setTextColor(BLACK);

}


void loop(){
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(score);    

  if (digitalRead(leftBtnPin) == HIGH  && ship_x > 0) {   
    ship_x--;
  } 

  if (digitalRead(rightBtnPin) == HIGH && ship_x < 76) {   
    ship_x++;
  } 

  // Verifica se atira
  if (digitalRead(midBtnPin) == HIGH && shot_y == 0){
    shot_y = 32;
    shot_x = ship_x;
  }
  
  // Verifica se desenha o tiro
  if (shot_y > 0){  
    display.drawBitmap(shot_x, shot_y, shot_bmp, 8, 5, 1);
    shot_y--;
  }
  
  // Verifica se o tiro acertou o UFO
  if (
      shot_y>0 && 
      (ufo_y >= shot_y && ufo_y <= (shot_y+5)) &&
      (ufo_x >= shot_x && ufo_x <= (shot_x+4))
      ){
    score = score + (50-shot_y);
    randomSeed(shot_y);
    ufo_x = random(76);
    ufo_y = 0;
    shot_y = 0;
    ufo_vel = ufo_vel + 0.1;
    if (change_dir > 10){
      change_dir -= 5;
    }
  }
  
  // Alcançando os limites da tela ou em uma chance em [valor em change_dir] o UFO muda de direçao
  ufo_x = ufo_x + (int)ufo_vel * ufo_dir;
  if (ufo_x>76 || ufo_x < 0 || (random(change_dir) == 1)){
     ufo_dir = ufo_dir * -1;
   }
   
   // a cada 8 pixels horizontais o UFO chega mais perto
   ufo_y_delay--;
   if (ufo_y_delay<1){
     ufo_y++;
     ufo_y_delay = 8;  
   }
   if (ufo_y>48)
     ufo_y = 0;
   
   
  // Verifica colisoes entre a nave e o UFO
  if (ufo_y > 32 && (ufo_x >= ship_x && ufo_x < ship_x + SHIP_W)){
    lives--;
    if (lives == 0){
      display.clearDisplay();
      display.setCursor(13,13);
      display.println("GAME OVER");
      display.print(" SCORE: ");
      display.println(score);
      display.display(); 
      while(digitalRead(midBtnPin) == LOW){
      }
      resetFunc();
    }
    ufo_y = 0;
    display.clearDisplay();
    display.setCursor(13,13);
    display.print("LIVES: ");
    display.println(lives);
    display.display();
    delay(2000);
  }
  
  display.drawBitmap(ufo_x, ufo_y,  ufo_bmp, SHIP_W, SHIP_H, 1);
  
  display.drawPixel(random(83), random(47), 1);
  
  // ship display
  display.drawBitmap(ship_x, 40,  ship_bmp, SHIP_W, SHIP_H, 1);
  display.display();  
  delay(16);
  
}





