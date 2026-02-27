/**
 * @file	ntc_cal.hpp
 * @brief	Interactive 3-point NTC calibration over USB serial
 */

#pragma once

#include <cstdint>

#include "Particle.h"
#include "config.hpp"

/**
 * @brief	Ntc Steinhart–Hart coefficients
 *
 * @note	Model: 1/T(K) = A + B*ln(R) + C*ln(R)^3
 */
struct ntc_sh_coefficients_t final
{
	float a;
	float b;
	float c;
};

/**
 * @brief	Ntc calibration point
 */
struct ntc_cal_point_t final
{
	bool valid;
	std::int32_t reference_temperature_centi_c;
	std::uint16_t adc_count;
	std::uint32_t adc_mv;
	std::uint32_t ntc_resistance_ohms;
};

/**
 * @brief	Ntc calibration persistent data
 */
struct ntc_cal_storage_t final
{
	std::uint32_t magic;
	std::uint16_t version;
	std::uint16_t reserved;

	std::uint32_t series_resistor_ohms;
	std::uint32_t reference_mv;

	ntc_sh_coefficients_t coefficients;
};

/**
 * @brief	Ntc calibration service
 */
class ntc_calibration_service_t final
{
public:
	/**
	 * @brief	Initialise calibration service
	 * @param	adc_pin NTC ADC pin
	 * @return	true on success, false otherwise
	 */
	bool initialise(pin_t adc_pin);

	/**
	 * @brief	Service calibration CLI
	 * @param	now_ms Current time in ms
	 */
	void service(std::uint32_t now_ms);

	/**
	 * @brief	Test whether calibration mode is active
	 * @return	true if active, otherwise false
	 */
	bool is_active(void) const;

	/**
	 * @brief	Get latest solved coefficients
	 * @param	out_coeff Output coefficients
	 * @return	true if available, false otherwise
	 */
	bool get_coefficients(ntc_sh_coefficients_t *out_coeff) const;

	/**
	 * @brief	Set coefficients directly
	 * @param	coeff New coefficients
	 * @return	true if set, false otherwise
	 *
	 * @note		This marks coefficients as valid.
	 * @warning	This does not automatically write to EEPROM.
	 */
	bool set_coefficients(ntc_sh_coefficients_t coeff);

	/**
	 * @brief	Convert ADC millivolts to temperature using solved coefficients
	 * @param	adc_mv ADC node voltage in mV
	 * @param	out_temperature_centi_c Output temperature
	 * @return	true on success, false otherwise
	 */
	bool adc_mv_to_temperature_centi_c(
	    std::uint32_t adc_mv,
	    std::int32_t *out_temperature_centi_c) const;

private:
	enum class mode_t : std::uint8_t
	{
		inactive = 0u,
		active
	};

	enum class parse_result_t : std::uint8_t
	{
		ok = 0u,
		empty,
		too_long,
		error
	};

	static constexpr std::uint8_t max_line_length = 80u;

	/**
	 * @brief	Clear session state including points and coefficients
	 */
	void clear_session(void);

	/**
	 * @brief	Invalidate stored calibration in EEPROM
	 * @return	true on success, false otherwise
	 */
	bool clear_eeprom(void);

	bool load_from_eeprom(void);
	bool save_to_eeprom(void);

	void print_help(void) const;
	void print_status(void) const;

	parse_result_t read_line(char *out_line, std::uint8_t out_size);

	bool handle_command(const char *line);

	bool parse_u32_arg(
	    const char *line,
	    const char *key,
	    std::uint32_t *out_value) const;

	bool parse_point_args(
	    const char *line,
	    std::uint8_t *out_index,
	    std::int32_t *out_temp_centi) const;

	bool parse_point_index(
	    const char *line,
	    std::uint8_t *out_index) const;

	bool parse_coeff_args(
	    const char *line,
	    ntc_sh_coefficients_t *out_coeff) const;

	bool set_point(std::uint8_t index, std::int32_t temp_centi_c);
	bool capture_point(std::uint8_t index);

	bool solve_coefficients(void);

	bool adc_count_to_mv(
	    std::uint16_t adc_count,
	    std::uint32_t *out_mv) const;

	bool adc_mv_to_resistance_ohms(
	    std::uint32_t adc_mv,
	    std::uint32_t *out_resistance_ohms) const;

	bool analog_read_oversampled(
	    pin_t pin,
	    std::uint8_t samples,
	    std::uint16_t *out_adc_count) const;

	bool solve_3x3(float a[3][3], float b[3], float x[3]) const;

	void set_mode(mode_t mode);

	pin_t adc_pin_{PIN_INVALID};
	mode_t mode_{mode_t::inactive};

	std::uint32_t series_resistor_ohms_{app_config_t::ntc_series_resistor_ohms};
	std::uint32_t reference_mv_{app_config_t::adc_reference_mv};

	ntc_cal_point_t points_[3]{};

	bool coefficients_valid_{false};
	ntc_sh_coefficients_t coefficients_{};

	char line_buffer_[max_line_length + 1u]{};
	std::uint8_t line_length_{0u};
};