 
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#define TFT_CS    10
#define TFT_RST   6 
#define TFT_DC    7 

#define TFT_SCLK 13   
#define TFT_MOSI 11  
#include <avr/pgmspace.h>
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST); 
int rows[8] = { 14, 15, 16, 17, 18, 19, 20, 21 };
int cols[8] = { 6, 7, 8, 9, 10, 11, 12, 13 };
int x=0;
void setup() {
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST77XX_BLACK);
  Serial.begin(9600); 
    for(int j=0;j<8;j++)
  {
    pinMode(rows[j],OUTPUT);
    pinMode(cols[j],OUTPUT);
  }

  pinMode(2,INPUT_PULLUP);
  
  
  attachInterrupt(2,displaymic,LOW);

}

void displaymic()
{
  tft.setCursor(2,2);
  tft.setTextSize(5);
  tft.print("&!");
  delay(100);
}


void loop() {
  
}
