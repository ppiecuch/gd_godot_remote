/* GodotRemote.h */
#pragma once

#include "GRDevice.h"
#include "GRNotifications.h"
#include "GRUtils.h"
#include "core/image.h"
#include "core/pool_vector.h"
#include "core/reference.h"

using namespace GRUtils;

class GodotRemote : public Object {
	GDCLASS(GodotRemote, Object);

	friend class GRDevice;
	static GodotRemote *singleton;

public:
	enum DeviceType {
		DEVICE_AUTO = 0,
		DEVICE_SERVER = 1,
		DEVICE_CLIENT = 2,
	};

	enum LogLevel {
		LL_DEBUG = __LL_DEBUG,
		LL_NORMAL = __LL_NORMAL,
		LL_WARNING = __LL_WARNING,
		LL_ERROR = __LL_ERROR,
		LL_NONE,
	};

	static String ps_general_autoload_name;
	static String ps_general_port_name;
	static String ps_general_loglevel_name;

	static String ps_notifications_enabled_name;
	static String ps_noticications_position_name;
	static String ps_notifications_duration_name;

	static String ps_server_config_adb_name;
	static String ps_server_stream_skip_frames_name;
	static String ps_server_stream_enabled_name;
	static String ps_server_compression_type_name;
	static String ps_server_jpg_quality_name;
	static String ps_server_jpg_buffer_mb_size_name;
	static String ps_server_auto_adjust_scale_name;
	static String ps_server_scale_of_sending_stream_name;
	static String ps_server_password_name;

	static String ps_server_custom_input_scene_name;
	static String ps_server_custom_input_scene_compressed_name;
	static String ps_server_custom_input_scene_compression_type_name;

private:
	bool is_autostart = false;
	bool is_notifications_enabled = true;

	class GRDevice *device = nullptr;

	void register_and_load_settings();

	void _create_notification_manager();
	void _remove_notifications_manager();

#ifdef TOOLS_ENABLED
	int64_t adb_pid = 0;
	class Timer *adb_start_timer = nullptr;

	void _prepare_editor();
	void _run_emitted();
	void _adb_port_forwarding();
	void _adb_start_timer_timeout();
#endif

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	// GRNotifications
	class GRNotificationPanel *get_notification(String title) const;
	Array get_all_notifications() const;
	Array get_notifications_with_title(String title) const;

	void set_notifications_layer(int layer) const;
	int get_notifications_layer() const;

	void set_notifications_position(GRNotifications::NotificationsPosition positon) const;
	GRNotifications::NotificationsPosition get_notifications_position() const;

	void set_notifications_enabled(bool _enabled) const;
	bool get_notifications_enabled() const;

	void set_notifications_duration(float _duration) const;
	float get_notifications_duration() const;

	void set_notifications_style(Ref<class GRNotificationStyle> _style) const;
	Ref<class GRNotificationStyle> get_notifications_style() const;

	void add_notification_or_append_string(String title, String text, GRNotifications::NotificationIcon icon, bool new_string = true, float duration_multiplier = 1.f) const;
	void add_notification_or_update_line(String title, String id, String text, GRNotifications::NotificationIcon icon, float duration_multiplier = 1.f) const;
	void add_notification(String title, String text, GRNotifications::NotificationIcon icon, bool update_existing = true, float duration_multiplier = 1.f) const;
	void remove_notification(String title, bool all_entries = true) const;
	void remove_notification_exact(Node *_notif) const;
	void clear_notifications() const;
	// GRNotifications end

	// GRUtils functions binds for GDScript
	void set_log_level(LogLevel lvl) const;
	void set_gravity(const Vector3 &p_gravity) const;
	void set_accelerometer(const Vector3 &p_accel) const;
	void set_magnetometer(const Vector3 &p_magnetometer) const;
	void set_gyroscope(const Vector3 &p_gyroscope) const;
	// GRUtils end

	class GRDevice *get_device() const;
	class String get_version() const;

	// must be call_deffered
	void create_and_start_device(DeviceType type = DeviceType::DEVICE_AUTO);
	bool create_remote_device(DeviceType type = DeviceType::DEVICE_AUTO);
	bool start_remote_device();
	bool remove_remote_device();

	static GodotRemote *get_singleton();
	void _init();
	void _deinit();
};

VARIANT_ENUM_CAST(GodotRemote::DeviceType)
VARIANT_ENUM_CAST(GodotRemote::LogLevel)
