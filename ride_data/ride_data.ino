#include "Wire.h"
/*
 * Bluetooth / IMU Sketch for RideData.
 * Uses 
 * Bluetooth Shield V2
 * SKU: 113030019 from SeeedStudio
 * 
 * Grove IMU 9DOF v2.0
 * SKU: 101020080 from SeeedStudio
 * 
 * 
 */
// I2Cdev and MPU9250 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU9250.h"
#include "BMP180.h"

#include <SoftwareSerial.h>   //Software Serial Port
#define RxD 7
#define TxD 6

#define DEBUG_ENABLED  1

//#define PIN_TEMP    A5


SoftwareSerial blueToothSerial(RxD,TxD);

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU9250 accelgyro;
I2Cdev   I2C_M;

uint8_t buffer_m[6];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;


float heading;
float tiltheading;

float Axyz[3];
float Gxyz[3];
float Mxyz[3];

#define sample_num_mdate  5000

volatile float mx_sample[3];
volatile float my_sample[3];
volatile float mz_sample[3];

static float mx_centre = 0;
static float my_centre = 0;
static float mz_centre = 0;

volatile int mx_max = 0;
volatile int my_max = 0;
volatile int mz_max = 0;

volatile int mx_min = 0;
volatile int my_min = 0;
volatile int mz_min = 0;


float temperature;
float pressure;
float atm;
float altitude;
BMP180 Barometer;


int revCount;
boolean gyr = false;
boolean accel = true;
void setup() {
    Serial.begin(9600);
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    setupBlueToothConnection();
    
    Wire.begin();
    accelgyro.initialize();
   // accelgyro.setExternalFrameSync(3);

    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU9250 connection successful" : "MPU9250 connection failed");


}

void loop() {

  char recvChar;
    while(1)
    {
        if(blueToothSerial.available())
        {        
            recvChar = blueToothSerial.read();
            Serial.print(recvChar);            
            getCompassDate_calibrated();
            getHeading();               
            getTiltHeading();
          
            
            /*Serial.println("calibration parameter: ");
            Serial.print(mx_centre);
            Serial.print("         ");
            Serial.print(my_centre);
            Serial.print("         ");
            Serial.println(mz_centre);
            Serial.println("     ");
                Serial.println("Compass Value of X,Y,Z:");
            Serial.print(Mxyz[0]);
            Serial.print(",");
            Serial.print(Mxyz[1]);
            Serial.print(",");
            Serial.println(Mxyz[2]);
            Serial.println("The clockwise angle between the magnetic north and X-Axis:");
            Serial.print(heading);
            Serial.println(" ");
            Serial.println("The clockwise angle between the magnetic north and the projection of the positive X-Axis in the horizontal plane:");
            Serial.println(tiltheading);
            Serial.println("   ");*/

    
            if(recvChar == 'a' || recvChar == 'A')
            {
              getGyro_Data();
              getAccel_Data();
              getCompassDate_calibrated();
              getHeading();
              getTiltHeading();
              
              temperature = Barometer.bmp180GetTemperature(Barometer.bmp180ReadUT()); //Get the temperature, bmp180ReadUT MUST be called first
              pressure = Barometer.bmp180GetPressure(Barometer.bmp180ReadUP());//Get the temperature
              altitude = Barometer.calcAltitude(pressure); //Uncompensated caculation - in Meters
              atm = pressure / 101325;

                blueToothSerial.print(altitude, 2); //display 2 decimal places 15
                blueToothSerial.print(", ");
                blueToothSerial.print(atm, 4); //display 2 decimal places 15
                blueToothSerial.print(", ");
                
                blueToothSerial.print(pressure, 0); //whole number only. 13
                blueToothSerial.print(", ");
                blueToothSerial.print(temperature, 2); //display 2 decimal places 12
                blueToothSerial.print(", ");
                blueToothSerial.print(tiltheading); // 11
                blueToothSerial.print(", ");
                blueToothSerial.print(heading); // 10
                blueToothSerial.print(", ");
                
                blueToothSerial.print(Mxyz[0]); // 9 compass z
                blueToothSerial.print(", ");
                blueToothSerial.print(Mxyz[1]); // 8 compass y
                blueToothSerial.print(",");
                blueToothSerial.print(Mxyz[2]); // start compass 7 compass x
                blueToothSerial.print(",");

                
                blueToothSerial.print(Axyz[2]);
                blueToothSerial.print(", ");
                blueToothSerial.print(Axyz[1]);
                blueToothSerial.print(", ");
                blueToothSerial.print(Axyz[0]);
                blueToothSerial.print(", ");
                
                blueToothSerial.print(Gxyz[2]);
                blueToothSerial.print(", ");
                blueToothSerial.print(Gxyz[1]);
                blueToothSerial.print(", ");
                blueToothSerial.print(Gxyz[0]);
                blueToothSerial.println(",");
            }
        }
        if(Serial.available())
        {
          recvChar = Serial.read();
          blueToothSerial.print(recvChar);
        }
        delay(200);
    }
    
    
}


