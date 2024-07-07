/* 
Most of the code is from a example of the Esp_Mail_Client, which I did not wrote. Make sure you take this library and not the esp32 mail client, which is no longer updated.
This is not perfect, this was a quick project and just a way to start a habbit in using github, so this is the first finished hobby project of hopefully many.
If you find any mistakes, suggestions or better ways of using github; feel free to contact me.

Good luck!
*/

#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif

#include <ESP_Mail_Client.h>

//define wifi
struct WiFiCredentials {
  const char* ssid;
  const char* password;
};

// List of WiFi credentials
WiFiCredentials wifiNetworks[] = {
  {"ssid1", "Pass1"},
  {"ssid2", "Pass2"},
}; //update for your own wifi credentials

const int wifiCount = sizeof(wifiNetworks) / sizeof(WiFiCredentials);


/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com */
#define SMTP_HOST "smtp.gmail.com"

/** The smtp port e.g.
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
 */
#define SMTP_PORT esp_mail_smtp_port_587 

/* The log in credentials, check guide to get a application password for your gmail, only possible if you have 2 factor authentication enabled */
#define AUTHOR_EMAIL "Sender@gmail.com"
#define AUTHOR_PASSWORD "SenderPassword"

/* Recipient email address */
//#define Recipient "test"
#define Ontvanger "Recipient1@gmail.com"
#define Ontvanger2 "Recipient2@gmail.com"
#define Ontvanger3 "Recipient3@gmail.com"

//message of the email:
#define Boodschap "Dag Moeke,\r\n\r\n Lotte heeft jouw nodig, bel je haar eens op? \r\n\r\n Groetjes van de ICE-knop van Lotte." //\r\n\r\n = new line start after this.
#define Boodschap2 "Dag Papa,\r\n\r\n Lotte heeft jouw nodig, bel je haar eens op? \r\n\r\n Groetjes van de ICE-knop van Lotte."
#define Boodschap3 "Dag Broerje,\r\n\r\n Lotte heeft jouw nodig, bel je haar eens op? \r\n\r\n Groetjes van de ICE-knop van Lotte."


/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Declare the Session_Config for user defined session credentials */
Session_Config config;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

// Define the button and LED pins.
const int buttonPin = 3;
const int buttonPin2 = 4;
const int buttonPin3 = 5;

const int ledPin = 6;
const int ledPin2 = 7;
const int ledPin3 = 8;

bool buttonState = false;
bool lastButtonState = false;
bool buttonState2 = false;
bool lastButtonState2 = false;
bool buttonState3 = false;
bool lastButtonState3 = false;

void setup()
{
  Serial.begin(115200);
  //while (!Serial);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(ledPin2, OUTPUT);

  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(ledPin3, OUTPUT);

  connectToWiFi();

  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);


  /** Enable the debug via Serial port
    * 0 for no debugging
    * 1 for basic level debugging
    *
    * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
    */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  /** Assign your host name or you public IPv4 or IPv6 only
    * as this is the part of EHLO/HELO command to identify the client system
    * to prevent connection rejection.
    * If host name or public IP is not available, ignore this or
    * use loopback address "127.0.0.1".
    *
    * Assign any text to this option may cause the connection rejection.
    */
  config.login.user_domain = F("127.0.0.1");
  
  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  SetupLED();

}

void loop()
{
  buttonState = digitalRead(buttonPin);
  buttonState2 = digitalRead(buttonPin2);
  buttonState3 = digitalRead(buttonPin3);

 
  if (buttonState == LOW && lastButtonState == HIGH) {
    digitalWrite(ledPin, HIGH); // Turn on the LED when the button is pressed
    Serial.println("Moeke");
    SendEmail(Ontvanger, Boodschap);
    flashLED(ledPin); // Flash the LED after sending the email
  } else {
    digitalWrite(ledPin, LOW); // Turn off the LED when the button is released
  }

  if (buttonState2 == LOW && lastButtonState2 == HIGH) {
    digitalWrite(ledPin2, HIGH); // Turn on the LED when the button is pressed
    Serial.println("Papa");
    SendEmail(Ontvanger2, Boodschap2);
    flashLED(ledPin2); // Flash the LED after sending the email
  } else {
    digitalWrite(ledPin2, LOW); // Turn off the LED when the button is released
  }

  if (buttonState3 == LOW && lastButtonState3 == HIGH) {
    digitalWrite(ledPin3, HIGH); // Turn on the LED when the button is pressed
    Serial.println("Jasper");
    SendEmail(Ontvanger3, Boodschap3);
    flashLED(ledPin3); // Flash the LED after sending the email
  } else {
    digitalWrite(ledPin3, LOW); // Turn off the LED when the button is released
  }

  lastButtonState = buttonState;
  lastButtonState2 = buttonState2;
  lastButtonState3 = buttonState3;
}

