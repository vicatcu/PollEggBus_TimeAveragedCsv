#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"
#include <avr/eeprom.h>

EggBus eggBus;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  

// each coefficient holds a float
#define NO2_C_COEFF_ADDR   (256)
#define NO2_RS_COEFF_ADDR  (260)
#define NO2_RS2_COEFF_ADDR (264)
#define NO2_RS3_COEFF_ADDR (268)
#define CO_C_COEFF_ADDR    (272)
#define CO_RS_COEFF_ADDR   (276)
#define CO_RS2_COEFF_ADDR  (280)
#define CO_RS3_COEFF_ADDR  (284)
#define O3_C_COEFF_ADDR    (288)
#define O3_RS_COEFF_ADDR   (292)
#define O3_RS2_COEFF_ADDR  (296)
#define O3_RS3_COEFF_ADDR  (300)

DHT dht(DHTPIN, DHTTYPE);
float no2_c_coeff = 0.0f, no2_rs_coeff = 0.0f, no2_rs2_coeff = 0.0f, no2_rs3_coeff = 0.0f;
float co_c_coeff = 0.0f, co_rs_coeff = 0.0f, co_rs2_coeff = 0.0f, co_rs3_coeff = 0.0f;
float o3_c_coeff = 0.0f, o3_rs_coeff = 0.0f, o3_rs2_coeff = 0.0f, o3_rs3_coeff = 0.0f;

void setup(){
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg - CSV data w/ Temperature and Humidity Compensation"));
  Serial.println(F("======================================================================\r\n"));
  Serial.println(F("Here are the stored coefficients - type Y <Enter> to change them or N <Enter> to leave them alone"));
  
  eeprom_read_block(&no2_c_coeff,   (const void *) NO2_C_COEFF_ADDR,   4);
  eeprom_read_block(&no2_rs_coeff,  (const void *) NO2_RS_COEFF_ADDR,  4);
  eeprom_read_block(&no2_rs2_coeff, (const void *) NO2_RS2_COEFF_ADDR, 4);
  eeprom_read_block(&no2_rs3_coeff, (const void *) NO2_RS3_COEFF_ADDR, 4);
  
  eeprom_read_block(&co_c_coeff,   (const void *) CO_C_COEFF_ADDR,   4);
  eeprom_read_block(&co_rs_coeff,  (const void *) CO_RS_COEFF_ADDR,  4);
  eeprom_read_block(&co_rs2_coeff, (const void *) CO_RS2_COEFF_ADDR, 4);
  eeprom_read_block(&co_rs3_coeff, (const void *) CO_RS3_COEFF_ADDR, 4);

  eeprom_read_block(&o3_c_coeff,   (const void *) O3_C_COEFF_ADDR,   4);
  eeprom_read_block(&o3_rs_coeff,  (const void *) O3_RS_COEFF_ADDR,  4);
  eeprom_read_block(&o3_rs2_coeff, (const void *) O3_RS2_COEFF_ADDR, 4);
  eeprom_read_block(&o3_rs3_coeff, (const void *) O3_RS3_COEFF_ADDR, 4);

  Serial.print(F("NO2 C coefficient: "));
  Serial.println(no2_c_coeff, 8);  
  Serial.print(F("NO2 RS coefficient: "));
  Serial.println(no2_rs_coeff, 8);
  Serial.print(F("NO2 RS^2 coefficient: "));
  Serial.println(no2_rs2_coeff, 8);
  Serial.print(F("NO2 RS^3 coefficient: "));
  Serial.println(no2_rs3_coeff, 8);
  
  Serial.print(F("CO C coefficient: "));
  Serial.println(co_c_coeff, 8);    
  Serial.print(F("CO RS coefficient: "));
  Serial.println(co_rs_coeff, 8);
  Serial.print(F("CO RS^2 coefficient: "));
  Serial.println(co_rs2_coeff, 8);
  Serial.print(F("CO RS^3 coefficient: "));
  Serial.println(co_rs3_coeff, 8);
  
  Serial.print(F("O3 C coefficient: "));
  Serial.println(o3_c_coeff, 8);    
  Serial.print(F("O3 RS coefficient: "));
  Serial.println(o3_rs_coeff, 8);
  Serial.print(F("O3 RS^2 coefficient: "));
  Serial.println(o3_rs2_coeff, 8);
  Serial.print(F("O3 RS^3 coefficient: "));
  Serial.println(o3_rs3_coeff, 8);
  
  boolean change_coefficients = false;
  for(;;){
    while(Serial.available() == 0){;};
    char ch = Serial.read();
    if(ch == 'N' || ch == 'n'){
      change_coefficients = false;
      break;
    }
    else if(ch == 'Y' || ch == 'y'){
      change_coefficients = true;
      break;      
    }
  }
  
  if(change_coefficients){   
    Serial.print(F("Enter New NO2 C coefficient: "));
    while(Serial.available() == 0){;}; 
    no2_c_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_c_coeff, (void *) NO2_C_COEFF_ADDR, 4);
    Serial.println(no2_c_coeff, 8);   
    
    Serial.print(F("Enter New NO2 RS coefficient: "));
    while(Serial.available() == 0){;}; 
    no2_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs_coeff, (void *) NO2_RS_COEFF_ADDR, 4);
    Serial.println(no2_rs_coeff, 8);
    
    Serial.print(F("Enter New NO2 RS^2 coefficient: "));
    while(Serial.available() == 0){;}; 
    no2_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs2_coeff, (void *) NO2_RS2_COEFF_ADDR, 4);
    Serial.println(no2_rs2_coeff, 8);
    
    Serial.print(F("Enter New NO2 RS^3 coefficient: "));
    while(Serial.available() == 0){;}; 
    no2_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs3_coeff, (void *) NO2_RS3_COEFF_ADDR, 4);
    Serial.println(no2_rs3_coeff, 8);

    Serial.print(F("Enter New CO C coefficient: "));
    while(Serial.available() == 0){;}; 
    co_c_coeff = Serial.parseFloat();
    eeprom_write_block(&co_c_coeff, (void *) CO_C_COEFF_ADDR, 4);
    Serial.println(co_c_coeff, 8);
    
    Serial.print(F("Enter New CO RS coefficient: "));
    while(Serial.available() == 0){;}; 
    co_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs_coeff, (void *) CO_RS_COEFF_ADDR, 4);
    Serial.println(co_rs_coeff, 8);
    
    Serial.print(F("Enter New CO RS^2 coefficient: "));
    while(Serial.available() == 0){;}; 
    co_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs2_coeff, (void *) CO_RS2_COEFF_ADDR, 4);
    Serial.println(co_rs2_coeff, 8);
    
    Serial.print(F("Enter New CO RS^3 coefficient: "));
    while(Serial.available() == 0){;}; 
    co_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs3_coeff, (void *) CO_RS3_COEFF_ADDR, 4);
    Serial.println(co_rs3_coeff, 8);
    
    Serial.print(F("Enter New O3 C coefficient: "));
    while(Serial.available() == 0){;}; 
    o3_c_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_c_coeff, (void *) O3_C_COEFF_ADDR, 4);
    Serial.println(o3_c_coeff, 8);    
    
    Serial.print(F("Enter New O3 RS coefficient: "));
    while(Serial.available() == 0){;}; 
    o3_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs_coeff, (void *) O3_RS_COEFF_ADDR, 4);
    Serial.println(o3_rs_coeff, 8);
    
    Serial.print(F("Enter New O3 RS^2 coefficient: "));
    while(Serial.available() == 0){;}; 
    o3_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs2_coeff, (void *) O3_RS2_COEFF_ADDR, 4);
    Serial.println(o3_rs2_coeff, 8);
    
    Serial.print(F("Enter New O3 RS^3 coefficient: "));
    while(Serial.available() == 0){;}; 
    o3_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs3_coeff, (void *) O3_RS3_COEFF_ADDR, 4);
    Serial.println(o3_rs3_coeff, 8); 
  }
  
  Serial.println();
  Serial.println(F("time_ms, humidity, temperature, no2_rs, no2_est, co_rs, co_est, o3_rs, o3_est, dust_concentration"));
  
}

