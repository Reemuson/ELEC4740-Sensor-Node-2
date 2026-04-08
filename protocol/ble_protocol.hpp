/**
 * @file ble_protocol.hpp
 * @brief Portable byte-level BLE protocol contract for ELEC4740.
 *
 * This header defines the authoritative protocol shared by the Control Node
 * (CN), Sensor Node 1 (SN1), and Sensor Node 2 (SN2). It is intentionally
 * transport-agnostic so the packet layer can be unit-tested on a host machine
 * without BLE hardware.
 */

#pragma once

#include <cstddef>
#include <cstdint>

static_assert(sizeof(std::uint8_t) == 1u, "unexpected uint8_t size");
static_assert(sizeof(std::uint16_t) == 2u, "unexpected uint16_t size");
static_assert(sizeof(std::int16_t) == 2u, "unexpected int16_t size");

/**
 * @brief Fixed BLE UUID strings shared by all nodes.
 */
struct ble_uuid_t final
{
	static constexpr const char *service =
	    "8f9d2a10-6a7b-4c7e-9f7b-2c6a0e1d8a40";
	static constexpr const char *telemetry =
	    "8f9d2a11-6a7b-4c7e-9f7b-2c6a0e1d8a40";
	static constexpr const char *event =
	    "8f9d2a12-6a7b-4c7e-9f7b-2c6a0e1d8a40";
	static constexpr const char *control =
	    "8f9d2a13-6a7b-4c7e-9f7b-2c6a0e1d8a40";
};

/**
 * @brief Supported BLE protocol versions.
 */
enum class ble_protocol_version_t : std::uint8_t
{
	v1 = 1u
};

/**
 * @brief Logical node identifiers used on the BLE link.
 */
enum class ble_node_id_t : std::uint8_t
{
	control = 0u,
	sn1 = 1u,
	sn2 = 2u
};

/**
 * @brief Persistent telemetry flags reported by sensor nodes.
 */
enum class ble_telemetry_flag_t : std::uint16_t
{
	help_active = (1u << 0),
	local_actuator_control_selected =
	    (1u << 1), ///< Local actuator control is currently selected.
	sensor_fault = (1u << 2),
	status_led_latched = (1u << 3),
	button_pressed = (1u << 4),
	cn_actuator_control_selected =
	    (1u << 5), ///< CN actuator control is currently selected.
	motion_detected = (1u << 6), ///< Motion detector output is currently active.
	sound_detected = (1u << 7) ///< Sound detector output is currently active.
};

/**
 * @brief Asynchronous event types reported by sensor nodes.
 */
enum class ble_event_type_t : std::uint8_t
{
	help_pressed = 1u,
	help_released = 2u,
	local_button_pressed = 3u,
	local_button_released = 4u,
	sensor_fault_asserted = 5u,
	sensor_fault_cleared = 6u,
	clear_acknowledged = 7u,
	sound_detected = 8u,
	motion_detected = 9u
};

/**
 * @brief Control flags written from CN to a target sensor node.
 */
enum class ble_control_flag_t : std::uint16_t
{
	actuator_override_enable = (1u << 0),
	clear_help_request = (1u << 1),
	clear_status_led_request = (1u << 2)
};

/**
 * @brief Shared scaling and timing constants for the application layer.
 *
 * `status_word` remains reserved in protocol v1. The recommended default is
 * zero unless both sides explicitly agree on additional meanings.
 */
struct ble_protocol_constants_t final
{
	static constexpr std::int32_t temperature_centi_per_c = 100;
	static constexpr std::int32_t light_deci_lux_per_lux = 10;
	static constexpr std::uint8_t duty_u8_max = 255u;

	static constexpr std::uint32_t default_telemetry_period_ms = 5000u;
	static constexpr std::uint32_t maximum_telemetry_period_ms = 10000u;

	static constexpr std::int16_t
	    default_temperature_delta_threshold_centi_c = 50;
	static constexpr std::uint16_t default_light_delta_threshold_deci_lux = 250u;

	static constexpr std::uint32_t critical_event_retry_timeout_ms = 750u;
	static constexpr std::uint32_t noisy_event_lockout_ms = 200u;

	static constexpr std::uint16_t status_word_none = 0u;
	static constexpr std::uint16_t reserved_field_must_be_zero = 0u;
};

/**
 * @brief Exact payload sizes for each logical packet type.
 */
struct ble_payload_size_t final
{
	static constexpr std::size_t telemetry = 13u;
	static constexpr std::size_t event = 8u;
	static constexpr std::size_t control = 9u;
};

/**
 * @brief Byte offsets for telemetry payload serialisation.
 */
struct ble_telemetry_packet_layout_t final
{
	static constexpr std::size_t protocol_version_offset = 0u;
	static constexpr std::size_t node_id_offset = 1u;
	static constexpr std::size_t sequence_offset = 2u;
	static constexpr std::size_t flags_offset = 3u;
	static constexpr std::size_t temperature_centi_c_offset = 5u;
	static constexpr std::size_t light_deci_lux_offset = 7u;
	static constexpr std::size_t fan_duty_applied_offset = 9u;
	static constexpr std::size_t led_duty_applied_offset = 10u;
	static constexpr std::size_t status_word_offset = 11u;
};

/**
 * @brief Byte offsets for event payload serialisation.
 */
struct ble_event_packet_layout_t final
{
	static constexpr std::size_t protocol_version_offset = 0u;
	static constexpr std::size_t node_id_offset = 1u;
	static constexpr std::size_t sequence_offset = 2u;
	static constexpr std::size_t event_type_offset = 3u;
	static constexpr std::size_t event_value_offset = 4u;
	static constexpr std::size_t timestamp_ms_mod_offset = 6u;
};

/**
 * @brief Byte offsets for control payload serialisation.
 */
struct ble_control_packet_layout_t final
{
	static constexpr std::size_t protocol_version_offset = 0u;
	static constexpr std::size_t target_node_id_offset = 1u;
	static constexpr std::size_t command_sequence_offset = 2u;
	static constexpr std::size_t command_flags_offset = 3u;
	static constexpr std::size_t fan_duty_override_offset = 5u;
	static constexpr std::size_t led_duty_override_offset = 6u;
	static constexpr std::size_t reserved_offset = 7u;
};

static_assert(
    ble_telemetry_packet_layout_t::status_word_offset + sizeof(std::uint16_t) ==
	ble_payload_size_t::telemetry,
    "telemetry layout mismatch");
