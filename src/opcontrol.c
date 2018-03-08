#include "api.h"
#include "uart.h"
#include "pros/apix.h"

#define OFF 3
#define GO_UP 1
#define GO_DOWN 0
#define HOLD 2



const int leftFront = 12, rightFront = 11, leftBack = 14, rightBack =13;
const int intake = 5; 
char downButton = 'A'; 
float upGoal = 105 * 5;
int goal_state = 0;
int last_goal_state = -1;
//0 nothing
//1 moving up
//2 moving down
//3 hold



void initMotors() {
  adi_port_config_set(downButton, E_ADI_LEGACY_BUTTON); 

  motor_set_brake_mode(leftFront, E_MOTOR_BRAKE_HOLD);
  motor_set_brake_mode(rightFront, E_MOTOR_BRAKE_HOLD);
  motor_set_brake_mode(leftBack, E_MOTOR_BRAKE_HOLD);
  motor_set_brake_mode(rightBack, E_MOTOR_BRAKE_HOLD);

  motor_encoder_set_units(leftFront, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(rightFront, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(leftBack, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(rightBack, E_MOTOR_ENCODER_COUNTS);

  motor_encoder_set_units(intake, E_MOTOR_ENCODER_DEGREES);

  
  motor_set_reverse(leftFront, true);
  motor_set_reverse(leftBack, true);
}

void setDrive(int16_t leftVel, int16_t rightVel) {
    motor_set_velocity(leftFront, leftVel);
    motor_set_velocity(leftBack, leftVel);

    motor_set_velocity(rightBack, rightVel);
    motor_set_velocity(rightFront, rightVel);
}
void debugDisplay(){
  display_center_printf(1, "Position: %1.2f", motor_get_position(intake)); 
  display_center_printf(2, "Velocity: %1.2f", motor_get_actual_velocity(intake)); 
  display_center_printf(3, "Voltage: %1.2f", motor_get_voltage(intake)); 
  display_center_printf(4, "Current: %d", motor_get_current(intake)); 
  display_center_printf(5, "Torque: %1.2f", motor_get_torque(intake)); 
  display_center_printf(6, "Power: %1.2f", motor_get_power(intake)); 
  display_center_printf(7, "Efficiency: %1.2f", motor_get_efficiency(intake)); 
  display_center_printf(8, "Temperature: %1.2f", motor_get_temperature(intake)); 
  display_center_printf(9, "state: %d", goal_state); 

}

void opcontrol() {
  serctl(SERCTL_DISABLE_COBS, NULL);//turns of dumb shit
  bool lastY = false, joystickMode = true;
  int16_t leftRPM = 0, rightRPM = 0;

  uint8_t packetID = 0;
  int32_t value = 0;
  
  initMotors();
  writeUart(0xF5, 50505);

  
  while (true) {
    int leftJOY = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
    int rightJOY = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_Y);
    if(abs(leftJOY)<15)
      leftJOY = 0;
    if(abs(rightJOY)<15)
      rightJOY = 0;

    //State Transitions
    if(joystickMode && controller_get_digital(CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
      goal_state = GO_DOWN;
    if(joystickMode && controller_get_digital(CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
      goal_state = GO_UP;
    if(joystickMode && controller_get_digital(CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_DOWN))
      goal_state = OFF;
    if(goal_state == GO_DOWN && adi_value_get(downButton) == 1){
      motor_reset_position(intake);
      goal_state = HOLD;
      writeUart(0xf3, goal_state); // I arrived at down
    }
    if(goal_state == GO_UP && abs(motor_get_position(intake) - upGoal) < 0.01 * upGoal){
      goal_state = HOLD;
      writeUart(0xf3, goal_state); // I arrived at up
    }

    readUart(&packetID, &value);
    switch (packetID) {
      case 0x1:
        leftRPM = value / 360.0f;
        break;
      case 0x2:
        rightRPM = value / 360.0f;
        break;
      case 0x3:
        if(!joystickMode)
          goal_state = value;
        break;
      case 0x16:
        break;
      case 0x17:
        break;
      case 0x18:
        break;
    }
    //do arm states
    if(goal_state != last_goal_state){
      switch(goal_state){
        case OFF:
          motor_set_brake_mode(intake, E_MOTOR_BRAKE_COAST);
          motor_set_velocity(intake,0);
          break;
        case GO_UP:
          motor_set_brake_mode(intake, E_MOTOR_BRAKE_HOLD);
          motor_set_absolute_target(intake,upGoal,100);
          break;
        case GO_DOWN:
          motor_set_brake_mode(intake, E_MOTOR_BRAKE_HOLD);
          motor_set_velocity(intake,-100);
          break;
        case HOLD:
          motor_set_velocity(intake,0);
          motor_set_brake_mode(intake, E_MOTOR_BRAKE_HOLD);
          break;
      }
    }
    if (controller_get_digital(CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y) && !lastY) {
      lastY = true;
      joystickMode = !joystickMode;
    } else {
      lastY = false;
    }

    if (joystickMode) {
      setDrive(leftJOY,
               rightJOY);
    } else {
      setDrive(leftRPM, rightRPM);
    }
    writeUart(0xf1, motor_get_position(leftFront));
    writeUart(0xf2, motor_get_position(rightFront));
    delay(10);
  }
}
