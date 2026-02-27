/**
 * @file	ble.hpp
 * @brief	BLE service stub
 *
 * @note	Part 1 does not require BLE functionality. This is a scaffold stub to
 *		be used for Part 2.
 */

#pragma once

#include <cstdint>

/**
 * @brief	BLE service state
 */
enum class ble_link_state_t : std::uint8_t
{
	disabled = 0u,
	initialising,
	advertising,
	linked
};

/**
 * @brief	BLE service interface
 */
struct ble_service_t final
{
	/**
	 * @brief	Initialise BLE service (stub)
	 * @return	true on success, false otherwise
	 */
	bool initialise(void);

	/**
	 * @brief	Service BLE state machine (stub)
	 * @param	now_ms Current time
	 */
	void service(std::uint32_t now_ms);

	/**
	 * @brief	Get current link state
	 * @return	Current link state
	 */
	ble_link_state_t link_state(void) const;

private:
	ble_link_state_t link_state_{ble_link_state_t::disabled};
};