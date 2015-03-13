#include <stdint.h>
#include <DHT.h>
#include "Wire.h"
#include "EggBus.h"
#include "Time.h"
#include <avr/eeprom.h>

EggBus eggBus;

#define DHTPIN 17 //analog pin 3
#define DHTTYPE DHT22  

// each coefficient holds a float
#define MAGIC_NUMBER         (0x5A)
#define NO2_COEFF_VALID      (252)
#define CO_COEFF_VALID       (253)
#define O3_COEFF_VALID       (254)
#define NO2_C_COEFF_ADDR     (256)
#define NO2_RS_COEFF_ADDR    (260)
#define NO2_RS2_COEFF_ADDR   (264)
#define NO2_RS3_COEFF_ADDR   (268)
#define CO_C_COEFF_ADDR      (272)
#define CO_RS_COEFF_ADDR     (276)
#define CO_RS2_COEFF_ADDR    (280)
#define CO_RS3_COEFF_ADDR    (284)
#define O3_C_COEFF_ADDR      (288)
#define O3_RS_COEFF_ADDR     (292)
#define O3_RS2_COEFF_ADDR    (296)
#define O3_RS3_COEFF_ADDR    (300)
#define NO2_TEMP_COEFF_ADDR  (304)
#define NO2_HUM_COEFF_ADDR   (308)
#define CO_TEMP_COEFF_ADDR   (312)
#define CO_HUM_COEFF_ADDR    (316)
#define O3_TEMP_COEFF_ADDR   (320)
#define O3_HUM_COEFF_ADDR    (324)
#define DUST_COEFF_VALID     (328)
#define DUST_C_COEFF_ADDR    (332)
#define DUST_RS_COEFF_ADDR   (336)
#define DUST_RS2_COEFF_ADDR  (340)
#define DUST_RS3_COEFF_ADDR  (344)
#define DUST_TEMP_COEFF_ADDR (348)
#define DUST_HUM_COEFF_ADDR  (352)

DHT dht(DHTPIN, DHTTYPE);

boolean no2_coeff_valid = false, co_coeff_valid = false, o3_coeff_valid = false, dust_coeff_valid = false;
float no2_c_coeff = 0.0f, no2_rs_coeff = 0.0f, no2_rs2_coeff = 0.0f, no2_rs3_coeff = 0.0f;
float co_c_coeff = 0.0f, co_rs_coeff = 0.0f, co_rs2_coeff = 0.0f, co_rs3_coeff = 0.0f;
float o3_c_coeff = 0.0f, o3_rs_coeff = 0.0f, o3_rs2_coeff = 0.0f, o3_rs3_coeff = 0.0f;
float dust_c_coeff = 0.0f, dust_rs_coeff = 0.0f, dust_rs2_coeff = 0.0f, dust_rs3_coeff = 0.0f;
float no2_temperature_coeff = 0.0f, no2_humidity_coeff = 0.0f;
float co_temperature_coeff = 0.0f, co_humidity_coeff = 0.0f;
float o3_temperature_coeff = 0.0f, o3_humidity_coeff = 0.0f;
float dust_temperature_coeff = 0.0f, dust_humidity_coeff = 0.0f;

void setup(){
  Serial.begin(9600);
  Serial.println(F("Air Quality Egg - CSV data w/ Temperature and Humidity Compensation"));
  Serial.println(F("======================================================================\r\n"));

  Serial.println(F("Here are the stored coefficients:"));
  printCoefficients();
  
  optionallyUpdateCoefficients();

  Serial.println(F("Here are the updated coefficients:"));
  printCoefficients();
  
  requestCurrentTime();
  
  Serial.println();
  Serial.println(F("time_ms, humidity[%], temperature]degC], no2_rs[kohms], no2_est[ppb], co_rs[kohms], co_est[ppb], o3_rs[kohms], o3_est[ppb], dust_concentration[pcs/283mL]"));
  
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
        no2_rs /= 1000.0; // convert  to kohms
        no2_rs_squared = no2_rs * no2_rs;
        no2_rs_cubed = no2_rs_squared * no2_rs;
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("CO")) == 0){
        co_rs = eggBus.getSensorResistance(ii);
        co_rs /= 1000.0; // convert  to kohms
        co_rs_squared = co_rs * co_rs;
        co_rs_cubed = co_rs_squared * co_rs;        
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("O3")) == 0){
        o3_rs = eggBus.getSensorResistance(ii);
        o3_rs /= 1000.0;
        o3_rs_squared = o3_rs * o3_rs;
        o3_rs_cubed = o3_rs_squared * o3_rs;              
      }
      else if(strcmp_P(eggBus.getSensorType(ii), PSTR("Dust")) == 0){
        dust_concentration = eggBus.getSensorValue(ii);
      }      
    }
  }

  //Serial.print(millis(), DEC);
  printTime();
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
    + (no2_temperature_coeff * temperature)
    + (no2_humidity_coeff * humidity)
    + (no2_rs_coeff * no2_rs)
    + (no2_rs2_coeff * no2_rs_squared)
    + (no2_rs3_coeff * no2_rs_cubed);
  
  co_est = co_c_coeff 
    + (co_temperature_coeff * temperature)
    + (co_humidity_coeff * humidity)  
    + (co_rs_coeff * co_rs)
    + (co_rs2_coeff * co_rs_squared)
    + (co_rs3_coeff * co_rs_cubed);

  o3_est = o3_c_coeff 
    + (o3_temperature_coeff * temperature)
    + (o3_humidity_coeff * humidity)    
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

