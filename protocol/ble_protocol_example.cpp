/**
 * @file ble_protocol_example.cpp
 * @brief Minimal Photon 2 sensor-node example using ble_protocol.hpp.
 *
 * `ble_*` is the shared packet contract.
 * `BLE` / `Ble*` is Particle Device OS.
 */

#include "Particle.h"

#include "ble_protocol.hpp"

#include <cstddef>
#include <cstdint>

SYSTEM_MODE(MANUAL);

namespace
{
	constexpr ble_node_id_t example_node_id = ble_node_id_t::sn2;
	constexpr char ble_device_name[] = "ELEC4740-SN2";
	constexpr std::int8_t ble_tx_power_dbm = -12;
	constexpr std::uint16_t preferred_min_interval_units = 80u;
	constexpr std::uint16_t preferred_max_interval_units = 160u;
	constexpr std::uint16_t preferred_latency = 0u;
	constexpr std::uint16_t preferred_timeout_units = 600u;

	struct sensor_node_state_t final
	{
		std::uint8_t telemetry_sequence;
		std::uint8_t event_sequence;
		std::uint8_t last_command_sequence;
		bool last_command_sequence_valid;

		bool help_active;
		bool sensor_fault;
		bool status_led_latched;
		bool button_pressed;
		bool motion_detected;
		bool sound_detected;
		bool cn_actuator_control_selected;

		std::int16_t temperature_centi_c;
		float light_lux_float;
		std::uint8_t local_fan_duty_u8;
		std::uint8_t local_led_duty_u8;
		std::uint8_t remote_fan_duty_u8;
		std::uint8_t remote_led_duty_u8;
		std::uint8_t fan_duty_u8;
		std::uint8_t led_duty_u8;
		std::uint16_t status_word;
	};

	struct control_mailbox_t final
	{
		bool pending;
		std::size_t length;
		std::uint8_t buffer[ble_payload_size_t::control];
	};

	static sensor_node_state_t g_state{};
	static control_mailbox_t g_control_mailbox{};

	static bool g_initial_snapshot_requested = false;
	static bool g_restart_advertising_requested = false;

	static std::uint32_t g_last_telemetry_ms = 0u;

	static BleCharacteristic g_telemetry_characteristic(
	    "telemetry",
	    BleCharacteristicProperty::NOTIFY,
	    ble_uuid_t::telemetry,
	    ble_uuid_t::service);

	static BleCharacteristic g_event_characteristic(
	    "event",
	    BleCharacteristicProperty::NOTIFY,
	    ble_uuid_t::event,
	    ble_uuid_t::service);

	static void on_control_written(const std::uint8_t *data,
				       std::size_t length,
				       const BlePeerDevice &peer,
				       void *context);

	static BleCharacteristic g_control_characteristic(
	    "control",
	    BleCharacteristicProperty::WRITE_WO_RSP,
	    ble_uuid_t::control,
	    ble_uuid_t::service,
	    on_control_written,
	    nullptr);

	static std::uint16_t convert_light_lux_to_deci_lux(float light_lux_float)
	{
		float clamped_lux = light_lux_float;
		float scaled_deci_lux = 0.0f;

		if (clamped_lux < 0.0f)
		{
			clamped_lux = 0.0f;
		}

		scaled_deci_lux = clamped_lux *
				  static_cast<float>(ble_protocol_constants_t::light_deci_lux_per_lux);

		if (scaled_deci_lux > 65535.0f)
		{
			scaled_deci_lux = 65535.0f;
		}

		return static_cast<std::uint16_t>(scaled_deci_lux + 0.5f);
	}

	static void update_applied_duty_outputs(void)
	{
		if (g_state.cn_actuator_control_selected)
		{
			g_state.fan_duty_u8 = g_state.remote_fan_duty_u8;
			g_state.led_duty_u8 = g_state.remote_led_duty_u8;
			return;
		}

		g_state.fan_duty_u8 = g_state.local_fan_duty_u8;
		g_state.led_duty_u8 = g_state.local_led_duty_u8;
	}

