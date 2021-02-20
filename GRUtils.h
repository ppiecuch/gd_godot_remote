/* GRUtils.h */
#ifndef GRUTILS_H
#define GRUTILS_H

#include <vector>
#include <map>

#include "core/object.h"
#include "core/image.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/variant.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "main/input_default.h"

#include "GRLiterals.h"

#ifdef DEBUG_ENABLED

#define TimeCountInit() int simple_time_counter = OS::get_singleton()->get_ticks_usec()
#define TimeCountReset() simple_time_counter = OS::get_singleton()->get_ticks_usec()
// Shows delta between this and previous counter. Need to call TimeCountInit before
#define TimeCount(str)                                                                                                                                      \
	GRUtils::_log(str + String(": ") + String::num((OS::get_singleton()->get_ticks_usec() - simple_time_counter) / 1000.0, 3) + " ms", LogLevel::LL_DEBUG); \
	simple_time_counter = OS::get_singleton()->get_ticks_usec()

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(m_constant, m_name) \
	ClassDB::bind_integer_constant(get_class_static(), __constant_get_enum_name(m_constant, m_name), m_name, ((int)(m_constant)));

#else

#define TimeCountInit()
#define TimeCountReset()
#define TimeCount(str)

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(m_constant, m_name) \
	ClassDB::bind_integer_constant(get_class_static(), StringName(), m_name, ((int)(m_constant)));

#endif // DEBUG_ENABLED

#define LEAVE_IF_EDITOR()                          \
	if (Engine::get_singleton()->is_editor_hint()) \
		return;

#define newref(_class) Ref<_class>(memnew(_class))
#define max(x, y) (x > y ? x : y)
#define min(x, y) (x < y ? x : y)
#define _log(val, ll) log_str(val, ll, __FILE__, __LINE__)
#define is_vector_contains(vec, val) (std::find(vec.begin(), vec.end(), val) != vec.end())

#define GR_VERSION(x, y, z)                            \
	if (_grutils_data->internal_VERSION.size() == 0) { \
		_grutils_data->internal_VERSION.append(x);     \
		_grutils_data->internal_VERSION.append(y);     \
		_grutils_data->internal_VERSION.append(z);     \
	}

#define GR_PACKET_HEADER(a, b, c, d)                         \
	if (_grutils_data->internal_PACKET_HEADER.size() == 0) { \
		_grutils_data->internal_PACKET_HEADER.append(a);     \
		_grutils_data->internal_PACKET_HEADER.append(b);     \
		_grutils_data->internal_PACKET_HEADER.append(c);     \
		_grutils_data->internal_PACKET_HEADER.append(d);     \
	}

#define CON_ADDRESS(con) str(con->get_connected_host()) + ":" + str(con->get_connected_port())

// Get Project Setting
#define GET_PS(setting_name) \
	ProjectSettings::get_singleton()->get_setting(setting_name)
// Get Project Setting and set it to variable
#define GET_PS_SET(variable_to_store, setting_name) \
	variable_to_store = ProjectSettings::get_singleton()->get_setting(setting_name)

#define THREAD_FUNC static
#define THREAD_DATA void *

enum LogLevel {
	LL_DEBUG = __LL_DEBUG,
	LL_NORMAL = __LL_NORMAL,
	LL_WARNING = __LL_WARNING,
	LL_ERROR = __LL_ERROR,
	LL_NONE,
};

