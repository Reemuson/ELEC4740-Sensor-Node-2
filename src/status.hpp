/**
 * @file	status.hpp
 * @brief	Photon 2 on-module RGB status LED control
 */

#pragma once

#include <cstdint>

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
 * @brief	RGB status LED service
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
	rgb_mode_t last_mode_{rgb_mode_t::system_default};
};