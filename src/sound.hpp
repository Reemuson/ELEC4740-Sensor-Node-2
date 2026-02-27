/**
 * @file	sound.hpp
 * @brief	Sound event service with deglitch spacing
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	Sound service output
 */
struct sound_event_t final
{
	bool triggered;
	std::uint32_t pulse_count;
};

/**
 * @brief	Sound service
 */
struct sound_service_t final
{
	/**
	 * @brief	Initialise sound input and ISR
	 * @param	pin Sound input pin (active low)
	 * @return	true on success, false otherwise
	 */
	bool initialise(pin_t pin);

	/**
	 * @brief	Service sound alert hold timer and events
	 * @param	now_ms Current time
	 * @param	alert_hold_ms Hold time after a trigger
	 * @param	out_event Output event
	 * @return	true on success, false otherwise
	 */
	bool service(std::uint32_t now_ms,
		     std::uint32_t alert_hold_ms,
		     sound_event_t *out_event);

	/**
	 * @brief	Test whether sound alert is currently active
	 * @param	now_ms Current time
	 * @param	alert_hold_ms Hold time
	 * @return	true if active, otherwise false
	 */
	bool alert_is_active(std::uint32_t now_ms,
			     std::uint32_t alert_hold_ms) const;

private:
	static void isr_thunk(void);

	static volatile std::uint32_t g_pulse_count;
	static volatile std::uint32_t g_last_pulse_us;

	pin_t pin_{PIN_INVALID};
	std::uint32_t last_event_ms_{0u};
};