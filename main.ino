#define ten    600000  //milliseconds in 10 minutes
#define five   300000  //milliseconds in 5 minutes 
#define one     60000  //milliseconds in 1 minute

// constants won't change. Used here to 
// set pin numbers:
const int relayPin =  8;      // the number of the LED pin
const int tripSwitch = 2;
const int runRelayPin = 3;
const int buttonPin = 4;
const int interruptPin1 = 0;
const int interruptPin2 = 1;
const int ledPin = 13;
const int on = HIGH;
const int off = LOW;

// Variables will change:
volatile int switchState = LOW;              // relay state
volatile int tripSwitchState = LOW;          // trip switch state
volatile int runRelayStatus = LOW;           // run relay button state to manually trigger relay
int buttonStatus = 0;
int valveStage = 0;
int restartStatus = 0;

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 1000;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(9600);
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(runRelayPin, INPUT);
  pinMode(tripSwitch, INPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(runRelayPin, HIGH);
  digitalWrite(tripSwitch, HIGH);
  digitalWrite(buttonPin, HIGH);
  digitalWrite(relayPin, LOW);
  attachInterrupt(interruptPin1, notifyTripSwitch, RISING);
  attachInterrupt(interruptPin2, runRelay, CHANGE);
}

void loop()
{
  buttonStatus = !digitalRead(buttonPin);  // read input value
  if (buttonStatus == HIGH) {         // check if the input is HIGH (button released)
    digitalWrite(ledPin, HIGH);
    startCycle();
  }
  else{
    digitalWrite(ledPin, LOW);
  }
}

void runRelay(){
  runRelayStatus = !digitalRead(runRelayPin);
  buttonStatus = LOW;
  switchState = runRelayStatus;
  triggerRelay(switchState);
  digitalWrite(ledPin, LOW);
  valveStage = 0;
  restartStatus = 1;
}

void triggerRelay() 
{
  switchState = !digitalRead(relayPin);
  digitalWrite(relayPin,switchState);
}

void triggerRelay(int instatus){
  switchState = instatus;
  digitalWrite(relayPin,instatus);
}

void notifyTripSwitch()
{
  // implement debouncing to filter out false readings
  tripSwitchState = !digitalRead(tripSwitch);
  if (tripSwitchState == LOW){
    if ((millis() - lastDebounceTime) > debounceDelay){
      triggerRelay(off);
      lastDebounceTime = millis();
      Serial.println("boom");
      cycle();
    }
  }
  else{
    lastDebounceTime = millis();
  }
}

void startCycle() 
{
  valveStage = 1;
  restartStatus = 0;
  cycle();
}

void cycle(long delayin)
{
  long currTime = millis();
  triggerRelay();
  while ((millis() - currTime <= delayin) & (restartStatus == 0)){}
}

void cycle() 
{
  Serial.println(valveStage);
  switch (valveStage) {
    case 1:  // fill
      Serial.println("fill delaying....1 minute");
      cycle(one);
      Serial.println("cycle 1 complete");
      valveStage++;
      Serial.println("ended fill");
      break;
    case 2:  // brine draw
      Serial.println("brine delaying....10 minute");
      cycle(ten);
      Serial.println("cycle 2 complete");
      valveStage++;
      Serial.println("ended brine draw");
      break;
    case 3:  // backwash
      Serial.println("backwash delaying....5 minute");
      cycle(five);
      Serial.println("cycle 3 complete");
      valveStage++;
      Serial.println("ended backwash");
      break;
    case 4:  // rinse 
      Serial.println("rinse delaying....5 minute");
      cycle(five);
      Serial.println("cycle 4 complete");
      valveStage++;
      Serial.println("ended rinse");
      break;
    case 5:  // service
      Serial.println("service... no delay");
      triggerRelay();
      valveStage++;
      Serial.println("ended service");
      break;
    default:
      valveStage = 0;
      Serial.println("full cycle ended");
      break;
  }
}
