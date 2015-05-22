#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"
#include <avr/eeprom.h>

EggBus eggBus;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);

#define BUFFER_DEPTH 20
float humidity_buffer[BUFFER_DEPTH], temperature_buffer[BUFFER_DEPTH];
float no2_raw_buffer[BUFFER_DEPTH], co_raw_buffer[BUFFER_DEPTH], o3_raw_buffer[BUFFER_DEPTH], dust_raw_buffer[BUFFER_DEPTH];
float no2_calc_buffer[BUFFER_DEPTH], co_calc_buffer[BUFFER_DEPTH], o3_calc_buffer[BUFFER_DEPTH], dust_calc_buffer[BUFFER_DEPTH];

uint8_t buffer_index = 0;
boolean start_recording = false;
uint32_t loop_count = 0;

void setup(){
  Serial.begin(115200);
  Serial.println(F("Air Quality Egg - CSV data w/ Time Averaged"));
  Serial.println(F("======================================================================"));
  Serial.print(F("humidity[%], temperature]degC], no2_rs[kohms], co_rs[kohms], o3_rs[kohms], dust_rs [%], "));
  Serial.print(F("no2[ppb], co[ppm], o3[ppb], dust[pcs/283mL]"));
  Serial.println();
}

void loop(){
  uint8_t   egg_bus_address;
  float temperature = 0.0f, humidity = 0.0f;
  float no2_rs = 0.0f, co_rs = 0.0f, o3_rs = 0.0f, dust_pct_occupancy = 0.0f;
  float no2_calc = 0.0f, co_calc = 0.0f, o3_calc = 0.0f, dust_calc = 0.0f;
  
  eggBus.init();
  
  // collect sensor resistances
  while((egg_bus_address = eggBus.next())){
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++){         
      if(strcmp_P(eggBus.getSensorType(ii), PSTR("NO2")) == 0){
        no2_rs = eggBus.getSensorResistance(ii);
        no2_calc = eggBus.getSensorValue(ii);
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("CO")) == 0){
        co_rs = eggBus.getSensorResistance(ii);
        co_calc = eggBus.getSensorValue(ii);        
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("O3")) == 0){
        o3_rs = eggBus.getSensorResistance(ii);
        o3_calc = eggBus.getSensorValue(ii);        
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("Dust")) == 0){
        dust_pct_occupancy = eggBus.getSensorResistance(ii);
        dust_calc = eggBus.getSensorValue(ii);        
      }
    }
  }

  humidity = getHumidity();
  temperature = getTemperature();

  humidity_buffer[buffer_index] = humidity;
  temperature_buffer[buffer_index] = temperature;

  no2_calc_buffer[buffer_index] = no2_calc;
  co_calc_buffer[buffer_index] = co_calc;
  o3_calc_buffer[buffer_index] = o3_calc;
  dust_calc_buffer[buffer_index] = dust_calc;  
  
  no2_raw_buffer[buffer_index] = no2_rs;
  co_raw_buffer[buffer_index] = co_rs;
  o3_raw_buffer[buffer_index] = o3_rs;
  dust_raw_buffer[buffer_index] = dust_pct_occupancy;
  
  if(buffer_index >= BUFFER_DEPTH){
    buffer_index = 0;
    start_recording = true;
    loop_count = 0;
  }

  buffer_index++;

  delay(3000);
  
  if(start_recording && (loop_count % 60) == 0){
    Serial.println(millis());
    Serial.print(F(","));
    
    Serial.print(buffer_average(humidity_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(temperature_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(no2_raw_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(co_raw_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(o3_raw_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(dust_raw_buffer), 3);
    Serial.print(F(", "));

    Serial.print(buffer_average(no2_calc_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(co_calc_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(o3_calc_buffer), 3);
    Serial.print(F(", "));
    
    Serial.print(buffer_average(dust_calc_buffer), 3);
    Serial.print(F(", "));
        
    Serial.println();
  }
  
  loop_count++;
}

//--------- DHT22 humidity ---------
float getHumidity(){
  float h = dht.readHumidity();
  if (isnan(h)){
    //failed to get reading from DHT    
    delay(2500);
    h = dht.readHumidity();
    if(isnan(h)){
      return -1;
    }
  }
  else return h;
}

//--------- DHT22 temperature ---------
float getTemperature(){
  float t = dht.readTemperature();
  if (isnan(t)){
    //failed to get reading from DHT
    delay(2500);
    t = dht.readTemperature();
    if(isnan(t)){
      return -1; 
    }
  } 
  return t;
}

float buffer_average(float * buf){
  float avg = 0.0f;
  for(uint8_t ii = 0; ii < BUFFER_DEPTH; ii++){
     avg += buf[ii];
  }
  
  return avg / (1.0f * BUFFER_DEPTH);
}

