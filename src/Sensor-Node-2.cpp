/**
 * @file	main.cpp
 * @brief	Scaffold for SN2
 */

#include "Particle.h"

SYSTEM_MODE(MANUAL);

namespace
{
	/**
	 * @brief	Application pin map
	 */
	struct pin_map_t
	{
		pin_t adc_pot_pin;
		pin_t adc_ntc_pin;
		pin_t pwm_fan_pin;
		pin_t gpio_button_pin;
		pin_t gpio_sound_pin;
		pin_t led_red_pin;
		pin_t led_green_pin;
	};

	static const pin_map_t g_pins =
	    {
		.adc_pot_pin = A1,
		.adc_ntc_pin = A5,
		.pwm_fan_pin = A2,
		.gpio_button_pin = S4,
		.gpio_sound_pin = D2,
		.led_red_pin = D4,
		.led_green_pin = D3};

	static const uint32_t g_serial_baudrate = 115200U;

	static const uint16_t g_adc_max_count = 4095U;
	static const uint32_t g_adc_reference_mv = 3300U;

	static const uint32_t g_poll_period_ms = 200U;
	static const uint32_t g_heartbeat_period_ms = 500U;

	static const uint32_t g_button_debounce_ms = 40U;

	static const uint32_t g_sound_min_pulse_spacing_us = 2000U;

	static const uint32_t g_pwm_frequency_hz = 25000U;

	static const uint8_t g_pwm_duty_min = 0U;
	static const uint8_t g_pwm_duty_max = 255U;

	static volatile bool g_button_irq_flag = false;

	static volatile uint32_t g_sound_pulse_count = 0U;
	static volatile uint32_t g_sound_last_pulse_us = 0U;

	/**
	 * @brief	Convert ADC counts to millivolts
	 * @param	adc_count ADC raw count
	 * @return	Millivolts corresponding to the ADC count
	 */
	static uint32_t adc_count_to_mv(uint16_t adc_count)
	{
		uint32_t mv = 0U;

		if (adc_count > g_adc_max_count)
		{
			adc_count = g_adc_max_count;
		}

		mv = (static_cast<uint32_t>(adc_count) * g_adc_reference_mv) /
		     static_cast<uint32_t>(g_adc_max_count);

		return mv;
	}

	/**
	 * @brief	Map ADC counts to 8-bit PWM duty
	 * @param	adc_count ADC raw count
	 * @return	PWM duty in range [0..255]
	 */
	static uint8_t adc_count_to_pwm_duty(uint16_t adc_count)
	{
		uint32_t duty = 0U;

		if (adc_count > g_adc_max_count)
		{
			adc_count = g_adc_max_count;
		}

		duty = (static_cast<uint32_t>(adc_count) *
			static_cast<uint32_t>(g_pwm_duty_max)) /
		       static_cast<uint32_t>(g_adc_max_count);

		if (duty > static_cast<uint32_t>(g_pwm_duty_max))
		{
			duty = static_cast<uint32_t>(g_pwm_duty_max);
		}

		return static_cast<uint8_t>(duty);
	}

	/**
	 * @brief	Read ADC with simple oversampling and channel-settle discard
	 * @param	pin ADC pin
	 * @param	samples Number of samples to average (bounded)
	 * @param	out_adc_count Output averaged ADC count
	 * @return	true on success, false otherwise
	 */
	static bool analog_read_oversampled(
	    pin_t pin,
	    uint8_t samples,
	    uint16_t *out_adc_count)
	{
		uint32_t sum = 0U;
		uint8_t index = 0U;
		int reading = 0;

		if ((pin == PIN_INVALID) || (out_adc_count == nullptr))
		{
			return false;
		}

		if (samples == 0U)
		{
			return false;
		}

		if (samples > 32U)
		{
			samples = 32U;
		}

		reading = analogRead(pin);
		(void)reading;

		for (index = 0U; index < samples; index++)
		{
			reading = analogRead(pin);

			if (reading < 0)
			{
				*out_adc_count = 0U;
				return false;
			}

			if (reading > static_cast<int>(g_adc_max_count))
			{
				reading = static_cast<int>(g_adc_max_count);
			}

			sum += static_cast<uint32_t>(reading);
		}

		*out_adc_count = static_cast<uint16_t>(sum / samples);

		return true;
	}

	/**
	 * @brief	Set a GPIO output safely
	 * @param	pin Target pin
	 * @param	state Logic level to drive
	 * @return	true on success, false otherwise
	 */
	static bool safe_digital_write(pin_t pin, bool state)
	{
		if (pin == PIN_INVALID)
		{
			return false;
		}

		digitalWrite(pin, state ? HIGH : LOW);

		return true;
	}

	/**
	 * @brief	Set PWM output safely at a specified frequency
	 * @param	pin Target PWM pin
	 * @param	duty PWM duty [0..255]
	 * @return	true on success, false otherwise
	 */
	static bool safe_pwm_write(pin_t pin, uint8_t duty)
	{
		if (pin == PIN_INVALID)
		{
			return false;
		}

		analogWrite(pin, static_cast<int>(duty), g_pwm_frequency_hz);

		return true;
	}

