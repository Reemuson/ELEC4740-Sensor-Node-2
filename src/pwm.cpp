/**
 * @file	pwm.cpp
 * @brief	PWM control service implementation
 */

#include "pwm.hpp"

#include "config.hpp"

bool pwm_service_t::initialise(pin_t pin)
{
	if (pin == PIN_INVALID)
	{
		return false;
	}

	pinMode(pin, OUTPUT);
	analogWrite(pin, 0, app_config_t::pwm_frequency_hz);

	return true;
}

bool pwm_service_t::write(
    pin_t pin,
    std::uint8_t duty,
    std::uint32_t frequency_hz)
{
	if (pin == PIN_INVALID)
	{
		return false;
	}

	analogWrite(pin, static_cast<int>(duty), frequency_hz);

	return true;
}

std::uint8_t pwm_service_t::adc_count_to_duty(std::uint16_t adc_count)
{
	std::uint32_t duty = 0u;

	if (adc_count > app_config_t::adc_max_count)
	{
		adc_count = app_config_t::adc_max_count;
	}

	duty = (static_cast<std::uint32_t>(adc_count) *
		static_cast<std::uint32_t>(app_config_t::pwm_duty_max)) /
	       static_cast<std::uint32_t>(app_config_t::adc_max_count);

	if (duty > static_cast<std::uint32_t>(app_config_t::pwm_duty_max))
	{
		duty = static_cast<std::uint32_t>(app_config_t::pwm_duty_max);
	}

	return static_cast<std::uint8_t>(duty);
}