static_assert(
    ble_event_packet_layout_t::timestamp_ms_mod_offset +
	    sizeof(std::uint16_t) ==
	ble_payload_size_t::event,
    "event layout mismatch");
static_assert(
    ble_control_packet_layout_t::reserved_offset + sizeof(std::uint16_t) ==
	ble_payload_size_t::control,
    "control layout mismatch");

/**
 * @brief Periodic telemetry snapshot sent from SN1 or SN2 to CN.
 *
 * Recommended semantic mapping:
 * - `temperature_centi_c`: temperature in signed centi-degC
 * - `light_deci_lux`: light in unsigned deci-lux
 * - `fan_duty_applied`: current fan duty applied by the node in the canonical
 *   0..255 range
 * - `led_duty_applied`: current LED duty applied by the node in the canonical
 *   0..255 range
 * - `flags`: persistent node state, including digital motion/sound detector
 *   levels
 *
 * Wire layout, 13 bytes:
 * - byte 0: protocol_version
 * - byte 1: node_id
 * - byte 2: sequence
 * - bytes 3-4: flags
 * - bytes 5-6: temperature_centi_c
 * - bytes 7-8: light_deci_lux
 * - byte 9: fan_duty_applied
 * - byte 10: led_duty_applied
 * - bytes 11-12: status_word
 */
struct telemetry_packet_t final
{
	ble_protocol_version_t protocol_version;
	ble_node_id_t node_id;
	std::uint8_t sequence;
	std::uint16_t flags;
	std::int16_t temperature_centi_c;
	std::uint16_t light_deci_lux;
	std::uint8_t fan_duty_applied;
	std::uint8_t led_duty_applied;
	std::uint16_t status_word;
};

/**
 * @brief Immediate event report sent from SN1 or SN2 to CN.
 *
 * `sound_detected` and `motion_detected` are asserted-edge events. The current
 * detector level is still reflected in telemetry flags.
 *
 * Wire layout, 8 bytes:
 * - byte 0: protocol_version
 * - byte 1: node_id
 * - byte 2: sequence
 * - byte 3: event_type
 * - bytes 4-5: event_value
 * - bytes 6-7: timestamp_ms_mod
 */
struct event_packet_t final
{
	ble_protocol_version_t protocol_version;
	ble_node_id_t node_id;
	std::uint8_t sequence;
	ble_event_type_t event_type;
	std::int16_t event_value;
	std::uint16_t timestamp_ms_mod;
};

/**
 * @brief Control command written from CN to a target sensor node.
 *
 * Wire layout, 9 bytes:
 * - byte 0: protocol_version
 * - byte 1: target_node_id
 * - byte 2: command_sequence
 * - bytes 3-4: command_flags
 * - byte 5: fan_duty_override
 * - byte 6: led_duty_override
 * - bytes 7-8: reserved
 */
struct control_packet_t final
{
	ble_protocol_version_t protocol_version;
	ble_node_id_t target_node_id;
	std::uint8_t command_sequence;
	std::uint16_t command_flags;
	std::uint8_t fan_duty_override;
	std::uint8_t led_duty_override;
	std::uint16_t reserved;
};

/**
 * @brief Simple deterministic test summary returned by the built-in runner.
 */
struct ble_protocol_test_result_t final
{
	std::uint16_t tests_run;
	std::uint16_t tests_passed;
	std::uint16_t tests_failed;
};

/**
 * @brief Convert a telemetry flag enum to its raw bit mask.
 * @param flag Flag to convert.
 * @return Raw flag mask.
 */
static inline constexpr std::uint16_t ble_telemetry_flag_mask(
    ble_telemetry_flag_t flag)
{
	return static_cast<std::uint16_t>(flag);
}

/**
 * @brief Convert a control flag enum to its raw bit mask.
 * @param flag Flag to convert.
 * @return Raw flag mask.
 */
static inline constexpr std::uint16_t ble_control_flag_mask(
    ble_control_flag_t flag)
{
	return static_cast<std::uint16_t>(flag);
}

/**
 * @brief Get the bit mask of all telemetry flags defined in protocol v1.
 * @return Mask of valid telemetry bits.
 */
static inline constexpr std::uint16_t ble_telemetry_flag_known_mask(void)
{
	return static_cast<std::uint16_t>(
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::help_active) |
	    ble_telemetry_flag_mask(
		ble_telemetry_flag_t::local_actuator_control_selected) |
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::sensor_fault) |
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::status_led_latched) |
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::button_pressed) |
	    ble_telemetry_flag_mask(
		ble_telemetry_flag_t::cn_actuator_control_selected) |
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::motion_detected) |
	    ble_telemetry_flag_mask(ble_telemetry_flag_t::sound_detected));
}

/**
 * @brief Get the bit mask of all control flags defined in protocol v1.
 * @return Mask of valid control bits.
 */
static inline constexpr std::uint16_t ble_control_flag_known_mask(void)
{
	return static_cast<std::uint16_t>(
	    ble_control_flag_mask(ble_control_flag_t::actuator_override_enable) |
	    ble_control_flag_mask(ble_control_flag_t::clear_help_request) |
	    ble_control_flag_mask(
		ble_control_flag_t::clear_status_led_request));
}

/**
 * @brief Test whether a telemetry flag is set in a raw flag field.
 * @param flags Raw telemetry flag field.
 * @param flag Flag to test.
 * @return true if the flag is set, otherwise false.
 */
static inline constexpr bool ble_telemetry_flag_is_set(
    std::uint16_t flags,
    ble_telemetry_flag_t flag)
{
	return (flags & ble_telemetry_flag_mask(flag)) != 0u;
}

/**
 * @brief Set or clear a telemetry flag in a raw flag field.
 * @param flags Raw telemetry flag field.
 * @param flag Flag to update.
 * @param set true to set the flag, false to clear it.
 * @return Updated raw telemetry flag field.
 */
static inline constexpr std::uint16_t ble_telemetry_flag_update(
    std::uint16_t flags,
    ble_telemetry_flag_t flag,
    bool set)
{
	if (set)
	{
		return static_cast<std::uint16_t>(
		    flags | ble_telemetry_flag_mask(flag));
	}

	return static_cast<std::uint16_t>(
	    flags & static_cast<std::uint16_t>(~ble_telemetry_flag_mask(flag)));
}

/**
 * @brief Test whether a control flag is set in a raw flag field.
 * @param flags Raw control flag field.
 * @param flag Flag to test.
 * @return true if the flag is set, otherwise false.
 */
