const int STOPPED = 0;
const int MOVING = 1;
int distance = 0;

int state[2] = {STOPPED, STOPPED};
bool opening[2] = {true, true};
bool homing[2] = {false, false};
int pulsePin[2] = {9, 10};
int dirPin[2] = {32, 33};
int limitOpenPin[2] = {30, 31};
int limitDist[2] = {1000, 1000};

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
    delayMicroseconds(500);
    digitalWrite(pulsePin[door], LOW);
    delayMicroseconds(500);
  
    distance += 1;
    if (limitSwitchOpen(door) && opening[door]) {
      stopDoor(door);
    }
    if (!homing && (distance >= limitDist[door])) {
      stopDoor(door);
    }
  }
}

void stopDoor(int door) {
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
      moveDoor(selectedDoor);
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
        if (state[selectedDoor] != MOVING) state[selectedDoor] = MOVING;
        break;
      case 3: 
        state[selectedDoor] = STOPPED;
        break;
      case 4: 
        opening[selectedDoor] = !opening[selectedDoor];
        setDirection(selectedDoor);
        break;
      case 5:
        Serial.print(String("Limit switch is "));
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
    } // end of switch
  }
  Serial.println("DONE in getJob().");
}

void setDirection(int door) {
  if (opening[door]) {
    digitalWrite(dirPin[door], HIGH);
  }
  else {
    digitalWrite(dirPin[door], LOW);
  }
}

void displayStatus() {
  Serial.println(String("Selected door is: ") + String(selectedDoor));
  for (int i = 0; i < 2; i++) {
    Serial.print(String("Door ") + String(i) + String(" is "));
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
}
