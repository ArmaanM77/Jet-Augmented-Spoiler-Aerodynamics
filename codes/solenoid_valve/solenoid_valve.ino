int relay1 = 10;  // IN1 connected to D7


void setup() {
  // put your setup code here, to run once:
  pinMode(relay1, OUTPUT);
  digitalWrite(relay1, HIGH); 
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(relay1, LOW);   // Relay ON
  delay(5000);                 // Solenoid active for 2 sec
  digitalWrite(relay1, HIGH);  // Relay OFF
  delay(5000);

}
