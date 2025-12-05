
const int LED_PIN = 6;     
const int BUTTON_PIN = 3;   


bool ledState = LOW;        
bool buttonState = LOW;     
bool lastButtonState = LOW; 

unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;    

void setup() {
  pinMode(LED_PIN, OUTPUT);       
  pinMode(BUTTON_PIN, INPUT_PULLUP); 

void loop() {
  
  buttonState = digitalRead(BUTTON_PIN);
  
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    
    ledState = !ledState;
    
    digitalWrite(LED_PIN, ledState);
  }
  
  
  lastButtonState = buttonState;
}