void printCoefficients(void){

  uint8_t val = 0;
  val = eeprom_read_byte((const uint8_t *) NO2_COEFF_VALID);
  if(val == MAGIC_NUMBER) no2_coeff_valid = true;
  val = eeprom_read_byte((const uint8_t *) CO_COEFF_VALID);
  if(val == MAGIC_NUMBER) co_coeff_valid = true;
  val = eeprom_read_byte((const uint8_t *) O3_COEFF_VALID);
  if(val == MAGIC_NUMBER) o3_coeff_valid = true;
  val = eeprom_read_byte((const uint8_t *) DUST_COEFF_VALID);
  if(val == MAGIC_NUMBER) dust_coeff_valid = true;  
  
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

  eeprom_read_block(&dust_c_coeff,   (const void *) DUST_C_COEFF_ADDR,   4);
  eeprom_read_block(&dust_rs_coeff,  (const void *) DUST_RS_COEFF_ADDR,  4);
  eeprom_read_block(&dust_rs2_coeff, (const void *) DUST_RS2_COEFF_ADDR, 4);
  eeprom_read_block(&dust_rs3_coeff, (const void *) DUST_RS3_COEFF_ADDR, 4);

  eeprom_read_block(&no2_temperature_coeff, (const void *) NO2_TEMP_COEFF_ADDR, 4);
  eeprom_read_block(&no2_humidity_coeff, (const void *) NO2_HUM_COEFF_ADDR, 4);
  eeprom_read_block(&co_temperature_coeff, (const void *) CO_TEMP_COEFF_ADDR, 4);
  eeprom_read_block(&co_humidity_coeff, (const void *) CO_HUM_COEFF_ADDR, 4);
  eeprom_read_block(&o3_temperature_coeff, (const void *) O3_TEMP_COEFF_ADDR, 4);
  eeprom_read_block(&o3_humidity_coeff, (const void *) O3_HUM_COEFF_ADDR, 4);
  eeprom_read_block(&dust_temperature_coeff, (const void *) DUST_TEMP_COEFF_ADDR, 4);
  eeprom_read_block(&dust_humidity_coeff, (const void *) DUST_HUM_COEFF_ADDR, 4);

  Serial.print(F("NO2 Temperature coefficient [ppb/degC]: "));
  Serial.println(no2_temperature_coeff, 8);
  Serial.print(F("NO2 Humidity coefficient [ppb/(%RH)]: "));
  Serial.println(no2_humidity_coeff, 8);
  Serial.print(F("NO2 C coefficient [ppb]: "));
  Serial.println(no2_c_coeff, 8);
  Serial.print(F("NO2 RS coefficient [ppb/kohm]: "));
  Serial.println(no2_rs_coeff, 8);
  Serial.print(F("NO2 RS^2 coefficient [ppb/kohm^2]: "));
  Serial.println(no2_rs2_coeff, 8);
  Serial.print(F("NO2 RS^3 coefficient [ppb/kohm^3]: "));
  Serial.println(no2_rs3_coeff, 8);

  Serial.print(F("CO Temperature coefficient [ppb/degC]: "));
  Serial.println(co_temperature_coeff, 8);
  Serial.print(F("CO Humidity coefficient [ppb/(%RH)]: "));
  Serial.println(co_humidity_coeff, 8);
  Serial.print(F("CO C coefficient [ppb]: "));
  Serial.println(co_c_coeff, 8);
  Serial.print(F("CO RS coefficient [ppb/kohm]: "));
  Serial.println(co_rs_coeff, 8);
  Serial.print(F("CO RS^2 coefficient [ppb/kohm^2]: "));
  Serial.println(co_rs2_coeff, 8);
  Serial.print(F("CO RS^3 coefficient [ppb/kohm^3]: "));
  Serial.println(co_rs3_coeff, 8);

  Serial.print(F("O3 Temperature coefficient [ppb/degC]: "));
  Serial.println(o3_temperature_coeff, 8);
  Serial.print(F("O3 Humidity coefficient [ppb/(%RH)]: "));
  Serial.println(o3_humidity_coeff, 8);
  Serial.print(F("O3 C coefficient [ppb]: "));
  Serial.println(o3_c_coeff, 8);
  Serial.print(F("O3 RS coefficient [ppb/kohm]: "));
  Serial.println(o3_rs_coeff, 8);
  Serial.print(F("O3 RS^2 coefficient [ppb/kohm^2]: "));
  Serial.println(o3_rs2_coeff, 8);
  Serial.print(F("O3 RS^3 coefficient [ppb/kohm^3]: "));
  Serial.println(o3_rs3_coeff, 8);

  Serial.print(F("Dust Temperature coefficient [(ug/m^3)/degC]: "));
  Serial.println(dust_temperature_coeff, 8);
  Serial.print(F("Dust Humidity coefficient [(ug/m^3)/(%RH)]: "));
  Serial.println(dust_humidity_coeff, 8);
  Serial.print(F("Dust C coefficient [(ug/m^3)]: "));
  Serial.println(dust_c_coeff, 8);
  Serial.print(F("Dust RS coefficient [(ug/m^3)/(%Occupancy)]: "));
  Serial.println(dust_rs_coeff, 8);
  Serial.print(F("Dust RS^2 coefficient [(ug/m^3)/(%Occupancy)^2]: "));
  Serial.println(dust_rs2_coeff, 8);
  Serial.print(F("Dust RS^3 coefficient [(ug/m^3)/(%Occupancy)^3]: "));
  Serial.println(dust_rs3_coeff, 8);
}

