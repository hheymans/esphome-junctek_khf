#include "junctek_khf.h"

#include "esphome/core/log.h"
#include "esphome/core/optional.h"


#include <string>
#include <string.h>

#include <setjmp.h>

static jmp_buf parsing_failed;
static const char *const TAG = "JunkTek KH-F";

esphome::optional<int> try_getval(const char*& cursor)
{
  long val;
  const char* pos = cursor;
  char* end = nullptr;
  val = strtoll(pos, &end, 10);
  if (end == pos || end == nullptr)
  {
    return nullopt;
  }
  if (*end != ',' && *end != '.')
  {
    ESP_LOGE("JunkTekKHF", "Error no coma %s", cursor);
    return nullopt;
  }
  cursor = end + 1; // Skip coma
  return val;
}

// Get a value where it's expected to be "<number>[,.], incrementing the cursor past the end"
int getval(const char*& cursor)
{
  auto val = try_getval(cursor);
  if (!val)
  {
    longjmp(parsing_failed, 1);
  }
  return *val;
}
  

JuncTekKHF::JuncTekKHF(unsigned address, bool invert_current)
  : address_(address)
  , invert_current_(invert_current)
{
}

void JuncTekKHF::dump_config()
{
  ESP_LOGCONFIG(TAG, "junctek_khf:");
  ESP_LOGCONFIG(TAG, "  address: %d", this->address_);
  ESP_LOGCONFIG(TAG, "  invert_current: %s", this->invert_current_ ? "True" : "False");
}

void JuncTekKHF::handle_settings(const char* buffer)
{
  ESP_LOGD("JunkTekKHF", "Settings %s", buffer);
  const char* cursor = buffer;
  const int address = getval(cursor);
  if (address != this->address_)
    return;
  const int checksum = getval(cursor);
  if (! verify_checksum(checksum, cursor))
    return;

  const float overVoltage = getval(cursor) / 100.0;
  const float underVoltage = getval(cursor) / 100.0;
  const float positiveOverCurrent = getval(cursor) / 1000.0;
  const float negativeOverCurrent = getval(cursor) / 100.00;
  const float overPowerProtection = getval(cursor) / 100.00;
  const float overTemperature = getval(cursor) - 100.0;
  const int protectionRecoverySeconds = getval(cursor);
  const int delayTime = getval(cursor);
  const float batteryAmpHourCapacity = getval(cursor) / 10.0;
  const int voltageCalibration = getval(cursor);
  const int currentCalibration = getval(cursor);
  const float temperatureCalibration = getval(cursor) - 100.0;
  const int reserved = getval(cursor);
  const int relayNormallyOpen = getval(cursor);
  const int currentratio = getval(cursor);
  // NOTE these are in the docs, but I don't seem to get them
  //const int voltageCurveScale = getval(cursor);
  //const int currentCurveScale = getval(cursor);

  // Save the capacity for calculating the %
  this->battery_capacity_ = batteryAmpHourCapacity;
  

 if (over_voltage_set_sensor_)
    this->over_voltage_set_sensor_->publish_state(overVoltage); 
 if (under_voltage_set_sensor_)
    this->under_voltage_set_sensor_->publish_state(underVoltage); 
 if (positive_overcurrent_set_sensor_)
    this->positive_overcurrent_set_sensor_->publish_state(positiveOverCurrent); 
 if (negative_overcurrent_set_sensor_)
    this->negative_overcurrent_set_sensor_->publish_state(negativeOverCurrent); 
 if (over_power_protection_set_sensor_)
    this->over_power_protection_set_sensor_->publish_state(overPowerProtection); 
 if (over_temperature_set_sensor_)
    this->over_temperature_set_sensor_->publish_state(overTemperature); 
 if (protection_recovery_seconds_set_sensor_)
    this->protection_recovery_seconds_set_sensor_->publish_state(protectionRecoverySeconds); 
 if (delay_time_set_sensor_)
    this->delay_time_set_sensor_->publish_state(delayTime); 
 if (battery_amphour_capacity_set_sensor_)
    this->battery_amphour_capacity_set_sensor_->publish_state(batteryAmpHourCapacity); 
 if (voltage_calibration_set_sensor_)
    this->voltage_calibration_set_sensor_->publish_state(voltageCalibration); 
 if (current_calibration_set_sensor_)
    this->current_calibration_set_sensor_->publish_state(currentCalibration); 
 if (temperature_calibration_set_sensor_)
    this->temperature_calibration_set_sensor_->publish_state(temperatureCalibration); 
 if (reserved_set_sensor_)
    this->reserved_set_sensor_->publish_state(reserved); 
 if (relay_normally_open_sensor_)
    this->relay_normally_open_sensor_->publish_state(relayNormallyOpen); 
 if (current_ratio_set_sensor_)
    this->current_ratio_set_sensor_->publish_state(currentratio); 


  this->last_settings_ = esphome::millis();
}

