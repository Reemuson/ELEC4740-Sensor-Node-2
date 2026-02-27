/**
 * @file	ntc_cal.cpp
 * @brief	Interactive 3-point NTC calibration over USB serial
 */

#include "ntc_cal.hpp"
#include "config.hpp"
#include "runtime.hpp"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

bool ntc_calibration_service_t::initialise(pin_t adc_pin)
{
	std::uint8_t i = 0u;

	if (adc_pin == PIN_INVALID)
	{
		return false;
	}

	adc_pin_ = adc_pin;

	for (i = 0u; i < 3u; i++)
	{
		points_[i].valid = false;
		points_[i].reference_temperature_centi_c = 0;
		points_[i].adc_count = 0u;
		points_[i].adc_mv = 0u;
		points_[i].ntc_resistance_ohms = 0u;
	}

	line_length_ = 0u;
	line_buffer_[0] = '\0';

	coefficients_valid_ = false;
	coefficients_.a = 0.0f;
	coefficients_.b = 0.0f;
	coefficients_.c = 0.0f;

	set_mode(mode_t::inactive);

	(void)load_from_eeprom();

	return true;
}

void ntc_calibration_service_t::service(std::uint32_t now_ms)
{
	char line[max_line_length + 1u]{};
	parse_result_t r = parse_result_t::empty;

	(void)now_ms;

	r = read_line(line, static_cast<std::uint8_t>(sizeof(line)));
	if (r == parse_result_t::empty)
	{
		return;
	}

	if (r == parse_result_t::too_long)
	{
		Serial.println("cal: line too long");
		return;
	}

	if (r == parse_result_t::error)
	{
		Serial.println("cal: read error");
		return;
	}

	(void)handle_command(line);
}

bool ntc_calibration_service_t::is_active(void) const
{
	return (mode_ == mode_t::active);
}

bool ntc_calibration_service_t::get_coefficients(
    ntc_sh_coefficients_t *out_coeff) const
{
	if (out_coeff == nullptr)
	{
		return false;
	}

	if (!coefficients_valid_)
	{
		return false;
	}

	*out_coeff = coefficients_;
	return true;
}

bool ntc_calibration_service_t::set_coefficients(ntc_sh_coefficients_t coeff)
{
	const float max_abs = 1.0e3f;

	if (!std::isfinite(coeff.a) ||
	    !std::isfinite(coeff.b) ||
	    !std::isfinite(coeff.c))
	{
		return false;
	}

	if ((fabsf(coeff.a) > max_abs) ||
	    (fabsf(coeff.b) > max_abs) ||
	    (fabsf(coeff.c) > max_abs))
	{
		return false;
	}

	coefficients_ = coeff;
	coefficients_valid_ = true;

	return true;
}

void ntc_calibration_service_t::clear_session(void)
{
	std::uint8_t i = 0u;

	for (i = 0u; i < 3u; i++)
	{
		points_[i].valid = false;
		points_[i].reference_temperature_centi_c = 0;
		points_[i].adc_count = 0u;
		points_[i].adc_mv = 0u;
		points_[i].ntc_resistance_ohms = 0u;
	}

	coefficients_valid_ = false;
	coefficients_.a = 0.0f;
	coefficients_.b = 0.0f;
	coefficients_.c = 0.0f;
}

bool ntc_calibration_service_t::clear_eeprom(void)
{
	ntc_cal_storage_t s{};

	s.magic = 0u;
	s.version = 0u;
	s.reserved = 0u;
	s.series_resistor_ohms = 0u;
	s.reference_mv = 0u;
	s.coefficients.a = 0.0f;
	s.coefficients.b = 0.0f;
	s.coefficients.c = 0.0f;

	EEPROM.put(app_config_t::ntc_eeprom_address, s);

	return true;
}

void ntc_calibration_service_t::set_mode(mode_t mode)
{
	mode_ = mode;
}

