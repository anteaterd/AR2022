#include <TroykaDHT.h>
DHT dht(2, DHT21);  //датчик темп. и влаги подключен в порт D2
#define HEAT 4
#define LIGHT 5
#define PUMP 6
#define VENT 7
#define LIGHT_SENSOR A0
#define AIR_HUMIDITY_SENSOR A1  // ввести второй датчик влажности почвы
#define DIRT_HUMIDITY_SENSOR A2

int MIN_DIRT_HUMIDITY = 50;  // Влажность почвы, %
int MAX_DIRT_HUMIDITY = 70;
int MIN_AIR_HUMIDITY = 50;  // Влажность воздуха, %
int MAX_AIR_HUMIDITY = 70;

int SECONDS = 0;  //секунды с начала дня, 16*60*60 - начало ночи, ночь 8 часов, с 16 до 24
//обозначаем глобальные переменные для показаний датчика темп. и влажности воздуха

int CURRENT_TEMPERATURE = 20;  // Текущая температура, град C
int CURRENT_AIR_HUMIDITY = analogRead(AIR_HUMIDITY_SENSOR);

enum Regime {DAY,NIGHT};
enum Regime CURR_REGIME = DAY; 
enum Regime_pump {humidity_ok, irrigation, hold};
enum Regime_pump regime_pump = humidity_ok;

void timer(){
  SECONDS++;
  if(SECONDS >= 60 * 60 * 24) SECONDS = 0;
  if (SECONDS >= 60*60*16) CURR_REGIME = NIGHT;
  delay(1000);  // 1 секунда пауза
}


void control_light() {  // Датчик смотрит вверх, лампа смотрит вниз
  if (CURR_REGIME == DAY) {
    if (analogRead(LIGHT_SENSOR) < 100) digitalWrite(LIGHT, HIGH);
    else digitalWrite(LIGHT, LOW);
  } else digitalWrite(LIGHT, LOW);
}


struct COOLER {
  bool status_vent;
  bool status_air_humidity;
  bool status_heat;
  }
  COOLER = {false,false,false};
  
  struct HEATER {
  bool status_vent;
  bool status_air_humidity;
  bool status_heat;
  }
  HEATER = {false,false,false};
  
void cooler() {
    if( COOLER.status_vent
        or COOLER.status_air_humidity
        or COOLER.status_heat ) {
        digitalWrite(VENT, HIGH);  // включить
    } else digitalWrite(VENT, LOW);  // выключить
}

void heater() {
    if HEATER.status_heat {
        digitalWrite(HEAT, LOW);  // выключить
    } else digitalWrite(HEAT, HIGH);  // включить
}


void periodic_ventilation() {  
  if(2*60*60 < SECONDS & SECONDS < 3*60*60 & CURRENT_AIR_HUMIDITY > MIN_AIR_HUMIDITY) { COOLER.status_vent = true; return;}  // интервал времени проветривания 
  if(6*60*60 < SECONDS & SECONDS < 7*60*60 & CURRENT_AIR_HUMIDITY > MIN_AIR_HUMIDITY) { COOLER.status_vent = true; return;}
  if(10*60*60 < SECONDS & SECONDS < 11*60*60 & CURRENT_AIR_HUMIDITY > MIN_AIR_HUMIDITY) { COOLER.status_vent = true; return;}
  if(14*60*60 < SECONDS & SECONDS < 15*60*60 & CURRENT_AIR_HUMIDITY > MIN_AIR_HUMIDITY) { COOLER.status_vent = true; return;}
  COOLER.status_vent = false;
}


void control_temperature(){  // TODO
  dht.read();
  if (CURRENT_TEMPERATURE < 15) { // если t<15 град, включается обогреватель 
    digitalWrite(HEAT, HIGH);
  }
  if (CURRENT_TEMPERATURE > 35) {  // если t>35 град, то включается кулер, обогреватель выключается
    COOLER.status_heat = true;    
    digitalWrite(HEAT, LOW);
  } else {
    COOLER.status_heat = false;  // если 15<t<35 град, то кулер и обогреватель выключены
    digitalWrite(HEAT, LOW);
  }
}


void irrigate(){  // функция для включения полива
  int min_hold_time = 60;  // время ожидания между поливами
  int max_irrigation_time = 10;  // время полива
  static int current_hold_time = 0;
  static int current_irrigation_time = 0;
  digitalWrite(PUMP, HIGH);
  if (current_irrigation_time > max_irrigation_time){
    digitalWrite(PUMP, LOW);
    if (current_hold_time > min_hold_time){
      current_hold_time = 0;
      current_irrigation_time = 0;
    }
    else current_hold_time++;
  }
  else current_irrigation_time++;
}


void control_dirt_humidity(){  
  int dirt_humidity = analogRead(DIRT_HUMIDITY_SENSOR);
  if (dirt_humidity < MIN_DIRT_HUMIDITY) irrigate();
}


void control_air_humidity(){  
  CURRENT_AIR_HUMIDITY = analogRead(AIR_HUMIDITY_SENSOR);
  if (CURRENT_AIR_HUMIDITY > MAX_AIR_HUMIDITY) COOLER.status_air_humidity = true;
  else COOLER.status_air_humidity = false;
}


void setup() {
  pinMode(HEAT, OUTPUT);
  pinMode(VENT, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(PUMP, OUTPUT);
  Serial.begin(9600);
  delay(2000);
}


void loop() {
  timer();
  periodic_ventilation();
  control_temperature();
  control_dirt_humidity();
  control_air_humidity();
  control_light();
  cooler();
  heater();
}