void SendEmail(const char* Recipient, const char* Inhoud){
   /* Declare the message class */
    SMTP_Message message;

    /* Set the message headers */
    /* Set the message headers */
    message.sender.name = F("Lotte's ICE-knop");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = F("Lotte Heeft u nodig");
    message.addRecipient(F("Someone"), Recipient);

    /** The option to add soft line break to to the message for
     * the long text message > 78 characters (rfc 3676)
     * Some Servers may not compliant with this standard.
     */
    message.text.flowed = true;

    /** if the option message.text.flowed is true,
     * the following plain text message will be wrapped.
     */
    // message.text.content = F("The text below is the long quoted text which breaks into several lines.\r\n\r\n>> Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\r\n\r\nThis is the normal short text.\r\n\r\nAnother long text, abcdefg hijklmnop qrstuv wxyz abcdefg hijklmnop qrstuv wxyz abcdefg hijklmnop qrstuv wxyz.");
    message.text.content = Inhoud ;
    /** The Plain text message character set e.g.
     * us-ascii
     * utf-8
     * utf-7
     * The default value is utf-8
     */
    //message.text.charSet = F("us-ascii");

    /** The content transfer encoding e.g.
     * enc_7bit or "7bit" (not encoded)
     * enc_qp or "quoted-printable" (encoded)
     * enc_base64 or "base64" (encoded)
     * enc_binary or "binary" (not encoded)
     * enc_8bit or "8bit" (not encoded)
     * The default value is "7bit"
     */
    message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    /** The message priority
     * esp_mail_smtp_priority_high or 1
     * esp_mail_smtp_priority_normal or 3
     * esp_mail_smtp_priority_low or 5
     * The default value is esp_mail_smtp_priority_low
     */
    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

    /** The Delivery Status Notifications e.g.
     * esp_mail_smtp_notify_never
     * esp_mail_smtp_notify_success
     * esp_mail_smtp_notify_failure
     * esp_mail_smtp_notify_delay
     * The default value is esp_mail_smtp_notify_never
     */
    // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    /* Set the custom message header */
    message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

    /* Connect to the server */
    if (!smtp.connect(&config))
    {
        MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return;
    }

    if (!smtp.isLoggedIn())
    {
        Serial.println("Not yet logged in.");
    }
    else
    {
        if (smtp.isAuthenticated())
            Serial.println("Successfully logged in.");
        else
            Serial.println("Connected with no Auth.");
    }

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message))
        MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

    // to clear sending result log
    // smtp.sendingResult.clear();

    MailClient.printf("Free Heap: %d\n", MailClient.getFreeHeap());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
    /* Print the current status */
    Serial.println(status.info());

    /* Print the sending result */
    if (status.success())
    {
        // MailClient.printf used in the examples is for format printing via debug Serial port
        // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
        // In ESP8266 and ESP32, you can use Serial.printf directly.

        Serial.println("----------------");
        MailClient.printf("Message sent success: %d\n", status.completedCount());
        MailClient.printf("Message sent failed: %d\n", status.failedCount());
        Serial.println("----------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            /* Get the result item */
            SMTP_Result result = smtp.sendingResult.getItem(i);

            // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
            // your device time was synched with NTP server.
            // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
            // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

            MailClient.printf("Message No: %d\n", i + 1);
            MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
            MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
            MailClient.printf("Recipient: %s\n", result.recipients.c_str());
            MailClient.printf("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("----------------\n");

        // You need to clear sending result as the memory usage will grow up.
        smtp.sendingResult.clear();
    }
}

void flashLED(int ledPinx) {
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPinx, HIGH);
    delay(500);
    digitalWrite(ledPinx, LOW);
    delay(500);
  }
  digitalWrite(ledPinx, HIGH);
  delay(1000);
  digitalWrite(ledPinx, LOW);
  delay(500);
}

void SetupLED() { //just a slightly more appealing visual, very poorly coded ;)
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin3, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    delay(500);
  }
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin3, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  //delay(500);
}

void LoadingLed() { // nice visual effect while waiting for wifi, again just poorly coded but it works and I was ready on time (like 30 minutes before having to leave to the party...
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);  
  digitalWrite(ledPin2, HIGH);
  delay(200);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, HIGH);
  delay(200);
  digitalWrite(ledPin3, LOW);
  delay(100); 
}

void connectToWiFi() { //Connecting to multiple wifi's so I could show it with my hotspot on my phone and a powerbank, but also already registered the wifi of the house it was going to end up in.
  bool connected = false;
  for (int i = 0; i < wifiCount; i++) {
    WiFi.begin(wifiNetworks[i].ssid, wifiNetworks[i].password);
    Serial.print("Connecting to Wi-Fi: ");
    Serial.print(wifiNetworks[i].ssid);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      Serial.print(".");
      LoadingLed();
      //delay(500);
      attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected to ");
      Serial.print(wifiNetworks[i].ssid);
      Serial.print(" with IP: ");
      Serial.println(WiFi.localIP());
      connected = true;
      break;
    } else {
      Serial.print("Failed to connect to ");
      Serial.println(wifiNetworks[i].ssid);
    }
  }

  if (!connected) {
    Serial.println("Could not connect to any WiFi network.");
    // Optional: add logic to handle failure to connect to any WiFi network
  }
  
}
