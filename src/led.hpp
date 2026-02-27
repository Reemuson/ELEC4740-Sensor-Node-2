/**
 * @file	led.hpp
 * @brief	LED control service for bicolour LED and module LED
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
 * @brief	LED mode
 */
enum class led_mode_t : std::uint8_t
{
	off = 0u,
	on,
	flash_slow,
	flash_fast
};

/**
 * @brief	Bicolour LED command
 */
struct led_command_t final
{
	led_mode_t red;
	led_mode_t green;
};

/**
 * @brief	LED pin configuration
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
 * @brief	LED service
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
	 * @brief	Service LED timing and update outputs
	 * @param	now_ms Current time
	 * @param	command LED command
	 */
	void service(std::uint32_t now_ms, const led_command_t &command);

private:
	bool write_led(pin_t pin, led_polarity_t polarity, bool on);
	bool compute_flash(std::uint32_t now_ms, led_mode_t mode);

	led_pins_t pins_{};
};