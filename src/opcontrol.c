#include "api.h"
#include "uart.h"

const int leftMotor1 = 1, leftMotor2 = 2;
const int rightMotor1 = 3, rightMotor2 = 4;

void initMotors() {
  motor_reset_position(leftMotor1);
  motor_reset_position(leftMotor2);
  motor_reset_position(rightMotor1);
  motor_reset_position(rightMotor2);

  motor_set_brake_mode(leftMotor1, E_MOTOR_BRAKE_COAST);
  motor_set_brake_mode(leftMotor2, E_MOTOR_BRAKE_COAST);
  motor_set_brake_mode(rightMotor1, E_MOTOR_BRAKE_COAST);
  motor_set_brake_mode(rightMotor2, E_MOTOR_BRAKE_COAST);

  motor_encoder_set_units(leftMotor1, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(leftMotor2, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(rightMotor1, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(rightMotor2, E_MOTOR_ENCODER_COUNTS);
}

void setDrive(int16_t leftVel, int16_t rightVel) {
  motor_set_velocity(leftMotor1, leftVel);
  motor_set_velocity(leftMotor2, leftVel);
  motor_set_velocity(rightMotor1, rightVel);
  motor_set_velocity(rightMotor2, rightVel);
}

void opcontrol() {
  initMotors();
  
  while (true) {
    writeUart(0xF5, 50505);
    
    delay(15);
  }

}
