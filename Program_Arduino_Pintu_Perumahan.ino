#include "CTBot.h"
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

CTBot myBot;
CTBotInlineKeyboard myKbd;  // custom inline keyboard object helper

#define DAFTAR_FINGERPRINT_CALLBACK  "daftarFingerprint"
#define HAPUS_FINGERPRINT_CALLBACK "hapusFingerprint"
#define CEK_FINGERPRINT_CALLBACK "cekFingerprint"
#define RESET_FINGERPRINT_CALLBACK "resetFingerprint"

String ssid = "Luioni";     // REPLACE mySSID WITH YOUR WIFI SSID
String pass = "12121212"; // REPLACE myPassword YOUR WIFI PASSWORD, IF ANY
String token = "5372679986:AAE37-eDQEBHY6KmuKHRHbd02xLjEf27qIE";   // REPLACE myToken WITH YOUR TELEGRAM BOT TOKEN

TBMessage msg;

Servo myservo;
int pos = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)

SoftwareSerial mySerial(2, 0);
#else

#define mySerial Serial1

#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

String systemMode;

void setup() {
  myservo.attach(13);
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   Memulai...   ");
  Serial.println("Starting TelegramBot...");

  myBot.wifiConnect(ssid, pass);

  myBot.setTelegramToken(token);

  if (myBot.testConnection())
    Serial.println("\nKoneksi Berhasil OK");
  else
    Serial.println("\nKoneksi Tidak Berhasil NotOK");

  systemMode = "BukaGerbang";

  while (!Serial);
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  finger.begin(57600);
  finger.getTemplateCount();

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" SENSOR FINGER  ");
    lcd.setCursor(0, 1);
    lcd.print("  PRINT GAGAL   ");

    delay(2000);
    ESP.restart();

  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

}

void loop() {
  //Serial.println(systemMode);
  if (systemMode == "BukaGerbang") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      MOHON     ");
    lcd.setCursor(0, 1);
    lcd.print(" TEMPELKAN JARI ");
    int idFinger = 0;
    idFinger = cekFingerprint();
    if (idFinger > 0) {

      for (pos = 0; pos <= 180; pos += 1) {
        myservo.write(pos);
        delay(5);
      }
      delay(5000);
      for (pos = 180; pos >= 0; pos -= 1) {
        myservo.write(pos);
        delay(5);
      }

      Serial.println("Sidik Jari Ditemukan");
      lcd.clear();
      lcd.print("  SELAMAT DATANG");
      lcd.setCursor(0, 1);
      lcd.print("  SAMPAI JUMPA  ");
      delay(2000);
      myBot.sendMessage(msg.sender.id, "ID Finger #" + (String)idFinger + " ditemukan. \nSelamat Datang dan Sampai Jumpa ");

    }

    if (myBot.getNewMessage(msg)) {
      if (msg.messageType == CTBotMessageText) {
        if (msg.text.equalsIgnoreCase("/sidikjari")) {
          fingerPrintMode();
        } else {
          myBot.sendMessage(msg.sender.id, "Silahkan pilih menu pada bagian menu command");
        }
      } else if (msg.messageType == CTBotMessageQuery) {
        fingerPrintTele();
      }
    }
  } else if (systemMode == "Daftar Fingerprint") {
    daftarFingerprintTele();
  } else if (systemMode == "Hapus Fingerprint") {
    lcd.clear();
    lcd.print("     MEMULAI    ");
    lcd.setCursor(0, 1);
    lcd.print("  PEMELIHARAAN  ");
    hapusFingerprintTele();
  } else if (systemMode == "Cek Fingerprint") {
    cekFingerprintTele();
  } else if (systemMode == "Reset Fingerprint") {
    lcd.clear();
    lcd.print("     MEMULAI    ");
    lcd.setCursor(0, 1);
    lcd.print("  PEMELIHARAAN  ");
    resetFingerprintTele();
  }
}