ntc_calibration_service_t::parse_result_t
ntc_calibration_service_t::read_line(char *out_line, std::uint8_t out_size)
{
	int c = 0;

	if ((out_line == nullptr) || (out_size < 2u))
	{
		return parse_result_t::error;
	}

	while (Serial.available() > 0)
	{
		c = Serial.read();
		if (c < 0)
		{
			return parse_result_t::error;
		}

		if ((c == '\r') || (c == '\n'))
		{
			if (line_length_ == 0u)
			{
				continue;
			}

			line_buffer_[line_length_] = '\0';
			std::strncpy(out_line, line_buffer_,
				     static_cast<std::size_t>(out_size - 1u));
			out_line[out_size - 1u] = '\0';

			line_length_ = 0u;
			line_buffer_[0] = '\0';

			return parse_result_t::ok;
		}

		if (line_length_ >= max_line_length)
		{
			while (Serial.available() > 0)
			{
				c = Serial.read();
				if ((c == '\r') || (c == '\n'))
				{
					break;
				}
			}

			line_length_ = 0u;
			line_buffer_[0] = '\0';
			return parse_result_t::too_long;
		}

		line_buffer_[line_length_] = static_cast<char>(c);
		line_length_++;
	}

	return parse_result_t::empty;
}

bool ntc_calibration_service_t::parse_coeff_args(
    const char *line,
    ntc_sh_coefficients_t *out_coeff) const
{
	double a = 0.0;
	double b = 0.0;
	double c = 0.0;

	if ((line == nullptr) || (out_coeff == nullptr))
	{
		return false;
	}

	if (std::sscanf(line, "cal coeff set %lf %lf %lf", &a, &b, &c) != 3)
	{
		return false;
	}

	out_coeff->a = static_cast<float>(a);
	out_coeff->b = static_cast<float>(b);
	out_coeff->c = static_cast<float>(c);

	return true;
}

void ntc_calibration_service_t::print_help(void) const
{
	Serial.println("cal help:");
	Serial.println("  cal help");
	Serial.println("  cal show");
	Serial.println("  cal start");
	Serial.println("  cal abort");
	Serial.println("  cal clear");
	Serial.println("  cal eeprom clear");
	Serial.println("  cal set r_series_ohms <ohms>");
	Serial.println("  cal set ref_mv <mV>");
	Serial.println("  cal point set <1..3> <temp_centi_c>");
	Serial.println("  cal point capture <1..3>");
	Serial.println("  cal solve");
	Serial.println("  cal save");
	Serial.println("  cal coeff set <A> <B> <C>");
	Serial.println("");
	Serial.println("notes:");
	Serial.println("  temp_centi_c: eg 2500 = 25.00C");
	Serial.println("  commands must be terminated by CR/LF");
}

void ntc_calibration_service_t::print_status(void) const
{
	std::uint8_t i = 0u;

	Serial.printf("cal: mode=%s\r\n",
		      (mode_ == mode_t::active) ? "active" : "inactive");

	Serial.printf("cal: r_series_ohms=%lu ref_mv=%lu\r\n",
		      static_cast<unsigned long>(series_resistor_ohms_),
		      static_cast<unsigned long>(reference_mv_));

	for (i = 0u; i < 3u; i++)
	{
		const ntc_cal_point_t &p = points_[i];

		Serial.printf(
		    "cal: p%u valid=%u t_ref=%ld adc=%u mv=%lu r=%lu\r\n",
		    static_cast<unsigned>(i + 1u),
		    static_cast<unsigned>(p.valid ? 1u : 0u),
		    static_cast<long>(p.reference_temperature_centi_c),
		    static_cast<unsigned>(p.adc_count),
		    static_cast<unsigned long>(p.adc_mv),
		    static_cast<unsigned long>(p.ntc_resistance_ohms));
	}

	Serial.printf("cal: coeff_valid=%u A=%.9f B=%.9f C=%.9f\r\n",
		      static_cast<unsigned>(coefficients_valid_ ? 1u : 0u),
		      static_cast<double>(coefficients_.a),
		      static_cast<double>(coefficients_.b),
		      static_cast<double>(coefficients_.c));
}

