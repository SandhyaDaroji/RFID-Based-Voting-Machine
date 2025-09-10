#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Push button pins
int buttons[4] = {A0, A1, A2, A3};

// Authorised card UIDs (2 voters only) - replace with actual UIDs
String authorisedCards[2] = {
  "A1B2C3D4",  // Voter 1 UID
  "11223344"   // Voter 2 UID
};

// Vote counter
int votes[4] = {0, 0, 0, 0};

// Track which authorised voters have voted
bool hasVotedStatus[2] = {false, false};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.begin(16, 2);
  lcd.print("RFID Voting");

  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  delay(2000);
  lcd.clear();
}

void loop() {
  // Check if voting is complete
  if (hasVotedStatus[0] && hasVotedStatus[1]) {
    showWinner();
    while (true); // Stop program
  }

  lcd.setCursor(0, 0);
  lcd.print("Tap Your Card");

  // Wait for card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  String uid = getUID();
  Serial.print("Card UID: ");
  Serial.println(uid);

  // Check if authorised
  int voterIndex = getVoterIndex(uid);
  if (voterIndex == -1) {
    lcd.clear();
    lcd.print("Unauthorised");
    lcd.setCursor(0, 1);
    lcd.print("Card!");
    delay(2000);
    lcd.clear();
    rfid.PICC_HaltA();
    return;
  }

  // Check if already voted
  if (hasVotedStatus[voterIndex]) {
    lcd.clear();
    lcd.print("Vote Already");
    lcd.setCursor(0, 1);
    lcd.print("Casted!");
    delay(2000);
    lcd.clear();
    rfid.PICC_HaltA();
    return;
  }

  // Allow voting
  lcd.clear();
  lcd.print("Vote Now");
  lcd.setCursor(0, 1);
  lcd.print("BTN1-4 to Vote");
  delay(1500);

  int choice = waitForVote();
  if (choice != -1) {
    votes[choice]++;
    hasVotedStatus[voterIndex] = true;

    lcd.clear();
    lcd.print("Voted for");
    lcd.setCursor(0, 1);
    lcd.print("Party ");
    lcd.print(choice + 1);
    delay(2000);
  }

  lcd.clear();
  rfid.PICC_HaltA();
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

int getVoterIndex(String uid) {
  for (int i = 0; i < 2; i++) {
    if (authorisedCards[i] == uid) return i;
  }
  return -1;
}

int waitForVote() {
  while (true) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttons[i]) == LOW) {
        return i; // Candidate 1-4
      }
    }
  }
}

void showWinner() {
  lcd.clear();
  lcd.print("Voting Over");
  delay(2000);

  // Find winner
  int maxVotes = 0;
  int winner = -1;
  bool tie = false;

  for (int i = 0; i < 4; i++) {
    if (votes[i] > maxVotes) {
      maxVotes = votes[i];
      winner = i;
      tie = false;
    } else if (votes[i] == maxVotes && votes[i] != 0) {
      tie = true;
    }
  }

  lcd.clear();
  if (tie) {
    lcd.print("It's a Tie!");
  } else {
    lcd.print("Winner: Party");
    lcd.setCursor(0, 1);
    lcd.print(winner + 1);
  }
delay(5000);
}
