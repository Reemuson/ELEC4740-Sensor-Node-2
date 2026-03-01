/**
 * @file	led.hpp
 * @brief	LED control service interface
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	LED drive polarity
 */
enum class led_polarity_t : std::uint8_t
{
	active_high = 0u,
	active_low
};

/**
 * @brief	LED modes
 */
enum class led_mode_t : std::uint8_t
{
	off = 0u,
	on,
	flash_slow,
	flash_fast
};

/**
 * @brief	LED pin set
 */
struct led_pins_t final
{
	pin_t red_pin;
	pin_t green_pin;
	pin_t module_pin;
	led_polarity_t red_polarity;
	led_polarity_t green_polarity;
	led_polarity_t module_polarity;
};

/**
 * @brief	LED command
 */
struct led_command_t final
{
	led_mode_t red;
	led_mode_t green;
};

/**
 * @brief	LED control service
 */
struct led_service_t final
{
	/**
	 * @brief	Initialise LED pins
	 * @param	pins LED pin configuration
	 * @return	true on success, false otherwise
	 */
	bool initialise(const led_pins_t &pins);

	/**
	 * @brief	Service LED outputs
	 * @param	now_ms Current time
	 * @param	command LED command
	 */
	void service(std::uint32_t now_ms, const led_command_t &command);

	/**
	 * @brief	Set steady bicolour indication
	 * @param	red true to enable red channel
	 * @param	green true to enable green channel
	 */
	void set_bicolour(bool red, bool green);

private:
	bool write_led(pin_t pin, led_polarity_t polarity, bool on);
	bool compute_flash(std::uint32_t now_ms, led_mode_t mode);

	led_pins_t pins_{};
	bool bicolour_red_{false};
	bool bicolour_green_{false};
};