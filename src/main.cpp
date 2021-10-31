#include "main.h"
#include "EZ-Template/AutonSelector.hpp"
#include <iostream>
using namespace std;
/**
 * Disables all tasks.
 *
 * This runs during disabled and initialize to turn off all user created tasks.
 */
void disable_all_tasks()
{
  drive_pid.suspend();
}


/**
 * Autonomous selector using LLEMU.
 *
 * When this function is called, display the current selected auton to the brain.
 * When is_auton is true, the autonomous mode will run.  Otherwise, it will only
 * print to brain.
 */

// Global for updating SD
void update_auto_sd();


// Page up/down
AutonSelector autoSelector {};
void page_up()
{

  if (autoSelector.CurrentAutonPage == autoSelector.AutonCount - 1)
    autoSelector.CurrentAutonPage = 0;
  else
    autoSelector.CurrentAutonPage++;
  update_auto_sd();
  autoSelector.PrintSelectedAuto();
}
void page_down()
{
  if (autoSelector.CurrentAutonPage == 0)
    autoSelector.CurrentAutonPage = autoSelector.AutonCount - 1;
  else
    autoSelector.CurrentAutonPage--;
  update_auto_sd();
  autoSelector.PrintSelectedAuto();
}


/**
 * Store the last page to the micro SD card
 *
 * This allows you to select your autonomous mode while in the queue or in the pits.
 * If you powercycle the robot or turn off the code, the autonomous mode you selected
 * will still hold.
 */
void
update_auto_sd() {
  // If no SD card, return
  if (!IS_SD_CARD) return;

  FILE* usd_file_write = fopen("/usd/auto.txt", "w");
  std::string cp_str = std::to_string(autoSelector.CurrentAutonPage);
  char const *cp_c = cp_str.c_str();
  fputs(cp_c, usd_file_write);
  fclose(usd_file_write);
}

void
init_auto_sd() {
  // If no SD card, return
  if (!IS_SD_CARD)  return;

  // Auton Selector
  FILE* usd_file_read = fopen("/usd/auto.txt", "r");
  char buf[5];
  fread(buf, 1, 5, usd_file_read);
  autoSelector.CurrentAutonPage = std::stoi(buf);
  fclose(usd_file_read);

  if(autoSelector.CurrentAutonPage>autoSelector.AutonCount-1 || autoSelector.CurrentAutonPage<0)
    autoSelector.CurrentAutonPage=0;
  update_auto_sd();
}


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize()
{
  print_ez_template();
  pros::delay(500);

  if (!IS_SD_CARD) printf("No SD Card Found!\n");

  disable_all_tasks();

  init_auto_sd();
  init_curve_sd();

  pros::lcd::initialize();
  autoSelector.PrintSelectedAuto();
  pros::lcd::register_btn0_cb(page_down);
  pros::lcd::register_btn2_cb(page_up);
  if (!imu_calibrate())
  {
    pros::lcd::set_text(7, "IMU failed to calibrate!");
  }

  chassis_motor_init({-11, -5, -7}, {3, 2, 17});
}


/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled()
{
  disable_all_tasks();
}


/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize()
{
  disable_all_tasks();
}


/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous()
{
  tare_gyro();
  reset_drive_sensor();
  set_drive_brake(MOTOR_BRAKE_HOLD);
  drive_pid.resume();

  autoSelector.CallSelectedAuto();
}


/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol()
{
  drive_pid.suspend();
  reset_drive_sensor();
  set_drive_brake(MOTOR_BRAKE_COAST); // This is preference to what you like to drive on

  while (true) {

    chassis_tank(); // Tank control
    // chassis_arcade_standard(split);
    // chassis_arcade_standard(single);
    // chassis_arcade_flipped(split);
    // chassis_arcade_flipped(single);

    pros::delay(DELAY_TIME);
  }
}
