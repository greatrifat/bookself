#include <FirebaseESP8266.h>
#include <Arduino.h>
#include  <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>
#include <Servo.h>

#define FIREBASE_HOST "fir-bookcase-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH  "zCyCPtQKzdB2QJeMFpjCtG0xi0V6rPy3ZL5biHvV"
#define WIFI_SSID "Endeavour" // Change the name of your WIFI
#define WIFI_PASSWORD "mathematics" // Change the password of your WIFI

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

/* The sign in credentials */
#define AUTHOR_EMAIL "aimersteam007@gmail.com"
#define AUTHOR_PASSWORD "ywvflikecxyphgre"


FirebaseData firebaseData;
FirebaseData EmailData;
FirebaseData returnData;
FirebaseData statusData;
FirebaseJson json;

//---------------Servo-----------------------------------------
Servo myservo; 
//-------------mail----------
/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);
//==============END OF HEAD=================


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  Serial.print("Connecting...");
  WiFi.begin (WIFI_SSID, WIFI_PASSWORD);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  } 
  Serial.println("WiFi connected.");
  Serial.println("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);



  //-------------------Servo----------------
  myservo.attach(D2);
}

void loop() {
  // put your main code here, to run repeatedly:
  float done = 3.0;
  if (Firebase.getString(EmailData, "/Bookcase_micro/Email")){      
      Serial.println(EmailData.stringData()); 
  }
  delay(1000);

  if (Firebase.getString(statusData, "/Bookcase_micro/status")){
      Serial.println("status=");     
      Serial.print(statusData.stringData());
      if (statusData.stringData() == "2") {
      Serial.println("email sending");
      sendEmail(EmailData.stringData());  
      if (Firebase.setFloat(statusData, "/Bookcase_micro/status", done)) //passing done=3.0 to firebase
        {
          Serial.println("email sent status PASSED"); 
        }
      }   
  }
  delay(100);

  if (Firebase.getString(returnData, "/Bookcase_micro/return")){
      Serial.println("return=");
      Serial.print(returnData.stringData());
      if (returnData.stringData() == "0") { 
      Serial.println("return box open");
      myservo.write(80);  // door open 
      
      delay(3000);
      myservo.write(0);
      if (Firebase.setFloat(returnData, "/Bookcase_micro/return", done)) //passing done=2.0 to firebase
          {
            Serial.println("return box closed PASSED");
          }
      }
  }
  delay(100);  

}

void sendEmail(String email){
  
  /* The SMTP Session object used for Email sending */
  SMTPSession smtp;
  delay(2000);
  /* Callback function to get the Email sending status */
  void smtpCallback(SMTP_Status status);
  
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;
  delay(2000);
 /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "Smart Book Self";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Team Aimers";
  message.addRecipient("Sara", email);

  /*Send HTML message*/
  String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Please return your book with in 7 days</h1><p>- Sent from Smart Bookself</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

   /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());


}
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");
  }
}
