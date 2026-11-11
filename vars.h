#include "Arduino.h"

#define seaLevelPressure 1012

sensors_event_t event;

float flat, flon;
unsigned long age;

unsigned long gpsReadStart, gpsInterval = 200;

float pitch;
float roll;
float yaw;

float pitch_angle;
float roll_angle;
