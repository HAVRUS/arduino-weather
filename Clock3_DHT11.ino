/* MH-Real-Time Clock Module 2 DS1302:
    VCC = 5V
    GND = GND
    CLK = D2
    DAT = D3
    RST = D4

   LCD1602 (I2C):
   VCC = 5V
   GND = GND
   SDA = A4
   SCL = A5

   DHT11:
   PLUS = 5V
   OUT = D8
   GND = GND

   BMP280 (I2C):
   VCC = 3.3v
   GND = GND
   SCL = A5
   SDA = A4
*/

#include <stdio.h>

#include <Wire.h> // Шина I2C

#include <DS1302.h>
#include <dht11.h>
#include <iarduino_Pressure_BMP.h>
#include <LCD_1602_RUS.h>

#define RST_PIN 4
#define DAT_PIN 3
#define CLK_PIN 2
#define dht11_PIN 8

DS1302 rtc(RST_PIN, DAT_PIN, CLK_PIN);
dht11 dht;
iarduino_Pressure_BMP bmp(0x76); // Адрес можно чекнуть через i2c_scanner
LCD_1602_RUS lcd(0x3f, 16, 2); // Задаем дисплей

char buf_full[50]; // Стандартный формат
char buf_day[17]; // Только дата
char buf_time[17]; // Только время

String dayAsString(const Time::Day day)
{
  switch (day)
  {
    case Time::kSunday: return "Sun"; // Вс
    case Time::kMonday: return "Mon"; // Пн
    case Time::kTuesday: return "Tue"; // Вт
    case Time::kWednesday: return "Wed"; // Ср
    case Time::kThursday: return "Thu"; // Чт
    case Time::kFriday: return "Fri"; // Пт
    case Time::kSaturday: return "Sat"; // Сб
  }
  return "(N/A)";
}

/*
  String dayAsString(const Time::Day day)
  {
  switch (day)
  {
    case Time::kSunday: return (L"Вс");
    case Time::kMonday: return (L"Пн");
    case Time::kTuesday: return (L"Вт");
    case Time::kWednesday: return (L"Ср");
    case Time::kThursday: return (L"Чт");
    case Time::kFriday: return (L"Пт");
    case Time::kSaturday: return (L"Сб");
  }
  return "(N/A)";
  }
*/

int count;
long count2;
bool count3;

byte updatedC[8] = // Массив для символа обновления показаний (*)
{
  B00000,
  B00000,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

void setup()
{
  Serial.begin(9600);

  rtc.writeProtect(false);
  rtc.halt(false);
  /*
  Time t(2018, 4, 01, 22, 42, 0, Time::kSunday); // ГГГГ, М, ДД, ЧЧ, ММ, СС, ДЕНЬ НЕДЕЛИ
  rtc.time(t); // Установить дату и время
  */
  
  bmp.begin();
  
  lcd.init();
  lcd.backlight(); // Включаем подсветку дисплея
  lcd.createChar(1, updatedC); // Создаем символ обновления показаний
}

void printTime()
{
  Time t = rtc.time(); // Получить дату и время
  const String day = dayAsString(t.day); // День недели
  snprintf(buf_full, sizeof(buf_full), "%s %02d-%02d-%04d %02d:%02d:%02d",
           day.c_str(),
           t.date, t.mon, t.yr,
           t.hr, t.min, t.sec);

  snprintf(buf_day, sizeof(buf_day), "%02d-%02d-%04d|%s ",
           t.date, t.mon, t.yr,
           day.c_str());

  snprintf(buf_time, sizeof(buf_time), " %02d:%02d:%02d |",
           t.hr, t.min, t.sec);
}

void checkStatus()
{
  if (count3 == false)
  {
    lcd.setCursor(15, 0); // Устанавливаем курсор на вторую строку и 15-й символ.
    lcd.print(char(1)); // Выводим на экран *
    count3 = true;
  }
  else
  {
    lcd.setCursor(15, 0); // Устанавливаем курсор на вторую строку и 15-й символ.
    lcd.print(" "); // Удаляем с экрана *
    count3 = false;
  }
}

void checkSensors()
{
  int chk;
  chk = dht.read(dht11_PIN);
  switch (chk)
  {
    case DHTLIB_OK:
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.println("DHT11: Checksum error, \t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.println("DHT11: Time out error, \t");
      break;
    default:
      Serial.println("DHT11: Unknown error, \t");
      break;
  }
  
  if (bmp.read(1) == 0)                             
  {
    Serial.println("BMP280: Connection error, \t");
  }
}

void sendData()
{
  if (count2 % 600 == 1) // Передаем данные через последовательный порт каждые 10 минут
  {
    Serial.println(buf_full);
    Serial.print("Temp: ");
    Serial.print(dht.temperature);
    Serial.print(" C");
    Serial.print(" Humidity: ");
    Serial.print(dht.humidity);
    Serial.print("%");
    Serial.print(" Pressure: ");
    Serial.print(bmp.pressure);
    Serial.println(" mm Hg");
    Serial.print("Count 1: ");
    Serial.print(count);
    Serial.print(" Count 2: ");
    Serial.print(count2);
    Serial.println(" ");
  }
}

void loop()
{
  printTime();
  checkSensors();
  checkStatus();

  int temperature_res = dht.temperature;
  int humidity_res = dht.humidity;
  int pressure_res = (int)bmp.pressure;
  
  count2 = (millis() / 1000);
  if ((count >= 20) && (count <= 29))
  {
    lcd.setCursor(11, 1); // Устанавливаем курсор на вторую строку и 11-й символ.
    lcd.print(pressure_res);
    lcd.setCursor(14, 1); // Устанавливаем курсор на вторую строку и 13-й символ.
    lcd.print("mm");
    count++;
  }
  else if ((count >= 10) && (count <= 19))
  {
    lcd.setCursor(11, 1); // Устанавливаем курсор на вторую строку и 11-й символ.
    lcd.print(temperature_res);
    lcd.setCursor(13, 1); // Устанавливаем курсор на вторую строку и 13-й символ.
    lcd.print(L"°C "); // Выводим на экран °C
    count++;
  }
  else if ((count >= 0) && (count <= 9))
  {
    lcd.setCursor(11, 1); // Устанавливаем курсор на вторую строку и 11-й символ.
    lcd.print(humidity_res);
    lcd.setCursor(13, 1); // Устанавливаем курсор на вторую строку и 13-й символ.
    lcd.print("%  "); // Выводим на экран %
    count++;
  }
  else
  {
    count = 0;
  }

  lcd.setCursor(0, 0); // Устанавливаем курсор на вторую строку и 0-й символ.
  lcd.print(buf_day); // Выводим на экран дату
  lcd.setCursor(0, 1); // Устанавливаем курсор на вторую строку и 0-й символ.
  lcd.print(buf_time); // Выводим на экран время
  
  sendData();
  
  delay(925); // Задержка с калибровкой
}
