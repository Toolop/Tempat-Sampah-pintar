#include <TinyGPS++.h> //library gps neo6m
#include <HardwareSerial.h> //library untuk pembacaan gps
#include <WiFi.h> // library untuk wifi pada esp32
#include <Wire.h> //library untuk servo
#include <BlynkSimpleEsp32.h> //library untuk blink
#include <ESP32Servo.h> //library untuk servo pada esp3d

Servo testservo; //inisiasi nama servo

int laserPin = 18; //inisiasi laser module
#define LDR 33 //inisiasi sensor cahaya
#define trig 5 //inisasi pemancar pada ultranosik
#define echo 4  //inisasi penerima pada ultrasonik
   
float latitude , longitude; //menyimpan nilai posisi gps pada peta
String  lat_str , lng_str; //menyimpan nilai gps dalam bentuk string
const unsigned long petainterval = 0.2; //menyimpan waktu untuk menjalankan subprogram peta
const unsigned long permeninterval = 0.1; //menyimpan waktu untuk menjalankan subprogram servo dan laser
const char *ssid =  "WR";     // memasukkan wifi id
const char *pass =  "cerberus26"; // memasukkan WiFi Password
unsigned long permentimer; //digunakan untuk menyimpan millis subprogram servo dan laser
unsigned long petatimer; //digunakan untuk menyimpan milis subporgram gps dan ultrasonik
char auth[] = "HkwMslyhbsEJOhbHtnZjfye6FcJFu_MP";  //token yang berasal dari blynk
WiFiClient client; // inisasi wificlient
TinyGPSPlus gps; //inisiasi gps
HardwareSerial SerialGPS(1); 

void setup()
{ 
  //void setup digunakan untuk menginisiasi pin dan juga menjalankan beberapa komponen yang dibutuhkan seperti wifi
  Serial.begin(115200);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  permentimer = millis (); 
  petatimer = millis ();
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");              //melakukan print hingga wifi berhasil terkoneksi
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  pinMode(laserPin, OUTPUT);
  pinMode (trig, OUTPUT);
  pinMode (echo, INPUT);
  testservo.attach(13);
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  Blynk.begin(auth, ssid, pass);
  Blynk.virtualWrite(V0, "clr"); 
}

void peta(){
  //void peta merupakan subprogram untuk menjalankan ultrasonik dan gps
  digitalWrite(trig, LOW);
  delay(10);
  digitalWrite(trig, HIGH); //ultrasonik memancarkan gelombang ultrasonik
  delay(10);
  digitalWrite(trig, LOW); //ultrasonik mematikan gelombang ultrasnoik
   
  long durasi;
  float jarak;
   
  durasi = pulseIn (echo, HIGH); //mendapatkan selisih waktu antara memancarkan dan menerima (durasi) dari ultrasnik
  jarak = durasi * 0.034/2; // menapatkan jarak dalam cm yaitu dengan mengkali dengan kecepatan suara dibagi 2 karena sensor memancarkan dan menerima dari pantulan jarak ke tujuan

  Serial.print("Jarak (cm) : ");
  Serial.print(jarak);
  Serial.println(" cm");
  
  if (jarak < 20 ){ //jika tempat sampah penuh 
  
    while (SerialGPS.available() > 0) { 
      //untuk memastikan gps menyala dan dapat membaca datanya dengan benar
      if (gps.encode(SerialGPS.read()))
      {
        if (gps.location.isValid())
        {
          latitude = gps.location.lat();
          lat_str = String(latitude , 6);
          longitude = gps.location.lng();
          lng_str = String(longitude , 6);
          Serial.print("Latitude = ");
          Serial.println(lat_str);
          Serial.print("Longitude = ");
          Serial.println(lng_str); //melakukan print ke serial monitor lokasi gps
          Blynk.virtualWrite(V0, 1, latitude, longitude, "Location"); //melakukan pring ke serial monitor lokasi gps
        }
      }
    }
  }
   else{ //jika tempat sampah tidak penuh
     Blynk.virtualWrite(V0, "clr"); //mereset posisi gps sehingga pada blynk akan hilang
   }
  Blynk.run(); //menjalankan blynk
  petatimer = millis ();   // untuk mengulangi waktu mulai 
}

void permen(){ 
  //subprogram untuk menjalankan servo dan laser
  int val = analogRead(LDR); //menerima pembacaan sensor cahaya
  Serial.println(val); //melakukan print ke monitor untuk melakukan debugging
  if (val >= 1000){ //jika tidak terkena laser (sampah ada yang masuk) 
     testservo.write(0); //mengeluarkan permen
  }
  else{ //jika tidak ada yang masuk
      testservo.write(120); //tutup gerbang permen
  } 
  permentimer = millis ();   //untuk mengulangi waktu 
}

void loop()
{
  //void loop digunakan untuk menjalankan 2 buah subprgoram menggunaakn milis dikarenakan 2 subprogram harus berjalan secara konkuren
  digitalWrite(laserPin, HIGH);
  if ( (millis () - petatimer) >= petainterval)
    peta();

  if ( (millis () - permentimer) >= permeninterval) 
    permen();
  
}
