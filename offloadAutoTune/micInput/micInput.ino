void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);
  pinMode(A5, OUTPUT);
  pinMode(A1, INPUT);
  Serial.begin(115200);
  analogReadResolution(12);
  analogWriteResolution(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t input = analogRead(A0);
  float knobValue = analogRead(A1);
  knobValue -= 2048;
  knobValue /= 2048;
  knobValue *= 12;
  char freqSelect = knobValue;
  
  uint8_t lo = input & 0xFF;
  uint8_t hi = input >> 8;
  Serial.write(lo);
  Serial.write(hi);
  Serial.write(freqSelect);
  
  while(Serial.available() < 3);
  
  lo = Serial.read();
  hi = Serial.read();

  while(Serial.available() > 0)
    int temp = Serial.read();

  lo &= 0xFF;
  hi &= 0xFF;
  uint16_t output = lo | uint16_t(hi) << 8;

  analogWrite(A5, output);
}