static inline constexpr bool ble_control_flag_is_set(
    std::uint16_t flags,
    ble_control_flag_t flag)
{
	return (flags & ble_control_flag_mask(flag)) != 0u;
}

/**
 * @brief Set or clear a control flag in a raw flag field.
 * @param flags Raw control flag field.
 * @param flag Flag to update.
 * @param set true to set the flag, false to clear it.
 * @return Updated raw control flag field.
 */
static inline constexpr std::uint16_t ble_control_flag_update(
    std::uint16_t flags,
    ble_control_flag_t flag,
    bool set)
{
	if (set)
	{
		return static_cast<std::uint16_t>(
		    flags | ble_control_flag_mask(flag));
	}

	return static_cast<std::uint16_t>(
	    flags & static_cast<std::uint16_t>(~ble_control_flag_mask(flag)));
}

/**
 * @brief Validate a raw protocol version value.
 * @param protocol_version Raw protocol version byte.
 * @return true if the version is supported, otherwise false.
 */
static inline constexpr bool ble_validate_protocol_version(
    std::uint8_t protocol_version)
{
	return protocol_version ==
	       static_cast<std::uint8_t>(ble_protocol_version_t::v1);
}

/**
 * @brief Validate a typed protocol version value.
 * @param protocol_version Typed protocol version.
 * @return true if the version is supported, otherwise false.
 */
static inline constexpr bool ble_validate_protocol_version(
    ble_protocol_version_t protocol_version)
{
	return protocol_version == ble_protocol_version_t::v1;
}

/**
 * @brief Validate a raw node identifier.
 * @param node_id Raw node identifier.
 * @return true if the node ID is defined in protocol v1, otherwise false.
 */
static inline constexpr bool ble_validate_node_id(std::uint8_t node_id)
{
	return (node_id == static_cast<std::uint8_t>(ble_node_id_t::control)) ||
	       (node_id == static_cast<std::uint8_t>(ble_node_id_t::sn1)) ||
	       (node_id == static_cast<std::uint8_t>(ble_node_id_t::sn2));
}

/**
 * @brief Validate a typed node identifier.
 * @param node_id Typed node identifier.
 * @return true if the node ID is defined in protocol v1, otherwise false.
 */
static inline constexpr bool ble_validate_node_id(ble_node_id_t node_id)
{
	return ble_validate_node_id(static_cast<std::uint8_t>(node_id));
}

/**
 * @brief Validate that a node ID refers to SN1 or SN2.
 * @param node_id Raw node identifier.
 * @return true if the node is a sensor node, otherwise false.
 */
static inline constexpr bool ble_validate_sensor_node_id(std::uint8_t node_id)
{
	return (node_id == static_cast<std::uint8_t>(ble_node_id_t::sn1)) ||
	       (node_id == static_cast<std::uint8_t>(ble_node_id_t::sn2));
}

/**
 * @brief Validate that a node ID refers to SN1 or SN2.
 * @param node_id Typed node identifier.
 * @return true if the node is a sensor node, otherwise false.
 */
static inline constexpr bool ble_validate_sensor_node_id(ble_node_id_t node_id)
{
	return ble_validate_sensor_node_id(static_cast<std::uint8_t>(node_id));
}

/**
 * @brief Validate a raw event type value.
 * @param event_type Raw event type byte.
 * @return true if the event type is defined in protocol v1, otherwise false.
 */
static inline constexpr bool ble_validate_event_type(std::uint8_t event_type)
{
	switch (event_type)
	{
	case static_cast<std::uint8_t>(ble_event_type_t::help_pressed):
	case static_cast<std::uint8_t>(ble_event_type_t::help_released):
	case static_cast<std::uint8_t>(ble_event_type_t::local_button_pressed):
	case static_cast<std::uint8_t>(ble_event_type_t::local_button_released):
	case static_cast<std::uint8_t>(ble_event_type_t::sensor_fault_asserted):
	case static_cast<std::uint8_t>(ble_event_type_t::sensor_fault_cleared):
	case static_cast<std::uint8_t>(ble_event_type_t::clear_acknowledged):
	case static_cast<std::uint8_t>(ble_event_type_t::sound_detected):
	case static_cast<std::uint8_t>(ble_event_type_t::motion_detected):
		return true;

	default:
		return false;
	}
}

/**
 * @brief Validate a typed event type.
 * @param event_type Typed event type.
 * @return true if the event type is defined in protocol v1, otherwise false.
 */
static inline constexpr bool ble_validate_event_type(
    ble_event_type_t event_type)
{
	return ble_validate_event_type(static_cast<std::uint8_t>(event_type));
}

/**
 * @brief Validate a canonical actuator duty value.
 * @param duty_u8 Duty request in the protocol 0..255 range.
 * @return true if the value is within 0..255 inclusive, otherwise false.
 */
static inline constexpr bool ble_validate_duty_u8(std::uint16_t duty_u8)
{
	return duty_u8 <= ble_protocol_constants_t::duty_u8_max;
}

/**
 * @brief Clamp a canonical actuator duty value to the valid 0..255 range.
 * @param duty_u8 Duty request in the protocol 0..255 range.
 * @return Clamped duty value.
 */
static inline constexpr std::uint8_t ble_clamp_duty_u8(std::uint16_t duty_u8)
{
	return ble_validate_duty_u8(duty_u8)
		   ? static_cast<std::uint8_t>(duty_u8)
		   : ble_protocol_constants_t::duty_u8_max;
}

/**
 * @brief Validate a raw telemetry flag field against protocol v1.
 * @param flags Raw telemetry flag field.
 * @return true if no unknown bits are set, otherwise false.
 */
static inline constexpr bool ble_validate_telemetry_flags(
    std::uint16_t flags)
{
	return (flags & static_cast<std::uint16_t>(
			    ~ble_telemetry_flag_known_mask())) == 0u;
}

/**
 * @brief Validate a raw control flag field against protocol v1.
 * @param flags Raw control flag field.
 * @return true if no unknown bits are set, otherwise false.
 */
static inline constexpr bool ble_validate_control_flags(std::uint16_t flags)
{
	return (flags & static_cast<std::uint16_t>(
			    ~ble_control_flag_known_mask())) == 0u;
}

/**
 * @brief Write a 16-bit unsigned integer in little-endian order.
 * @param dst Destination buffer with at least two writable bytes.
 * @param value Value to write.
 *
 * If `dst` is null, the function does nothing.
 */
static inline void ble_write_u16_le(std::uint8_t *dst, std::uint16_t value)
{
	if (dst == nullptr)
	{
		return;
	}

	dst[0] = static_cast<std::uint8_t>(value & 0xFFu);
	dst[1] = static_cast<std::uint8_t>((value >> 8) & 0xFFu);
}