void optionallyUpdateCoefficients(void){
  boolean change_coefficients = false;
  char ch = 0;
  uint32_t now = millis();
  uint32_t timeout = now + 10000; // 10 seconds to type something

  Serial.print(F("Do you want to change the NO2 coefficients? Y/N: "));
  for(;;){

    if(millis() > timeout){
      Serial.print(F("Skipping coefficient entry"));
      return;
    }

    while(Serial.available() == 0){;};
    ch = Serial.read();
    if(ch == 'N' || ch == 'n'){
      change_coefficients = false;
      break;
    }
    else if(ch == 'Y' || ch == 'y'){
      change_coefficients = true;
      break;
    }
  }
  Serial.println(ch);

  if(change_coefficients){
    Serial.print(F("Enter New NO2 Temperature coefficient [ppb/degC]: "));
    while(Serial.available() == 0){;};
    no2_temperature_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_temperature_coeff, (void *) NO2_TEMP_COEFF_ADDR, 4);
    Serial.println(no2_temperature_coeff, 8);

    Serial.print(F("Enter New NO2 Humidity coefficient [ppb/(%RH)]: "));
    while(Serial.available() == 0){;};
    no2_humidity_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_humidity_coeff, (void *) NO2_HUM_COEFF_ADDR, 4);
    Serial.println(no2_humidity_coeff, 8);

    Serial.print(F("Enter New NO2 C coefficient [ppb]: "));
    while(Serial.available() == 0){;};
    no2_c_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_c_coeff, (void *) NO2_C_COEFF_ADDR, 4);
    Serial.println(no2_c_coeff, 8);

    Serial.print(F("Enter New NO2 RS coefficient [ppb/kohm]: "));
    while(Serial.available() == 0){;};
    no2_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs_coeff, (void *) NO2_RS_COEFF_ADDR, 4);
    Serial.println(no2_rs_coeff, 8);

    Serial.print(F("Enter New NO2 RS^2 coefficient [ppb/kohm^2]: "));
    while(Serial.available() == 0){;};
    no2_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs2_coeff, (void *) NO2_RS2_COEFF_ADDR, 4);
    Serial.println(no2_rs2_coeff, 8);

    Serial.print(F("Enter New NO2 RS^3 coefficient [ppb/kohm^3]: "));
    while(Serial.available() == 0){;};
    no2_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&no2_rs3_coeff, (void *) NO2_RS3_COEFF_ADDR, 4);
    Serial.println(no2_rs3_coeff, 8);

    eeprom_write_byte((uint8_t *) NO2_COEFF_VALID, MAGIC_NUMBER);
  }

  Serial.print(F("Do you want to change the CO coefficients? Y/N: "));
  for(;;){
    while(Serial.available() == 0){;};
    ch = Serial.read();
    if(ch == 'N' || ch == 'n'){
      change_coefficients = false;
      break;
    }
    else if(ch == 'Y' || ch == 'y'){
      change_coefficients = true;
      break;
    }
  }
  Serial.println(ch);

  if(change_coefficients){
    Serial.print(F("Enter New CO Temperature coefficient [ppb/degC]: "));
    while(Serial.available() == 0){;};
    co_temperature_coeff = Serial.parseFloat();
    eeprom_write_block(&co_temperature_coeff, (void *) CO_TEMP_COEFF_ADDR, 4);
    Serial.println(co_temperature_coeff, 8);

    Serial.print(F("Enter New CO Humidity coefficient [ppb/(%RH)]: "));
    while(Serial.available() == 0){;};
    co_humidity_coeff = Serial.parseFloat();
    eeprom_write_block(&co_humidity_coeff, (void *) CO_HUM_COEFF_ADDR, 4);
    Serial.println(co_humidity_coeff, 8);

    Serial.print(F("Enter New CO C coefficient [ppb]: "));
    while(Serial.available() == 0){;};
    co_c_coeff = Serial.parseFloat();
    eeprom_write_block(&co_c_coeff, (void *) CO_C_COEFF_ADDR, 4);
    Serial.println(co_c_coeff, 8);

    Serial.print(F("Enter New CO RS coefficient [ppb/kohm]: "));
    while(Serial.available() == 0){;};
    co_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs_coeff, (void *) CO_RS_COEFF_ADDR, 4);
    Serial.println(co_rs_coeff, 8);

    Serial.print(F("Enter New CO RS^2 coefficient [ppb/kohm^2]: "));
    while(Serial.available() == 0){;};
    co_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs2_coeff, (void *) CO_RS2_COEFF_ADDR, 4);
    Serial.println(co_rs2_coeff, 8);

    Serial.print(F("Enter New CO RS^3 coefficient [ppb/kohm^3]: "));
    while(Serial.available() == 0){;};
    co_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&co_rs3_coeff, (void *) CO_RS3_COEFF_ADDR, 4);
    Serial.println(co_rs3_coeff, 8);

    eeprom_write_byte((uint8_t *) CO_COEFF_VALID, MAGIC_NUMBER);
  }

  Serial.print(F("Do you want to change the O3 coefficients? Y/N: "));
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
  Serial.println(ch);

  if(change_coefficients){
    Serial.print(F("Enter New O3 Temperature coefficient [ppb/degC]: "));
    while(Serial.available() == 0){;};
    o3_temperature_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_temperature_coeff, (void *) O3_TEMP_COEFF_ADDR, 4);
    Serial.println(o3_temperature_coeff, 8);

    Serial.print(F("Enter New O3 Humidity coefficient [ppb/(%RH)]: "));
    while(Serial.available() == 0){;};
    o3_humidity_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_humidity_coeff, (void *) O3_HUM_COEFF_ADDR, 4);
    Serial.println(o3_humidity_coeff, 8);

    Serial.print(F("Enter New O3 C coefficient [ppb]: "));
    while(Serial.available() == 0){;};
    o3_c_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_c_coeff, (void *) O3_C_COEFF_ADDR, 4);
    Serial.println(o3_c_coeff, 8);

    Serial.print(F("Enter New O3 RS coefficient [ppb/kohm]: "));
    while(Serial.available() == 0){;};
    o3_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs_coeff, (void *) O3_RS_COEFF_ADDR, 4);
    Serial.println(o3_rs_coeff, 8);

    Serial.print(F("Enter New O3 RS^2 coefficient [ppb/kohm^2]: "));
    while(Serial.available() == 0){;};
    o3_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs2_coeff, (void *) O3_RS2_COEFF_ADDR, 4);
    Serial.println(o3_rs2_coeff, 8);

    Serial.print(F("Enter New O3 RS^3 coefficient [ppb/kohm^3]: "));
    while(Serial.available() == 0){;};
    o3_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&o3_rs3_coeff, (void *) O3_RS3_COEFF_ADDR, 4);
    Serial.println(o3_rs3_coeff, 8);

    eeprom_write_byte((uint8_t *) O3_COEFF_VALID, MAGIC_NUMBER);

  }

  Serial.print(F("Do you want to change the Dust coefficients? Y/N: "));
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
  Serial.println(ch);

  if(change_coefficients){
    Serial.print(F("Enter New Dust Temperature coefficient [(ug/m^3)/degC]: "));
    while(Serial.available() == 0){;};
    dust_temperature_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_temperature_coeff, (void *) DUST_TEMP_COEFF_ADDR, 4);
    Serial.println(dust_temperature_coeff, 8);

    Serial.print(F("Enter New O3 Humidity coefficient [(ug/m^3)/(%RH)]: "));
    while(Serial.available() == 0){;};
    dust_humidity_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_humidity_coeff, (void *) DUST_HUM_COEFF_ADDR, 4);
    Serial.println(dust_humidity_coeff, 8);

    Serial.print(F("Enter New O3 C coefficient [ug/m^3]: "));
    while(Serial.available() == 0){;};
    dust_c_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_c_coeff, (void *) DUST_C_COEFF_ADDR, 4);
    Serial.println(dust_c_coeff, 8);

    Serial.print(F("Enter New O3 RS coefficient [(ug/m^3)/(%Occupancy)]: "));
    while(Serial.available() == 0){;};
    dust_rs_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_rs_coeff, (void *) DUST_RS_COEFF_ADDR, 4);
    Serial.println(dust_rs_coeff, 8);

    Serial.print(F("Enter New O3 RS^2 coefficient [(ug/m^3)/(%Occupancy)^2]: "));
    while(Serial.available() == 0){;};
    dust_rs2_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_rs2_coeff, (void *) DUST_RS2_COEFF_ADDR, 4);
    Serial.println(dust_rs2_coeff, 8);

    Serial.print(F("Enter New O3 RS^3 coefficient [(ug/m^3)/(%Occupancy)^3]: "));
    while(Serial.available() == 0){;};
    dust_rs3_coeff = Serial.parseFloat();
    eeprom_write_block(&dust_rs3_coeff, (void *) DUST_RS3_COEFF_ADDR, 4);
    Serial.println(dust_rs3_coeff, 8);

    eeprom_write_byte((uint8_t *) DUST_COEFF_VALID, MAGIC_NUMBER);

  }
} 

