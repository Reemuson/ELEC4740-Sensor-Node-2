/**
 * @file	adc_service.hpp
 * @brief	ADC sampling service
 */

#pragma once

#include <cstdint>

#include "Particle.h"

/**
 * @brief	ADC sample container
 */
struct adc_sample_t final
{
	std::uint16_t pot_adc;
	std::uint16_t ntc_adc;

	std::uint32_t pot_mv;
	std::uint32_t ntc_mv;
};

/**
 * @brief	ADC service interface
 */
struct adc_service_t final
{
	/**
	 * @brief	Initialise ADC service
	 * @return	true on success, false otherwise
	 */
	bool initialise(void);

	/**
	 * @brief	Sample ADC channels
	 * @param	pot_pin Potentiometer ADC pin
	 * @param	ntc_pin NTC ADC pin
	 * @param	out_sample Output sample
	 * @return	true on success, false otherwise
	 */
	bool sample(pin_t pot_pin,
		    pin_t ntc_pin,
		    adc_sample_t *out_sample);
};