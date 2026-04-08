/**
 * @file	status.hpp
 * @brief	Top-level status indicator services
 */

#pragma once

#include <cstdint>

#include "ble.hpp"
#include "led.hpp"

/**
 * @brief	RGB LED mode
 */
enum class rgb_mode_t : std::uint8_t
{
	system_default = 0u,
	off,
	flash_green,
	solid_green
};

/**
 * @brief	Low-level Photon 2 RGB status LED service
 */
struct rgb_status_service_t final
{
	/**
	 * @brief	Initialise RGB LED service
	 * @return	true on success, false otherwise
	 */
	bool initialise(void);

	/**
	 * @brief	Service RGB LED output
	 * @param	now_ms Current time in ms
	 * @param	mode Desired mode
	 */
	void service(std::uint32_t now_ms, rgb_mode_t mode);

private:
	bool controlled_{false};
};

/**
 * @brief	High-level indicator inputs
 */
struct status_view_t final
{
	bool calibration_active;
	bool fault_active;
	bool help_active;
	bool sound_alert_active;
	ble_link_state_t ble_link_state;
};

/**
 * @brief	Top-level status indicator service
 *
 * @note	Owns both the external bicolour LED and the Photon 2 RGB LED
 *		so application code only provides a simple state snapshot.
 */
struct status_service_t final
{
	/**
	 * @brief	Initialise indicator hardware
	 * @param	led_pins Bicolour LED pin configuration
	 * @return	true on success, false otherwise
	 */
	bool initialise(const led_pins_t &led_pins);

	/**
	 * @brief	Service indicator outputs from the current status snapshot
	 * @param	now_ms Current time in ms
	 * @param	view Current indicator inputs
	 */
	void service(std::uint32_t now_ms, const status_view_t &view);

	/**
	 * @brief	Show the boot-time debug-mode indication
	 * @param	now_ms Current time in ms
	 */
	void indicate_debug_mode(std::uint32_t now_ms);

private:
	led_service_t bicolour_led_{};
	rgb_status_service_t rgb_led_{};
};
