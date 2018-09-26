#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

EthernetUDP Udp;

Servo frontRight;
Servo frontLeft;
Servo backRight;
Servo backLeft;
Servo vertical;
Servo motors[5] = {frontRight, frontLeft, backRight, backLeft, vertical};

byte ip[]                    = {10, 10, 5, 124};
byte mac[]                   = {0x90, 0xa2, 0xda, 0x00, 0x23, 0x16};
const unsigned int localPort = 1369;
byte        remoteIp[4] = {10, 10, 5, 123};
unsigned int remotePort = 9631;
char recvdBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
char replyBuffer[UDP_TX_PACKET_MAX_SIZE + 1];

boolean button;
boolean firstTime = true;

int joystickDeadzone;

int right[2]; // element one is x, two is y
float *rightAngleMagPtr;
float rightAngleMag[2]; // element one is angle. two is magnitude

int left[2];
float *leftAngleMagPtr;
float leftAngleMag[2];

byte gear; // 1 is A, 2 is B, 3 is Y
boolean buttonX = false;

int power = 0;

void setup()
{
  pinMode(4, OUTPUT);
  analogWrite(4, HIGH);

  for (int i = 0; i < 5; i++)
  {
    motors[i].attach(9 - i);
    motors[i].writeMicroseconds(1500);
  }

  delay(1500);

  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.println("local init done");
}

void loop()
{
  rightAngleMag[0] = 0;
  rightAngleMag[1] = 0;

  leftAngleMag[0] = 0;
  leftAngleMag[1] = 0;

  int recvdSize = Udp.parsePacket(); // note that this includes the UDP header

  if (recvdSize)
  {
    Udp.read(recvdBuffer, UDP_TX_PACKET_MAX_SIZE);

    //    Serial.print("Received packet data size = ");
    Serial.println(recvdSize);
    //    Serial.print(" from ");
    //    Serial.print(Udp.remoteIP());
    //    Serial.print(", port number: ");
    //    Serial.print(Udp.remotePort());
    //    Serial.println();
    //    Serial.println("Contents: ");

    //    Serial.print("{");
    //    for (int i = 0; i < recvdSize; i++)
    //    {
    //      Serial.print((byte) recvdBuffer[i]);
    //      Serial.print(i != recvdSize - 1 ? ", " : "}");
    //    }
    //    Serial.println();

    if (firstTime)
    {
      firstTime = !firstTime;
      button = (recvdBuffer[0] == 1) ? true : false;
      if (!button) joystickDeadzone = recvdBuffer[1];
    }

    else
    {
      if (button)
      {
        power = recvdBuffer[0];
      }

      else
      {
        right[0] = (byte) recvdBuffer[0] * ((byte) recvdBuffer[1] == 0 ? 1 : -1);
        right[1] = (byte) recvdBuffer[2] * ((byte) recvdBuffer[3] == 0 ? 1 : -1);

        left[0] = (byte) recvdBuffer[4] * ((byte) recvdBuffer[5] == 0 ? 1 : -1);
        left[1] = (byte) recvdBuffer[6] * ((byte) recvdBuffer[7] == 0 ? 1 : -1);

        rightAngleMagPtr = returnAngleMag(right[0], right[1]);
        rightAngleMag[0] = *rightAngleMagPtr;
        rightAngleMag[1] = *(rightAngleMagPtr + 1);

        leftAngleMagPtr = returnAngleMag(left[0], left[1]);
        leftAngleMag[0] = *leftAngleMagPtr;
        leftAngleMag[1] = *(leftAngleMagPtr + 1);

        gear = (byte) recvdBuffer[8];
        buttonX = ((byte) recvdBuffer[9]) ? !buttonX : buttonX;
      } // if not button
    } // not firstTime
  } // if something was recieved over ethernet

  if (!firstTime)
  {
    if (button)
    {
      for (int i = 0; i < 5; i++)
      {
        motors[i].writeMicroseconds(power * -1 * 200 + 1500);
      }
    }

    else
    {
      float sine = sin(PI * ((rightAngleMag[0]) / 180 - .25)) * rightAngleMag[1] / (255.0 - joystickDeadzone);
      float cosine = cos(PI * ((rightAngleMag[0] / 180) - .25)) * rightAngleMag[1] / (255.0 - joystickDeadzone);
      
      motors[0].writeMicroseconds(1500 + (400 * gear) + (400.0 / 3 * sine));
      motors[1].writeMicroseconds(1500 + (400 * gear) + (400.0 / 3 * cosine));
      motors[2].writeMicroseconds(1500 - (400 * gear) - (400.0 / 3 * cosine));
      motors[3].writeMicroseconds(1500 - (400 * gear) - (400.0 / 3 * sine));
      
      motors[4].writeMicroseconds(1500 + 400 * (sin(leftAngleMag[0] / 180 * PI) * leftAngleMag[1] / (255.0 - joystickDeadzone)));
    } // if not button
  } // if not firstTime (send values to thrusters)
} // Loop function

float *returnAngleMag(float x, float y)
{
  static float angleMag[2];

  angleMag[0] = atan2(y, x) / PI * 180.0;
  angleMag[0] += (angleMag[0] > 0) ? 0 : 360;
  angleMag[1] = pow(pow(x, 2) + pow(y, 2), 0.5) - joystickDeadzone;

  return angleMag;
}