/**
 * @brief Write a 16-bit signed integer in little-endian order.
 * @param dst Destination buffer with at least two writable bytes.
 * @param value Value to write.
 *
 * If `dst` is null, the function does nothing.
 */
static inline void ble_write_i16_le(std::uint8_t *dst, std::int16_t value)
{
	ble_write_u16_le(dst, static_cast<std::uint16_t>(value));
}

/**
 * @brief Read a 16-bit unsigned integer in little-endian order.
 * @param src Source buffer with at least two readable bytes.
 * @return Parsed value, or zero if `src` is null.
 */
static inline std::uint16_t ble_read_u16_le(const std::uint8_t *src)
{
	if (src == nullptr)
	{
		return 0u;
	}

	return static_cast<std::uint16_t>(
	    static_cast<std::uint16_t>(src[0]) |
	    static_cast<std::uint16_t>(
		static_cast<std::uint16_t>(src[1]) << 8));
}

/**
 * @brief Read a 16-bit signed integer in little-endian order.
 * @param src Source buffer with at least two readable bytes.
 * @return Parsed value, or zero if `src` is null.
 */
static inline std::int16_t ble_read_i16_le(const std::uint8_t *src)
{
	return static_cast<std::int16_t>(ble_read_u16_le(src));
}

/**
 * @brief Build a telemetry packet with a valid protocol version and clamped
 *        actuator values.
 * @param node_id Source sensor node ID.
 * @param sequence Telemetry sequence number.
 * @param flags Raw telemetry flag field.
 * @param temperature_centi_c Temperature in signed centi-degC.
 * @param light_deci_lux Light in unsigned deci-lux.
 * @param fan_duty_applied Current fan duty applied by the node in the 0..255
 *        range.
 * @param led_duty_applied Current LED duty applied by the node in the 0..255
 *        range.
 * @param status_word Snapshot status word.
 * @return Initialised telemetry packet.
 */
static inline telemetry_packet_t ble_make_telemetry(
    ble_node_id_t node_id,
    std::uint8_t sequence,
    std::uint16_t flags,
    std::int16_t temperature_centi_c,
    std::uint16_t light_deci_lux,
    std::uint16_t fan_duty_applied,
    std::uint16_t led_duty_applied,
    std::uint16_t status_word)
{
	telemetry_packet_t packet{};

	packet.protocol_version = ble_protocol_version_t::v1;
	packet.node_id = node_id;
	packet.sequence = sequence;
	packet.flags = flags;
	packet.temperature_centi_c = temperature_centi_c;
	packet.light_deci_lux = light_deci_lux;
	packet.fan_duty_applied = ble_clamp_duty_u8(fan_duty_applied);
	packet.led_duty_applied = ble_clamp_duty_u8(led_duty_applied);
	packet.status_word = status_word;

	return packet;
}

/**
 * @brief Build an event packet with a valid protocol version.
 * @param node_id Source sensor node ID.
 * @param sequence Event sequence number.
 * @param event_type Event type.
 * @param event_value Event-specific signed value.
 * @param timestamp_ms_mod Timestamp modulo 65536 ms.
 * @return Initialised event packet.
 */
static inline event_packet_t ble_make_event(
    ble_node_id_t node_id,
    std::uint8_t sequence,
    ble_event_type_t event_type,
    std::int16_t event_value,
    std::uint16_t timestamp_ms_mod)
{
	event_packet_t packet{};

	packet.protocol_version = ble_protocol_version_t::v1;
	packet.node_id = node_id;
	packet.sequence = sequence;
	packet.event_type = event_type;
	packet.event_value = event_value;
	packet.timestamp_ms_mod = timestamp_ms_mod;

	return packet;
}

/**
 * @brief Build a control packet with a valid protocol version.
 * @param target_node_id Target sensor node ID.
 * @param command_sequence Command sequence number.
 * @param command_flags Raw control flag field.
 * @param fan_duty_override Fan override duty in the 0..255 range.
 * @param led_duty_override LED override duty in the 0..255 range.
 * @param reserved Reserved field, which should be zero in protocol v1.
 * @return Initialised control packet.
 */
static inline control_packet_t ble_make_control(
    ble_node_id_t target_node_id,
    std::uint8_t command_sequence,
    std::uint16_t command_flags,
    std::uint16_t fan_duty_override,
    std::uint16_t led_duty_override,
    std::uint16_t reserved = ble_protocol_constants_t::
	reserved_field_must_be_zero)
{
	control_packet_t packet{};
	const bool override_enabled = ble_control_flag_is_set(
	    command_flags, ble_control_flag_t::actuator_override_enable);

	packet.protocol_version = ble_protocol_version_t::v1;
	packet.target_node_id = target_node_id;
	packet.command_sequence = command_sequence;
	packet.command_flags = command_flags;
	packet.fan_duty_override =
	    override_enabled ? ble_clamp_duty_u8(fan_duty_override) : 0u;
	packet.led_duty_override =
	    override_enabled ? ble_clamp_duty_u8(led_duty_override) : 0u;
	packet.reserved = reserved;

	return packet;
}

/**
 * @brief Serialise a telemetry packet into its wire format.
 * @param dst Destination byte buffer.
 * @param dst_size Destination buffer size in bytes.
 * @param src Source telemetry packet.
 * @return true if the packet was written, otherwise false.
 *
 * On failure, the function returns false before writing any bytes.
 */
static inline bool ble_pack_telemetry(
    std::uint8_t *dst,
    std::size_t dst_size,
    const telemetry_packet_t *src)
{
	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (dst_size < ble_payload_size_t::telemetry)
	{
		return false;
	}

	if (!ble_validate_protocol_version(src->protocol_version) ||
	    !ble_validate_sensor_node_id(src->node_id) ||
	    !ble_validate_telemetry_flags(src->flags) ||
	    !ble_validate_duty_u8(src->fan_duty_applied) ||
	    !ble_validate_duty_u8(src->led_duty_applied))
	{
		return false;
	}

	dst[ble_telemetry_packet_layout_t::protocol_version_offset] =
	    static_cast<std::uint8_t>(src->protocol_version);
	dst[ble_telemetry_packet_layout_t::node_id_offset] =
	    static_cast<std::uint8_t>(src->node_id);
	dst[ble_telemetry_packet_layout_t::sequence_offset] = src->sequence;
	ble_write_u16_le(&dst[ble_telemetry_packet_layout_t::flags_offset],
			 src->flags);
	ble_write_i16_le(
	    &dst[ble_telemetry_packet_layout_t::temperature_centi_c_offset],
	    src->temperature_centi_c);
	ble_write_u16_le(
	    &dst[ble_telemetry_packet_layout_t::light_deci_lux_offset],
	    src->light_deci_lux);
	dst[ble_telemetry_packet_layout_t::fan_duty_applied_offset] =
	    src->fan_duty_applied;
	dst[ble_telemetry_packet_layout_t::led_duty_applied_offset] =
	    src->led_duty_applied;
	ble_write_u16_le(
	    &dst[ble_telemetry_packet_layout_t::status_word_offset],
	    src->status_word);

	return true;
}

