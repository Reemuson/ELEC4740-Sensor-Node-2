/**
 * @file	adc_service.cpp
 * @brief	ADC sampling service implementation
 */

#include "adc.hpp"

#include "config.hpp"

/**
 * @brief	Clamp ADC count
 * @param	adc_count Input count
 * @return	Clamped count
 */
static std::uint16_t adc_clamp_count(std::uint16_t adc_count)
{
	if (adc_count > app_config_t::adc_max_count)
	{
		return app_config_t::adc_max_count;
	}

	return adc_count;
}

/**
 * @brief	Convert ADC counts to millivolts
 * @param	adc_count ADC raw count
 * @return	Millivolts
 */
static std::uint32_t adc_count_to_mv(std::uint16_t adc_count)
{
	const std::uint32_t count =
	    static_cast<std::uint32_t>(adc_clamp_count(adc_count));

	return (count * app_config_t::adc_reference_mv) /
	       static_cast<std::uint32_t>(app_config_t::adc_max_count);
}

/**
 * @brief	Read ADC with oversampling and channel-settle discard
 * @param	pin ADC pin
 * @param	samples Samples to average
 * @param	out_adc_count Output averaged count
 * @return	true on success, false otherwise
 */
static bool adc_read_oversampled(
    pin_t pin,
    std::uint8_t samples,
    std::uint16_t *out_adc_count)
{
	std::uint32_t sum = 0u;
	std::uint8_t index = 0u;
	int reading = 0;

	if ((pin == PIN_INVALID) || (out_adc_count == nullptr))
	{
		return false;
	}

	if (samples == 0u)
	{
		return false;
	}

	if (samples > 32u)
	{
		samples = 32u;
	}

	reading = analogRead(pin);
	(void)reading;

	for (index = 0u; index < samples; index++)
	{
		reading = analogRead(pin);

		if (reading < 0)
		{
			return false;
		}

		if (reading > static_cast<int>(app_config_t::adc_max_count))
		{
			reading = static_cast<int>(app_config_t::adc_max_count);
		}

		sum += static_cast<std::uint32_t>(reading);
	}

	*out_adc_count = static_cast<std::uint16_t>(sum / samples);

	return true;
}

bool adc_service_t::initialise(void)
{
	return true;
}

bool adc_service_t::sample(
    pin_t pot_pin,
    pin_t ntc_pin,
    adc_sample_t *out_sample)
{
	std::uint16_t pot_adc = 0u;
	std::uint16_t ntc_adc = 0u;

	if (out_sample == nullptr)
	{
		return false;
	}

	if (!adc_read_oversampled(pot_pin, 8u, &pot_adc))
	{
		return false;
	}

	if (!adc_read_oversampled(ntc_pin, 10u, &ntc_adc))
	{
		return false;
	}

	out_sample->pot_adc = pot_adc;
	out_sample->ntc_adc = ntc_adc;
	out_sample->pot_mv = adc_count_to_mv(pot_adc);
	out_sample->ntc_mv = adc_count_to_mv(ntc_adc);

	return true;
}