#include "Arduino.h"
#include <Wire.h>
#include <math.h>

int pos=0;
#define BUFSIZE 10
char cmd[BUFSIZE];

#define BME280_ADDRESS   0x76

#define BME280_REG_DIG_T1    0x88
#define BME280_REG_DIG_T2    0x8A
#define BME280_REG_DIG_T3    0x8C

#define BME280_REG_DIG_P1    0x8E
#define BME280_REG_DIG_P2    0x90
#define BME280_REG_DIG_P3    0x92
#define BME280_REG_DIG_P4    0x94
#define BME280_REG_DIG_P5    0x96
#define BME280_REG_DIG_P6    0x98
#define BME280_REG_DIG_P7    0x9A
#define BME280_REG_DIG_P8    0x9C
#define BME280_REG_DIG_P9    0x9E

#define BME280_REG_DIG_H1    0xA1
#define BME280_REG_DIG_H2    0xE1
#define BME280_REG_DIG_H3    0xE3
#define BME280_REG_DIG_H4    0xE4
#define BME280_REG_DIG_H5    0xE5
#define BME280_REG_DIG_H6    0xE7

#define BME280_REG_CONTROLHUMID    0xF2
#define BME280_REG_CONTROL         0xF4
#define BME280_REG_CONFIG          0xF5
#define BME280_REG_PRESSUREDATA    0xF7
#define BME280_REG_TEMPDATA        0xFA
#define BME280_REG_HUMIDITYDATA    0xFD

// Calibration data
uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;
uint8_t dig_H1;
int16_t dig_H2;
uint8_t dig_H3;
int16_t dig_H4;
int16_t dig_H5;
int8_t  dig_H6;
int32_t t_fine;


void setup()
{
	Serial.begin(115200);
	Serial.println("Project BME280");
	Wire.begin();
}


void loop() {

	if (Serial.available()) {
		char ch = Serial.read();
		Serial.write(ch);
		if (ch == '\r') {
			Serial.write('\n');

			switch (cmd[0]){

//ADDRESS: 76
			case 'i':
				int a;
				for(a = 8; a < 120; a++) {
					Wire.beginTransmission(a);
					if (Wire.endTransmission() == 0)
						Serial.println(a, HEX);
					delay(1);
				}
			break;

//TEMPERATURE
			case 't':

				uint32_t t, adc_T;
				int32_t var1, var2, temp, t_fine;

				//t_sb[2:0]=000 (t_standby=0,5ms); filter[2:0]=100
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONFIG);
				Wire.write(byte(0x10));
				Wire.endTransmission();

				//sleep mode -> normal mode; temperature settings: osrs_t[2:0]=010
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROL);
				Wire.write(byte(0x57));
				Wire.endTransmission();

				//temperature registers: FA, FB, FC
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_TEMPDATA);
				Wire.endTransmission();

				Wire.requestFrom(BME280_ADDRESS, 3);
				t = Wire.read();
				t <<= 8;
				t |= Wire.read();
				t <<= 8;
				t |= Wire.read();

				//temperature compensation
				adc_T = t;
				var1 = (((adc_T >> 3) - ((int32_t)(BME280_REG_DIG_T1 << 1))) * ((int32_t)BME280_REG_DIG_T2)) >> 11;
				var2 = (((((adc_T >> 4) - ((int32_t)BME280_REG_DIG_T1)) * ((adc_T >> 4) - ((int32_t)BME280_REG_DIG_T1))) >> 12) * ((int32_t)BME280_REG_DIG_T3)) >> 14;
				t_fine = var1 + var2;

				temp = (t_fine * 5 + 128) >> 8;

				Serial.print("TEMPERATURA: ");
				Serial.print((float)temp/100.0, 2);
				Serial.println(" Celsius");

				Serial.print("             ");
				Serial.print(((((float)temp/100.0)*1.8)+32), 2);
				Serial.println(" Fahrenheit");

				Serial.print("             ");
				Serial.print((((float)temp/100.0)+273.15), 2);
				Serial.println(" Kelvin");

			break;

//HUMIDITY
			case 'h':

			//OBTER t_fine: humidade depende da temperatura
				uint32_t th, adc_Th;
				int32_t var1h, var2h, t_fineh;

				//t_sb[2:0]=000 (t_standby=0,5ms); filter[2:0]=100
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONFIG);
				Wire.write(byte(0x10));
				Wire.endTransmission();

				//sleep mode -> normal mode; temperature settings: osrs_t[2:0]=010
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROL);
				Wire.write(byte(0x57));
				Wire.endTransmission();

				//temperature registers: FA, FB, FC
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_TEMPDATA);
				Wire.endTransmission();

				Wire.requestFrom(BME280_ADDRESS, 3);
				th = Wire.read();
				th <<= 8;
				th |= Wire.read();
				th <<= 8;
				th |= Wire.read();

				//temperature compensation
				adc_Th = th;
				var1h = (((adc_Th >> 3) - ((int32_t)(BME280_REG_DIG_T1 << 1))) * ((int32_t)BME280_REG_DIG_T2)) >> 11;
				var2h = (((((adc_Th >> 4) - ((int32_t)BME280_REG_DIG_T1)) * ((adc_Th >> 4) - ((int32_t)BME280_REG_DIG_T1))) >> 12) * ((int32_t)BME280_REG_DIG_T3)) >> 14;
				t_fineh = var1h + var2h;
			//------------------------

				uint8_t h1, h2;
				uint16_t h;
				uint32_t hum, adc_H, v_x1_u32r;

				//t_sb[2:0]=000 (t_standby=0,5ms); filter[2:0]=100
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONFIG);
				Wire.write(byte(0x10));
				Wire.endTransmission();

				//sleep mode -> normal mode
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROL);
				Wire.write(byte(0x57));
				Wire.endTransmission();

				//humidity settings: osrs_h[2:0]=001
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROLHUMID);
				Wire.write(byte(0x01));
				Wire.endTransmission();

				//humidity registers: FD e FE
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_HUMIDITYDATA);
				Wire.endTransmission();

				Wire.requestFrom(BME280_ADDRESS, 2);
				h1 = Wire.read();
				h2 = Wire.read();
				h = (uint16_t) h1<<8 | h2;

				//humidity compensation
				adc_H = h;
				v_x1_u32r = (t_fineh - ((int32_t)76800));
				v_x1_u32r = (((((adc_H << 14) - (((int32_t)BME280_REG_DIG_H4) << 20) - (((int32_t)BME280_REG_DIG_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)BME280_REG_DIG_H6)) >> 10) * (((v_x1_u32r * ((int32_t)BME280_REG_DIG_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)BME280_REG_DIG_H2) + 8192) >> 14));
				v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)BME280_REG_DIG_H1)) >> 4));
				v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
				v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

				hum = (uint32_t)(v_x1_u32r>>12);

				Serial.print("HUMIDADE: ");
				Serial.print((float)hum/1024.0, 2);
				Serial.println(" %RH");

			break;