	static std::uint16_t build_telemetry_flags(void)
	{
		std::uint16_t flags = 0u;
		const bool local_actuator_control_selected = !g_state.cn_actuator_control_selected;

		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::help_active,
						  g_state.help_active);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::local_actuator_control_selected,
						  local_actuator_control_selected);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::sensor_fault,
						  g_state.sensor_fault);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::status_led_latched,
						  g_state.status_led_latched);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::button_pressed,
						  g_state.button_pressed);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::motion_detected,
						  g_state.motion_detected);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::sound_detected,
						  g_state.sound_detected);
		flags = ble_telemetry_flag_update(flags,
						  ble_telemetry_flag_t::cn_actuator_control_selected,
						  g_state.cn_actuator_control_selected);

		return flags;
	}

	static void start_ble_advertising(void)
	{
		BleAdvertisingData advertising_data{};
		BleAdvertisingData scan_response{};

		advertising_data.appendServiceUUID(ble_uuid_t::service);
		scan_response.appendLocalName(ble_device_name);

		const int advertise_result =
		    BLE.advertise(&advertising_data, &scan_response);

		Serial.printf("BLE advertise result=%d\r\n", advertise_result);
	}

	static void on_ble_connected(const BlePeerDevice &peer, void *context)
	{
		(void)peer;
		(void)context;

		SINGLE_THREADED_BLOCK()
		{
			g_initial_snapshot_requested = true;
		}
	}

	static void on_ble_disconnected(const BlePeerDevice &peer, void *context)
	{
		(void)peer;
		(void)context;

		SINGLE_THREADED_BLOCK()
		{
			g_restart_advertising_requested = true;
		}
	}

	static void on_control_written(const std::uint8_t *data,
				       std::size_t length,
				       const BlePeerDevice &peer,
				       void *context)
	{
		std::size_t index = 0u;

		(void)peer;
		(void)context;

		if ((data == nullptr) || (length != ble_payload_size_t::control))
		{
			return;
		}

		SINGLE_THREADED_BLOCK()
		{
			if (!g_control_mailbox.pending)
			{
				for (index = 0u; index < length; ++index)
				{
					g_control_mailbox.buffer[index] = data[index];
				}

				g_control_mailbox.length = length;
				g_control_mailbox.pending = true;
			}
		}
	}

	static void initialise_sensor_node_state(void)
	{
		g_state.telemetry_sequence = 0u;
		g_state.event_sequence = 0u;
		g_state.last_command_sequence = 0u;
		g_state.last_command_sequence_valid = false;

		g_state.help_active = false;
		g_state.sensor_fault = false;
		g_state.status_led_latched = false;
		g_state.button_pressed = false;
		g_state.motion_detected = false;
		g_state.sound_detected = false;
		g_state.cn_actuator_control_selected = false;

		g_state.temperature_centi_c = 2375;
		g_state.light_lux_float = 642.5f;
		g_state.local_fan_duty_u8 = 64u;
		g_state.local_led_duty_u8 = 0u;
		g_state.remote_fan_duty_u8 = 0u;
		g_state.remote_led_duty_u8 = 0u;
		g_state.fan_duty_u8 = 0u;
		g_state.led_duty_u8 = 0u;
		g_state.status_word = ble_protocol_constants_t::status_word_none;
		update_applied_duty_outputs();

		g_initial_snapshot_requested = false;
		g_restart_advertising_requested = false;
		g_control_mailbox.pending = false;
		g_control_mailbox.length = 0u;
	}

	static bool notify_telemetry_snapshot(const char *reason)
	{
		std::uint8_t buffer[ble_payload_size_t::telemetry] = {};
		const std::uint8_t sequence = g_state.telemetry_sequence;
		telemetry_packet_t packet{};
		const std::int32_t whole_c =
		    static_cast<std::int32_t>(g_state.temperature_centi_c) / 100;
		std::int32_t frac_c =
		    static_cast<std::int32_t>(g_state.temperature_centi_c) % 100;

		if (!BLE.connected())
		{
			return false;
		}

		if (frac_c < 0)
		{
			frac_c = -frac_c;
		}

		packet = ble_make_telemetry(
		    example_node_id,
		    sequence,
		    build_telemetry_flags(),
		    g_state.temperature_centi_c,
		    convert_light_lux_to_deci_lux(g_state.light_lux_float),
		    g_state.fan_duty_u8,
		    g_state.led_duty_u8,
		    g_state.status_word);

		if (!ble_pack_telemetry(buffer, sizeof(buffer), &packet))
		{
			return false;
		}

		if (g_telemetry_characteristic.setValue(buffer, sizeof(buffer)) !=
		    static_cast<int>(sizeof(buffer)))
		{
			return false;
		}

		g_state.telemetry_sequence =
		    static_cast<std::uint8_t>(sequence + 1u);

		Serial.printf(
		    "telemetry (%s): seq=%u temp=%ld.%02ldC light=%u.%01u lux "
		    "fan=%u led=%u motion=%u sound=%u\r\n",
		    reason,
		    static_cast<unsigned>(packet.sequence),
		    static_cast<long>(whole_c),
		    static_cast<long>(frac_c),
		    static_cast<unsigned>(packet.light_deci_lux / 10u),
		    static_cast<unsigned>(packet.light_deci_lux % 10u),
		    static_cast<unsigned>(packet.fan_duty_applied),
		    static_cast<unsigned>(packet.led_duty_applied),
		    ble_telemetry_flag_is_set(
			packet.flags, ble_telemetry_flag_t::motion_detected)
			? 1u
			: 0u,
		    ble_telemetry_flag_is_set(
			packet.flags, ble_telemetry_flag_t::sound_detected)
			? 1u
			: 0u);

		return true;
	}

	static bool notify_event(ble_event_type_t event_type,
				 std::int16_t event_value,
				 std::uint32_t now_ms,
				 const char *reason)
	{
		std::uint8_t buffer[ble_payload_size_t::event] = {};
		const std::uint8_t sequence = g_state.event_sequence;
		event_packet_t packet{};

		if (!BLE.connected())
		{
			return false;
		}

		packet = ble_make_event(example_node_id,
					sequence,
					event_type,
					event_value,
					static_cast<std::uint16_t>(now_ms & 0xFFFFu));

		if (!ble_pack_event(buffer, sizeof(buffer), &packet))
		{
			return false;
		}

		if (g_event_characteristic.setValue(buffer, sizeof(buffer)) !=
		    static_cast<int>(sizeof(buffer)))
		{
			return false;
		}

		g_state.event_sequence =
		    static_cast<std::uint8_t>(sequence + 1u);

		Serial.printf("event (%s): seq=%u type=%u value=%d\r\n",
			      reason,
			      static_cast<unsigned>(packet.sequence),
			      static_cast<unsigned>(static_cast<std::uint8_t>(
				  packet.event_type)),
			      static_cast<int>(packet.event_value));

		return true;
	}

	static bool update_motion_detector_state_and_notify(bool motion_detected,
							    std::uint32_t now_ms)
	{
		const bool rising_edge = motion_detected && !g_state.motion_detected;

		g_state.motion_detected = motion_detected;

		if (!rising_edge)
		{
			return false;
		}

		return notify_event(
		    ble_event_type_t::motion_detected, 1, now_ms, "motion detected");
	}

	static bool update_sound_detector_state_and_notify(bool sound_detected,
							   std::uint32_t now_ms)
	{
		const bool rising_edge = sound_detected && !g_state.sound_detected;

		g_state.sound_detected = sound_detected;

		if (!rising_edge)
		{
			return false;
		}

		return notify_event(
		    ble_event_type_t::sound_detected, 1, now_ms, "sound detected");
	}

	static void process_control_mailbox(std::uint32_t now_ms)
	{
		std::uint8_t local_buffer[ble_payload_size_t::control] = {};
		std::size_t index = 0u;
		std::size_t length = 0u;
		control_packet_t packet{};
		bool clear_help_requested = false;
		bool clear_led_requested = false;
		bool override_enabled = false;

		SINGLE_THREADED_BLOCK()
		{
			if (g_control_mailbox.pending)
			{
				length = g_control_mailbox.length;

				for (index = 0u; index < length; ++index)
				{
					local_buffer[index] = g_control_mailbox.buffer[index];
				}

				g_control_mailbox.pending = false;
				g_control_mailbox.length = 0u;
			}
		}

		if (length == 0u)
		{
			return;
		}

		if (!ble_unpack_control(&packet, local_buffer, length))
		{
			Serial.printf("control unpack failed\r\n");
			return;
		}

		if (packet.target_node_id != example_node_id)
		{
			Serial.printf("control ignored: wrong target node\r\n");
			return;
		}

		if (!ble_command_should_accept(packet.command_sequence,
					       g_state.last_command_sequence,
					       g_state.last_command_sequence_valid))
		{
			Serial.printf("control ignored: duplicate or stale seq=%u\r\n",
				      static_cast<unsigned>(
					  packet.command_sequence));
			return;
		}

		g_state.last_command_sequence = packet.command_sequence;
		g_state.last_command_sequence_valid = true;
		clear_help_requested = ble_control_flag_is_set(
		    packet.command_flags, ble_control_flag_t::clear_help_request);
		clear_led_requested = ble_control_flag_is_set(
		    packet.command_flags,
		    ble_control_flag_t::clear_status_led_request);
		override_enabled = ble_control_flag_is_set(
		    packet.command_flags,
		    ble_control_flag_t::actuator_override_enable);

		if (clear_help_requested)
		{
			g_state.help_active = false;
			g_state.button_pressed = false;
		}

		if (clear_led_requested)
		{
			g_state.status_led_latched = false;
		}

		if (override_enabled)
		{
			g_state.cn_actuator_control_selected = true;
			g_state.remote_fan_duty_u8 =
			    ble_clamp_duty_u8(packet.fan_duty_override);
			g_state.remote_led_duty_u8 =
			    ble_clamp_duty_u8(packet.led_duty_override);
		}
		else
		{
			g_state.cn_actuator_control_selected = false;
			g_state.remote_fan_duty_u8 = 0u;
			g_state.remote_led_duty_u8 = 0u;
		}

		update_applied_duty_outputs();

		Serial.printf("control applied: seq=%u clear_help=%u clear_led=%u "
			      "remote_override=%u fan=%u led=%u\r\n",
			      static_cast<unsigned>(packet.command_sequence),
			      clear_help_requested ? 1u : 0u,
			      clear_led_requested ? 1u : 0u,
			      override_enabled ? 1u : 0u,
			      static_cast<unsigned>(g_state.fan_duty_u8),
			      static_cast<unsigned>(g_state.led_duty_u8));

		if (clear_help_requested || clear_led_requested)
		{
			(void)notify_event(ble_event_type_t::clear_acknowledged,
					   0,
					   now_ms,
					   "clear acknowledged");
		}

		(void)notify_telemetry_snapshot("post-control state echo");
	}

	static void initialise_sensor_node_ble(void)
	{
		const int ble_on_result = BLE.on();
		const int ble_begin_result = BLE.begin();
		const int tx_power_result = BLE.setTxPower(ble_tx_power_dbm);
		const int ppcp_result = BLE.setPPCP(preferred_min_interval_units,
						    preferred_max_interval_units,
						    preferred_latency,
						    preferred_timeout_units);

		BLE.onConnected(on_ble_connected, nullptr);
		BLE.onDisconnected(on_ble_disconnected, nullptr);

		(void)BLE.addCharacteristic(g_telemetry_characteristic);
		(void)BLE.addCharacteristic(g_event_characteristic);
		(void)BLE.addCharacteristic(g_control_characteristic);

		Serial.printf(
		    "BLE bring-up: on=%d begin=%d tx_power=%d ppcp=%d\r\n",
		    ble_on_result,
		    ble_begin_result,
		    tx_power_result,
		    ppcp_result);

		start_ble_advertising();
	}
}