void setupBlueToothConnection()
{          
  blueToothSerial.begin(9600);  
  
  blueToothSerial.print("AT");
  delay(400); 

 // blueToothSerial.print("AT+DEFAULT");             // Restore all setup value to factory setup
//  delay(2000); 
  
  blueToothSerial.print("AT+NAMESeeedBTSlave");    // set the bluetooth name as "SeeedBTSlave" ,the length of bluetooth name must less than 12 characters.
  delay(400);
  
 // blueToothSerial.print("AT+PIN0000");             // set the pair code to connect 
//  delay(400);
  
  blueToothSerial.print("AT+AUTH0");             //
  delay(400);
  
//  blueToothSerial.print("AT+DISC0");             //
//  delay(400);    

  blueToothSerial.flush();
  
}

void getAccel_Data(void)
{
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
    Axyz[0] = (double) ax / 4096;
    Axyz[1] = (double) ay / 4096;
    Axyz[2] = (double) az / 4096;
   

}

void getGyro_Data(void)
{
    accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);
      Gxyz[0] = (double) gx * 250 / 16384;
    Gxyz[1] = (double) gy * 250 / 16384;
    Gxyz[2] = (double) gz * 250 / 16384;
}



void getHeading(void)
{
    heading = 180 * atan2(Mxyz[1], Mxyz[0]) / PI;
    if (heading < 0) heading += 360;
}

void getTiltHeading(void)
{
    float pitch = asin(-Axyz[0]);
    float roll = asin(Axyz[1] / cos(pitch));

    float xh = Mxyz[0] * cos(pitch) + Mxyz[2] * sin(pitch);
    float yh = Mxyz[0] * sin(roll) * sin(pitch) + Mxyz[1] * cos(roll) - Mxyz[2] * sin(roll) * cos(pitch);
    float zh = -Mxyz[0] * cos(roll) * sin(pitch) + Mxyz[1] * sin(roll) + Mxyz[2] * cos(roll) * cos(pitch);
    tiltheading = 180 * atan2(yh, xh) / PI;
    if (yh < 0)    tiltheading += 360;
}



void Mxyz_init_calibrated ()
{

    Serial.println(F("Before using 9DOF,we need to calibrate the compass frist,It will takes about 2 minutes."));
    Serial.print("  ");
    Serial.println(F("During  calibratting ,you should rotate and turn the 9DOF all the time within 2 minutes."));
    Serial.print("  ");
    Serial.println(F("If you are ready ,please sent a command data 'ready' to start sample and calibrate."));
    while (!Serial.find("ready"));
    Serial.println("  ");
    Serial.println("ready");
    Serial.println("Sample starting......");
    Serial.println("waiting ......");

    get_calibration_Data ();

    Serial.println("     ");
    Serial.println("compass calibration parameter ");
    Serial.print(mx_centre);
    Serial.print("     ");
    Serial.print(my_centre);
    Serial.print("     ");
    Serial.println(mz_centre);
    Serial.println("    ");
}


void get_calibration_Data ()
{
    for (int i = 0; i < sample_num_mdate; i++)
    {
        get_one_sample_date_mxyz();
        /*
        Serial.print(mx_sample[2]);
        Serial.print(" ");
        Serial.print(my_sample[2]);                            //you can see the sample data here .
        Serial.print(" ");
        Serial.println(mz_sample[2]);
        */



        if (mx_sample[2] >= mx_sample[1])mx_sample[1] = mx_sample[2];
        if (my_sample[2] >= my_sample[1])my_sample[1] = my_sample[2]; //find max value
        if (mz_sample[2] >= mz_sample[1])mz_sample[1] = mz_sample[2];

        if (mx_sample[2] <= mx_sample[0])mx_sample[0] = mx_sample[2];
        if (my_sample[2] <= my_sample[0])my_sample[0] = my_sample[2]; //find min value
        if (mz_sample[2] <= mz_sample[0])mz_sample[0] = mz_sample[2];

    }

    mx_max = mx_sample[1];
    my_max = my_sample[1];
    mz_max = mz_sample[1];

    mx_min = mx_sample[0];
    my_min = my_sample[0];
    mz_min = mz_sample[0];



    mx_centre = (mx_max + mx_min) / 2;
    my_centre = (my_max + my_min) / 2;
    mz_centre = (mz_max + mz_min) / 2;

}






void get_one_sample_date_mxyz()
{
    getCompass_Data();
    mx_sample[2] = Mxyz[0];
    my_sample[2] = Mxyz[1];
    mz_sample[2] = Mxyz[2];
}

void getCompass_Data(void)
{
    I2C_M.writeByte(MPU9150_RA_MAG_ADDRESS, 0x0A, 0x01); //enable the magnetometer
    delay(10);
    I2C_M.readBytes(MPU9150_RA_MAG_ADDRESS, MPU9150_RA_MAG_XOUT_L, 6, buffer_m);

    mx = ((int16_t)(buffer_m[1]) << 8) | buffer_m[0] ;
    my = ((int16_t)(buffer_m[3]) << 8) | buffer_m[2] ;
    mz = ((int16_t)(buffer_m[5]) << 8) | buffer_m[4] ;

    Mxyz[0] = (double) mx * 1200 / 4096;
    Mxyz[1] = (double) my * 1200 / 4096;
    Mxyz[2] = (double) mz * 1200 / 4096;
}

void getCompassDate_calibrated ()
{
    getCompass_Data();
    Mxyz[0] = Mxyz[0] - mx_centre;
    Mxyz[1] = Mxyz[1] - my_centre;
    Mxyz[2] = Mxyz[2] - mz_centre;
}

