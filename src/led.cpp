/**
 * @file	led.cpp
 * @brief	LED control service implementation
 */

#include "led.hpp"

#include "config.hpp"
#include "gpio_helpers.hpp"

static const std::uint32_t g_flash_slow_ms = 500u;
static const std::uint32_t g_flash_fast_ms = 125u;

bool led_service_t::initialise(const led_pins_t &pins)
{
	if ((pins.red_pin == PIN_INVALID) || (pins.green_pin == PIN_INVALID))
	{
		return false;
	}

	pins_ = pins;

	pinMode(pins_.red_pin, OUTPUT);
	pinMode(pins_.green_pin, OUTPUT);

	if (pins_.module_pin != PIN_INVALID)
	{
		pinMode(pins_.module_pin, OUTPUT);
	}

	(void)write_led(pins_.red_pin, pins_.red_polarity, false);
	(void)write_led(pins_.green_pin, pins_.green_polarity, false);

	if (pins_.module_pin != PIN_INVALID)
	{
		(void)write_led(pins_.module_pin, pins_.module_polarity, false);
	}

	return true;
}

bool led_service_t::write_led(pin_t pin, led_polarity_t polarity, bool on)
{
	bool level = on;

	if (polarity == led_polarity_t::active_low)
	{
		level = !level;
	}

	return gpio_write(pin, level);
}

bool led_service_t::compute_flash(std::uint32_t now_ms, led_mode_t mode)
{
	std::uint32_t period = 0u;

	if (mode == led_mode_t::flash_slow)
	{
		period = g_flash_slow_ms;
	}
	else if (mode == led_mode_t::flash_fast)
	{
		period = g_flash_fast_ms;
	}
	else
	{
		return false;
	}

	return ((now_ms / period) % 2u) == 0u;
}

void led_service_t::service(std::uint32_t now_ms, const led_command_t &command)
{
	bool red_on = false;
	bool green_on = false;
	bool hb_on = false;

	if (command.red == led_mode_t::on)
	{
		red_on = true;
	}
	else if ((command.red == led_mode_t::flash_slow) ||
		 (command.red == led_mode_t::flash_fast))
	{
		red_on = compute_flash(now_ms, command.red);
	}

	if (command.green == led_mode_t::on)
	{
		green_on = true;
	}
	else if ((command.green == led_mode_t::flash_slow) ||
		 (command.green == led_mode_t::flash_fast))
	{
		green_on = compute_flash(now_ms, command.green);
	}

	hb_on = ((now_ms / app_config_t::heartbeat_period_ms) % 2u) == 0u;

	(void)write_led(pins_.red_pin, pins_.red_polarity, red_on);
	(void)write_led(pins_.green_pin, pins_.green_polarity, green_on);

	if (pins_.module_pin != PIN_INVALID)
	{
		(void)write_led(pins_.module_pin, pins_.module_polarity, hb_on);
	}
}