
// ********************* SD *********************
// https://create.arduino.cc/projecthub/electropeak/sd-card-module-with-arduino-how-to-read-write-data-37f390
#include <SPI.h>
#include <SD.h>

File logfile;

void setupSd() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
}

// ********************* RTC ********************
#include <RTClib.h>
RTC_DS1307 rtc;

void setupRtc() {
#ifndef ESP8266
  while (!Serial);
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}


// ********************* RC *********************
#include <RCSwitch.h>
RCSwitch rcSwitch = RCSwitch();

void setupRc() {
  rcSwitch.enableReceive(0);
}


// ********************* setup ******************
void setup() {
  Serial.begin(9600);

  setupSd();
  setupRtc();
  setupRc();
}


// ********************* loop *******************
void loop() {

  if (rcSwitch.available()) {

    digitalWrite(13, HIGH); // Toggle the onboard LED  if serial is available - Optional
    delay(1);
    digitalWrite(13, LOW);

    DateTime time = rtc.now();

    String timestamp = String(time.timestamp(DateTime::TIMESTAMP_DATE)) + " " + String(time.timestamp(DateTime::TIMESTAMP_TIME)) + ": ";

    outputSerial(rcSwitch.getReceivedValue(), rcSwitch.getReceivedBitlength(), rcSwitch.getReceivedDelay(), rcSwitch.getReceivedRawdata(), rcSwitch.getReceivedProtocol(), timestamp);

    logfile = SD.open("log.txt", FILE_WRITE);

    if (logfile) {
      logfile.print(String(time.timestamp(DateTime::TIMESTAMP_DATE)));
      logfile.print(" ");
      logfile.print(String(time.timestamp(DateTime::TIMESTAMP_TIME)));
      logfile.println();
      outputFile(rcSwitch.getReceivedValue(), rcSwitch.getReceivedBitlength(), rcSwitch.getReceivedDelay(), rcSwitch.getReceivedRawdata(), rcSwitch.getReceivedProtocol(), timestamp, logfile);

      logfile.close();

      Serial.print(timestamp);
      Serial.println("Saved to file");
    } else {
      Serial.print(timestamp);
      Serial.println("Save to file failed");
    }

    rcSwitch.resetAvailable();
  }
}

static const char* bin2tristate(const char* bin);
static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);

void outputSerial(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol, String timestamp) {

  const char* b = dec2binWzerofill(decimal, length);
  Serial.print(timestamp);
  Serial.print("Decimal: ");
  Serial.print(decimal);
  Serial.print(" (");
  Serial.print( length );
  Serial.print("Bit) Binary: ");
  Serial.print( b );
  Serial.print(" Tri-State: ");
  Serial.print( bin2tristate( b) );
  Serial.print(" PulseLength: ");
  Serial.print(delay);
  Serial.print(" microseconds");
  Serial.print(" Protocol: ");
  Serial.println(protocol);

  Serial.print(timestamp);
  Serial.print("Raw data: ");
  for (unsigned int i = 0; i <= length * 2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
}

void outputFile(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol, String timestamp, File file) {

  const char* b = dec2binWzerofill(decimal, length);
  file.print(timestamp);
  file.print("Decimal: ");
  file.print(decimal);
  file.print(" (");
  file.print( length );
  file.print("Bit) Binary: ");
  file.print( b );
  file.print(" Tri-State: ");
  file.print( bin2tristate( b) );
  file.print(" PulseLength: ");
  file.print(delay);
  file.print(" microseconds");
  file.print(" Protocol: ");
  file.println(protocol);

  file.print(timestamp);
  file.print("Raw data: ");
  for (unsigned int i = 0; i <= length * 2; i++) {
    file.print(raw[i]);
    file.print(",");
  }
  file.println();
}

static const char* bin2tristate(const char* bin) {
  static char returnValue[50];
  int pos = 0;
  int pos2 = 0;
  while (bin[pos] != '\0' && bin[pos + 1] != '\0') {
    if (bin[pos] == '0' && bin[pos + 1] == '0') {
      returnValue[pos2] = '0';
    } else if (bin[pos] == '1' && bin[pos + 1] == '1') {
      returnValue[pos2] = '1';
    } else if (bin[pos] == '0' && bin[pos + 1] == '1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos + 2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i = 0;

  while (Dec > 0) {
    bin[32 + i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j < bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}