void fingerPrintMode() {
  myKbd.flushData();
  myKbd.addButton("Daftar", DAFTAR_FINGERPRINT_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Hapus", HAPUS_FINGERPRINT_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Cek", CEK_FINGERPRINT_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Reset Fingerprint", RESET_FINGERPRINT_CALLBACK, CTBotKeyboardButtonQuery);
  myBot.sendMessage(msg.sender.id, "Silahkan pilih mode fingerprint dibawah:", myKbd);
}
void fingerPrintTele() {
  if (msg.callbackQueryData.equals(DAFTAR_FINGERPRINT_CALLBACK)) {
    systemMode = "Daftar Fingerprint";
    myKbd.flushData();
    myKbd.addButton("Batal", "batalFingerprint", CTBotKeyboardButtonQuery);
    myBot.endQuery(msg.callbackQueryID, "Mode telah diubah ke pendaftaran fingerprint", true);
    myBot.sendMessage(msg.sender.id, "Masukkan angka ID Fingerprint yang ingin didaftarkan:", myKbd);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   PROSES...    ");

    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");

  } else if (msg.callbackQueryData.equals(HAPUS_FINGERPRINT_CALLBACK)) {
    systemMode = "Hapus Fingerprint";
    myKbd.flushData();
    myKbd.addButton("Batal", "batalFingerprint", CTBotKeyboardButtonQuery);
    myBot.endQuery(msg.callbackQueryID, "Mode telah diubah ke hapus fingerprint", true);
    myBot.sendMessage(msg.sender.id, "Masukkan angka ID Fingerprint yang ingin dihapus:", myKbd);

  } else if (msg.callbackQueryData.equals(CEK_FINGERPRINT_CALLBACK)) {
    systemMode = "Cek Fingerprint";
    myKbd.flushData();
    myKbd.addButton("Batal", "batalFingerprint", CTBotKeyboardButtonQuery);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   TEMPELKAN    ");
    lcd.setCursor(0, 1);
    lcd.print("      JARI      ");
    myBot.endQuery(msg.callbackQueryID, "Mode telah diubah ke pengecekan fingerprint", true);
    myBot.sendMessage(msg.sender.id, "Silahkan tempelkan jari pada fingerprint.", myKbd);
  } else if (msg.callbackQueryData.equals(RESET_FINGERPRINT_CALLBACK)) {
    systemMode = "Reset Fingerprint";
    myKbd.flushData();
    myKbd.addButton("Batal", "batalFingerprint", CTBotKeyboardButtonQuery);
    myBot.endQuery(msg.callbackQueryID, "Mode telah diubah ke reset fingerprint", true);
    myBot.sendMessage(msg.sender.id, "Balas 'Y' (tanpa tanda kutip) untuk melanjutkan proses reset data.", myKbd);
  }
}


// Informasi Angka
boolean isValidNumber(String str) {
  boolean isNum = false;
  for (byte i = 0; i < str.length(); i++)
  {
    isNum = isDigit(str.charAt(i)) || str.charAt(i) == ' +' || str.charAt(i) == '.' || str.charAt(i) == ' -';
    if (!isNum) return false;
  }
  return isNum;
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}




//Sistem Menu Sidik Jari
void daftarFingerprintTele() {
  if (myBot.getNewMessage(msg)) {
    if (msg.messageType == CTBotMessageText) {
      id = msg.text.toInt();
      if (isValidNumber(msg.text) and id > 0 and id < 128) {
        Serial.print("Enrolling ID #");
        Serial.println(id);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   TEMPELKAN    ");
        lcd.setCursor(0, 1);
        lcd.print("      JARI      ");
        getFingerprintEnroll();
        systemMode = "BukaGerbang";
        myBot.sendMessage(msg.sender.id, "ID Fingerprint #" + msg.text + " berhasil terdaftar. \nMode telah berubah menjadi " + systemMode + " .");
      } else if (!isValidNumber(msg.text)) {
        myBot.sendMessage(msg.sender.id, "ID hanya angka! silahkan masukkan ID dengan benar.");
      } else if (id > 127) {
        myBot.sendMessage(msg.sender.id, "ID dilarang lebih dari 127! \nMasukkan angka ID yang ingin didaftarkan:");
      } else {
        myBot.sendMessage(msg.sender.id, "ID 0 dilarang! \nMasukkan angka ID yang ingin didaftarkan:");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      if (msg.callbackQueryData.equals("batalFingerprint")) {
        batalFingerprint();
      }
    }
  }
}

void cekFingerprintTele() {
  bool canceled = false;
  lcd.clear();
  lcd.print("     TEMPELKAN  ");
  lcd.setCursor(0, 1);
  lcd.print("    SIDIK JARI  ");
  Serial.println("Menunggu sidik jari ditempelkan...");
  int idFinger = 0;
  do {    // wait for sensors to stabilize
    idFinger = cekFingerprint();  // check the sensors
    if (myBot.getNewMessage(msg)) {
      if (msg.messageType == CTBotMessageQuery) {
        if (msg.callbackQueryData.equals("batalFingerprint")) {
          canceled = true;
          break;
        }
      }
    }
  } while (idFinger < 0);
  systemMode = "BukaGerbang";
  if (canceled) {
    batalFingerprint();
  } else {
    Serial.println("Sidik Jari ditemukan");
    lcd.clear();
    lcd.print("   SIDIK JARI   ");
    lcd.setCursor(0, 1);
    lcd.print("   DITEMUKAN    ");
    myBot.sendMessage(msg.sender.id, "ID Finger #" + (String)idFinger + " ditemukan. \nMode telah berubah menjadi " + systemMode + " .");
    delay(2000);
  }
}


void hapusFingerprintTele() {
  if (myBot.getNewMessage(msg)) {
    if (msg.messageType == CTBotMessageText) {
      id = msg.text.toInt();
      if (isValidNumber(msg.text)) {

        Serial.print("Deleting ID #");
        Serial.println(id);

        deleteFingerprint(id);

        systemMode = "BukaGerbang";
        myBot.sendMessage(msg.sender.id, "ID Fingerprint #" + msg.text + " berhasil dihapus. \nMode telah berubah menjadi " + systemMode + " .");
      } else {
        myBot.sendMessage(msg.sender.id, "ID hanya angka! silahkan masukkan ID dengan benar.", myKbd);
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      if (msg.callbackQueryData.equals("batalFingerprint")) {
        batalFingerprint();
      }
    }
  }
}


void resetFingerprintTele() {
  if (myBot.getNewMessage(msg)) {
    if (msg.messageType == CTBotMessageText) {

      if (msg.text.equals("Yes")) {

        Serial.println("Reseting Fingerprint");

        finger.emptyDatabase();

        Serial.println("Now database is empty :)");

        systemMode = "BukaGerbang";
        myBot.sendMessage(msg.sender.id, "Database fingerprint berhasil direset. \nMode telah berubah menjadi " + systemMode + " .");
      } else {
        myBot.sendMessage(msg.sender.id, "Balas 'Yes' (tanpa tanda kutip) untuk melanjutkan proses reset data.", myKbd);
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      if (msg.callbackQueryData.equals("batalFingerprint")) {
        batalFingerprint();
      }
    }
  }
}

void batalFingerprint() {
  systemMode = "BukaGerbang";
  myBot.sendMessage(msg.sender.id, "Aksi telah dibatalkan. \nMode telah berubah menjadi " + systemMode + " .");
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      DATA      ");
  lcd.setCursor(0, 1);
  lcd.print("   DITEMUKAN    ");

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);



  return finger.fingerID;
}

int cekFingerprint() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  return finger.fingerID;
}

void fingerNotFound() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     TIDAK      ");
  lcd.setCursor(0, 1);
  lcd.print("   DITEMUKAN    ");
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  lcd.setCursor(0, 0);
  lcd.print("    LEPASKAN    ");
  lcd.setCursor(0, 1);
  lcd.print("      JARI      ");
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.setCursor(0, 0);
  lcd.print(" TEMPELKAN JARI ");
  lcd.setCursor(0, 1);
  lcd.print("    KEMBALI     ");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
