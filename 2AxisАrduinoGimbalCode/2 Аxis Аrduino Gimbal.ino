#include <Wire.h>
#include <Servo.h>
#include <Smoothed.h>

#define SERVO_X                     5
#define SERVO_Y                     6

#define MPU_POWER_REG               0x6B
#define MPU_POWER_CYCLE             0b00000000

#define MPU_ACCEL_CFG_REG           0x1C
#define MPU_ACCEL_CFG_2G            0b00000000
#define MPU_ACCEL_READINGSCALE_2G   16384.0
#define MPU_ACCEL_CFG_4G            0b00001000
#define MPU_ACCEL_READINGSCALE_4G   8192.0
#define MPU_ACCEL_CFG_8G            0b00010000
#define MPU_ACCEL_READINGSCALE_8G   4096.0
#define MPU_ACCEL_READ_REG          0x3B
#define MPU_ACCEL_READ_REG_SIZE     6

#define MPU_I2C_ADDRESS             0b1101000

Servo ServoX, ServoY;
double AcX,AcY,AcZ,TotalVector;
int Pitch,Roll,ServoPitch,ServoRoll;

Smoothed <float> ServoPitchSm;
Smoothed <float> ServoRollSm;

void setup() {
  Serial.begin(9600);
  ServoX.attach(SERVO_X);
  ServoY.attach(SERVO_Y);
  Wire.begin();
  SetupMPU();                                       // configuring and waking up the MPU6050
  MoveServosToZeroPosition(); 
 
  ServoPitchSm.begin(SMOOTHED_EXPONENTIAL, 25);  // starts smoothing the data for the servos
  ServoRollSm.begin(SMOOTHED_EXPONENTIAL, 25);  // starts smoothing the data for the servos
}

void SetupMPU(){
  Wire.beginTransmission(MPU_I2C_ADDRESS);
  Wire.write(MPU_POWER_REG);
  Wire.write(MPU_POWER_CYCLE);
  Wire.endTransmission();

  Wire.beginTransmission(MPU_I2C_ADDRESS);
  Wire.write(MPU_ACCEL_CFG_REG);
  Wire.write(MPU_ACCEL_CFG_4G);
  Wire.endTransmission();
}


void loop() {
  MPUReadAccel();
  MPUCalculatePitchAndRoll();
  MoveServo();
  printData();
}

void MPUReadAccel(){
  Wire.beginTransmission(MPU_I2C_ADDRESS);
  Wire.write(MPU_ACCEL_READ_REG);
  Wire.endTransmission();
  Wire.requestFrom(MPU_I2C_ADDRESS, MPU_ACCEL_READ_REG_SIZE);

  AcX = (long) (Wire.read() << 8 | Wire.read()) / MPU_ACCEL_READINGSCALE_4G;
  AcY = (long)(Wire.read() << 8 | Wire.read()) / MPU_ACCEL_READINGSCALE_4G;
  AcZ = (long) (Wire.read() << 8 | Wire.read()) / MPU_ACCEL_READINGSCALE_4G;
}


void MPUCalculatePitchAndRoll(){
  TotalVector = sqrt((AcX * AcX)+ (AcY * AcY)+ (AcZ * AcZ));
  Pitch = asin((float)AcY / TotalVector) * 57.296;
  Roll =  asin((float)AcX / TotalVector) * -57.256;

}

void MoveServo(){
  ServoPitchSm.add(map(Pitch, -78, 80, 180, 0));
  ServoRollSm.add(map(Roll, -75, 80, 0, 180));
  
  ServoPitch = ServoPitchSm.get();
  ServoRoll = ServoRollSm.get();
 
  ServoX.write(ServoRoll);
  ServoY.write(ServoPitch);
}

void MoveServosToZeroPosition(){
  ServoX.write(0);
  ServoY.write(0);
  }
  
void printData(){
  Serial.print(" ServoPitch: ");
  Serial.print(ServoPitch);
  Serial.print(" ServoRoll: ");
  Serial.print(ServoRoll);
  Serial.print(" Accel (g)");
  Serial.print(" X=");
  Serial.print(AcX);
  Serial.print(" Y=");
  Serial.print(AcY);
  Serial.print(" Z=");
  Serial.println(AcZ);
  Serial.print(" Roll:");
  Serial.print(Roll);
  Serial.print(" Pitch:");
  Serial.print(Pitch);
}