	/**
	 * @brief	Button interrupt service routine
	 */
	static void button_isr(void)
	{
		g_button_irq_flag = true;
	}

	/**
	 * @brief	Sound interrupt service routine (LM393 open-collector active-low)
	 *
	 * @note	Deglitch by enforcing a minimum pulse spacing.
	 */
	static void sound_isr(void)
	{
		const uint32_t now_us = micros();
		const uint32_t delta_us = now_us - g_sound_last_pulse_us;

		if (delta_us >= g_sound_min_pulse_spacing_us)
		{
			g_sound_last_pulse_us = now_us;
			g_sound_pulse_count++;
		}
	}

	/**
	 * @brief	Initialise GPIO directions and safe defaults
	 */
	static void initialise_hardware(void)
	{
		pinMode(g_pins.led_red_pin, OUTPUT);
		pinMode(g_pins.led_green_pin, OUTPUT);

		pinMode(g_pins.pwm_fan_pin, OUTPUT);

		pinMode(g_pins.gpio_button_pin, INPUT_PULLUP);
		pinMode(g_pins.gpio_sound_pin, INPUT);

		(void)safe_digital_write(g_pins.led_red_pin, false);
		(void)safe_digital_write(g_pins.led_green_pin, false);
		(void)safe_pwm_write(g_pins.pwm_fan_pin, g_pwm_duty_min);

		attachInterrupt(g_pins.gpio_button_pin, button_isr, FALLING);
		attachInterrupt(g_pins.gpio_sound_pin, sound_isr, FALLING);
	}

	/**
	 * @brief	Simple heartbeat LED toggle
	 * @param	now_ms Current time in ms
	 */
	static void service_heartbeat(uint32_t now_ms)
	{
		static uint32_t last_toggle_ms = 0U;
		static bool led_state = false;

		if ((now_ms - last_toggle_ms) >= g_heartbeat_period_ms)
		{
			last_toggle_ms = now_ms;
			led_state = !led_state;
			(void)safe_digital_write(g_pins.led_green_pin, led_state);
		}
	}

	/**
	 * @brief	Service button debouncing and behaviour
	 * @param	now_ms Current time in ms
	 */
	static void service_button(uint32_t now_ms)
	{
		static bool debounce_active = false;
		static uint32_t debounce_start_ms = 0U;
		static bool red_led_state = false;

		if (g_button_irq_flag)
		{
			g_button_irq_flag = false;
			debounce_active = true;
			debounce_start_ms = now_ms;
		}

		if (debounce_active)
		{
			if ((now_ms - debounce_start_ms) >= g_button_debounce_ms)
			{
				debounce_active = false;

				if (digitalRead(g_pins.gpio_button_pin) == LOW)
				{
					red_led_state = !red_led_state;
					(void)safe_digital_write(
					    g_pins.led_red_pin,
					    red_led_state);
				}
			}
		}
	}

	/**
	 * @brief	Service ADC and PWM control loop
	 * @param	now_ms Current time in ms
	 */
	static void service_analog_and_pwm(uint32_t now_ms)
	{
		static uint32_t last_poll_ms = 0U;

		uint16_t pot_adc = 0U;
		uint16_t ntc_adc = 0U;

		uint32_t pot_mv = 0U;
		uint32_t ntc_mv = 0U;

		uint8_t pwm_duty = 0U;

		if ((now_ms - last_poll_ms) < g_poll_period_ms)
		{
			return;
		}

		last_poll_ms = now_ms;

		if (!analog_read_oversampled(g_pins.adc_pot_pin, 8U, &pot_adc))
		{
			return;
		}

		if (!analog_read_oversampled(g_pins.adc_ntc_pin, 10U, &ntc_adc))
		{
			return;
		}

		pwm_duty = adc_count_to_pwm_duty(pot_adc);
		(void)safe_pwm_write(g_pins.pwm_fan_pin, pwm_duty);

		pot_mv = adc_count_to_mv(pot_adc);
		ntc_mv = adc_count_to_mv(ntc_adc);

		Serial.printf(
		    "pot=%4u (%4lumV) ntc=%4u (%4lumV) pwm=%3u "
		    "sound=%lu\r\n",
		    static_cast<unsigned>(pot_adc),
		    static_cast<unsigned long>(pot_mv),
		    static_cast<unsigned>(ntc_adc),
		    static_cast<unsigned long>(ntc_mv),
		    static_cast<unsigned>(pwm_duty),
		    static_cast<unsigned long>(g_sound_pulse_count));
	}
}

void setup(void)
{
	Serial.begin(g_serial_baudrate);

	initialise_hardware();
}

void loop(void)
{
	const uint32_t now_ms = millis();

	service_heartbeat(now_ms);
	service_button(now_ms);
	service_analog_and_pwm(now_ms);
}