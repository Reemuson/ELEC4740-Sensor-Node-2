/**
 * @file	sound.cpp
 * @brief	Sound event service implementation
 */

#include "sound.hpp"

#include "timebase.hpp"

volatile std::uint32_t sound_service_t::g_pulse_count = 0u;
volatile std::uint32_t sound_service_t::g_last_pulse_us = 0u;

static const std::uint32_t g_sound_min_pulse_spacing_us = 2000u;

void sound_service_t::isr_thunk(void)
{
	const std::uint32_t now_us = timebase_now_us();
	const std::uint32_t delta_us = now_us - g_last_pulse_us;

	if (delta_us >= g_sound_min_pulse_spacing_us)
	{
		g_last_pulse_us = now_us;
		g_pulse_count++;
	}
}

bool sound_service_t::initialise(pin_t pin)
{
	if (pin == PIN_INVALID)
	{
		return false;
	}

	pin_ = pin;
	pinMode(pin_, INPUT);

	g_pulse_count = 0u;
	g_last_pulse_us = 0u;
	last_event_ms_ = 0u;

	attachInterrupt(pin_, isr_thunk, FALLING);

	return true;
}

bool sound_service_t::service(
    std::uint32_t now_ms,
    std::uint32_t alert_hold_ms,
    sound_event_t *out_event)
{
	static std::uint32_t last_seen_count = 0u;
	std::uint32_t count = 0u;

	if (out_event == nullptr)
	{
		return false;
	}

	count = g_pulse_count;

	out_event->pulse_count = count;
	out_event->triggered = false;

	if (count != last_seen_count)
	{
		last_seen_count = count;
		last_event_ms_ = now_ms;
		out_event->triggered = true;
	}

	(void)alert_hold_ms;

	return true;
}

bool sound_service_t::alert_is_active(
    std::uint32_t now_ms,
    std::uint32_t alert_hold_ms) const
{
	if ((now_ms - last_event_ms_) < alert_hold_ms)
	{
		return true;
	}

	return false;
}