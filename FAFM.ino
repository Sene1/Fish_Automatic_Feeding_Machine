#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 3

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

const int RST_PIN = 9; // RFID RST 핀
const int SS_PIN = 10; // RFID SS 핀
const int RELAY_PIN = 6; // 릴레이 핀
const int BUZZER_PIN = 5; // 수동부저 핀

MFRC522 mfrc522(SS_PIN, RST_PIN);   // MFRC522 인스턴스

byte CardUidByte[4] = {0x81, 0x99, 0x1E, 0x24}; // 플라스틱 카드 UID (1)
byte CardUidByte2[4] = {0xA3, 0x4B, 0x7E, 0x2F}; // 키링 UID (2)
boolean state = false; //서보모터 상태값

RTC_DS3231 rtc;
Servo servo;
const int SERVO_PIN = 7; // 서보모터 핀
int pos = 0; // 서보모터 각도
int buzzer = 0; // 수동부저 작동 수

char DOW[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);  
  while (!Serial);     
  SPI.begin();         // SPI 시작
  mfrc522.PCD_Init();  // MFRC522 card 초기화
  pinMode(RELAY_PIN, OUTPUT); // 릴레이 핀 (출력)
  pinMode(BUZZER_PIN, OUTPUT); // 수동부저 핀 (출력)
  servo.attach(SERVO_PIN);
  servo.write(0);
  lcd.init();
  lcd.backlight(); // LCD 백라이트 ON
  lcd.begin(16, 2); 
  rtc.begin();
  sensors.begin();
  delay(50);
}

void loop() {
  DateTime now = rtc.now();
  sensors.requestTemperatures(); // 수온센서에서 온도 불러오기
  Serial.print("Temperature is: "); 
  Serial.println(sensors.getTempCByIndex(0));
  lcd.clear(); // LCD 지우기

    lcd.setCursor(0, 0); // (0, 0)에 커서 설정
    lcd.print(now.year(), DEC); // 년
    lcd.print("/");
    lcd.print(now.month(), DEC); // 월
    lcd.print("/");
    lcd.print(now.day(), DEC); // 일
    lcd.print("(");
    lcd.print(DOW[now.dayOfTheWeek()]);
    lcd.print(")");
  
    lcd.setCursor(0, 1); // (0, 1)에 커서 설정
    lcd.print(now.hour(), DEC); // 시
    lcd.print(":");
    lcd.print(now.minute(), DEC); // 분
    lcd.print(":");
    lcd.print(now.second(), DEC); // 초
    lcd.print("  ");
    lcd.print(int(sensors.getTempCByIndex(0))); // 수온
    lcd.print(char(223));
    lcd.print("C");

    delay (1000);

  // 13시 10분에 서보모터 작동
  if(now.hour()==13&&now.minute()==10&&now.second()==0){
    for (pos = 0; pos <= 180; pos += 1) { 
      servo.write(pos);             
      delay(3);                      
    }
    for (pos = 180; pos >= 0; pos -= 1) { 
      servo.write(pos);              
      delay(3);                       
      }
  }

  // 어항 수온 제어 (23도 이하일 경우 히터 ON)
  if (sensors.getTempCByIndex(0)<23) {
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
    
  
  // 새 카드 확인
  if (!mfrc522.PICC_IsNewCardPresent()) return; 

  // 카드 읽기
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // 읽은 Card UID와 등록된 Card UID가 일치하는 확인 (1번 카드)
  if (mfrc522.uid.uidByte[0] == CardUidByte[0] && mfrc522.uid.uidByte[1] == CardUidByte[1] &&
        mfrc522.uid.uidByte[2] == CardUidByte[2] && mfrc522.uid.uidByte[3] == CardUidByte[3] ){

           // 서보모터 작동
           Serial.println("Open");
           for (pos = 0; pos <= 180; pos += 1) { 
              servo.write(pos);             
              delay(3); 
            }
           delay(1000);
           Serial.println("Close");
           for (pos = 180; pos >= 0; pos -= 1) { 
              servo.write(pos);              
              delay(3);                       
            }
           delay(2000);
   }
   // 읽은 Card UID와 등록된 Card UID가 일치하는 확인 (2번 카드)
   else if (mfrc522.uid.uidByte[0] == CardUidByte2[0] && mfrc522.uid.uidByte[1] == CardUidByte2[1] &&
        mfrc522.uid.uidByte[2] == CardUidByte2[2] && mfrc522.uid.uidByte[3] == CardUidByte2[3] ){

         // 서보모터 작동
           Serial.println("Open");
           for (pos = 0; pos <= 180; pos += 1) { 
              servo.write(pos);             
              delay(3); 
            }
           delay(1000);
           Serial.println("Close");
           for (pos = 180; pos >= 0; pos -= 1) { 
              servo.write(pos);              
              delay(3);                       
            }
           delay(2000);
   }
   // 알 수 없는 카드
   else 
   {
        Serial.println("Unknown");
        for (buzzer = 0; buzzer <= 2; buzzer += 1) { 
              tone(BUZZER_PIN, 3500);
              Serial.println(buzzer);              
              delay(200);
              noTone(BUZZER_PIN);                      
            }
        delay(2000);
   }
   
   }
