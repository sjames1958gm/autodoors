const int STOPPED = 0;
const int MOVING = 1;
int distance = 0;

int state[2] = {STOPPED, STOPPED};
bool opening[2] = {true, true};
bool homing[2] = {false, false};
bool backingOff[2] = {false, false};
int pulsePin[2] = {9, 10};
int dirPin[2] = {2, 3};
int limitOpenPin[2] = {4, 5};
int limitDist[2] = {1000, 1000};
String doorName[2] = {"right", "left"};
int doorOpening[2] = {HIGH, LOW};
int doorClosing[2] = {LOW, HIGH};
int doorTravelled[2] = {18977, 19327};
bool dualMode = false;
int speed = 200;

int selectedDoor = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(dirPin[0], OUTPUT);
  pinMode(dirPin[1], OUTPUT);
  pinMode(pulsePin[0], OUTPUT);  
  pinMode(pulsePin[1], OUTPUT);  
  pinMode(limitOpenPin[0], INPUT);
  pinMode(limitOpenPin[1], INPUT);
  digitalWrite(pulsePin[0], LOW);
  digitalWrite(pulsePin[1], LOW);
  setDirection(0);
  setDirection(1);
}

void loop() {
  doJobs();
}

void moveDoor(int door) {
  if (state[door] == MOVING) {

    digitalWrite(pulsePin[door], HIGH);
    delayMicroseconds(speed);
    digitalWrite(pulsePin[door], LOW);
    delayMicroseconds(speed);
  
    distance += 1;
    if (backingOff[door]) {
      backingOff[door] = limitSwitchOpen(door);
      if (!backingOff[door]) {
        Serial.println("Backing off complete");
        stopDoor(door);
      }
    } else {
      if (limitSwitchOpen(door) && opening[door]) {
        opening[door] = false;
        setDirection(door);
        Serial.println("Backing off");
        backingOff[door] = true;
      }
      if (!homing && (distance >= limitDist[door])) {
        stopDoor(door);
      }
    }
  }
}

void stopDoor(int door) {
  Serial.println(doorName[door] + String("Travelled ") + String(distance));
  state[door] = STOPPED;
  homing[door] = false;
  distance = 0;
}

bool limitSwitchOpen(int door) {
  return digitalRead(limitOpenPin[door]) == LOW;
}

void doJobs(){
  int jobNumber;
  getJobMenu();
  bool done = false;
  while (!done) {
    while (!Serial.available()){
      if (dualMode) {
        moveDoor(0);
        moveDoor(1);
      } else {
        moveDoor(selectedDoor);
      }
    }
    delay(10);
    while(Serial.available())
    {
      jobNumber = Serial.parseInt();
      if (Serial.read() != '\n') { Serial.println("going to "+String(jobNumber)); }
    } 
  
    switch (jobNumber) {
      case 0:
        getJobMenu();
        break;
      case 1:
        displayStatus();
        break;
      case 2:
        if (dualMode) {
          Serial.println(String("Moving both doors"));
          state[0] = MOVING;
          state[1] = MOVING;
        } else {
          Serial.println(String("Moving door: ") + String(selectedDoor));
          state[selectedDoor] = MOVING;
        }
        break;
      case 3: 
        if (dualMode) {
          stopDoor(0);
          stopDoor(1);
        } else {
          stopDoor(selectedDoor);
        }
        break;
      case 4: 
        if (dualMode) {
          opening[0] = !opening[0];
          setDirection(0);
          opening[1] = !opening[1];
          setDirection(1);
        } else {
          opening[selectedDoor] = !opening[selectedDoor];
          setDirection(selectedDoor);
          Serial.print(String("Door ") + doorName[selectedDoor] + String(" is "));
          Serial.println(opening[selectedDoor] ? "opening" : "closing");
        }
        break;
      case 5:
        Serial.print(String("Door ") + doorName[selectedDoor] + String(" limit switch is "));
        Serial.println(limitSwitchOpen(selectedDoor) ? "on" : "off");
        break;
      case 6:
        selectedDoor = (selectedDoor + 1) % 2;
        Serial.print(String("Selected door: "));
        Serial.println(selectedDoor); 
        break;
      case 7:
        Serial.println("Homing - opening to limit switch");
        homing[selectedDoor] = true;
        opening[selectedDoor] = true;
        state[selectedDoor] = MOVING;
        break;
      case 8:
        dualMode = !dualMode;
        Serial.println(dualMode ? "Dual Mode" : "Single Mode");
        break;
    } // end of switch
  }
  Serial.println("DONE in getJob().");
}

void setDirection(int door) {
  if (opening[door]) {
    Serial.println(doorName[door] + " setDirection, opening");
    digitalWrite(dirPin[door], doorOpening[door]);
  }
  else {
    Serial.println(doorName[door] + "setDirection, closing");
    digitalWrite(dirPin[door], doorClosing[door]);
  }
}

void displayStatus() {
  Serial.println(String("Selected door is: ") + doorName[selectedDoor]);
  for (int i = 0; i < 2; i++) {
    Serial.print(doorName[i] + String(" is "));
    Serial.print(String(state[i] == MOVING ? "MOVING" : "STOPPED"));
    Serial.println(String(", ") + String(opening[i] ? "OPENING" : "CLOSING"));
  }
}


// Simple menu for various commands
void getJobMenu() {
 Serial.println("GETJOB():  enter the item number to run");
 Serial.println("0. display menu");
 Serial.println("1. display status");
 Serial.println("2. move door");
 Serial.println("3. stop door");
 Serial.println("4. change direction");
 Serial.println("5. read limit switch");
 Serial.println("6. select door");
 Serial.println("7. home");
 Serial.println("8. mode switch");
}
