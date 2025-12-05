
const int ldrPin = A1;          
const int topLEDPin = 6;        
const int middleLEDPin = 7;     
const int bottomLEDPin = 8;     


const int darknessThreshold = 150; 

void setup() {
  
  pinMode(topLEDPin, OUTPUT);
  pinMode(middleLEDPin, OUTPUT);
  pinMode(bottomLEDPin, OUTPUT);
}

void loop() {
  
  int ldrValue = analogRead(ldrPin);

  
  if (ldrValue < darknessThreshold)
  {
    
    digitalWrite(topLEDPin, HIGH);
    digitalWrite(middleLEDPin, HIGH);
    digitalWrite(bottomLEDPin, HIGH);
  }
   else
  {
    
    digitalWrite(topLEDPin, LOW);
    digitalWrite(middleLEDPin, LOW);
    digitalWrite(bottomLEDPin, LOW);
  }

  
  delay(500); 
}
