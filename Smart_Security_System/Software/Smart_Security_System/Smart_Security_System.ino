/*******************************************************************
    A telegram bot that sends you a message when ESP
    starts up
    
    The button must be pressed to set the alarm. 
    It will be in sleep mode unless the button is pressed. 
    When we read the defined rfid card, the system puts 
    itself into sleep mode. It starts again when the undefined 
    rfid card is read.

    Written by Emre Erkani
    Date: 22.03.2023
    Revison Date: 01.04.2023 
    Version: V1.0
    Revision 2 Date: 25.04.2023
    Version: V1.1
    LinkedIn : https://www.linkedin.com/in/emreerkani/
 *******************************************************************/

#include <WiFi.h>                         //wifi kütüphanesini ekliyoruz.
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>         // telegram bot kütüphanesini ekliyoruz.
#include <ArduinoJson.h>

#include<SPI.h>                          // rfid için spı kütüphanesini ekliyoruz.
#include<MFRC522.h>                      // rfid kütüphanesini ekliyoruz.

#define RST_PIN         4               // rfid reset pin
#define SS_PIN          5               // rfid data pin

MFRC522 mfrc522(SS_PIN, RST_PIN);       


#define WIFI_SSID "xxxxx"      // wifi adı
#define WIFI_PASSWORD "xxxxx"      // wifi şifresi


#define BOT_TOKEN "xxxxx"      // telegram http ıp kodu


#define CHAT_ID "xxxxx"            // telegram ıd

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const int hareketSensoru = 27;
bool hareketAlgilandi = false;
int hareketvar=0;
const int buton = 25;
int sayac = 0;
const int led = 26;

const int led_yesil = 22 ;
const int led_kirmizi = 21 ;

const int buzzer = 2;

// pır sensörü hareket algılaması için yapılan kesme fonksiyonu
void IRAM_ATTR ISR() {
  Serial.println("HAREKET ALGILANDI!!!");
  hareketAlgilandi = true;
  hareketvar=1;
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  pinMode(hareketSensoru, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hareketSensoru), ISR, RISING); 

  SPI.begin();                  // SPI haberleşmesi başlatılıyor.
  mfrc522.PCD_Init();           // kartı çalıştırılıyor.

  pinMode(led_yesil, OUTPUT);
  pinMode(led_kirmizi, OUTPUT);  

  pinMode(buton, INPUT);
  pinMode(led,OUTPUT);

  pinMode(buzzer,OUTPUT);

 //wifi bağlantı aşamaları
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  bot.sendMessage(CHAT_ID, "Sistem Baslatildi", "");  // sistem başlatıldı mesajı telegrama gönderilir.
  Serial.println("Sistem baslatildi");                // sistem başlatıldı mesajı seri portta görülür.

}

void loop() {

// buton aktiflik kontol yapılanması
  int alarm = digitalRead(buton); 
  if(alarm == HIGH){
    hareketAlgilandi = false;
    hareketvar=0;
    sayac = 1;
    delay(100);
    bot.sendMessage(CHAT_ID, "Alarm Kuruldu!!", "");
    Serial.println("Alarm Kuruldu");
    delay(10000);
    attachInterrupt(digitalPinToInterrupt(hareketSensoru), ISR, RISING);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
    delay(100);
  }

  if(sayac == 1 ){
    if ((hareketAlgilandi) && hareketvar==1) {
    hareketvar++;
    //telegrama mesaj gönder
    bot.sendMessage(CHAT_ID, "Hareket Tespit Edildi!!", "");
    Serial.println("Hareket Tespit Edildi");
    hareketAlgilandi = false;
    }
  }

//rfid kart tanımlaması
   if(!mfrc522.PICC_IsNewCardPresent()){      // yeni kart yaklaştırıldı mı?
      return;
      }
     
    if(!mfrc522.PICC_ReadCardSerial()){      // var olan kart okundu mu?
      return;
      }
    Serial.print("Kart numarasi :");
    String bilgi = "";
    byte kod;
    for(byte i = 0; i<mfrc522.uid.size; i++){
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      bilgi.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      bilgi.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    // for kullanılma amacı her sayıyı birbirleri ile karşılaştırılarak 
    // sayı sistemi oluşturulur.
    Serial.println();
    Serial.print("Mesaj : ");
    bilgi.toUpperCase();
    if(bilgi.substring(1) == "B4 83 DA 1D" || bilgi.substring(1) == "07 EB B1 60" ){
      Serial.println("Giris kabul edildi");
      Serial.println();
      digitalWrite(led_yesil, HIGH);
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
      bot.sendMessage(CHAT_ID, "Giris Kabul Edildi", "");
      delay(1000);
      digitalWrite(led_yesil, LOW);

      sayac = 0;
      hareketAlgilandi = false;
      hareketvar = 0;
    }
    else{
      Serial.println("Giris reddedildi");
      digitalWrite(led_kirmizi, HIGH);
      digitalWrite(buzzer, HIGH);
      bot.sendMessage(CHAT_ID, "Giris Reddedildi", "");
      delay(1000);
      digitalWrite(led_kirmizi, LOW);
      digitalWrite(buzzer, LOW);
      attachInterrupt(digitalPinToInterrupt(hareketSensoru), ISR, RISING);
      sayac = 1;
    }
}
