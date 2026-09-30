#include <ESP_Mail_Client.h>

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587

#define SENDER_EMAIL "jedlik.orarend@gmail.com"
#define SENDER_PASSWORD "duzj roob gwsm xdwn"

/* Recipient's email*/
#define RECIPIENT_EMAIL "soos.gabor@jedlik.eu"

/* Declare the global used SMTPSession object for SMTP transport */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void emailSend(int akku) {
  MailClient.networkReconnect(true);
  smtp.debug(1);
  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);
  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;
  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = SENDER_EMAIL;
  config.login.password = SENDER_PASSWORD;
  config.login.user_domain = F("127.0.0.1");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;
  /* Declare the message class */
  SMTP_Message message;
  if (ANALOG_PIN == 36) {  // Az ESP32 V4.0 és az ESP32-E lapkák esetén. Ezeket az értékeket kalibrálni kell!
    warningAccuLevel = 1900;
    criticalAccuLevel = 1850;
    urgentAccuLevel = 1800;
  } else {  //ESP32-6 Ezeket az értékeket kalibrálni kell!
    warningAccuLevel = 1900;
    criticalAccuLevel = 1850;
    urgentAccuLevel = 1800;
  }

  /* Set the message headers */
  String nameMsg = "Jedlik órarend " + Terem;
  message.sender.name = nameMsg;
  message.sender.email = SENDER_EMAIL;
  if (akku < urgentAccuLevel) message.subject = F("Sürgős figyelmeztetés elem lemerülésére! Sürgősen töltés szükséges!!!");
  else if (akku < criticalAccuLevel) message.subject = F("Kritikus figyelmeztetés elem lemerülésére! Kritikus figyelmeztetés, töltés szükséges!!");
  else if (akku < warningAccuLevel) message.subject = F("Figyelmeztetés elem lemerülésére! Töltés szükséges!");
  message.addRecipient(F("Órarend admin"), RECIPIENT_EMAIL);
  //Send raw text message
  String textMsg = Terem + " terem órarend elemfeszültsége 3,3 V alá esett! Töltés szükséges!\n";
  textMsg += "Akku feszültség: " + String((akku / 2048.0) * 3.3) + " Volt\n";
  textMsg += "Az eddigi frissülések száma: " + String(bootCount) + "\n";
  textMsg += "Az aktuális frissülés ideje: " + String(aktHo) + ". hónap " + String(aktNap) + ". nap " + String(aktOraSzam) + " óra " + String(aktPercSzam) + " perc " + String(aktMpSzam) + " mp\n";
  textMsg += "A következő frissülés " + String(timeToSleep) + " mp múlva lesz,\n";
  textMsg += "tehát a következő frissülés " + String(timeToSleep / 86400) + " nap, " + String((timeToSleep % 86400) / 3600) + " óra, " + String((timeToSleep % 3600) / 60) + " perc múlva lesz.\n";
  message.text.content = textMsg;
  message.text.charSet = F("utf-8");
  message.text.transfer_encoding = "base64";
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // set the custom message header
  message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  /* Connect to the server */
  if (!smtp.connect(&config)) {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()) {
    DEBUG_PRINTLN("\nNot yet logged in.");
  } else {
    if (smtp.isAuthenticated())
      DEBUG_PRINTLN("\nSuccessfully logged in.");
    else
      DEBUG_PRINTLN("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  DEBUG_PRINTLN(status.info());

  /* Print the sending result */
  if (status.success()) {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    DEBUG_PRINTLN("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    DEBUG_PRINTLN("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    DEBUG_PRINTLN("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}