void setup(void)
{
	Serial.begin(115200);
	waitFor(Serial.isConnected, 3000);

	Serial.printf("\r\n%s BLE protocol example\r\n", ble_device_name);
	initialise_sensor_node_state();
	initialise_sensor_node_ble();

	g_last_telemetry_ms = millis();
}

void loop(void)
{
	const std::uint32_t now_ms = millis();
	bool send_initial_snapshot = false;
	bool restart_advertising = false;

	SINGLE_THREADED_BLOCK()
	{
		send_initial_snapshot = g_initial_snapshot_requested;
		restart_advertising = g_restart_advertising_requested;
		g_initial_snapshot_requested = false;
		g_restart_advertising_requested = false;
	}

	if (send_initial_snapshot)
	{
		(void)notify_telemetry_snapshot("initial snapshot");
	}

	if (restart_advertising)
	{
		start_ble_advertising();
	}

	process_control_mailbox(now_ms);

	// Application hooks:
	// - update g_state.temperature_centi_c / light_lux_float from sensors
	// - call update_motion_detector_state_and_notify(...) on SN1
	// - call update_sound_detector_state_and_notify(...) on SN2
	// - update local_fan_duty_u8 / local_led_duty_u8, then call update_applied_duty_outputs()

	// Or however you wish to do it instead. But hopefully this example shows you the basics.

	if ((now_ms - g_last_telemetry_ms) >=
	    ble_protocol_constants_t::default_telemetry_period_ms)
	{
		g_last_telemetry_ms = now_ms;
		(void)notify_telemetry_snapshot("periodic snapshot");
	}
}