bool ntc_calibration_service_t::handle_command(const char *line)
{
	std::uint32_t u32 = 0u;
	std::int32_t i32 = 0;
	std::uint8_t index = 0u;
	ntc_sh_coefficients_t coeff{};

	if (line == nullptr)
	{
		return false;
	}

	if (std::strncmp(line, "cal coeff set ", 14) == 0)
	{
		if (!parse_coeff_args(line, &coeff))
		{
			Serial.println("cal: coeff set: bad args");
			Serial.println("cal: usage: cal coeff set <A> <B> <C>");
			return true;
		}

		if (!set_coefficients(coeff))
		{
			Serial.println("cal: coeff set: rejected");
			return true;
		}

		Serial.println("cal: coefficients set");
		return true;
	}

	if (std::strncmp(line, "cal help", 8) == 0)
	{
		print_help();
		return true;
	}

	if (std::strncmp(line, "cal show", 8) == 0)
	{
		print_status();
		return true;
	}

	if (std::strncmp(line, "cal start", 9) == 0)
	{
		clear_session();
		set_mode(mode_t::active);
		runtime_set_calibration_active(true);
		Serial.println("cal: active (debug output paused)");
		return true;
	}

	if (std::strncmp(line, "cal abort", 9) == 0)
	{
		set_mode(mode_t::inactive);
		runtime_set_calibration_active(false);
		Serial.println("cal: inactive (debug output resumes)");
		return true;
	}

	if (std::strncmp(line, "cal clear", 9) == 0)
	{
		clear_session();
		Serial.println("cal: cleared session (points, coefficients)");
		return true;
	}

	if (std::strncmp(line, "cal eeprom clear", 16) == 0)
	{
		(void)clear_eeprom();
		Serial.println("cal: eeprom cleared");
		return true;
	}

	if (std::strncmp(line, "cal set ", 8) == 0)
	{
		if (parse_u32_arg(line, "r_series_ohms", &u32))
		{
			series_resistor_ohms_ = u32;
			Serial.println("cal: r_series_ohms updated");
			return true;
		}

		if (parse_u32_arg(line, "ref_mv", &u32))
		{
			reference_mv_ = u32;
			Serial.println("cal: ref_mv updated");
			return true;
		}

		Serial.println("cal: set: unknown key");
		return true;
	}

	if (std::strncmp(line, "cal point set ", 14) == 0)
	{
		if (!parse_point_args(line, &index, &i32))
		{
			Serial.println("cal: point set: bad args");
			Serial.println(
			    "cal: usage: cal point set <1..3> <temp_centi_c>");
			return true;
		}

		if (!set_point(index, i32))
		{
			Serial.println("cal: point set: failed");
			return true;
		}

		Serial.println("cal: point set ok");
		return true;
	}

	if (std::strncmp(line, "cal point capture ", 18) == 0)
	{
		if (!parse_point_index(line, &index))
		{
			Serial.println("cal: point capture: bad index");
			Serial.println("cal: usage: cal point capture <1..3>");
			return true;
		}

		if (!capture_point(index))
		{
			Serial.println("cal: point capture: failed");
			return true;
		}

		Serial.println("cal: point captured");
		return true;
	}

	if (std::strncmp(line, "cal solve", 9) == 0)
	{
		if (!solve_coefficients())
		{
			Serial.println("cal: solve failed");
			return true;
		}

		Serial.println("cal: solve ok");
		print_status();
		return true;
	}

	if (std::strncmp(line, "cal save", 8) == 0)
	{
		if (!coefficients_valid_)
		{
			Serial.println("cal: save: no coefficients");
			return true;
		}

		if (!save_to_eeprom())
		{
			Serial.println("cal: save failed");
			return true;
		}

		Serial.println("cal: saved");
		return true;
	}

	Serial.println("cal: unknown command (try: cal help)");
	return true;
}

bool ntc_calibration_service_t::parse_u32_arg(
    const char *line,
    const char *key,
    std::uint32_t *out_value) const
{
	const char *p = nullptr;
	char pattern[32]{};
	std::size_t n = 0u;

	if ((line == nullptr) || (key == nullptr) || (out_value == nullptr))
	{
		return false;
	}

	std::snprintf(pattern, sizeof(pattern), "cal set %s ", key);
	n = std::strlen(pattern);

	if (std::strncmp(line, pattern, n) != 0)
	{
		return false;
	}

	p = line + n;
	if (*p == '\0')
	{
		return false;
	}

	*out_value = static_cast<std::uint32_t>(std::strtoul(p, nullptr, 10));
	return true;
}

bool ntc_calibration_service_t::parse_point_args(
    const char *line,
    std::uint8_t *out_index,
    std::int32_t *out_temp_centi) const
{
	unsigned idx = 0u;
	long temp = 0;

	if ((line == nullptr) || (out_index == nullptr) ||
	    (out_temp_centi == nullptr))
	{
		return false;
	}

	if (std::sscanf(line, "cal point set %u %ld", &idx, &temp) != 2)
	{
		return false;
	}

	if ((idx < 1u) || (idx > 3u))
	{
		return false;
	}

	*out_index = static_cast<std::uint8_t>(idx);
	*out_temp_centi = static_cast<std::int32_t>(temp);

	return true;
}

