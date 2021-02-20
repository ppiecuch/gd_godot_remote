/* GRServer.h */
#pragma once

#ifndef NO_GODOTREMOTE_SERVER

#include "GRDevice.h"
#include "core/io/compression.h"
#include "core/io/stream_peer_tcp.h"
#include "core/io/tcp_server.h"
#include "modules/regex/regex.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"

class GRServer : public GRDevice {
	GDCLASS(GRServer, GRDevice);

private:
	class ListenerThreadParamsServer : public Object {
		GDCLASS(ListenerThreadParamsServer, Object);

	public:
		GRServer *dev = nullptr;
		Thread thread;
		bool stop_thread = false;
		bool finished = false;

		void close_thread() {
			stop_thread = true;
			thread.wait_to_finish();
		}

		~ListenerThreadParamsServer() {
			LEAVE_IF_EDITOR();
			close_thread();
		}
	};

	class ConnectionThreadParamsServer : public Object {
		GDCLASS(ConnectionThreadParamsServer, Object);

	public:
		String device_id = "";
		GRServer *dev = nullptr;
		Ref<PacketPeerStream> ppeer;
		Thread thread;
		bool break_connection = false;
		bool finished = false;

		void close_thread() {
			break_connection = true;
			thread.wait_to_finish();
		}

		~ConnectionThreadParamsServer() {
			LEAVE_IF_EDITOR();
			close_thread();
			if (ppeer.is_valid()) {
				ppeer.unref();
			}
		}
	};

private:
	Mutex connection_mutex;
	ListenerThreadParamsServer *server_thread_listen = nullptr;
	Ref<TCP_Server> tcp_server;
	class GRSViewport *resize_viewport = nullptr;
	int client_connected = 0;

	bool using_client_settings = false;
	bool using_client_settings_recently_updated = false;

	String password;
	String custom_input_scene;
	bool custom_input_scene_was_updated = false;
	bool auto_adjust_scale = false;

	bool custom_input_pck_compressed = true;
	Compression::Mode custom_input_pck_compression_type = Compression::MODE_FASTLZ;
	const String custom_input_scene_regex_resource_finder_pattern = "\\\"(res://.*?)\\\"";
	Ref<class RegEx> custom_input_scene_regex_resource_finder;

	float prev_avg_fps = 0;
	void _adjust_viewport_scale();

	void _load_settings();
	void _update_settings_from_client(const std::map<int, Variant> settings);
	void _remove_resize_viewport(Node *vp);
	void _on_grviewport_deleting();

	virtual void _reset_counters() override;

	THREAD_FUNC void _thread_listen(THREAD_DATA p_userdata);
	THREAD_FUNC void _thread_connection(THREAD_DATA p_userdata);

	static AuthResult _auth_client(GRServer *dev, Ref<PacketPeerStream> &ppeer, Dictionary &ret_data, bool refuse_connection = false);
	Ref<GRPacketCustomInputScene> _create_custom_input_pack(String _scene_path, bool compress = true, Compression::Mode compression_type = Compression::MODE_FASTLZ);
	void _scan_resource_for_dependencies_recursive(String _dir, std::vector<String> &_arr);

protected:
	virtual void _internal_call_only_deffered_start() override;
	virtual void _internal_call_only_deffered_stop() override;

	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_auto_adjust_scale(bool _val);
	bool is_auto_adjust_scale();
	void set_password(String _pass);
	String get_password();
	void set_custom_input_scene(String _scn);
	String get_custom_input_scene();
	void set_custom_input_scene_compressed(bool _is_compressed);
	bool is_custom_input_scene_compressed();
	void set_custom_input_scene_compression_type(int _type);
	int get_custom_input_scene_compression_type();

	// VIEWPORT
	bool set_video_stream_enabled(bool val);
	bool is_video_stream_enabled();
	bool set_compression_type(ImageCompressionType _type);
	ImageCompressionType get_compression_type();
	bool set_jpg_quality(int _quality);
	int get_jpg_quality();
	bool set_skip_frames(int fps);
	int get_skip_frames();
	bool set_render_scale(float _scale);
	float get_render_scale();
	// NOT VIEWPORT

	GRSViewport *get_gr_viewport();
	void force_update_custom_input_scene();

	void _init();
	void _deinit();
};

class GRSViewport : public Viewport {
	GDCLASS(GRSViewport, Viewport);
	friend GRServer;
	friend class ImgProcessingStorageViewport;
	_THREAD_SAFE_CLASS_;

public:
	class ImgProcessingStorageViewport : public Object {
		GDCLASS(ImgProcessingStorageViewport, Object);

	public:
		PoolByteArray ret_data;
		GRDevice::ImageCompressionType compression_type = GRDevice::ImageCompressionType::COMPRESSION_UNCOMPRESSED;
		int width, height, format;
		int bytes_in_color, jpg_quality;
		bool is_empty = false;

		void _init() {
			LEAVE_IF_EDITOR();
			ret_data = PoolByteArray();
		};

		void _deinit() {
			LEAVE_IF_EDITOR();
			ret_data.resize(0);
		}
	};

private:
	Thread _thread_process;
	Ref<Image> last_image;
	ImgProcessingStorageViewport *last_image_data = nullptr;

	void _close_thread();
	void _set_img_data(ImgProcessingStorageViewport *_data);
	void _on_renderer_deleting();

	THREAD_FUNC void _processing_thread(THREAD_DATA p_user);

protected:
	Viewport *main_vp = nullptr;
	class GRSViewportRenderer *renderer = nullptr;
	bool video_stream_enabled = true;
	float rendering_scale = 0.3f;
	float auto_scale = 0.5f;
	int jpg_quality = 80;
	int skip_frames = 0;
	GRDevice::ImageCompressionType compression_type = GRDevice::ImageCompressionType::COMPRESSION_UNCOMPRESSED;

	uint16_t frames_from_prev_image = 0;
	bool is_empty_image_sended = false;

	static void _bind_methods();
	void _notification(int p_notification);
	void _update_size();

public:
	ImgProcessingStorageViewport *get_last_compressed_image_data();
	bool has_compressed_image_data();
	void force_get_image();

	void set_video_stream_enabled(bool val);
	bool is_video_stream_enabled();
	void set_rendering_scale(float val);
	float get_rendering_scale();
	void set_compression_type(GRDevice::ImageCompressionType val);
	GRDevice::ImageCompressionType get_compression_type();
	void set_jpg_quality(int _quality);
	int get_jpg_quality();
	void set_skip_frames(int skip);
	int get_skip_frames();

	void _init();
	void _deinit();
};

class GRSViewportRenderer : public Control {
	GDCLASS(GRSViewportRenderer, Control);

protected:
	Viewport *vp = nullptr;

	static void _bind_methods();
	void _notification(int p_notification);

public:
	Ref<ViewportTexture> tex;

	void _init();
	void _deinit();
};

#endif // !NO_GODOTREMOTE_SERVER
