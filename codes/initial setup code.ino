// HSS86 Hybrid Servo Control via Arduino
// 400 steps per revolution (0.9° per step)
// 'f' -> 2 steps forward
// 'b' -> 2 steps backward
// 'p' -> 90° forward (100 steps)
// 'q' -> 90° backward (100 steps)

#define PUL 8    // Pulse (Step) pin
#define DIR 9    // Direction pin
#define ENA 10   // Enable pin (optional)

const int STEPS_PER_PRESS = 2;        // small step movement
const int STEPS_PER_90DEG = 100;      // 90° = 100 steps
const unsigned int PULSE_US = 800;    // pulse width (microseconds)

// ---------------------- SETUP ----------------------
void setup() {
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Initialize all outputs
  digitalWrite(PUL, LOW);
  digitalWrite(DIR, LOW);
  digitalWrite(ENA, LOW);   // LOW enables driver on most HSS86 modules

  Serial.begin(9600);
  Serial.println("Press keys: f (2 steps FWD), b (2 steps BWD), p (90° FWD), q (90° BWD)");
}

// ---------------------- MAIN LOOP ----------------------
void loop() {
  if (Serial.available() > 0) {
    char key = Serial.read();

    switch (key) {
      case 'f': case 'F':
        Serial.println("Moving 2 steps forward...");
        moveSteps(STEPS_PER_PRESS, true);
        break;

      case 'b': case 'B':
        Serial.println("Moving 2 steps backward...");
        moveSteps(STEPS_PER_PRESS, false);
        break;

      case 'p': case 'P':
        Serial.println("Moving 90° forward...");
        moveSteps(STEPS_PER_90DEG, true);
        break;

      case 'q': case 'Q':
        Serial.println("Moving 90° backward...");
        moveSteps(STEPS_PER_90DEG, false);
        break;

      default:
        Serial.println("Invalid key. Use f, b, p, or q.");
        break;
    }

    // Clear any leftover characters (like newline)
    while (Serial.available()) Serial.read();
  }
}

// ---------------------- MOVE FUNCTION ----------------------
void moveSteps(int steps, bool forward) {
  digitalWrite(DIR, forward ? HIGH : LOW);

  for (int i = 0; i < steps; i++) {
    digitalWrite(PUL, HIGH);
    delayMicroseconds(PULSE_US);
    digitalWrite(PUL, LOW);
    delayMicroseconds(PULSE_US);
  }
}
