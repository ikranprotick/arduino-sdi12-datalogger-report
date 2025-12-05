//Required Arduino Library
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"  //Library for BME Sensor
#include <BH1750.h>           //Library for digital light sensor
#include <DueTimer.h>

//SD card libraries - all libraries required for sd card functionality
#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <RingBuf.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

//Include library for clock
#include <RTClib.h>

//create an instance of RTC
RTC_DS1307 rtc;


SdFs sd;
FsFile file;
//^ configuration for FAT16/FAT32 and exFAT.

// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = A3;
//
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 12;
const uint8_t SOFT_MOSI_PIN = 11;
const uint8_t SOFT_SCK_PIN  = 13;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)

//define sea level pressure for BME Sensor
#define SEALEVELPRESSURE_HPA (1013.25)

//TFT LCD Display library
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735

BH1750 lightMeter(0x23);  // Initialisation and declaration for digital light sensor
Adafruit_BME680 bme;      // I2C initialisation and declaration for BME Sensor

//TFT LCD Declaration
#define TFT_CS 10
#define TFT_RST 6
#define TFT_DC 7
#define TFT_SCLK 13
#define TFT_MOSI 11
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//SDI-12 Setup
#define DIRO 7

String command;
int deviceAddress = 0;
String deviceIdentification = "allccccccccmmmmmmvvvxxx";

const int pushButtons[4] = { 5, 4, 3, 2 };
int startTime, currentTime, duration, zeroCount;
float temperature, pressure, humidity, gasResist, lux;
bool dataReady;
bool graphState = false;
bool setScale = false;
DateTime CurrentTime;
String outputMessage, tempAddress;
bool sdRun = false;
//bool storedData ==true
int sdInterrupt = 1;
float prevValue = 0.0;
bool dataOutput[2] = { false, false };

int selection[2] = { 1, 1 };
int currentMenu = 0;  // 0 - idle Menu, 1 - Main menu, 2 - Sensor Menu, 3 - SD card menu, 4 - no menu, graphical display

//Debounce counters
static unsigned long last_interrupt_time = 0;
unsigned long interrupt_time = 0;

float interrupt_counter = 0;

int counter_for_timer_3 = 0; // counter for continous data read command
bool timer_flag = false;     // flag for continuous data read command

//Function prototypes
void SDI12_Command(String input);
void SDI12_Output(String message);
void get_Sensor_Data();
void Continuous();
void store_Data();
void Read_SD(File file);
void home_Menu();
void main_Menu();
void sensor_Menu();
void sd_Card_Menu();
void up_scroll();
void down_scroll();
void select_Button();
void back_Button();
void select_menu();
void select_sensor();
void temperature_Graph();
void pressure_Graph();
void humidty_Graph();
void AQ_graph();
void light_Graph();



void Continuous() {    
     timer_flag = true;      
    counter_for_timer_3++;
    Serial.print("Timer Interrupt: ");
    Serial.println(counter_for_timer_3);
    if (counter_for_timer_3 >= 10) {
        Timer3.stop();
        counter_for_timer_3 = 0;
        Serial.println("Timer stopped");
    }
}


