// 温湿度センサー AM2301B I2C アクセステスト
#include <Wire.h>
#define BAUDRATE 9600
#define AM2301BADDR 0x38

// ----- Senser Data
struct AM2301B_s {
  int humidity ;   // 湿度
  float temperature; // 温度
} am2301b ;

// AM2301読み取り
bool readAM2301B( ) {
  uint8_t readData[8] ;

  Wire.requestFrom(AM2301BADDR, 1);
  uint8_t stat = 0 ;
  while( Wire.available() == 0) ;
  if (Wire.available() > 0) {
      stat = Wire.read();
  }
  if ((stat & 0x80) == 1) { // BUSY FLAG Check
    return false ;
  }

  Wire.beginTransmission(AM2301BADDR);
  Wire.write(0x71);
  Wire.endTransmission();
  delay(10); // wait 10 ms

  Wire.beginTransmission(AM2301BADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(80); // wait SecnserRead

  // 6Byte + CRC Read
  Wire.requestFrom(AM2301BADDR, 7);
  int len = 0 ;
  while( len < 7) {
    if (Wire.available() > 0) {
      readData[len] = Wire.read();
      len ++ ;
    }
  }
  
  // ----- CRC8 -----CRC[7:0]= 1 + x^4 + x^5 + x^8  : B1 0011 0001
  uint8_t crc = 0xFF;
  for (int i = 0; i < 6 ; i++) {
    uint8_t b = readData[i];
    crc ^= b;
    for (int x = 0; x < 8; x++) {
      if (crc & 0x80) {
        crc <<= 1;
        crc ^= 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
 
  uint32_t t1,t2,t3 ;
  t1 = readData[1] ; t2 = readData[2] ; t3 = readData[3] & 0xF0 ;
  uint32_t h = t1 << 12 | t2 << 4 | t3 >> 4 ; 

  t1 = readData[3] & 0x0F; t2 = readData[4] ; t3 = readData[5] ;
  uint32_t t = t1 << 16 | t2 << 8 | t3 ;

  // ----- 取得データ格納 -----
  am2301b.humidity = (h * 100 ) >> 20 ;
  am2301b.temperature = (( t * 200) >> 18) / 4. - 50;
  uint8_t cr = readData[6] ;

  return (cr == crc) ;
}

void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("\nAM2301B - Read") ;

  Wire.begin();
  delay(100) ;  // センサーが安定するまでの待ち時間
}

void loop() {
  if (readAM2301B( )) {
    Serial.print("humidity:") ;
    Serial.print(am2301b.humidity) ;
    Serial.print("  temperature:") ;
    Serial.println(am2301b.temperature) ;
  }
  delay(5000) ;
}