/**
 * @brief Deserialise a telemetry packet from its wire format.
 * @param dst Destination telemetry packet.
 * @param src Source byte buffer.
 * @param src_size Source buffer size in bytes.
 * @return true if the packet was parsed successfully, otherwise false.
 *
 * On failure, `dst` is left unchanged.
 */
static inline bool ble_unpack_telemetry(
    telemetry_packet_t *dst,
    const std::uint8_t *src,
    std::size_t src_size)
{
	telemetry_packet_t packet{};

	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (src_size < ble_payload_size_t::telemetry)
	{
		return false;
	}

	packet.protocol_version = static_cast<ble_protocol_version_t>(
	    src[ble_telemetry_packet_layout_t::protocol_version_offset]);
	packet.node_id = static_cast<ble_node_id_t>(
	    src[ble_telemetry_packet_layout_t::node_id_offset]);
	packet.sequence = src[ble_telemetry_packet_layout_t::sequence_offset];
	packet.flags = ble_read_u16_le(
	    &src[ble_telemetry_packet_layout_t::flags_offset]);
	packet.temperature_centi_c = ble_read_i16_le(
	    &src[ble_telemetry_packet_layout_t::temperature_centi_c_offset]);
	packet.light_deci_lux = ble_read_u16_le(
	    &src[ble_telemetry_packet_layout_t::light_deci_lux_offset]);
	packet.fan_duty_applied =
	    src[ble_telemetry_packet_layout_t::fan_duty_applied_offset];
	packet.led_duty_applied =
	    src[ble_telemetry_packet_layout_t::led_duty_applied_offset];
	packet.status_word = ble_read_u16_le(
	    &src[ble_telemetry_packet_layout_t::status_word_offset]);

	if (!ble_validate_protocol_version(packet.protocol_version) ||
	    !ble_validate_sensor_node_id(packet.node_id) ||
	    !ble_validate_telemetry_flags(packet.flags) ||
	    !ble_validate_duty_u8(packet.fan_duty_applied) ||
	    !ble_validate_duty_u8(packet.led_duty_applied))
	{
		return false;
	}

	*dst = packet;
	return true;
}

/**
 * @brief Serialise an event packet into its wire format.
 * @param dst Destination byte buffer.
 * @param dst_size Destination buffer size in bytes.
 * @param src Source event packet.
 * @return true if the packet was written, otherwise false.
 *
 * On failure, the function returns false before writing any bytes.
 */
static inline bool ble_pack_event(
    std::uint8_t *dst,
    std::size_t dst_size,
    const event_packet_t *src)
{
	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (dst_size < ble_payload_size_t::event)
	{
		return false;
	}

	if (!ble_validate_protocol_version(src->protocol_version) ||
	    !ble_validate_sensor_node_id(src->node_id) ||
	    !ble_validate_event_type(src->event_type))
	{
		return false;
	}

	dst[ble_event_packet_layout_t::protocol_version_offset] =
	    static_cast<std::uint8_t>(src->protocol_version);
	dst[ble_event_packet_layout_t::node_id_offset] =
	    static_cast<std::uint8_t>(src->node_id);
	dst[ble_event_packet_layout_t::sequence_offset] = src->sequence;
	dst[ble_event_packet_layout_t::event_type_offset] =
	    static_cast<std::uint8_t>(src->event_type);
	ble_write_i16_le(&dst[ble_event_packet_layout_t::event_value_offset],
			 src->event_value);
	ble_write_u16_le(
	    &dst[ble_event_packet_layout_t::timestamp_ms_mod_offset],
	    src->timestamp_ms_mod);

	return true;
}

/**
 * @brief Deserialise an event packet from its wire format.
 * @param dst Destination event packet.
 * @param src Source byte buffer.
 * @param src_size Source buffer size in bytes.
 * @return true if the packet was parsed successfully, otherwise false.
 *
 * On failure, `dst` is left unchanged.
 */
static inline bool ble_unpack_event(
    event_packet_t *dst,
    const std::uint8_t *src,
    std::size_t src_size)
{
	event_packet_t packet{};

	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (src_size < ble_payload_size_t::event)
	{
		return false;
	}

	packet.protocol_version = static_cast<ble_protocol_version_t>(
	    src[ble_event_packet_layout_t::protocol_version_offset]);
	packet.node_id = static_cast<ble_node_id_t>(
	    src[ble_event_packet_layout_t::node_id_offset]);
	packet.sequence = src[ble_event_packet_layout_t::sequence_offset];
	packet.event_type = static_cast<ble_event_type_t>(
	    src[ble_event_packet_layout_t::event_type_offset]);
	packet.event_value = ble_read_i16_le(
	    &src[ble_event_packet_layout_t::event_value_offset]);
	packet.timestamp_ms_mod = ble_read_u16_le(
	    &src[ble_event_packet_layout_t::timestamp_ms_mod_offset]);

	if (!ble_validate_protocol_version(packet.protocol_version) ||
	    !ble_validate_sensor_node_id(packet.node_id) ||
	    !ble_validate_event_type(packet.event_type))
	{
		return false;
	}

	*dst = packet;
	return true;
}

/**
 * @brief Serialise a control packet into its wire format.
 * @param dst Destination byte buffer.
 * @param dst_size Destination buffer size in bytes.
 * @param src Source control packet.
 * @return true if the packet was written, otherwise false.
 *
 * On failure, the function returns false before writing any bytes.
 */
