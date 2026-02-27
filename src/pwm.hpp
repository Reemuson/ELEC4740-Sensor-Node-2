/**
 * @file	pwm.hpp
 * @brief	PWM control service
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	PWM service interface
 */
struct pwm_service_t final
{
	/**
	 * @brief	Initialise PWM output
	 * @param	pin PWM pin
	 * @return	true on success, false otherwise
	 */
	bool initialise(pin_t pin);

	/**
	 * @brief	Write PWM duty
	 * @param	pin PWM pin
	 * @param	duty Duty 0..255
	 * @param	frequency_hz PWM frequency
	 * @return	true on success, false otherwise
	 */
	bool write(pin_t pin, std::uint8_t duty, std::uint32_t frequency_hz);

	/**
	 * @brief	Map ADC counts to PWM duty
	 * @param	adc_count ADC raw count
	 * @return	Duty 0..255
	 */
	std::uint8_t adc_count_to_duty(std::uint16_t adc_count);
};