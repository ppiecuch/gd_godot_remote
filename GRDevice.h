/* GRDevice.h */
#pragma once

#include <vector>

#include "GRLiterals.h"
#include "GRInputData.h"
#include "GRPacket.h"
#include "GRUtils.h"

#include "scene/main/node.h"

class GRDevice : public Node {
	GDCLASS(GRDevice, Node);

public:
	enum class AuthResult {
		OK = 0,
		ERROR = 1,
		TIMEOUT = 2,
		TRY_TO_CONNECT = 3,
		REFUSE_CONNECTION = 4,
		VERSION_MISMATCH = 5,
		INCORRECT_PASSWORD = 6,
		PASSWORD_REQUIRED = 7,
	};

	enum WorkingStatus {
		STATUS_STOPPED,
		STATUS_WORKING,
		STATUS_STOPPING,
		STATUS_STARTING,
	};

	enum TypesOfServerSettings {
		SERVER_SETTINGS_USE_INTERNAL = 0,
		SERVER_SETTINGS_VIDEO_STREAM_ENABLED = 1,
		SERVER_SETTINGS_COMPRESSION_TYPE = 2,
		SERVER_SETTINGS_JPG_QUALITY = 3,
		SERVER_SETTINGS_SKIP_FRAMES = 4,
		SERVER_SETTINGS_RENDER_SCALE = 5,
	};

	enum Subsampling {
		SUBSAMPLING_Y_ONLY = __SUBSAMPLING_Y_ONLY,
		SUBSAMPLING_H1V1 = __SUBSAMPLING_H1V1,
		SUBSAMPLING_H2V1 = __SUBSAMPLING_H2V1,
		SUBSAMPLING_H2V2 = __SUBSAMPLING_H2V2
	};

	enum ImageCompressionType {
		COMPRESSION_UNCOMPRESSED = __COMPRESSION_UNCOMPRESSED,
		COMPRESSION_JPG = __COMPRESSION_JPG,
		COMPRESSION_PNG = __COMPRESSION_PNG,
	};

private:
	WorkingStatus working_status = WorkingStatus::STATUS_STOPPED;

protected:
	template <class T>
	T _find_queued_packet_by_type() {
		for (unsigned i = 0; i < send_queue.size(); i++) {
			T o = send_queue[i];
			if (o.is_valid()) {
				return o;
			}
		}
		return T();
	}

	GRUtils::iterable_queue<uint64_t> fps_queue;
 	GRUtils::iterable_queue<uint64_t> ping_queue;
 	float avg_ping = 0, min_ping = 0, max_ping = 0;
 	float avg_fps = 0, min_fps = 0, max_fps = 0;
 	uint32_t avg_ping_max_count = 100;

	Mutex send_queue_mutex;
	std::vector<Ref<GRPacket> > send_queue;

	void set_status(WorkingStatus status);
	void _update_avg_ping(uint64_t ping);
	void _update_avg_fps(uint64_t frametime);
	static float _ping_calc_modifier(double i);
 	static float _fps_calc_modifier(double i);
	void _send_queue_resize(int new_size);
	Ref<GRPacket> _send_queue_pop_front();

	virtual void _reset_counters();
	virtual void _internal_call_only_deffered_start() {};
	virtual void _internal_call_only_deffered_stop() {};

	static void _bind_methods();
	void _notification(int p_notification);

public:
	uint16_t port = 51341;

	float get_avg_ping();
	float get_min_ping();
	float get_max_ping();
	float get_avg_fps();
	float get_min_fps();
	float get_max_fps();
	uint16_t get_port();
	void set_port(uint16_t _port);

	void send_packet(Ref<GRPacket> packet);
	void send_user_data(Variant packet_id, Variant user_data, bool full_objects = false);

	void start();
	void stop();
	void restart();
	void _internal_call_only_deffered_restart();

	virtual WorkingStatus get_status();

	void _init();
	void _deinit();
};

VARIANT_ENUM_CAST(GRDevice::WorkingStatus)
VARIANT_ENUM_CAST(GRDevice::Subsampling)
VARIANT_ENUM_CAST(GRDevice::ImageCompressionType)
VARIANT_ENUM_CAST(GRDevice::TypesOfServerSettings)