static inline bool ble_pack_control(
    std::uint8_t *dst,
    std::size_t dst_size,
    const control_packet_t *src)
{
	const bool override_enabled = (src != nullptr) &&
				      ble_control_flag_is_set(
					  src->command_flags,
					  ble_control_flag_t::
					      actuator_override_enable);

	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (dst_size < ble_payload_size_t::control)
	{
		return false;
	}

	if (!ble_validate_protocol_version(src->protocol_version) ||
	    !ble_validate_sensor_node_id(src->target_node_id) ||
	    !ble_validate_control_flags(src->command_flags) ||
	    !ble_validate_duty_u8(src->fan_duty_override) ||
	    !ble_validate_duty_u8(src->led_duty_override) ||
	    (src->reserved != ble_protocol_constants_t::reserved_field_must_be_zero))
	{
		return false;
	}

	if ((!override_enabled) &&
	    ((src->fan_duty_override != 0u) || (src->led_duty_override != 0u)))
	{
		return false;
	}

	dst[ble_control_packet_layout_t::protocol_version_offset] =
	    static_cast<std::uint8_t>(src->protocol_version);
	dst[ble_control_packet_layout_t::target_node_id_offset] =
	    static_cast<std::uint8_t>(src->target_node_id);
	dst[ble_control_packet_layout_t::command_sequence_offset] =
	    src->command_sequence;
	ble_write_u16_le(&dst[ble_control_packet_layout_t::command_flags_offset],
			 src->command_flags);
	dst[ble_control_packet_layout_t::fan_duty_override_offset] =
	    src->fan_duty_override;
	dst[ble_control_packet_layout_t::led_duty_override_offset] =
	    src->led_duty_override;
	ble_write_u16_le(&dst[ble_control_packet_layout_t::reserved_offset],
			 src->reserved);

	return true;
}

/**
 * @brief Deserialise a control packet from its wire format.
 * @param dst Destination control packet.
 * @param src Source byte buffer.
 * @param src_size Source buffer size in bytes.
 * @return true if the packet was parsed successfully, otherwise false.
 *
 * On failure, `dst` is left unchanged.
 */
static inline bool ble_unpack_control(
    control_packet_t *dst,
    const std::uint8_t *src,
    std::size_t src_size)
{
	control_packet_t packet{};
	bool override_enabled = false;

	if ((dst == nullptr) || (src == nullptr))
	{
		return false;
	}

	if (src_size < ble_payload_size_t::control)
	{
		return false;
	}

	packet.protocol_version = static_cast<ble_protocol_version_t>(
	    src[ble_control_packet_layout_t::protocol_version_offset]);
	packet.target_node_id = static_cast<ble_node_id_t>(
	    src[ble_control_packet_layout_t::target_node_id_offset]);
	packet.command_sequence =
	    src[ble_control_packet_layout_t::command_sequence_offset];
	packet.command_flags = ble_read_u16_le(
	    &src[ble_control_packet_layout_t::command_flags_offset]);
	packet.fan_duty_override =
	    src[ble_control_packet_layout_t::fan_duty_override_offset];
	packet.led_duty_override =
	    src[ble_control_packet_layout_t::led_duty_override_offset];
	packet.reserved = ble_read_u16_le(
	    &src[ble_control_packet_layout_t::reserved_offset]);

	override_enabled = ble_control_flag_is_set(
	    packet.command_flags,
	    ble_control_flag_t::actuator_override_enable);

	if (!ble_validate_protocol_version(packet.protocol_version) ||
	    !ble_validate_sensor_node_id(packet.target_node_id) ||
	    !ble_validate_control_flags(packet.command_flags) ||
	    !ble_validate_duty_u8(packet.fan_duty_override) ||
	    !ble_validate_duty_u8(packet.led_duty_override) ||
	    (packet.reserved !=
	     ble_protocol_constants_t::reserved_field_must_be_zero))
	{
		return false;
	}

	if ((!override_enabled) &&
	    ((packet.fan_duty_override != 0u) ||
	     (packet.led_duty_override != 0u)))
	{
		return false;
	}

	*dst = packet;
	return true;
}

/**
 * @brief Determine whether `new_seq` is newer than `last_seq` for an unsigned
 *        8-bit wrapping sequence.
 * @param new_seq Candidate sequence number.
 * @param last_seq Previously accepted sequence number.
 * @return true if `new_seq` is considered newer, otherwise false.
 *
 * The rule is modulo-256 half-range comparison:
 * - delta = (new_seq - last_seq) mod 256
 * - newer when delta is in the range 1..127
 * - duplicate when delta is 0
 * - old or ambiguous when delta is in the range 128..255
 */
static inline constexpr bool ble_sequence_is_newer_u8(
    std::uint8_t new_seq,
    std::uint8_t last_seq)
{
	const std::uint8_t delta =
	    static_cast<std::uint8_t>(new_seq - last_seq);

	return (delta != 0u) && (delta < 0x80u);
}

/**
 * @brief Decide whether an event packet should be accepted.
 * @param new_seq Candidate event sequence number.
 * @param last_seq Previously accepted event sequence number.
 * @param last_seq_valid true if `last_seq` is valid, false for first packet.
 * @return true if the event should be accepted, otherwise false.
 */
static inline constexpr bool ble_event_should_accept(
    std::uint8_t new_seq,
    std::uint8_t last_seq,
    bool last_seq_valid)
{
	return last_seq_valid ? ble_sequence_is_newer_u8(new_seq, last_seq) : true;
}

/**
 * @brief Decide whether a control packet should be accepted.
 * @param new_seq Candidate command sequence number.
 * @param last_seq Previously accepted command sequence number.
 * @param last_seq_valid true if `last_seq` is valid, false for first packet.
 * @return true if the command should be accepted, otherwise false.
 */
static inline constexpr bool ble_command_should_accept(
    std::uint8_t new_seq,
    std::uint8_t last_seq,
    bool last_seq_valid)
{
	return last_seq_valid ? ble_sequence_is_newer_u8(new_seq, last_seq) : true;
}

/**
 * @brief Known-good logical telemetry packet test vector.
 */
static inline constexpr telemetry_packet_t ble_test_telemetry_packet_vector_1{
    ble_protocol_version_t::v1,
    ble_node_id_t::sn2,
    0x7Au,
    static_cast<std::uint16_t>(
	ble_telemetry_flag_mask(ble_telemetry_flag_t::help_active) |
	ble_telemetry_flag_mask(ble_telemetry_flag_t::status_led_latched) |
	ble_telemetry_flag_mask(
	    ble_telemetry_flag_t::cn_actuator_control_selected)),
    2367,
    6425u,
    115u,
    64u,
    0x1234u};

/**
 * @brief Known-good telemetry packet wire bytes for vector 1.
 */