bool ntc_calibration_service_t::parse_point_index(
    const char *line,
    std::uint8_t *out_index) const
{
	unsigned idx = 0u;

	if ((line == nullptr) || (out_index == nullptr))
	{
		return false;
	}

	if (std::sscanf(line, "cal point capture %u", &idx) != 1)
	{
		return false;
	}

	if ((idx < 1u) || (idx > 3u))
	{
		return false;
	}

	*out_index = static_cast<std::uint8_t>(idx);
	return true;
}

bool ntc_calibration_service_t::set_point(
    std::uint8_t index,
    std::int32_t temp_centi_c)
{
	const std::uint8_t i = static_cast<std::uint8_t>(index - 1u);

	if ((index < 1u) || (index > 3u))
	{
		return false;
	}

	points_[i].reference_temperature_centi_c = temp_centi_c;
	return true;
}

bool ntc_calibration_service_t::analog_read_oversampled(
    pin_t pin,
    std::uint8_t samples,
    std::uint16_t *out_adc_count) const
{
	std::uint32_t sum = 0u;
	std::uint8_t i = 0u;
	int reading = 0;

	if ((pin == PIN_INVALID) || (out_adc_count == nullptr))
	{
		return false;
	}

	if (samples == 0u)
	{
		return false;
	}

	if (samples > app_config_t::ntc_max_oversample)
	{
		samples = app_config_t::ntc_max_oversample;
	}

	for (i = 0u; i < samples; i++)
	{
		reading = analogRead(pin);
		if (reading < 0)
		{
			return false;
		}

		if (reading > app_config_t::adc_max_count)
		{
			reading = app_config_t::adc_max_count;
		}

		sum += static_cast<std::uint32_t>(reading);
	}

	*out_adc_count = static_cast<std::uint16_t>(sum / samples);
	return true;
}

bool ntc_calibration_service_t::adc_count_to_mv(
    std::uint16_t adc_count,
    std::uint32_t *out_mv) const
{
	const std::uint32_t max_count = app_config_t::adc_max_count;

	if (out_mv == nullptr)
	{
		return false;
	}

	if (adc_count > max_count)
	{
		adc_count = static_cast<std::uint16_t>(max_count);
	}

	*out_mv = (static_cast<std::uint32_t>(adc_count) * reference_mv_) /
		  max_count;

	return true;
}

bool ntc_calibration_service_t::adc_mv_to_resistance_ohms(
    std::uint32_t adc_mv,
    std::uint32_t *out_resistance_ohms) const
{
	float v = 0.0f;
	float vr = 0.0f;
	float r = 0.0f;

	if (out_resistance_ohms == nullptr)
	{
		return false;
	}

	if ((reference_mv_ == 0u) || (series_resistor_ohms_ == 0u))
	{
		return false;
	}

	if (adc_mv >= (reference_mv_ - 1u))
	{
		return false;
	}

	v = static_cast<float>(adc_mv);
	vr = static_cast<float>(reference_mv_);
	r = (v * static_cast<float>(series_resistor_ohms_)) / (vr - v);

	if (r <= 0.0f)
	{
		return false;
	}

	*out_resistance_ohms = static_cast<std::uint32_t>(r + 0.5f);
	return true;
}

bool ntc_calibration_service_t::capture_point(std::uint8_t index)
{
	const std::uint8_t i = static_cast<std::uint8_t>(index - 1u);

	std::uint16_t adc = 0u;
	std::uint32_t mv = 0u;
	std::uint32_t r = 0u;

	if (mode_ != mode_t::active)
	{
		Serial.println("cal: not active (use cal start)");
		return false;
	}

	if ((index < 1u) || (index > 3u))
	{
		return false;
	}

	if (!analog_read_oversampled(adc_pin_, app_config_t::ntc_max_oversample, &adc))
	{
		return false;
	}

	if (!adc_count_to_mv(adc, &mv))
	{
		return false;
	}

	if (!adc_mv_to_resistance_ohms(mv, &r))
	{
		return false;
	}

	points_[i].adc_count = adc;
	points_[i].adc_mv = mv;
	points_[i].ntc_resistance_ohms = r;
	points_[i].valid = true;

	return true;
}

