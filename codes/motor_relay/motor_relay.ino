#include <math.h>

// ===================== TUNABLE CONSTANTS ================================
const long STEPS_PER_REV      = 400;                 // 0.9°/step -> 400 steps/rev
const long TOTAL_SWEEP_STEPS  = (long) lround(STEPS_PER_REV * (90.0/360.0)); // 90°
const long DEGREE_RELAY       = 60;
const long DEGREE_PIV         = 30;

const long RELAY_ON_AT_STEPS  = (long) lround(DEGREE_RELAY / 0.9); // relay ON, stays ON
const long PIV_PULSE_AT_STEPS = (long) lround(DEGREE_PIV   / 0.9); // PIV pulse once

// Step pulse timing
unsigned long STEP_HIGH_US = 650;
unsigned long STEP_LOW_US  = 650;

const unsigned int PIV_PULSE_MS = 10;    // PIV pulse width

// ===================== PIN MAP =========================================
const int PIN_STEP  = 12; // PUL-
const int PIN_DIR   = 9;  // DIR-
const int PIN_PIV   = 7;  // PIV trigger output
const int PIN_RELAY = 8;  // Relay output (LOW = ON, HIGH = OFF)

// ===================== STATE ===========================================
volatile long steps_from_zero = 0;  // +ve after forward
bool relay_engaged = false;
bool moving = false;
bool started = false;                // becomes true after 's'
bool pending_go_home = false;        // set if 'b' pressed while moving

// ===================== HELPER FUNCTIONS =========================================
inline void setDir(bool forward) { digitalWrite(PIN_DIR, forward ? HIGH : LOW); }

inline void stepOnceRaw(bool forward) {
  setDir(forward);
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(STEP_HIGH_US);
  digitalWrite(PIN_STEP, LOW);
  delayMicroseconds(STEP_LOW_US);
  steps_from_zero += (forward ? 1 : -1);
}

void pulsePIV() {
  digitalWrite(PIN_PIV, HIGH);
  delay(PIV_PULSE_MS);
  digitalWrite(PIN_PIV, LOW);
}

void relayOn()  { if (!relay_engaged) { digitalWrite(PIN_RELAY, LOW);  relay_engaged = true; } }
void relayOff() { if ( relay_engaged) { digitalWrite(PIN_RELAY, HIGH); relay_engaged = false; } }

void moveForwardWithEvents(long target_steps) {
  moving = true;
  for (long i = 0; i < target_steps; ++i) {
    stepOnceRaw(true);
    if (!relay_engaged && steps_from_zero == RELAY_ON_AT_STEPS) relayOn();
    if (steps_from_zero == PIV_PULSE_AT_STEPS)                  pulsePIV();
  }
  moving = false;
}

void returnToZero_and_RelayOff() {
  if (steps_from_zero <= 0) { relayOff(); return; }
  moving = true;
  long n = steps_from_zero;
  for (long i = 0; i < n; ++i) {
    stepOnceRaw(false);
  }
  moving = false;
  relayOff();
}

// RPM setter 
void setRPM(float rpm) {
  if (rpm <= 0) return;
  double step_freq_hz = (double)STEPS_PER_REV * (double)rpm / 60.0;
  double period_us = 1e6 / step_freq_hz;
  unsigned long half = (unsigned long)(period_us / 2.0);
  STEP_HIGH_US = (half < 2 ? 2 : half);
  STEP_LOW_US  = STEP_HIGH_US;
}

// ===================== ARDUINO =========================================
void setup() {
  Serial.begin(115200); // CHANGE BAUD IF NEEDED

  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  pinMode(PIN_PIV, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);

  digitalWrite(PIN_STEP, LOW);
  digitalWrite(PIN_PIV, LOW);
  digitalWrite(PIN_RELAY, HIGH);
  setDir(true);

  // setRPM(20);

  Serial.println(F("Press 's' to start (forward to 90°, relay @60°, PIV @30°)."));
  Serial.println(F("After it finishes and holds, press 'b' to return home + relay OFF."));
  Serial.println(F("(Baud 115200, No line ending)"));
}

void loop() {
  // ---- Single serial handler ----
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if ((c == 's' || c == 'S')) {
      if (!started && !moving) {
        started = true;
        Serial.println(F("Start: moving to 90°..."));
        moveForwardWithEvents(TOTAL_SWEEP_STEPS);
        Serial.println(F("Hold at 90°. Press 'b' to return home."));
        
      }
     
    }
    else if ((c == 'b' || c == 'B')) {
      if (moving) {
        // Queue it to run right after the current motion completes
        pending_go_home = true;
      } else if (started) {
        returnToZero_and_RelayOff();
        Serial.println(F("Done: at zero, relay OFF."));
        
        started = false;  // allow another 's' cycle without reset
        Serial.println(F("Ready. Press 's' to run again."));
      }
      
    }
    // ignores all other keys
  }


  if (!moving && pending_go_home && started) {
    pending_go_home = false;
    returnToZero_and_RelayOff();
    Serial.println(F("Done (queued): at zero, relay OFF."));
    started = false;  // allow another 's'
    Serial.println(F("Ready. Press 's' to run again."));
  }

  delay(2);
}