void requestCurrentTime(){
  char buf[8] = {0};
  uint8_t buf_idx = 0;
  char c;
  int m=1,d=1,y=2013,h=0,mn=0,s=0;
  uint8_t num_ints_received = 0;
  
  while(Serial.available()) Serial.read(); // flush the input buffers
  
  Serial.print(F("Current Date/Time (M/D/Y H:M:S): "));
  // collect 6 integers
  for(;;){
     if(Serial.available()){
       c = Serial.read();
       Serial.print(c);
       if(c >= '0' && c <= '9' && buf_idx < 7){
         buf[buf_idx++] = c;
         buf[buf_idx] = '\0';
       }
       else if(c == '/' || c == ':' || c == ' ' || c == '\n' || c == '\r'){
         switch(num_ints_received){
         case 0:
           m = atoi(buf);
           break;
         case 1:
           d = atoi(buf);
           break;
         case 2:
           y = atoi(buf);
           break;
         case 3:
           h = atoi(buf);
           break;
         case 4:
           mn = atoi(buf);
           break; 
         case 5:
           s = atoi(buf);
           break;         
         }
         buf[0] = '\0';
         buf_idx = 0;
         num_ints_received++;
       }
       
       if(c == '\n' || c == '\r'){
         setTime(h,mn,s,d,m,y);
         Serial.println();
         Serial.print("Time Set: ");
         printTime();
         Serial.println();
         return;
       }       
     }
  }
}

void printTime(){
   time_t _now = now();
   
   Serial.print(month(_now));
   Serial.print(F("/"));
   Serial.print(day(_now));
   Serial.print(F("/"));   
   Serial.print(year(_now));
   Serial.print(F(" "));   
   if(hour(_now) < 10) Serial.print(F("0"));
   Serial.print(hour(_now));
   Serial.print(F(":"));  
   if(minute(_now) < 10) Serial.print(F("0")); 
   Serial.print(minute(_now));
   Serial.print(F(":"));   
   if(second(_now) < 10) Serial.print(F("0"));
   Serial.print(second(_now));  
}
