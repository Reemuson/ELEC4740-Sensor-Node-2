/**
 * @file	button.cpp
 * @brief	Button service implementation
 */

#include "button.hpp"

volatile bool button_service_t::g_irq_flag = false;

void button_service_t::isr_thunk(void)
{
	g_irq_flag = true;
}

bool button_service_t::initialise(pin_t pin)
{
	if (pin == PIN_INVALID)
	{
		return false;
	}

	pin_ = pin;
	pinMode(pin_, INPUT_PULLUP);

	g_irq_flag = false;
	attachInterrupt(pin_, isr_thunk, FALLING);

	help_latched_ = false;
	debounce_active_ = false;
	debounce_start_ms_ = 0u;

	return true;
}

bool button_service_t::service(
    std::uint32_t now_ms,
    std::uint32_t debounce_ms,
    button_event_t *out_event)
{
	bool pressed = false;

	if (out_event == nullptr)
	{
		return false;
	}

	out_event->toggled = false;

	if (g_irq_flag)
	{
		g_irq_flag = false;
		debounce_active_ = true;
		debounce_start_ms_ = now_ms;
	}

	if (!debounce_active_)
	{
		return true;
	}

	if ((now_ms - debounce_start_ms_) < debounce_ms)
	{
		return true;
	}

	debounce_active_ = false;

	pressed = (digitalRead(pin_) == LOW);
	if (!pressed)
	{
		return true;
	}

	help_latched_ = !help_latched_;
	out_event->toggled = true;

	return true;
}

bool button_service_t::help_is_active(void) const
{
	return help_latched_;
}