/**
 * @file	state.hpp
 * @brief	System state machine implementation
 */

#pragma once

#include <cstdint>

#include "error_codes.hpp"
#include "system_state.hpp"

/**
 * @brief	System context
 */
struct system_context_t final
{
	system_state_t state;
	error_code_t error_code;
	bool help_active;
	bool sound_alert_active;
};

/**
 * @brief	System state machine
 */
struct state_machine_t final
{
	/**
	 * @brief	Initialise the system context
	 * @param	now_ms Current time
	 * @param	out_ctx Output context
	 * @return	true on success, false otherwise
	 */
	bool initialise(std::uint32_t now_ms, system_context_t *out_ctx);

	/**
	 * @brief	Process events and update context
	 * @param	now_ms Current time
	 * @param	event Event to apply
	 * @param	error_code Error code for fault events
	 * @param	out_ctx Context to update
	 * @return	true on success, false otherwise
	 */
	bool dispatch(std::uint32_t now_ms,
		      app_event_t event,
		      error_code_t error_code,
		      system_context_t *out_ctx);

private:
	std::uint32_t boot_start_ms_{0u};
};