#include <XBOXUSB.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

USB Usb;
XBOXUSB Xbox(&Usb);
EthernetUDP Udp;

byte ip[]                                 = {10, 10, 5, 123};
byte mac[]                             = {0x90, 0xa2, 0xda, 0x00, 0x11, 0x07};
const unsigned int localPort  = 9631;
byte        remoteIp[4] = {10, 10, 5, 124};
unsigned int remotePort = 1369;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
const int joystickDeadzone = 8000;
boolean button = false;
//byte initBuffer[2] = {button, joystickDeadzone / 128.0};

boolean firstTime = true;
boolean sendStuff = false;

byte last;
boolean lastVert;
boolean lastDir;
boolean servoOn = false;

byte numJoystickVals = 8;
int rightx;
int righty;
int leftx;
int lefty;

void setup()
{
  Serial.begin(9600);
  delay(1);
  Ethernet.begin(mac, ip);
  delay(1);
  Udp.begin(localPort);
  delay(1);

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  if (!button)
  {
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif

    if (Usb.Init() == -1)
    {
      Serial.println(F("\r\nOSC did not start"));
      while (1); //halt
    }

    Serial.println(F("\r\nXBOX USB Library Started..."));
  }

  else
  {
    lastVert = false;
    lastDir = false;
  }
}

void loop()
{
  byte stuff[80];
  
  if (firstTime)
  {
    stuff[0] = (byte) button;
    stuff[1] = abs(joystickDeadzone) / 128.0;
    
    Udp.beginPacket(remoteIp, remotePort);
    Udp.write(stuff, 80);
    Udp.endPacket();

    firstTime = false;
  }
  
  else
  {
    int sizeOfPacket = Udp.parsePacket();
    if (sizeOfPacket)
    {
      Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  
      Serial.println("*****received packet info*****");
      //      Serial.println(sizeOfPacket);
      ////      Serial.println(Udp.remoteIp());
      ////      Serial.println(Udp.remotePort());
      //
      //      Serial.print("Contents: ");
      //      Serial.println(packetBuffer);
      //      Serial.println("************************************");
      //      Serial.println();
    }
    
    if (button)
    {
      if (lastVert != (boolean) digitalRead(2))
      {
        if (digitalRead(2) == HIGH) sendStuff = true;
        servoOn = true;
        lastVert = (boolean) digitalRead(2);
      }
  
      else if (lastDir != (boolean) digitalRead(3))
      {
        if (digitalRead(3) == HIGH) sendStuff = true;
        servoOn = false;
        lastDir = (boolean) digitalRead(3);
      }
    } // if 'button' is true
  
    else
    {
      Usb.Task();
  
      if (Xbox.Xbox360Connected)
      {
        for (byte i = 0; i < 4 + numJoystickVals; i++)
        {
          stuff[i] = 0;
        }
  
        rightx = constrain(Xbox.getAnalogHat(RightHatX), -32767, 32767);
        righty = constrain(Xbox.getAnalogHat(RightHatY), -32767, 32767);
        leftx = constrain(Xbox.getAnalogHat(LeftHatX), -32767, 32767);
        lefty = constrain(Xbox.getAnalogHat(LeftHatY), -32767, 32767);
  
        if  (pow(pow(rightx, 2) + pow(righty, 2), 0.5) > joystickDeadzone)
        {
          stuff[1] = rightx > 0 ? 0 : 1;
          stuff[0] = abs(rightx) / 128.0;
  
          stuff[3] = righty > 0 ? 0 : 1;
          stuff[2] = abs(righty) / 128.0;
  
          sendStuff = true;
          Serial.println("rightJS");
        }
  
        if  (pow(pow(leftx, 2) + pow(lefty, 2), 0.5) > joystickDeadzone)
        {
          stuff[5] = leftx > 0 ? 0 : 1;
          stuff[4] = abs(leftx) / 128.0;
  
          stuff[7] = lefty > 0 ? 0 : 1;
          stuff[6] = abs(lefty) / 128.0;
  
          sendStuff = true;
          Serial.println("leftJS");
        }
  
        if (Xbox.getButtonClick(A))
        {
          stuff[numJoystickVals] = 0;
          sendStuff = true;
          Serial.println("buttonA");
        }

        else if (Xbox.getButtonClick(B))
        {
          stuff[numJoystickVals] = 1;
          sendStuff = true;
          Serial.println("buttonB");
        }

        else if (Xbox.getButtonClick(Y))
        {
          stuff[numJoystickVals] = 2;
          sendStuff = true;
          Serial.println("buttonY");
        }

        if (Xbox.getButtonClick(X))
        {
          stuff[numJoystickVals + 1] = 1;
          sendStuff = true;
          Serial.println("buttonX");
        }
      } // If the controller is connected
      delay(1);
    } // if 'button' is false
  
    if (sendStuff)
    {
      if (button)
      {
        Udp.beginPacket(remoteIp, remotePort);
        Udp.write((byte) servoOn);
        Udp.endPacket();
      }
  
      else
      {
        Udp.beginPacket(remoteIp, remotePort);
        Udp.write(stuff, 80);
        Udp.endPacket();
      }
  
      Serial.println("sent");
      Serial.println();
  
      sendStuff = false;
    } // if sendStuff
  } // if not firstTime
} // Loop function