void loop(){
  uint8_t   egg_bus_address;
  float temperature = 0.0f, humidity = 0.0f, no2_est = 0.0f, co_est = 0.0f, o3_est = 0.0f;
  float no2_rs = 0.0f, co_rs = 0.0f, o3_rs = 0.0f, dust_concentration = 0.0f;
  float no2_rs_squared = 0.0f, no2_rs_cubed = 0.0f, co_rs_squared = 0.0f, co_rs_cubed = 0.0f, o3_rs_squared = 0.0f, o3_rs_cubed = 0.0f;
  
  eggBus.init();
    
  // collect sensor resistances
  while((egg_bus_address = eggBus.next())){
    uint8_t numSensors = eggBus.getNumSensors();
    for(uint8_t ii = 0; ii < numSensors; ii++){         
      if(strcmp_P(eggBus.getSensorType(ii), PSTR("NO2")) == 0){
        no2_rs = eggBus.getSensorResistance(ii);
        no2_rs_squared = no2_rs * no2_rs;
        no2_rs_cubed = no2_rs_squared * no2_rs;
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("CO")) == 0){
        co_rs = eggBus.getSensorResistance(ii);
        co_rs_squared = co_rs * co_rs;
        co_rs_cubed = co_rs_squared * co_rs;        
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("O3")) == 0){
        o3_rs = eggBus.getSensorResistance(ii);
        o3_rs_squared = o3_rs * o3_rs;
        o3_rs_cubed = o3_rs_squared * o3_rs;              
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("Dust")) == 0){
        dust_concentration = eggBus.getSensorValue(ii);
      }      
      Serial.println();
    }
  }

  Serial.print(millis(), DEC);
  Serial.print(F(", "));    
  
  humidity = getHumidity();
  Serial.print(humidity, 3);
  Serial.print(F(", ")); 
  delay(1000);  

  temperature = getTemperature();
  Serial.print(temperature, 3);
  Serial.print(F(", ")); 
  delay(1000);  
  
  no2_est = no2_c_coeff 
    + (no2_rs_coeff * no2_rs)
    + (no2_rs2_coeff * no2_rs_squared)
    + (no2_rs3_coeff * no2_rs_cubed);
  
  co_est = co_c_coeff 
    + (co_rs_coeff * co_rs)
    + (co_rs2_coeff * co_rs_squared)
    + (co_rs3_coeff * co_rs_cubed);

  o3_est = o3_c_coeff 
    + (o3_rs_coeff * o3_rs)
    + (o3_rs2_coeff * o3_rs_squared)
    + (o3_rs3_coeff * o3_rs_cubed);  
  
  Serial.print(no2_rs, 3);
  Serial.print(F(", "));  
  Serial.print(no2_est, 3);
  Serial.print(F(", "));
  
  Serial.print(co_rs, 3);
  Serial.print(F(", "));   
  Serial.print(co_est, 3);
  Serial.print(F(", "));   
  
  Serial.print(o3_rs, 3);
  Serial.print(F(", "));   
  Serial.print(o3_est, 3);
  Serial.print(F(", "));    
  
  Serial.print(dust_concentration, 3);
  
  Serial.println();
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
