#include <Arduino.h>                //If the code breaks sry i cant code well and gemini helped a bunch so blame it on him
#include <DFRobotDFPlayerMini.h>
#include <TFT_eSPI.h> 

const int PIN_BTN_LADDER = A2;   
const int PIN_BATTERY_ADC = A3;  

const int VAL_SELECT   = 100; //change this crap when i get the actual stuff cuz resistors are funny 
const int VAL_DOWN     = 300; //change this crap when i get the actual stuff cuz resistors are funny 
const int VAL_UP       = 500; //change this crap when i get the actual stuff cuz resistors are funny  
const int VAL_VOL_UP   = 700; //change this crap when i get the actual stuff cuz resistors are funny 
const int VAL_VOL_DOWN = 900; //change this crap when i get the actual stuff cuz resistors are funny 
const int ADC_TOLERANCE = 50; //change this crap when i get the actual stuff cuz resistors are funny  

enum Button { BTN_NONE, BTN_SELECT, BTN_DOWN, BTN_UP, BTN_VOL_UP, BTN_VOL_DOWN }; //No Digital Situation

TFT_eSPI tft = TFT_eSPI(); 
DFRobotDFPlayerMini myDFPlayer;

Button currentButton = BTN_NONE;
Button lastButton = BTN_NONE;
unsigned long lastDebounceTime = 0;
const int debounceDelay = 50; 
unsigned long selectPressTimer = 0; 

bool inMenuMode = true; 
bool isPlaying = false;
int currentVolume = 15; 
unsigned long lastBatteryCheck = 0;
int currentBatteryPercent = 0;

int currentFolderIndex = 1;  
int currentSongIndex = 1;    
int selectedListItem = 0;    

// Content Setup 
String folderName = "FOLDER 01";
const int numSongsInFolder = 2; 
String songNames[numSongsInFolder] = {"SONG A", "SONG B"};

Button getPressedButton() {
  int val = analogRead(PIN_BTN_LADDER);
  if (abs(val - VAL_SELECT) < ADC_TOLERANCE)   return BTN_SELECT;
  if (abs(val - VAL_DOWN) < ADC_TOLERANCE)     return BTN_DOWN;
  if (abs(val - VAL_UP) < ADC_TOLERANCE)       return BTN_UP;
  if (abs(val - VAL_VOL_UP) < ADC_TOLERANCE)   return BTN_VOL_UP;
  if (abs(val - VAL_VOL_DOWN) < ADC_TOLERANCE) return BTN_VOL_DOWN;
  return BTN_NONE; 
}

void setup() {
  Serial.begin(115200);
  
  randomSeed(micros()); //Gambling

  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Booting OS...");

  Serial1.begin(9600);
  if (!myDFPlayer.begin(Serial1)) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.println("Audio Error!");
    while(true); 
  }

  myDFPlayer.volume(currentVolume);
  updateScreenUI(); 
}

