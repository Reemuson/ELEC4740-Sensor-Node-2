/**
 * @file	system_state.hpp
 * @brief	System status snapshot helpers
 */

#pragma once

#include <cstdint>

#include "error_codes.hpp"

/**
 * @brief	High-level system state derived from the current status snapshot
 */
enum class system_state_t : std::uint8_t
{
	ok = 0u,
	help_active,
	fault
};

/**
 * @brief	Current application status used by control and indicators
 */
struct system_status_t final
{
	error_code_t fault_code;
	bool help_active;
	bool sound_alert_active;
};

/**
 * @brief	Initialise the system status snapshot
 * @param	out_status Output status to initialise
 */
static inline void system_status_initialise(system_status_t *out_status)
{
	if (out_status == nullptr)
	{
		return;
	}

	out_status->fault_code = error_code_t::none;
	out_status->help_active = false;
	out_status->sound_alert_active = false;
}

/**
 * @brief	Test whether the system currently has a latched fault
 * @param	status Status snapshot
 * @return	true if a fault is present, false otherwise
 */
static inline bool system_status_has_fault(const system_status_t &status)
{
	return (status.fault_code != error_code_t::none);
}

/**
 * @brief	Derive the user-visible system state
 * @param	status Status snapshot
 * @return	Derived system state
 */
static inline system_state_t system_status_get_state(
    const system_status_t &status)
{
	if (system_status_has_fault(status))
	{
		return system_state_t::fault;
	}

	if (status.help_active)
	{
		return system_state_t::help_active;
	}

	return system_state_t::ok;
}
