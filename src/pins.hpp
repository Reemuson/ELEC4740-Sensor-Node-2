/**
 * @file	pins.hpp
 * @brief	Board pin mapping for SN2
 */

#pragma once

#include "Particle.h"

/**
 * @brief	SN2 board pin mapping
 */
struct board_pins_t final
{
	pin_t adc_pot_pin;
	pin_t adc_ntc_pin;
	pin_t pwm_fan_pin;
	pin_t gpio_button_pin;
	pin_t gpio_sound_pin;

	pin_t led_bicolour_red_pin;
	pin_t led_bicolour_green_pin;

	pin_t led_on_module_pin;
};

/**
 * @brief	Get the fixed SN2 pin mapping
 * @return	Const reference to pin map
 */
static inline const board_pins_t &board_pins_get(void)
{
	static const board_pins_t pins =
	    {
		.adc_pot_pin = A1,
		.adc_ntc_pin = A5,
		.pwm_fan_pin = A2,
		.gpio_button_pin = S4,
		.gpio_sound_pin = D2,
		.led_bicolour_red_pin = D4,
		.led_bicolour_green_pin = D3,
		.led_on_module_pin = D7};

	return pins;
}