void loop() {

  Button reading = getPressedButton();
  Button pressedThisFrame = BTN_NONE; 

  if (reading != lastButton) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButton) {
      currentButton = reading;
      if (currentButton != BTN_NONE) pressedThisFrame = currentButton;
    }
  }
  lastButton = reading;

  if (currentButton == BTN_SELECT && !inMenuMode) {            // Too lazy for 6th button
    if (selectPressTimer == 0) selectPressTimer = millis();
    if (millis() - selectPressTimer > 1000) { 
      inMenuMode = true; 
      selectPressTimer = 0; 
      updateScreenUI();
    }
  } else {
    selectPressTimer = 0; 
  }

  if (inMenuMode) {
    if (pressedThisFrame == BTN_SELECT) {
      inMenuMode = false; 
      currentSongIndex = selectedListItem + 1; 
      myDFPlayer.playFolder(currentFolderIndex, currentSongIndex);
      isPlaying = true;
      updateScreenUI();
    }
    if (pressedThisFrame == BTN_DOWN && selectedListItem < numSongsInFolder - 1) {
      selectedListItem++;
      updateScreenUI();
    }
    if (pressedThisFrame == BTN_UP && selectedListItem > 0) {
      selectedListItem--;
      updateScreenUI();
    }
  } 

  else {
    if (pressedThisFrame == BTN_SELECT) {
      isPlaying = !isPlaying;
      if (isPlaying) myDFPlayer.start();
      else myDFPlayer.pause();
      updateScreenUI(); 
    }
    if (pressedThisFrame == BTN_DOWN && currentSongIndex < numSongsInFolder) {
      currentSongIndex++;
      myDFPlayer.playFolder(currentFolderIndex, currentSongIndex);
      isPlaying = true;
      updateScreenUI();
    }
    if (pressedThisFrame == BTN_UP && currentSongIndex > 1) {
      currentSongIndex--;
      myDFPlayer.playFolder(currentFolderIndex, currentSongIndex);
      isPlaying = true;
      updateScreenUI();
    }
  }

  if (pressedThisFrame == BTN_VOL_UP && currentVolume < 30) {
    currentVolume++;
    myDFPlayer.volume(currentVolume);
    updateScreenUI(); 
  }
  if (pressedThisFrame == BTN_VOL_DOWN && currentVolume > 0) {
    currentVolume--;
    myDFPlayer.volume(currentVolume);
    updateScreenUI();
  }

  if (millis() - lastBatteryCheck > 10000) {  //battery cuz i wanna use it outside my house
    checkBattery();
    lastBatteryCheck = millis();
    updateScreenUI(); 
  }
}

void checkBattery() {
  int rawValue = analogRead(PIN_BATTERY_ADC);
  float pinVoltage = (rawValue / 4095.0f) * 3.3f;
  float batteryVoltage = pinVoltage * 2.0f;
  currentBatteryPercent = constrain((int)map((int)(batteryVoltage * 100.0f), 330, 420, 0, 100), 0, 100);
  currentBatteryPercent = constrain(currentBatteryPercent, 0, 100);
}

void updateScreenUI() {
  int screenW = tft.width();
  int screenH = tft.height();

  tft.fillRect(0, 0, screenW, 35, TFT_DARKGREY); 
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  
  if (isPlaying) {
    tft.setTextColor(TFT_GREEN, TFT_DARKGREY);
    tft.print("▶ "); 
  } else {
    tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
    tft.print("II ");
  }
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.print(" Vol: ");
  tft.print(currentVolume);
  tft.print("      Bat: "); 
  tft.print(currentBatteryPercent);
  tft.print("%");

  tft.fillRect(0, 35, screenW, screenH - 35, TFT_BLACK); 
  
  if (inMenuMode) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(10, 50);
    tft.print(folderName);

    tft.setTextSize(2); 
    int startY = 90; 
    int spacing = 35; 

    for (int i = 0; i < numSongsInFolder; i++) {
      tft.setCursor(20, startY + (i * spacing)); 
      if (i == selectedListItem) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK); 
        tft.print("└─ ");
        tft.print(songNames[i]);
        
        // Draw the arrow on the right side
        tft.setCursor(screenW - 50, startY + (i * spacing));
        tft.print("<-"); 
      } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK); 
        tft.print("└─ ");
        tft.print(songNames[i]);
      }
    }
  } 
  else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(3);
    
    int textX = (screenW - (songNames[currentSongIndex - 1].length() * 18)) / 2;
    if (textX < 0) textX = 10;
    tft.setCursor(textX, 60);
    tft.print(songNames[currentSongIndex - 1]); 

    // vinyl aka cooler cd
    int cx = screenW / 2; 
    int cy = 150; 
    int outerRadius = 40;
    int innerRadius = 15;
    
    // Generate a random color for vinyl aka GAMBLING
    uint16_t randomColor = random(0xFFFF);

    tft.fillCircle(cx, cy, outerRadius, TFT_DARKGREY); 
    tft.fillCircle(cx, cy, innerRadius, randomColor); 
    tft.fillCircle(cx, cy, 4, TFT_BLACK);              
  }
}