#include <toneAC.h>
#include <LiquidCrystal.h>

// For LCD
const int rs = 4,
          en = 5,
          d4 = 6,
          d5 = 7,
          d6 = 8,
          d7 = 9;

byte customEighth[] = { 
  B00000,
  B00100,
  B00110,
  B00101,
  B00100,
  B01100,
  B01100,
  B00000
};

byte customDoubleEighth[] = {
  B00000,
  B01111,
  B01001,
  B01001,
  B01001,
  B11011,
  B11011,
  B00000
};

byte customDoubleSixteenth[] = {
  B00000,
  B01111,
  B01001,
  B01111,
  B01001,
  B11011,
  B11011,
  B00000
};

byte customQuarter[] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B00100,
  B01100,
  B01100,
  B00000
};

byte customRest[] = {
  B00000,
  B11010,
  B11110,
  B00010,
  B00100,
  B00100,
  B01000,
  B00000
};

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte scrollBuffer[32]; // Larger than 16 to allow full scrolling
int bufferIndex = 0;

// For Piano

// Buzzer draws from D11 and D12 using toneAC for More Power and Increased Control of Volume

int volumePin = A0;
int notes[8][12] = {
  {31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58},                         // Octave 0 (B0)
  {62, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110, 117},                      // Octave 1
  {123, 131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233},             // Octave 2
  {247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466},             // Octave 3
  {494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932},             // Octave 4
  {988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865},  // Octave 5
  {1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729}, // Octave 6 (B6)
};

const char* noteSymbols[] = {"B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
int buttonPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34};
int numKeys = sizeof(buttonPins) / sizeof(buttonPins[0]);

int octaveUpPin = 2;
int octaveDownPin = 3;

int volumeValue;
int volume;
int currentOctave = 2; // Default octave
bool keyTouched = false;

void displayWelcome() {
  // First Row
  lcd.setCursor(0, 0);
  for (int c = 0; c < 16; c++) {
    if (c >= 4 && c < 11) { // Stationary
      if (c == 4) lcd.print(" Piano ");
    } else { // Moving Notes Background
      int bufIndex = (bufferIndex + c) % 32;
      lcd.write(byte(scrollBuffer[bufIndex]));
    }
  }
  
  // Second Row
  lcd.setCursor(0, 1);
  for (int c = 0; c < 16; c++) {
    if (c >= 4 && c < 11) {
      if (c == 4) lcd.print(" Ready ");
    } else {
      int bufIndex = (bufferIndex + c + 16) % 32; // + 16
      lcd.write(byte(scrollBuffer[bufIndex]));
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  for (int pin : buttonPins) {
    pinMode(pin, INPUT_PULLUP); // internal PullUp all pins.
  }
  pinMode(octaveUpPin, INPUT_PULLUP);
  pinMode(octaveDownPin, INPUT_PULLUP);
  // lcd.begin(16, 2);
  lcd.begin(16, 2); // Initialize LCD
  lcd.createChar(0, customEighth);
  lcd.createChar(1, customDoubleEighth);
  lcd.createChar(2, customQuarter);
  lcd.createChar(3, customRest);
  lcd.createChar(4, customDoubleSixteenth);
  
  // Initially fill the buffer with scrolling symbols
  for (int i = 0; i < 32; i++) scrollBuffer[i] = i % 5;
  displayWelcome();

  // For debugging
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  volumeValue = analogRead(volumePin);
  volume = map(volumeValue, 0, 1024, 0, 10);
  bool notePlayed = false;
  
  if (!keyTouched) { // Controls the "Welcome" display
    bufferIndex = (bufferIndex + 1) % 32;
    lcd.setCursor(0, 0);
    displayWelcome();
    delay(1000);
  }
  
  if (digitalRead(octaveUpPin) == LOW) { // Change Octaves (move Up)
    currentOctave = min(currentOctave + 1, 6); // If at max octave (6), stay there
    delay(200); // Debouncing control
  }
  if (digitalRead(octaveDownPin) == LOW) {
    currentOctave = max(currentOctave - 1, 0); // Ensure it doesn't go below the first octave
    delay(200);
  }
  
  for (int i = 0; i < numKeys; i++) {
    if (digitalRead(buttonPins[i]) == LOW) { // If any key is pressed
      keyTouched = true;
      notePlayed = true;
      toneAC(notes[currentOctave][i], volume);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(noteSymbols[i]);
      lcd.print(" ");
      lcd.print(currentOctave);
      break;
    }
  }
  
  if (keyTouched) {
    lcd.setCursor(0, 1);
    lcd.print("Vol: ");
    lcd.print(volume);
    lcd.setCursor(8, 1);
    lcd.print("Oct: ");
    lcd.print(currentOctave);
    lcd.print("/6");
  }

  if (!notePlayed) { toneAC(); }
  Serial.println(volume);
}