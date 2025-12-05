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
String li[] PROGMEM= {" 103484717", "Wali Ikram Protick"};
String buf[8];
const char std_id[] PROGMEM = {"103484717"};
const char std_name[]PROGMEM = {"Wali Ikram Protick"};

char c_ar[100];
void setup()
{
  
  Serial.begin(9600);
  buf[0] = constro(std_id);
  buf[1]= constro(std_name);
  

  tft.initR(INITR_BLACKTAB); 
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  Serial.println(buf[0]);
}
String constro(const char chh[])
{
  String str;
  for(int i=0;i<strlen(chh);i++)
  {
    char c=pgm_read_byte_near(chh+i);
    str+=c;
    Serial.println(c);
  }
  return str;
}
void loop() 
{
  
 for(int i=0;i<(sizeof(buf)/sizeof(String));i++){
    for (int j=0;j<=(sizeof(buf)/sizeof(String))*(sizeof(buf)/sizeof(String));j++)
    {
     tft.fillScreen(ST77XX_BLACK);

      tft.setCursor(128-j,0);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(buf[i]);
      
      delay(200);
    }
    delay(100);
 }
  
}