#include "api.h"

float getNormalized(int current,int min, int max){
  return ((float)(current - min))/((float)(max-min));
}

float sign(float val) {
  if (val < 0) {
    return -1.0;
  } else if (val > 0) {
    return 1.0;
  }

  return 0.0;
}

void opcontrol() {
  const int leftMotor = 10, rightMotor = 20;

  bool lastY = false, lineFollowMode = true;
  
  motor_set_position(leftMotor, 0);
  motor_set_position(rightMotor, 0);
  motor_set_brake_mode(leftMotor, E_MOTOR_BRAKE_COAST);
  motor_set_brake_mode(rightMotor, E_MOTOR_BRAKE_COAST);
  motor_encoder_set_units(leftMotor, E_MOTOR_ENCODER_COUNTS);
  motor_encoder_set_units(rightMotor, E_MOTOR_ENCODER_COUNTS);
  //motor_set_reverse(rightMotor, true);

  adi_port_config_set('A', E_ADI_LEGACY_LINE_SENSOR);
  adi_port_config_set('B', E_ADI_LEGACY_LINE_SENSOR);
  delay(500);
  
  const int rotation_amount = 450;
  int max_left = 0, min_left = 4096;
  int max_right = 0, min_right = 4096;

  motor_set_absolute_target(leftMotor,rotation_amount,20);
  motor_set_absolute_target(rightMotor,rotation_amount,20);

  while (abs(motor_get_position(leftMotor) - rotation_amount) > 5) {
    int current_left = adi_value_get('A');
    int current_right = adi_value_get('B');
    printf("%d, ", current_left);

    max_left = max_left > current_left ? max_left : current_left;
    min_left = min_left < current_left ? min_left : current_left;

    max_right = max_right > current_right ? max_right : current_right;
    min_right = min_right < current_right ? min_right : current_right;
  }

  motor_set_absolute_target(leftMotor, -rotation_amount, 20);
  motor_set_absolute_target(rightMotor, -rotation_amount, 20);

  while (abs(motor_get_position(leftMotor) + rotation_amount) > 5){
    int current_left = adi_value_get('A');
    int current_right = adi_value_get('B');

    max_left = max_left > current_left ? max_left : current_left;
    min_left = min_left < current_left ? min_left : current_left;

    max_right = max_right > current_right ? max_right : current_right;
    min_right = min_right < current_right ? min_right : current_right;
  }

  motor_set_absolute_target(leftMotor, 0, 20);
  motor_set_absolute_target(rightMotor, 0, 20);
  delay(1000);

  float integral = 0, lastError = 0;

  while (true) {
    int left = controller_get_analog(CONTROLLER_MASTER, ANALOG_LEFT_Y);
    int right = controller_get_analog(CONTROLLER_MASTER, ANALOG_RIGHT_Y);

    // printf("joystick: %d, %d\n", left, right);
    // printf("motor: %1.2f, %d, %1.2f, %1.2f, %1.2f, %1.2f, %1.2f\n",
    //   motor_get_position(1), motor_get_velocity(1),
    //   motor_get_voltage(1), motor_get_power(1),
    //   motor_get_torque(1), motor_get_efficiency(1), motor_get_temperature(1));
    
    if (controller_get_digital(CONTROLLER_MASTER, DIGITAL_Y) && !lastY) {
      lastY = true;
      lineFollowMode = !lineFollowMode;
    } else {
      lastY = false;
    }

    if (lineFollowMode) {
      const float kP = 20.0, kI = 0, kD = 1, offset = 20;

      float left_normalized = getNormalized(adi_value_get('A'), min_left, max_left);
      float right_normalized = getNormalized(adi_value_get('B'), min_right, max_right);
      float diff = right_normalized - left_normalized;
      float error = diff > 0 ? diff * diff * diff : diff * diff * diff;
      integral += error;
      int output = kP * error + kI * integral + kD * (error - lastError);
      lastError = error;

      motor_set_velocity(leftMotor,  -output + offset);
      motor_set_velocity(rightMotor, -1 * (output + offset));

      display_center_printf(8, "Left: %1.2f, Right: %1.2f", left_normalized, right_normalized);
      display_center_printf(9, "error: %1.2f, sum: %1.2f", error, integral);
    } else {
      motor_set_velocity(leftMotor, left);
      motor_set_velocity(rightMotor, right);
    }
    
    // display_erase();
    // display_center_printf(1, "Read: %c", fgetc(stdin));
    display_center_printf(1, "Position: %1.2f", motor_get_position(leftMotor));
    display_center_printf(2, "Velocity: %1.2f", motor_get_actual_velocity(leftMotor));
    display_center_printf(3, "Voltage: %1.2f", motor_get_voltage(leftMotor));
    display_center_printf(4, "Torque: %1.2f", motor_get_torque(leftMotor));
    display_center_printf(5, "Power: %1.2f", motor_get_power(leftMotor));
    display_center_printf(6, "Efficiency: %1.2f", motor_get_efficiency(leftMotor));
    display_center_printf(7, "Temperature: %1.2f", motor_get_temperature(leftMotor));
    
    delay(10);
  }
}