bool ntc_calibration_service_t::solve_3x3(
    float a[3][3],
    float b[3],
    float x[3]) const
{
	int i = 0;
	int j = 0;
	int k = 0;

	float m = 0.0f;

	float aa[3][3]{};
	float bb[3]{};

	for (i = 0; i < 3; i++)
	{
		bb[i] = b[i];
		for (j = 0; j < 3; j++)
		{
			aa[i][j] = a[i][j];
		}
	}

	for (i = 0; i < 3; i++)
	{
		const float pivot = aa[i][i];

		if (fabsf(pivot) < 1e-12f)
		{
			return false;
		}

		for (j = i; j < 3; j++)
		{
			aa[i][j] /= pivot;
		}

		bb[i] /= pivot;

		for (k = 0; k < 3; k++)
		{
			if (k == i)
			{
				continue;
			}

			m = aa[k][i];

			for (j = i; j < 3; j++)
			{
				aa[k][j] -= m * aa[i][j];
			}

			bb[k] -= m * bb[i];
		}
	}

	for (i = 0; i < 3; i++)
	{
		x[i] = bb[i];
	}

	return true;
}

bool ntc_calibration_service_t::solve_coefficients(void)
{
	float a[3][3]{};
	float b[3]{};
	float x[3]{};

	std::uint8_t i = 0u;

	for (i = 0u; i < 3u; i++)
	{
		if (!points_[i].valid)
		{
			return false;
		}
	}

	for (i = 0u; i < 3u; i++)
	{
		const float r = static_cast<float>(points_[i].ntc_resistance_ohms);
		const float ln_r = logf(r);

		const float t_c =
		    static_cast<float>(points_[i].reference_temperature_centi_c) /
		    100.0f;

		const float t_k = t_c + 273.15f;

		a[i][0] = 1.0f;
		a[i][1] = ln_r;
		a[i][2] = ln_r * ln_r * ln_r;

		b[i] = 1.0f / t_k;
	}

	if (!solve_3x3(a, b, x))
	{
		return false;
	}

	coefficients_.a = x[0];
	coefficients_.b = x[1];
	coefficients_.c = x[2];
	coefficients_valid_ = true;

	return true;
}

bool ntc_calibration_service_t::adc_mv_to_temperature_centi_c(
    std::uint32_t adc_mv,
    std::int32_t *out_temperature_centi_c) const
{
	std::uint32_t r = 0u;
	float ln_r = 0.0f;
	float inv_t = 0.0f;
	float t_k = 0.0f;
	float t_c = 0.0f;

	if (out_temperature_centi_c == nullptr)
	{
		return false;
	}

	if (!coefficients_valid_)
	{
		return false;
	}

	if (!adc_mv_to_resistance_ohms(adc_mv, &r))
	{
		return false;
	}

	ln_r = logf(static_cast<float>(r));
	inv_t = coefficients_.a +
		(coefficients_.b * ln_r) +
		(coefficients_.c * ln_r * ln_r * ln_r);

	if (inv_t <= 0.0f)
	{
		return false;
	}

	t_k = 1.0f / inv_t;
	t_c = t_k - 273.15f;

	*out_temperature_centi_c =
	    static_cast<std::int32_t>((t_c * 100.0f) + 0.5f);

	return true;
}

bool ntc_calibration_service_t::load_from_eeprom(void)
{
	ntc_cal_storage_t s{};

	EEPROM.get(app_config_t::ntc_eeprom_address, s);

	if ((s.magic != app_config_t::ntc_storage_magic) ||
	    (s.version != app_config_t::ntc_storage_version))
	{
		/* EEPROM empty or invalid -> load factory defaults */

		coefficients_.a = app_config_t::ntc_default_a;
		coefficients_.b = app_config_t::ntc_default_b;
		coefficients_.c = app_config_t::ntc_default_c;

		coefficients_valid_ = true;

		Serial.println("cal: using factory default coefficients");

		return false;
	}

	series_resistor_ohms_ = s.series_resistor_ohms;
	reference_mv_ = s.reference_mv;

	coefficients_ = s.coefficients;
	coefficients_valid_ = true;

	Serial.println("cal: loaded coefficients from eeprom");

	return true;
}

bool ntc_calibration_service_t::save_to_eeprom(void)
{
	ntc_cal_storage_t s{};

	s.magic = app_config_t::ntc_storage_magic;
	s.version = app_config_t::ntc_storage_version;
	s.reserved = 0u;

	s.series_resistor_ohms = series_resistor_ohms_;
	s.reference_mv = reference_mv_;

	s.coefficients = coefficients_;

	EEPROM.put(app_config_t::ntc_eeprom_address, s);

	return true;
}