void JuncTekKHF::handle_status(const char* buffer)
{
  ESP_LOGD("JunkTekKHF", "Status %s", buffer);
  const char* cursor = buffer;
  const int address = getval(cursor);
  if (address != this->address_)
    return;
 
  const int checksum = getval(cursor);
  if (! verify_checksum(checksum, cursor))
    return;

  const float voltage = getval(cursor) / 100.0;
  const float amps = getval(cursor) / 100.0;
  const float ampHourRemaining = getval(cursor) / 1000.0;
  const float wattHourDischarge = getval(cursor) / 100.00;
  const float wattHourRemaining = getval(cursor) / 100.0;
  const float runtimeSeconds = getval(cursor);
  const float temperature = getval(cursor) - 100.0;
  const float powerInWatts = getval(cursor) / 100.0;
  const int relayStatus = getval(cursor);
  const int direction = getval(cursor);
  const int batteryLifeMinutes = getval(cursor);
  const float timeAdj = getval(cursor);
  ESP_LOGV("JunkTekKHF", "Recv %f %f %d %f %f %f %f", voltage, ampHourRemaining, direction, powerInWatts, amps, temperature, wattHourRemaining);
  if (ah_battery_level_sensor_)
    this->ah_battery_level_sensor_->publish_state(ampHourRemaining); 
 if (wh_battery_discharge_sensor_)
    this->wh_battery_discharge_sensor_->publish_state(wattHourDischarge); 
 if (wh_battery_level_sensor_)
    this->wh_battery_level_sensor_->publish_state(wattHourRemaining); 
 if (running_time_sensor_)
    this->running_time_sensor_->publish_state(runtimeSeconds); 
 if (time_adj_sensor_)
    this->time_adj_sensor_->publish_state(timeAdj); 
 if (battery_life_sensor_)
    this->battery_life_sensor_->publish_state(batteryLifeMinutes); 
 if (relay_status_sensor_)
    this->relay_status_sensor_->publish_state(relayStatus); 
 if (direction_sensor_)
    this->direction_sensor_->publish_state(direction); 
 
 if (voltage_sensor_)
    this->voltage_sensor_->publish_state(voltage);
 if (battery_level_sensor_ && this->battery_capacity_)
    this->battery_level_sensor_->publish_state(ampHourRemaining * 100.0 / *this->battery_capacity_);
  if (current_sensor_)
  {
    float adjustedCurrent = direction == 0 ? amps : -amps;
    if (invert_current_)
      adjustedCurrent *= -1;
    current_sensor_->publish_state(adjustedCurrent);
  }
  if (power_sensor_)
    this->power_sensor_->publish_state(voltage * amps);
  if (temperature_)
    this->temperature_->publish_state(temperature);

  this->last_stats_ = esphome::millis();
}

void JuncTekKHF::handle_line()
{
  //A failure in parsing will return back to here with a non-zero value
  if (setjmp(parsing_failed))
    return;
  
  const char* buffer = &this->line_buffer_[0];
  if (buffer[0] != ':' || buffer[1] != 'r')
    return;
  if (strncmp(&buffer[2], "50=", 3) == 0)
    handle_status(&buffer[5]);
  else if (strncmp(&buffer[2], "51=", 3) == 0)
    handle_settings(&buffer[5]);

  return;
}

bool JuncTekKHF::readline()
{
  while (available()) {
    const char readch = read();
    if (readch > 0) {
      switch (readch) {
        case '\n': // Ignore new-lines
          break;
        case '\r': // Return on CR
          this->line_pos_ = 0;  // Reset position index ready for next time
          return true;
        default:
          if (this->line_pos_ < MAX_LINE_LEN - 1)
          {
            this->line_buffer_[this->line_pos_++] = readch;
            this->line_buffer_[this->line_pos_] = 0;
          }
      }
    }
  }
  return false;
}

bool JuncTekKHF::verify_checksum(int checksum, const char* buffer)
{
  long total = 0;
  while (auto val = try_getval(buffer))
  {
    total += *val;
  }
  const bool checksum_valid = (total % 255) + 1 == checksum;
  ESP_LOGD("JunkTekKHF", "Recv checksum %d total %ld valid %d", checksum, total, checksum_valid);
  return checksum_valid;
}

void JuncTekKHF::loop()
{
  const unsigned long start_time = esphome::millis();

  if (!this->last_settings_ || (*this->last_settings_ + (30 * 1000)) < start_time)
  {
    this->last_settings_ = start_time;
    char buffer[20];
    sprintf(buffer, ":R51=%d,2,1,\r\n", this->address_);
    write_str(buffer);
  }

  if (!this->last_stats_ || (*this->last_stats_ + (10 * 1000)) < start_time)
  {
    this->last_stats_ = start_time;
    char buffer[20];
    sprintf(buffer, ":R50=%d,2,1,\r\n", this->address_);
    write_str(buffer);
  }

  if (readline())
  {
    handle_line();
  }
}

float JuncTekKHF::get_setup_priority() const
{
  return setup_priority::DATA;
}