static inline constexpr std::uint8_t
    ble_test_telemetry_wire_vector_1[ble_payload_size_t::telemetry] = {
	0x01u, 0x02u, 0x7Au, 0x29u, 0x00u, 0x3Fu, 0x09u,
	0x19u, 0x19u, 0x73u, 0x40u, 0x34u, 0x12u};

/**
 * @brief Known-good logical event packet test vector.
 */
static inline constexpr event_packet_t ble_test_event_packet_vector_1{
    ble_protocol_version_t::v1,
    ble_node_id_t::sn1,
    0xD3u,
    ble_event_type_t::help_pressed,
    1,
    0xBEEFu};

/**
 * @brief Known-good event packet wire bytes for vector 1.
 */
static inline constexpr std::uint8_t
    ble_test_event_wire_vector_1[ble_payload_size_t::event] = {
	0x01u, 0x01u, 0xD3u, 0x01u, 0x01u, 0x00u, 0xEFu, 0xBEu};

/**
 * @brief Known-good logical control packet test vector.
 */
static inline constexpr control_packet_t ble_test_control_packet_vector_1{
    ble_protocol_version_t::v1,
    ble_node_id_t::sn2,
    0x2Au,
    static_cast<std::uint16_t>(
	ble_control_flag_mask(ble_control_flag_t::actuator_override_enable) |
	ble_control_flag_mask(ble_control_flag_t::clear_help_request) |
	ble_control_flag_mask(
	    ble_control_flag_t::clear_status_led_request)),
    115u,
    64u,
    0u};

/**
 * @brief Known-good control packet wire bytes for vector 1.
 */
static inline constexpr std::uint8_t
    ble_test_control_wire_vector_1[ble_payload_size_t::control] = {
	0x01u, 0x02u, 0x2Au, 0x07u, 0x00u, 0x73u, 0x40u, 0x00u, 0x00u};

/**
 * @brief Compare two raw byte buffers for equality.
 * @param lhs First buffer.
 * @param rhs Second buffer.
 * @param length Number of bytes to compare.
 * @return true if all bytes match, otherwise false.
 */
static inline bool ble_test_buffer_equal(
    const std::uint8_t *lhs,
    const std::uint8_t *rhs,
    std::size_t length)
{
	std::size_t index = 0u;

	if ((lhs == nullptr) || (rhs == nullptr))
	{
		return false;
	}

	for (index = 0u; index < length; ++index)
	{
		if (lhs[index] != rhs[index])
		{
			return false;
		}
	}

	return true;
}

/**
 * @brief Compare two logical telemetry packets for equality.
 * @param lhs First packet.
 * @param rhs Second packet.
 * @return true if every field matches, otherwise false.
 */
static inline bool ble_test_telemetry_equal(
    const telemetry_packet_t &lhs,
    const telemetry_packet_t &rhs)
{
	return (lhs.protocol_version == rhs.protocol_version) &&
	       (lhs.node_id == rhs.node_id) &&
	       (lhs.sequence == rhs.sequence) && (lhs.flags == rhs.flags) &&
	       (lhs.temperature_centi_c == rhs.temperature_centi_c) &&
	       (lhs.light_deci_lux == rhs.light_deci_lux) &&
	       (lhs.fan_duty_applied == rhs.fan_duty_applied) &&
	       (lhs.led_duty_applied == rhs.led_duty_applied) &&
	       (lhs.status_word == rhs.status_word);
}

/**
 * @brief Compare two logical event packets for equality.
 * @param lhs First packet.
 * @param rhs Second packet.
 * @return true if every field matches, otherwise false.
 */
static inline bool ble_test_event_equal(
    const event_packet_t &lhs,
    const event_packet_t &rhs)
{
	return (lhs.protocol_version == rhs.protocol_version) &&
	       (lhs.node_id == rhs.node_id) &&
	       (lhs.sequence == rhs.sequence) &&
	       (lhs.event_type == rhs.event_type) &&
	       (lhs.event_value == rhs.event_value) &&
	       (lhs.timestamp_ms_mod == rhs.timestamp_ms_mod);
}

/**
 * @brief Compare two logical control packets for equality.
 * @param lhs First packet.
 * @param rhs Second packet.
 * @return true if every field matches, otherwise false.
 */
static inline bool ble_test_control_equal(
    const control_packet_t &lhs,
    const control_packet_t &rhs)
{
	return (lhs.protocol_version == rhs.protocol_version) &&
	       (lhs.target_node_id == rhs.target_node_id) &&
	       (lhs.command_sequence == rhs.command_sequence) &&
	       (lhs.command_flags == rhs.command_flags) &&
	       (lhs.fan_duty_override == rhs.fan_duty_override) &&
	       (lhs.led_duty_override == rhs.led_duty_override) &&
	       (lhs.reserved == rhs.reserved);
}

