#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <EEPROM.h>

// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

#define SHIP_W 8
#define SHIP_H 8

// Bitmaps
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
  

// button pins  
const int leftBtnPin = 2;
const int rightBtnPin = 8;
const int midBtnPin = 9;

// game variables
int ship_x = 10;

int shot_y = 0;
int shot_x = 0;

int ufo_x = 0;
int ufo_y = 0;
int ufo_dir = 1;
float ufo_vel = 1;
int ufo_y_delay = 8;
int change_dir = 50;

int lives = 3;
int score = 0;
int best = 0;

boolean started = false;

// Reset Function (send user to address 0 - start of program code in memory)
void(* resetFunc) (void) = 0;

// Sets up the hardware and text font
void setup()   {
  
  // Buttons
  pinMode(leftBtnPin, INPUT);
  pinMode(rightBtnPin, INPUT);
  pinMode(midBtnPin, INPUT);
  
  // Display
  Serial.begin(9600);
  display.begin();
  display.setContrast(55);

  // Font size and color (could only be black for 5110 LCD)
  display.setTextSize(1);
  display.setTextColor(BLACK);

}


void loop(){
  
  // Clear the screen
  display.clearDisplay();
  
  // Check if the game has started
  if (!started){
      // If not, shows the splashscreen and the highscore
      display.setCursor(13,13);
      display.println("HELLO UFO");
      display.print("  BEST:");
      
      // Read highscore from EEPROM (need to be converted from 2 bytes to Integer) 
      byte lowByte = EEPROM.read(0);
      byte highByte = EEPROM.read(1);
      best =  ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
      
      display.println(best);
      display.display(); 
      delay(2000);
      while(digitalRead(midBtnPin) == LOW){
      } 
      started = true;
  } 
    
  // Show score at top left corner of display
  display.setCursor(0,0);
  display.println(score);    

  // Check if left button was pressed and the ship is within display limits
  if (digitalRead(leftBtnPin) == HIGH  && ship_x > 0) {
    
    // Decrease Ship X coordinate  
    ship_x--;
  } 

  // Check if right button was pressed and the ship is within display limits
  if (digitalRead(rightBtnPin) == HIGH && ship_x < 76) { 
    
    // Increase Ship X coordinate  
    ship_x++;
  } 

  // Check if shot button was pressed and if there is no other shot in the screen
  if (digitalRead(midBtnPin) == HIGH && shot_y == 0){
    
    // Locate the shot Y coordinate just above the ship
    shot_y = 32;
    
    // Set the shot X coodinate as the same as the ship
    shot_x = ship_x;
    
    // Beep!
    tone(12, 440, 250);
  }
  
  // Check if the shot has to be drawn (shot Y coordinate is not 0)
  if (shot_y > 0){  
    
    // Draw to shot at the framebuffer
    display.drawBitmap(shot_x, shot_y, shot_bmp, 8, 5, 1);
    
    // Decrease shot Y coordinate making it move upwards 
    shot_y--;
  }
  
  // Check if the shot hit the UFO
  if (
      shot_y>0 &&                                  // Shot is being displayed? (this is necessary because the UFO could be at 0 Y coordinate)
      (ufo_y >= shot_y && ufo_y <= (shot_y+5)) &&  // UFOs and shot Y coordinates + height intersect?
      (ufo_x >= shot_x && ufo_x <= (shot_x+4))     // UFOs and shot X coordinates + width intersect?
      ){
        
    // Increase the score by 50 points minus the Y coordinate of the UFO
    // Nearer the top equals more points
    score = score + (50-shot_y);
    
    // Initialize the random number generator with the Y coordinate of the UFO at
    // the moment it was shot (just to keep things "really random")
    randomSeed(shot_y);
    
    // Set the UFO X coordinate at some random point between 0 and 76
    ufo_x = random(76);
    
    // Set the UFO Y coordinate to the top of the screen
    ufo_y = 0;
    
    // Set the shot Y coordinate to 0 which disables it
    shot_y = 0;
    
    // Increase UFO horizontal speed in steps of ten
    ufo_vel = ufo_vel + 0.1;
    
    // If the chances of the UFO to change direction are greater than 2
    if (change_dir > 2){
      
      // Decrease this chance 
      change_dir -= 1;
    }
    
    tone(12, 1000, 250);
    
  }
  
  // Refresh UFO X coordinate, considering its direction and horizontal speed
  ufo_x = ufo_x + (int)ufo_vel * ufo_dir;

  // By reaching the display bounderies or in one chance in [value in change_dir variable] the UFO change its horizontal direction
  if (ufo_x>76 || ufo_x < 0 || (random(change_dir) == 1)){
     ufo_dir = ufo_dir * -1;
   }
   
   // Each 8 horizontal pixels the UFO gets closer to the bottom
   ufo_y_delay--;
   if (ufo_y_delay<1){
     ufo_y++;
     ufo_y_delay = 8;  
   }
   if (ufo_y>48)
     ufo_y = 0;
   
   
  // Check collisions between the ship and the UFO OR if the UFO got passed the ship
  if ((ufo_y > 32 && (ufo_x >= ship_x && ufo_x < ship_x + SHIP_W)) || (ufo_y > 40)){
    lives--;
    tone(12, 100, 500);
    if (lives == 0){
      
      // Check if the score is higher than the best
      if (score > best){
        // If it is, save it to the EEPROM (need to be converted from 2 bytes to Integer) 
        byte lowByte = ((score >> 0) & 0xFF);
        byte highByte = ((score >> 8) & 0xFF);
        EEPROM.write(0, lowByte);
        EEPROM.write(1, highByte);       
      }
      
      display.clearDisplay();
      display.setCursor(13,13);
      display.println("GAME OVER");
      display.print("  SCORE:");
      display.println(score);
      display.display(); 
      delay(2000);
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
  
  // ship display
  display.drawBitmap(ship_x, 40,  ship_bmp, SHIP_W, SHIP_H, 1);
  display.display();  
  delay(16);
  
}