namespace GRUtils {
// DEFINES

class GRUtilsData : public Object {
	GDCLASS(GRUtilsData, Object);

public:
	int current_loglevel;
	PoolByteArray internal_PACKET_HEADER;
	PoolByteArray internal_VERSION;
};

#ifndef NO_GODOTREMOTE_SERVER
class GRUtilsDataServer {
public:
	PoolByteArray compress_buffer;
	int compress_buffer_size_mb;
};

extern GRUtilsDataServer *_grutils_data_server;
#endif

extern GRUtilsData *_grutils_data;

extern void init();
extern void deinit();

#ifndef NO_GODOTREMOTE_SERVER
extern void init_server_utils();
extern void deinit_server_utils();
extern PoolByteArray compress_buffer;
extern int compress_buffer_size_mb;
extern Error compress_jpg(PoolByteArray &ret, const PoolByteArray &img_data, int width, int height, int bytes_for_color = 4, int quality = 75, int subsampling = __SUBSAMPLING_H2V2);
#endif

extern Error compress_bytes(const PoolByteArray &bytes, PoolByteArray &res, int type);
extern Error decompress_bytes(const PoolByteArray &bytes, int output_size, PoolByteArray &res, int type);
extern void log_str(const Variant &val, int lvl = __LL_NORMAL, String file = "", int line = 0);

extern String str(const Variant &val);
extern String str_arr(const Array arr, const bool force_full = false, const int max_shown_items = 32, String separator = ", ");
extern String str_arr(const Dictionary arr, const bool force_full = false, const int max_shown_items = 32, String separator = ", ");
extern String str_arr(const uint8_t *data, const int size, const bool force_full = false, const int max_shown_items = 64, String separator = ", ");

extern bool validate_packet(const uint8_t *data);
extern bool validate_version(const PoolByteArray &data);
extern bool validate_version(const uint8_t *data);

extern bool compare_pool_byte_arrays(const PoolByteArray &a, const PoolByteArray &b);

extern void set_gravity(const Vector3 &p_gravity);
extern void set_accelerometer(const Vector3 &p_accel);
extern void set_magnetometer(const Vector3 &p_magnetometer);
extern void set_gyroscope(const Vector3 &p_gyroscope);

// LITERALS

// conversion from usec to msec. most useful to OS::delay_usec()
constexpr uint32_t operator"" _ms(unsigned long long val) {
	return val * 1000;
}

// IMPLEMENTATINS

template <class T>
extern String str_arr(const std::vector<T> arr, const bool force_full = false, const int max_shown_items = 64, String separator = ", ") {
	String res = "[ ";
	int s = arr.size();
	bool is_long = false;
	if (s > max_shown_items && !force_full) {
		s = max_shown_items;
		is_long = true;
	}
	for (int i = 0; i < s; i++) {
		res += str(arr[i]);
		if (i != s - 1 || is_long) {
			res += separator;
		}
	}

	if (is_long) {
		res += str(int64_t(arr.size()) - s) + " more items...";
	}

	return res + " ]";
};

template <class K, class V>
extern String str_arr(const std::map<K, V> arr, const bool force_full = false, const int max_shown_items = 32, String separator = ", ") {
	String res = "{ ";
	int s = (int)arr.size();
	bool is_long = false;
	if (s > max_shown_items && !force_full) {
		s = max_shown_items;
		is_long = true;
	}

	int i = 0;
	for (auto p : arr) {
		if (i++ >= s)
			break;
		res += str(p.first) + " : " + str(p.second);
		if (i != s - 1 || is_long) {
			res += separator;
		}
	}

	if (is_long) {
		res += String::num_int64(int64_t(arr.size()) - s) + " more items...";
	}

	return res + " }";
}

template <class T>
static String str_arr(PoolVector<T> arr, const bool force_full = false, const int max_shown_items = 64, String separator = ", ") {
	String res = "[ ";
	int s = arr.size();
	bool is_long = false;
	if (s > max_shown_items && !force_full) {
		s = max_shown_items;
		is_long = true;
	}

	auto r = arr.read();
	for (int i = 0; i < s; i++) {
		res += str(r[i]);
		if (i != s - 1 || is_long) {
			res += separator;
		}
	}
	r.release();

	if (is_long) {
		res += str(int64_t(arr.size()) - s) + " more items...";
	}

	return res + " ]";
};

static inline PoolByteArray get_packet_header() {
	return _grutils_data->internal_PACKET_HEADER;
}

static inline PoolByteArray get_gr_version() {
	return _grutils_data->internal_VERSION;
}

static inline void set_log_level(int lvl) {
	_grutils_data->current_loglevel = lvl;
}

template <class T>
inline void vec_remove_idx(std::vector<T> &v, const T &item) {
	v.erase(std::remove(v.begin(), v.end(), item), v.end());
}

template <class K, class V>
static Dictionary map_to_dict(std::map<K, V> m) {
	Dictionary res;
	for (auto p : m) {
		res[p.first] = p.second;
	}
	return res;
}

template <class K, class V>
static std::map<K, V> dict_to_map(Dictionary d) {
	std::map<K, V> res;
	Array keys = d.keys();
	Array values = d.values();
	for (int i = 0; i < keys.size(); i++) {
		res[keys[i]] = values[i];
	}
	keys.clear();
	values.clear();
	return res;
}

template <class V>
static Array vec_to_arr(std::vector<V> v) {
	Array res;
	res.resize((int)v.size());
	for (int i = 0; i < v.size(); i++) {
		res[i] = v[i];
	}
	return res;
}

template <class V>
static std::vector<V> arr_to_vec(Array a) {
	std::vector<V> res;
	res.resize(a.size());
	for (int i = 0; i < a.size(); i++) {
		res[i] = a[i];
	}
	return res;
}

extern Vector<Variant> vec_args(const std::vector<Variant> &args);

}; // namespace GRUtils

#endif // !GRUTILS_H