/**
 * @brief Pack telemetry vector 1 and compare the bytes to the known-good
 *        telemetry wire format.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_pack_telemetry_vector_1(void)
{
	std::uint8_t buffer[ble_payload_size_t::telemetry] = {};

	return ble_pack_telemetry(
		   buffer,
		   ble_payload_size_t::telemetry,
		   &ble_test_telemetry_packet_vector_1) &&
	       ble_test_buffer_equal(
		   buffer,
		   ble_test_telemetry_wire_vector_1,
		   ble_payload_size_t::telemetry);
}

/**
 * @brief Unpack telemetry wire vector 1 and compare it to the known-good
 *        logical telemetry packet.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_unpack_telemetry_vector_1(void)
{
	telemetry_packet_t packet{};

	return ble_unpack_telemetry(
		   &packet,
		   ble_test_telemetry_wire_vector_1,
		   ble_payload_size_t::telemetry) &&
	       ble_test_telemetry_equal(
		   packet,
		   ble_test_telemetry_packet_vector_1);
}

/**
 * @brief Pack event vector 1 and compare the bytes to the known-good event
 *        wire format.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_pack_event_vector_1(void)
{
	std::uint8_t buffer[ble_payload_size_t::event] = {};

	return ble_pack_event(
		   buffer,
		   ble_payload_size_t::event,
		   &ble_test_event_packet_vector_1) &&
	       ble_test_buffer_equal(
		   buffer,
		   ble_test_event_wire_vector_1,
		   ble_payload_size_t::event);
}

/**
 * @brief Unpack event wire vector 1 and compare it to the known-good logical
 *        event packet.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_unpack_event_vector_1(void)
{
	event_packet_t packet{};

	return ble_unpack_event(
		   &packet,
		   ble_test_event_wire_vector_1,
		   ble_payload_size_t::event) &&
	       ble_test_event_equal(packet, ble_test_event_packet_vector_1);
}

/**
 * @brief Pack control vector 1 and compare the bytes to the known-good control
 *        wire format.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_pack_control_vector_1(void)
{
	std::uint8_t buffer[ble_payload_size_t::control] = {};

	return ble_pack_control(
		   buffer,
		   ble_payload_size_t::control,
		   &ble_test_control_packet_vector_1) &&
	       ble_test_buffer_equal(
		   buffer,
		   ble_test_control_wire_vector_1,
		   ble_payload_size_t::control);
}

/**
 * @brief Unpack control wire vector 1 and compare it to the known-good logical
 *        control packet.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_unpack_control_vector_1(void)
{
	control_packet_t packet{};

	return ble_unpack_control(
		   &packet,
		   ble_test_control_wire_vector_1,
		   ble_payload_size_t::control) &&
	       ble_test_control_equal(packet, ble_test_control_packet_vector_1);
}

/**
 * @brief Pack and unpack a telemetry packet and verify that all fields survive
 *        the round trip.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_round_trip_telemetry(void)
{
	const telemetry_packet_t source = ble_make_telemetry(
	    ble_node_id_t::sn1,
	    0xFFu,
	    static_cast<std::uint16_t>(
		ble_telemetry_flag_mask(ble_telemetry_flag_t::sensor_fault) |
		ble_telemetry_flag_mask(ble_telemetry_flag_t::button_pressed) |
		ble_telemetry_flag_mask(ble_telemetry_flag_t::motion_detected) |
		ble_telemetry_flag_mask(ble_telemetry_flag_t::sound_detected)),
	    -534,
	    10995u,
	    255u,
	    1u,
	    0xA55Au);
	std::uint8_t buffer[ble_payload_size_t::telemetry] = {};
	telemetry_packet_t decoded{};

	return ble_pack_telemetry(
		   buffer,
		   ble_payload_size_t::telemetry,
		   &source) &&
	       ble_unpack_telemetry(
		   &decoded,
		   buffer,
		   ble_payload_size_t::telemetry) &&
	       ble_test_telemetry_equal(source, decoded);
}

/**
 * @brief Pack and unpack an event packet and verify that all fields survive the
 *        round trip.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_round_trip_event(void)
{
	const event_packet_t source = ble_make_event(
	    ble_node_id_t::sn2,
	    0x00u,
	    ble_event_type_t::motion_detected,
	    -12,
	    0x0007u);
	std::uint8_t buffer[ble_payload_size_t::event] = {};
	event_packet_t decoded{};

	return ble_pack_event(buffer, ble_payload_size_t::event, &source) &&
	       ble_unpack_event(&decoded, buffer, ble_payload_size_t::event) &&
	       ble_test_event_equal(source, decoded);
}

/**
 * @brief Pack and unpack a control packet and verify that all fields survive
 *        the round trip.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_round_trip_control(void)
{
	const control_packet_t source = ble_make_control(
	    ble_node_id_t::sn1,
	    0x80u,
	    static_cast<std::uint16_t>(
		ble_control_flag_mask(
		    ble_control_flag_t::actuator_override_enable) |
		ble_control_flag_mask(
		    ble_control_flag_t::clear_status_led_request)),
	    200u,
	    77u);
	std::uint8_t buffer[ble_payload_size_t::control] = {};
	control_packet_t decoded{};

	return ble_pack_control(
		   buffer,
		   ble_payload_size_t::control,
		   &source) &&
	       ble_unpack_control(
		   &decoded,
		   buffer,
		   ble_payload_size_t::control) &&
	       ble_test_control_equal(source, decoded);
}

/**
 * @brief Verify the wraparound behaviour of the 8-bit sequence comparison
 *        helper.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_sequence_wraparound(void)
{
	return ble_sequence_is_newer_u8(0u, 255u) &&
	       ble_sequence_is_newer_u8(1u, 0u) &&
	       !ble_sequence_is_newer_u8(255u, 0u) &&
	       !ble_sequence_is_newer_u8(42u, 42u) &&
	       !ble_sequence_is_newer_u8(128u, 0u);
}

/**
 * @brief Verify duplicate event rejection and first-packet acceptance.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_duplicate_event_reject(void)
{
	return ble_event_should_accept(10u, 0u, false) &&
	       ble_event_should_accept(11u, 10u, true) &&
	       !ble_event_should_accept(10u, 10u, true) &&
	       !ble_event_should_accept(9u, 10u, true) &&
	       ble_event_should_accept(0u, 255u, true);
}

/**
 * @brief Verify duplicate command rejection and first-packet acceptance.
 * @return true on success, otherwise false.
 */
static inline bool ble_test_duplicate_command_reject(void)
{
	return ble_command_should_accept(90u, 0u, false) &&
	       ble_command_should_accept(91u, 90u, true) &&
	       !ble_command_should_accept(90u, 90u, true) &&
	       !ble_command_should_accept(89u, 90u, true) &&
	       ble_command_should_accept(0u, 255u, true);
}

/**
 * @brief Record a test result into an aggregate counter.
 * @param result Test summary to update.
 * @param passed true if the test passed, false if it failed.
 */
static inline void ble_test_record(
    ble_protocol_test_result_t *result,
    bool passed)
{
	if (result == nullptr)
	{
		return;
	}

	++result->tests_run;

	if (passed)
	{
		++result->tests_passed;
	}
	else
	{
		++result->tests_failed;
	}
}

/**
 * @brief Run all built-in protocol tests.
 * @return Aggregate test summary.
 */
static inline ble_protocol_test_result_t ble_run_all_protocol_tests(void)
{
	ble_protocol_test_result_t result{0u, 0u, 0u};

	ble_test_record(&result, ble_test_pack_telemetry_vector_1());
	ble_test_record(&result, ble_test_unpack_telemetry_vector_1());
	ble_test_record(&result, ble_test_pack_event_vector_1());
	ble_test_record(&result, ble_test_unpack_event_vector_1());
	ble_test_record(&result, ble_test_pack_control_vector_1());
	ble_test_record(&result, ble_test_unpack_control_vector_1());
	ble_test_record(&result, ble_test_round_trip_telemetry());
	ble_test_record(&result, ble_test_round_trip_event());
	ble_test_record(&result, ble_test_round_trip_control());
	ble_test_record(&result, ble_test_sequence_wraparound());
	ble_test_record(&result, ble_test_duplicate_event_reject());
	ble_test_record(&result, ble_test_duplicate_command_reject());

	return result;
}