//PRESSURE
			case 'p':

			//OBTER t_fine: pressao depende da temperatura
				uint32_t tp, adc_Tp;
				int32_t var1p, var2p, t_finep;

				//t_sb[2:0]=000 (t_standby=0,5ms); filter[2:0]=100
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONFIG);
				Wire.write(byte(0x10));
				Wire.endTransmission();

				//sleep mode -> normal mode; temperature settings: osrs_t[2:0]=010
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROL);
				Wire.write(byte(0x57));
				Wire.endTransmission();

				//temperature registers: FA, FB, FC
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_TEMPDATA);
				Wire.endTransmission();

				Wire.requestFrom(BME280_ADDRESS, 3);
				tp = Wire.read();
				tp <<= 8;
				tp |= Wire.read();
				tp <<= 8;
				tp |= Wire.read();

				//temperature compensation
				adc_Tp = tp;
				var1p = (((adc_Tp >> 3) - ((int32_t)(BME280_REG_DIG_T1 << 1))) * ((int32_t)BME280_REG_DIG_T2)) >> 11;
				var2p = (((((adc_Tp >> 4) - ((int32_t)BME280_REG_DIG_T1)) * ((adc_Tp >> 4) - ((int32_t)BME280_REG_DIG_T1))) >> 12) * ((int32_t)BME280_REG_DIG_T3)) >> 14;
				t_finep = var1p + var2p;
			//------------------------

				uint32_t p;
				int32_t adc_P;
				int64_t varp1, varp2, pres;

				//t_sb[2:0]=000 (t_standby=0,5ms); filter[2:0]=100
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONFIG);
				Wire.write(byte(0x10));
				Wire.endTransmission();

				//sleep mode -> normal mode; pressure settings: osrs_p[2:0]=101
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_CONTROL);
				Wire.write(byte(0x57));
				Wire.endTransmission();

				//pressure registers: F7, F8, F9
				Wire.beginTransmission(BME280_ADDRESS);
				Wire.write(BME280_REG_PRESSUREDATA);
				Wire.endTransmission();

				Wire.requestFrom(BME280_ADDRESS, 3);
				p = Wire.read();
				p <<= 8;
				p |= Wire.read();
				p <<= 8;
				p |= Wire.read();

				//pressure compensation
				varp1 = ((int64_t)t_finep)-128000;
				varp2 = varp1 * varp1 * (int64_t)BME280_REG_DIG_P6;
				varp2 = varp2 + ((varp1*(int64_t)BME280_REG_DIG_P5)<<17);
				varp2 = varp2 + (((int64_t)BME280_REG_DIG_P4)<<35);
				varp1 = ((varp1 * varp1 * (int64_t)BME280_REG_DIG_P3)>>8) + ((varp1 * (int64_t)BME280_REG_DIG_P2)<<12);
				varp1 = (((((int64_t)1)<<47)+varp1))*((int64_t)BME280_REG_DIG_P1)>>33;
				if (varp1 == 0){
					Serial.println("ERRO"); } // avoid exception caused by division by zero
				pres = 1048576-adc_P;
				pres = (((pres<<31)-varp2)*3125)/varp1;
				varp1 = (((int64_t)BME280_REG_DIG_P9) * (pres>>13) * (pres>>13)) >> 25;
				varp2 = (((int64_t)BME280_REG_DIG_P8) * pres) >> 19;

				pres = ((pres + varp1 + varp2) >> 8) + (((int64_t)BME280_REG_DIG_P7)<<4);

				Serial.print("PRESSAO: ");
				Serial.print(pres/256.0, 0);
				Serial.println(" Pa");

//ALTITUDE
				long alt_sea=103125;
				short alt;
				alt=(1.0-pow(pres/alt_sea , 1/5.25588))/0.0000225577;

				Serial.print("ALTITUDE: ");
				Serial.print((float)alt/1000, 1);
				Serial.println(" m");
			break;

			}
			pos=0;
		} else
			if ((pos < BUFSIZE) && (ch >= ' '))
				cmd[pos++] = ch;
	}
}