void setup() {
  Serial.begin(9600);
  Serial.flush();

  // ================ SDI-12 ================
  Serial1.begin(1200, SERIAL_7E1);  //SDI-12 UART, configures serial port for 7 data bits, even parity, and 1 stop bit
  pinMode(DIRO, OUTPUT);            //DIRO Pin

  //HIGH to Receive from SDI-12
  digitalWrite(DIRO, HIGH);

  SDI12_Output("Iniatilising and Setting up SDI 12 connection and sensors");
  Wire.begin();  // Initialize the I2C bus (BH1750 library)

  //check if RTC is connected
  if (!rtc.begin()) {
    SDI12_Output("RTC NOT CONNECTED");
    while (1);
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));  //Adjust rtc to time sketch was uploaded
  // Check if RTC is running
  if (!rtc.isrunning()) {
    SDI12_Output("RTC is not running");
  }

  // ================ BH1750 Sensor ================
  SDI12_Output(F("BH1750 Test begin"));
  lightMeter.begin();

  // ================ BME680 ================
  SDI12_Output(F("BME680 Test begin"));
  if (!bme.begin(0x76)) {
    SDI12_Output("Could not find a valid BME680 sensor, check wiring!");
   while (1);
  }
  // Set the temperature, pressure and humidity oversampling
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_8X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320*C for 150 ms

  
  //Initalise and set up SD card
  if (!sd.begin(SD_CONFIG)) {
    SDI12_Output("SD card initialization failed!");
    sd.initErrorHalt();
    while (1);
  }

  //Open/Create file for writing/reading
  if (!file.open("sensorData.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }
  file.close(); //release file


  //Set the buttons for input with resistance to reduce debounce
  for (int i = 0; i < 3; i++) {
    pinMode(pushButtons[i], INPUT_PULLUP);
  }

  //Initialising tft display
  tft.initR(INITR_BLACKTAB);  // Init ST7735S chip, black tab
  tft.setRotation(3);         //rotate screen to landscape
  tft.fillScreen(ST77XX_BLACK);


//Hardware interrupt for menu buttons
  attachInterrupt(digitalPinToInterrupt(pushButtons[0]), up_scroll , RISING);
  attachInterrupt(digitalPinToInterrupt(pushButtons[1]), down_scroll , RISING);
  attachInterrupt(digitalPinToInterrupt(pushButtons[2]), select_Button , RISING);
  attachInterrupt(digitalPinToInterrupt(pushButtons[3]), back_Button , RISING);

  //Timer interrupt for data logger
  Timer3.attachInterrupt(store_Data);

  SDI12_Output("Setup complete. SDI 12 ready for commands");

  //Print the IDLE Menu startup
  home_Menu();
}

void loop() {
  int byte;
  //Receive SDI-12 over UART and then print to Serial Monitor
  if (Serial1.available()) {
    byte = Serial1.read();  //Reads incoming communication in bytes
    //Serial.println(byte);
    if (byte == 33) {  //If byte is command terminator (!)
      startTime = millis();
      SDI12_Command(command);
      command = "";  //reset command string
    } else {
      if (byte != 0) {          //do not add start bit (0)
        command += char(byte);  //append byte to command string
      }
    }
  }
  if (graphState == true){
    select_sensor();//call sensor menu to display the relevant graph as required
  }
  if (timer_flag == true){
  get_Sensor_Data();            //read sensor data when timer flag is set to true from Continuous loop
  timer_flag = false;           //reset flag to false 
  Serial.print("FLAG: ");
  Serial.println(timer_flag);    //print flag status
  outputMessage = String(deviceAddress) + "+" + String(temperature) + "+" + String(pressure) + "+" + String(humidity) + "+" + String(gasResist) + "+" + String(lux);
  SDI12_Output(outputMessage);    //print sensor data
  }
}

void SDI12_Command(String input) {
  /*Function to check for SDI-12 Commands input to the serial debug monitor*/

  //convert device address to string
  String address = String(deviceAddress);
  if (String(input.charAt(0)) == "?") {  //checking for ?! - Address Query command
    if (input.length() == 1) {
      SDI12_Output(address);  //return current device address
    } else {
      SDI12_Output("ERROR: Invalid Syntax for Address Query command");  //Provide error message if syntax for address query command is wrong
    }
  } else if (String(input.charAt(0)) == address) {  //Determines if the command is addressed for this device
    if (String(input.charAt(1)) == "A") {           // checking for aAb! - Change Address command
      //Checking command is 3 characters long
      if ((input.length() == 3) && (isDigit(input.charAt(2)))) {
        tempAddress = input.charAt(2);
        deviceAddress = tempAddress.toInt();  //update the device address
        SDI12_Output(tempAddress);            //return updated device address
        delay(1000);                          //Sensor should not respond to any other command for one second as required by SDI-12
      } else {
        SDI12_Output("ERROR: Invalid Syntax for Change Address command");  //Provide error message if syntax for Change Address command is wrong
      }
      
    } else if (String(input.charAt(1)) == "M") {  //Checking for aM! - Start Measurement command
      if (input.length() == 2) {
        get_Sensor_Data();  //Start sensor data measurement
        outputMessage = address;
        zeroCount = 3 - String(duration).length();
        for (int i = 0; i < zeroCount; i++) {
          outputMessage = outputMessage + "0";
        }
        outputMessage = outputMessage + String(duration) + "5";
        SDI12_Output(outputMessage);
      } else {
        SDI12_Output("ERROR: Invalid Syntax for Start Measurement command");  //Provide error message if syntax for Start Measurement command is wrong
      }

    } else if (String(input.charAt(1)) == "I") {  //Checking for aI! - Send Identification command
      if (input.length() == 2) {
      Serial.println("Initializing identification command");
      SDI12_Output(String(address) + "14" + "ENG20009" + "123456789");
      } else {
        SDI12_Output("ERROR: Invalid Syntax for Send Identification command");  //Provide error message if syntax for Send Identification command is wrong
      }

    } else if (String(input.charAt(1)) == "D") {  //Checking for aDb! - Send Data command
      if ((input.length() == 3) && (isDigit(input.charAt(2)))) {
        outputMessage = address;
        if (dataReady == false) {                                                   //Check if a data measurement has been made
          SDI12_Output("ERROR : No measurements have been made from the sensors");  //Provide an error message if no measurements have been made before send data command
        } else if (input.charAt(2) == '1') {
          outputMessage = outputMessage + "+" + String(temperature) + "+" + String(pressure) + "+" + String(humidity) + "+" + String(gasResist);
          SDI12_Output(outputMessage);
          //This resets program once M! and both send data command has been issued, so that M! must be reissued before another send data command
          dataOutput[0] = true;  //BME sensor data has been output and can be erased from memory
          if ((dataOutput[0] == true) && (dataOutput[1] == true)) {
            //if both sensors data has been output from memory, then
            dataReady = false;  //reset the flag so start measurement command must be sent again
            dataOutput[0] = 0;  //set both flags to false
            dataOutput[1] = 0;  //till next time data is output
          }
        } else if ((input.charAt(2) == '2')) {
          outputMessage = outputMessage + "+" + String(lux);
          SDI12_Output(outputMessage);
          //This resets program once M! and both send data command has been issued, so that M! must be reissued before another send data command
          dataOutput[1] = true;  //light sensor data has been output and can be erased from memory
          if ((dataOutput[0] == true) && (dataOutput[1] == true)) {
            //if both sensors data has been output from memory, then
            dataReady = false;      //reset the flag so start measurement command must be sent again
            dataOutput[0] = false;  //set both flags to false
            dataOutput[1] = false;  //till next time data is output
          }
        } else {
          SDI12_Output("ERROR: No sensors available at given sensor address");  //Provide an error message if sensor address is wrong
        }
      } else {
        SDI12_Output("ERROR: Invalid Command Syntax for Send Data Command");  //Provide error message if syntax for Send Data command is wrong
      }

    } else if (String(input.charAt(1)) == "R") {  //checking for aR! - Continous Measurement command
      if ((input.length() == 2)) {
        Timer3.attachInterrupt(Continuous);
        Timer3.start(1000000);
      } else {
        SDI12_Output("ERROR: Invalid Command Syntax for Continuous Measurement Command");  //Provide error message if syntax for Continuous Measurement command is wrong
      }
    }
  }
}

    void SDI12_Output(String message) {
      /*
  Function to send data to the the SDI-12 monitor
  - Takes one input argument message and prints it out
  */
      Serial.print("message: ");
      Serial.println(message);
      //Output message
      digitalWrite(DIRO, LOW);  //set to low to send data
      delay(100);
      Serial1.print(message + String("\r\n"));
      Serial1.flush();  //wait for print to finish
      Serial1.end();
      Serial1.begin(1200, SERIAL_7E1);
      digitalWrite(DIRO, HIGH);  //set high to receive data
    }

    void get_Sensor_Data() {
      /*Function to to perform a reading of sensor data and get a reading*/

      if (!bme.performReading()) {
        SDI12_Output("Failed to perform reading :(");  //Error message if unable to perform a reading
        return;
      }
      //Get the data from the sensor and store them as required
      temperature = bme.temperature;            //in degrees celcius
      pressure = bme.pressure / 100.0;          //in hPa
      humidity = bme.humidity;                  //in %
      gasResist = bme.gas_resistance / 1000.0;  //in KOhms
      lux = lightMeter.readLightLevel();        //in lux
      currentTime = millis();
      duration = (currentTime - startTime) / 1000;  //in seconds
      dataReady = true;                             //Update flag, so send data command is valid now
    }

    
void store_Data(){  
  //Function to store sensor data into the SD card
  //Get the current time and make a timestamp
  DateTime now = rtc.now();
  String TimeStamp=now.timestamp(DateTime::TIMESTAMP_FULL);
  //Get the current measurements from the sensor
  temperature=bme.temperature;
  pressure=bme.pressure/100.0;
  humidity=bme.humidity;
  gasResist=bme.gas_resistance/1000.0;
  lux=lightMeter.readLightLevel();
  //write data to file
  outputMessage = TimeStamp + " < T = " + String(temperature) + " P = " + String(pressure) + " H = " + String(humidity) + " G = " + String(gasResist) + " L = " + String(lux) + " > ";
  file.println(outputMessage);
}

void home_Menu(){
  /*Function to display an Idle Menu when logged out or starting up*/
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
  tft.setCursor(0,4);
  tft.println("\n  SDI12\n Project");
  tft.setTextSize(1);
  tft.print("\n");
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.println("  In Home Screen \n\n  Press (PB2) to Enter");
}

void main_Menu(){
  /*Function to display the main menu once logged in and used to select menu options*/
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
  tft.setCursor(15,10);
  tft.println(" Main Menu \n");
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  if (selection[0] == 1){
    //highlight the first selection
    tft.setTextColor(ST77XX_BLACK,ST77XX_WHITE);
    tft.println(" 1. Sensors \n");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    tft.println(" 2. SD Card \n");
    tft.println(" 3. Return ");
  }
  else if (selection[0] == 2){
    //highlight the second selection
    tft.println(" 1. Sensors \n");
    tft.setTextColor(ST77XX_BLACK,ST77XX_WHITE);
    tft.println(" 2. SD Card \n");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    tft.println(" 3. Return ");
  }else if (selection[0] == 3){
    //highlight the third selection
    tft.println(" 1. Sensors \n");
    tft.println(" 2. SD Card \n");
    tft.setTextColor(ST77XX_BLACK,ST77XX_WHITE);
    tft.println(" 3. Return ");
  }
}

void sensor_Menu(){
  /*Function to display the sensor menu to select sensor data to be displayed */
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
  tft.setCursor(4,10);
  tft.println("Select Sensor");
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.println("\n");
  if (selection[1] == 1){
    //highlight first selection
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.println(" 1. Temperature \n");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.println(" 2. Pressure \n");
    tft.println(" 3. Humidity \n");
    tft.println(" 4. Gas Quality \n");
    tft.println(" 5. Light Intensity ");
  }else if (selection[1] == 2){
    //highlight second selection
    tft.println(" 1. Temperature \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.println(" 2. Pressure \n");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.println(" 3. Humidity \n");
    tft.println(" 4. Gas Quality \n");
    tft.println(" 5. Light Intensity ");
  }else if (selection[1] == 3){
    //highlight third selection
    tft.println(" 1. Temperature \n");
    tft.println(" 2. Pressure \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.println(" 3. Humidity \n");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.println(" 4. Gas Quality \n");
    tft.println(" 5. Light Intensity ");
  }else if (selection[1] == 4){
    //highlight fourth selection
    tft.println(" 1. Temperature \n");
    tft.println(" 2. Pressure \n");
    tft.println(" 3. Humidity \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.println(" 4. Gas Quality \n");
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.println(" 5. Light Intensity ");
  }else if (selection[1] == 5){
    //highlight fifth selection
    tft.println(" 1. Temperature \n");
    tft.println(" 2. Pressure \n");
    tft.println(" 3. Humidity \n");
    tft.println(" 4. Gas Quality \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
    tft.println(" 5. Light Intensity ");
  }
}

void sd_Card_Menu(){
  /*Function to display the SD card Menu on the screen */
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN,ST77XX_BLACK);
  tft.setCursor(15,15);
  tft.println("  SD Card ");
  tft.setTextSize(1);
  tft.print("\n");

  //Lets the user know if data is being stored to the SD card or not
  if (sdRun == false){
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.println(" Data Logging Paused \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN);
    tft.println(" START \n");
  }else{
    tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    tft.println(" Data Logging Ongoing \n");
    tft.setTextColor(ST77XX_BLACK, ST77XX_RED);
    tft.println(" STOP  \n");
  }

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(" Timer interrupt: ");
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.print(sdInterrupt);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println(" s");

  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  //display instructions on how to use the SD card menu
  tft.println("\n (PB1) - Return\n (PB2) - Start/Stop Data\n (PB3) - Decrease interval\n (PB4) - Increase interval");
}


void up_scroll(){
  /*Function to move the menu selection up*/

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  //to prevent bounce or double press, wait 200ms
  if ((interrupt_time - last_interrupt_time) > 200) {
    if (currentMenu == 1){
      if (selection[0]!=1){
        //move the selection up provided it isn't at the top
        selection[0]--;
      }else{
        //if it is at the top, move the selection back to the bottom
        selection[0]=3;
      }
      main_Menu();//call main menu to display updated menu
    }else if (currentMenu == 2){
      if (selection[1]!=1){
        //move the selection up provided it isn't at the top
        selection[1]--;
      }else{
        //if it is at the top, move the selection back to the bottom
        selection[1]=5;
      }
      sensor_Menu();//call sensor menu to display updated sensor menu
    }else if ((currentMenu == 3) && (sdRun == false)){
      //If in SD card menu, increase the period provided it is not 15
      if (sdInterrupt !=15){
        sdInterrupt++;
        sd_Card_Menu();//update the display by calling the SD card menu
      }
    }
  }
  last_interrupt_time = interrupt_time;
}

void down_scroll(){
  /*Function to move the menu selection down*/

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  //to prevent bounce or double press, wait 200ms
  if ((interrupt_time - last_interrupt_time) > 200) {
    if (currentMenu == 1){
      //If in main menu
      if (selection[0]<3){
        //move the selection down provided it isn't at the bottome
        selection[0]++;
      }else{
        //if it is at the bottom, move the selection back to the top
        selection[0]=1;
      }
      main_Menu();//call main menu to display updated menu
    }else if (currentMenu == 2){
      //If in SD card menu
      if (selection[1]<5){
        //move the selection down provided it isn't at the bottome
        selection[1]++;
      }else{
        //if it is at the bottom, move the selection back to the top
        selection[1]=1;
      }
      sensor_Menu();//call sensor menu to display updated sensor menu
    }else if ((currentMenu == 3) && (sdRun == false)){
      //If in SD card menu, decrease the period provided it is not 1
      if (sdInterrupt !=1) {
        sdInterrupt--;
        sd_Card_Menu();//update the display by calling the SD card menu
      }
    }
  }
  last_interrupt_time = interrupt_time;
}

void select_Button(){
  /*Function: To make a selection in the menu*/
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  //to prevent bounce or double press, wait 200ms
  if (((interrupt_time - last_interrupt_time) > 200) && (currentMenu!=4)) {
    tft.fillScreen(ST77XX_BLACK);
    if (currentMenu == 0){
      //if in idle screen, go to main menu
      currentMenu=1;
    }else if (currentMenu == 1){
      //if in main menu
      if (selection[0] == 1){
        //if selection is sensor menu, so to main menu
        currentMenu=2;
      }else if (selection[0] == 2){
        //if selection is sd card menu, go to sd card menu
        currentMenu=3;
      }else if (selection[0] == 3){
        //if selection is log out, go to idle screen
        currentMenu=0;
      }
    }else if (currentMenu == 2){
      //if in sensor menu, go to selected graph
      currentMenu=4;
      graphState=true;
      setScale=true;
    }else if (currentMenu == 3){
      if (sdRun == false){
        //if in SD card menu, start timer interupt and open file
        sdRun=true;
        file.open("sensorData.txt", O_WRITE | O_AT_END);
        Timer3.start(sdInterrupt*1000000);
      }else{
        //if in SD card menu, stop timer interupt and close file
        sdRun = false;
        Timer3.stop();
        file.close();
      }
    }
    select_menu();//call the menu display function
  }
  last_interrupt_time = interrupt_time;
}

void back_Button(){
  /*To go back to the previous menu*/
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  //to prevent bounce or double press, wait 200ms
  if (((interrupt_time - last_interrupt_time) > 200) && (currentMenu!=0)) {
    tft.fillScreen(ST77XX_BLACK);
    if (currentMenu == 1){
      //if in main menu, then log out and return to idle screen
      currentMenu=0;
    }else if ((currentMenu == 2) || (currentMenu == 3)){
      //If in SD card or sensor menu, return to main menu
      currentMenu=1; 
    }else if (currentMenu == 4){
      //if in a graph, return to sensor menu
      tft.fillScreen(ST77XX_BLACK);
      graphState=false;
      prevValue=0;
      currentMenu=2;
    }
    select_menu();
  }
  last_interrupt_time = interrupt_time;
}

void select_menu(){
  /*Function for the program to select which menu to be displayed*/
  switch (currentMenu){
    case 0: home_Menu(); break;
    case 1: main_Menu(); break;
    case 2: sensor_Menu(); break;
    case 3: sd_Card_Menu(); break;
    case 4: break;
    default: break;
  }
}

void select_sensor(){
  /*Function to select which graph should be displayed*/
  switch (selection[1]){
    case 1: temperature_Graph() ; break;
    case 2: pressure_Graph()    ; break;
    case 3: humidity_Graph()    ; break;
    case 4: AQ_graph()         ; break;
    case 5: light_Graph()       ; break;
    default: break;
  }
}

void temperature_Graph(){
  /*Function to display the temperature graph using data from BME sensor*/
  if (setScale == true){
    //Only display the temperature scale and words once
    tft.drawFastVLine(30,4,120,ST77XX_WHITE);
    tft.drawFastVLine(70,4,120,ST77XX_WHITE);
    tft.drawFastHLine(30,84,40,ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(2,5);
    tft.print("  80");
    tft.setCursor(2,80);
    tft.print("   0");
    tft.setCursor(2,118);
    tft.print(" -40");
    tft.setTextSize(2);
    tft.setCursor(75,15);
    tft.print(" Temp:");
    tft.setCursor(95,70);
    tft.print(" \367C ");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    setScale = false;//setting it to false so it will redraw these elements
  }
  float temp = bme.readTemperature();
  tft.setCursor(85,40);
  tft.print(temp);
  if (prevValue<temp){
    tft.fillRect(31,84-round(temp),39,round(temp),ST77XX_RED);
    prevValue=temp;
  }else if (prevValue>temp){
    tft.fillRect(31,84-prevValue,39,round(prevValue-temp),ST77XX_BLACK);
    prevValue=temp;
  }
  delay(500);
}

void pressure_Graph(){
  /*Function to display the pressure graph using data from BME sensor*/
  if (setScale == true){
    tft.drawFastVLine(30,14,110,ST77XX_WHITE);
    tft.drawFastVLine(70,14,110,ST77XX_WHITE);
    tft.drawFastHLine(30,124,40,ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(2,15);
    tft.print("1100");
    tft.setCursor(2,118);
    tft.print("   0");
    tft.setTextSize(2);
    tft.setCursor(75,15);
    tft.print("Press:");
    tft.setCursor(90,70);
    tft.print(" hPa ");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    setScale = false;//setting it to false so it will redraw these elements
  }
  float press = ((bme.readPressure())/ 100.0);
  tft.setCursor(75,40);
  tft.print(press);
  if (prevValue<press){
    tft.fillRect(31,124-round(press/10.0),39,round(press/10.0),ST77XX_BLUE);
    prevValue=press;
  }else if (prevValue>press){
     tft.fillRect(31,124-round(prevValue/10.0),39,round(prevValue/10.0)-round(press/10.0),ST77XX_BLACK);
     prevValue=press;
  }
  delay(500);
}

void humidity_Graph(){
  /*Function to display the humidity graph using data from BME sensor*/
  if (setScale == true){
    tft.drawFastVLine(30,24,100,ST77XX_WHITE);
    tft.drawFastVLine(70,24,100,ST77XX_WHITE);
    tft.drawFastHLine(30,124,40,ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(2,15);
    tft.print(" 100");
    tft.setCursor(2,118);
    tft.print("   0");
    tft.setTextSize(2);
    tft.setCursor(75,15);
    tft.print("Humid:");
    tft.setCursor(90,70);
    tft.print("  %  ");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    setScale = false;//setting it to false so it will redraw these elements
  }
  float humid = bme.readHumidity();
  tft.setCursor(75,40);
  tft.print(humid);
  if (prevValue<humid){
    tft.fillRect(31,124-round(humid),39,round(humid),ST77XX_CYAN);
    prevValue=humid;
  }else if (prevValue>humid){
    tft.fillRect(31,124-round(prevValue),39,round(prevValue)-round(humid),ST77XX_BLACK);
    prevValue=humid;
  }
  delay(500);
}

void AQ_graph(){
  /*Function to display the gas resistance graph using data from BME sensor*/
  if (setScale == true){
    tft.drawFastVLine(30,24,100,ST77XX_WHITE);
    tft.drawFastVLine(70,24,100,ST77XX_WHITE);
    tft.drawFastHLine(30,124,40,ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(2,15);
    tft.print(" 500");
    tft.setCursor(2,118);
    tft.print("   0");
    tft.setTextSize(2);
    tft.setCursor(75,15);
    tft.print(" Gas\n");
    tft.print("\t\t\t   Resist:");
    tft.setCursor(90,90);
    tft.print("kOhms");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    setScale = false;//setting it to false so it will redraw these elements
  }
  float gas = bme.readGas()/1000;
  tft.setCursor(80,60);
  tft.print(gas);
  if (prevValue<gas){
    tft.fillRect(31,124-round(gas),39,round(gas),ST77XX_GREEN);
    prevValue=gas;
  }else if (prevValue>gas){
    tft.fillRect(31,124-round(prevValue),39,round(prevValue)-round(gas),ST77XX_BLACK);
    prevValue=gas;
  }
  delay(500);
}

void light_Graph(){
  /*Function to display the light graph using data from BVHL sensor*/
  if (setScale == true){
    tft.drawFastVLine(30,4,120,ST77XX_WHITE);
    tft.drawFastVLine(70,4,120,ST77XX_WHITE);
    tft.drawFastHLine(30,124,40,ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(2,15);
    tft.print("1100");
    tft.setCursor(2,118);
    tft.print("   0");
    tft.setTextSize(2);
    tft.setCursor(75,15);
    tft.print("Light:");
    tft.setCursor(90,70);
    tft.print("  lx ");
    tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
    setScale = false;//setting it to false so it will redraw these elements
  }
  float lx = lightMeter.readLightLevel();
  tft.setTextSize(2);
  tft.setCursor(75,40);
  tft.print(lx);
  if (prevValue<lx){
    tft.fillRect(31,124-round(lx/10),39,round(lx/10),ST77XX_YELLOW);
    prevValue=lx;
  }else if (prevValue>lx){
     tft.fillRect(31,124-round(prevValue/10),39,round(prevValue/10)-round(lx/10),ST77XX_BLACK);
     prevValue=lx;
  }
  delay(500);
}
