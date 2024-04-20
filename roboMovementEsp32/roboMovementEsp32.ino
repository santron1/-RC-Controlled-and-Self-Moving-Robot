// SO, THIS IS MY ROBOT CODE , WHICH CHANGES MODES THROUGH SWITCH FROM RC TO SELF MOVE
#include <ESP32Servo.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include <Arduino.h>
#include "DataParser.h"
#include <NewPing.h>

#define TRIGGER_PIN 1
#define ECHO_PIN 3
#define MAX_DISTANCE 200
#define switchPin 15
const char *ssid = "Redmi 9";
const char *password = "45454455";
const int udpPort = 12345;
DataParser dataParser;

int Speed = 50;
int Right_speed = 0;
int Left_speed = 0;
AsyncUDP udp;
Servo servo;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
const int in1 = 27;
const int in2 = 26;
const int ena = 14;
const int in3 = 25;
const int in4 = 33;
const int enb = 32;

bool obstacleDetected = false;

void setup()
{
  Serial.begin(115200);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);

  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);

  servo.attach(13);
  servo.writeMicroseconds(1500);
  pinMode(switchPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  if (udp.listen(udpPort))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet)
                 {
      String IncomingData = (char*)packet.data();
      dataParser.parseData(IncomingData, ',');

      Speed = (dataParser.getField(1)).toInt();
      Left_speed = Speed;
      Right_speed = Speed; });
  }
}

void loop()
{
  int switchState = digitalRead(switchPin);

  if (switchState == LOW)
  {
    Serial.println("Handling commands...");
    handleCommands();
  }
  else
  {
    Serial.println("Handling obstacle avoidance...");
    handleObstacleAvoidance();
  }

  delay(1000); // Add a delay to avoid overwhelming the serial monitor
}

void handleCommands()
{
  if (dataParser.getField(0) == "f")
  {
    forward(Left_speed, Right_speed);
    Serial.println("fwd");
    obstacleDetected = false; // Reset obstacle detection flag when moving forward
  }
  if (dataParser.getField(0) == "b")
  {
    backward(Left_speed, Right_speed);
    Serial.println("bck");
    obstacleDetected = false; // Reset obstacle detection flag when moving backward
  }
  if (dataParser.getField(0) == "l")
  {
    left(Left_speed, Right_speed);
    Serial.println("left");
    obstacleDetected = false; // Reset obstacle detection flag when turning left
  }
  if (dataParser.getField(0) == "r")
  {
    right(Left_speed, Right_speed);
    Serial.println("right");
    obstacleDetected = false; // Reset obstacle detection flag when turning right
  }
  if (dataParser.getField(0) == "s")
  {
    Stop();
    obstacleDetected = false; // Reset obstacle detection flag when stopping
  }
}

void handleObstacleAvoidance()
{
  int distance = sonar.ping_cm();
  Serial.print(distance);
  Serial.println(" cm");
  servo.writeMicroseconds(1500);

  delay(100); // Allow some time for the servo to move, adjust as needed

  // Check the distance at the center (head) of the robot
  int distanceCenter = sonar.ping_cm();
  Serial.print("Center distance: ");
  Serial.println(distanceCenter);

  if (distance < 20)
  {
    Stop();

    delay(500);

    backward(100, 100);
    Stop();
    delay(1000);

    // Assuming servo is mounted on the head and rotates to look around
    servo.writeMicroseconds(2300); // Turn right
    delay(1000);

    // Check the distance on the right side
    int distanceRight = sonar.ping_cm();
    Serial.print("Right distance: ");
    Serial.println(distanceRight);

    // Move the servo to the left to check the left side
    servo.writeMicroseconds(700); // Turn left
    delay(1000);

    // Check the distance on the left side
    int distanceLeft = sonar.ping_cm();
    Serial.print("Left distance: ");
    Serial.println(distanceLeft);

    if (distanceRight < 20 || distanceLeft < 20)
    {
      // Obstacle on the right or left, turn in the opposite direction
      if (distanceRight < distanceLeft)
      {
        // Turn left
        left(255, 255);
        delay(1000);
      }
      else
      {
        // Turn right
        right(255, 255);
        delay(1000);
      }

      Stop();
      delay(500);
    }
    else
    {
      // No obstacle on the right or left, move forward
      forward(255, 255);
    }

    // Move the servo back to the center
    servo.writeMicroseconds(1500);
    delay(500);
  }
  else
  {
    // No obstacle at the center, move forward
    forward(255, 255);

    // If the robot was moving forward and detects an obstacle, go backward briefly
    if (!obstacleDetected)
    {
      backward(100, 100);
      delay(500);
      Stop();
    }
    obstacleDetected = true;
  }
}
void forward(int left_speed, int right_speed)
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena, left_speed);
  analogWrite(enb, right_speed);
}

void backward(int left_speed, int right_speed)
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(ena, left_speed);
  analogWrite(enb, right_speed);
}

void left(int left_speed, int right_speed)
{
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(ena, left_speed);
  analogWrite(enb, right_speed);
}

void right(int left_speed, int right_speed)
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(ena, left_speed);
  analogWrite(enb, right_speed);
}

void Stop()
{
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(ena, 0);
  analogWrite(enb, 0);
}
