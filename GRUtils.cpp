/* GRUtils.cpp */

#include "GRUtils.h"
#include "GodotRemote.h"
#include "core/io/compression.h"

#ifndef NO_GODOTREMOTE_SERVER
// richgel999/jpeg-compressor: https://github.com/richgel999/jpeg-compressor
#include "jpge.h"
#endif

namespace GRUtils {
int current_loglevel = LogLevel::LL_Normal;

PoolByteArray internal_PACKET_HEADER = PoolByteArray();
PoolByteArray internal_VERSION = PoolByteArray();

#ifndef NO_GODOTREMOTE_SERVER
int compress_buffer_size_mb = 4;
PoolByteArray compress_buffer = PoolByteArray();
#endif

void init() {
	GR_PACKET_HEADER('G', 'R', 'H', 'D');
	GR_VERSION(1, 0, 0);

	GET_PS_SET(current_loglevel, GodotRemote::ps_general_loglevel_name);
}

void deinit() {
	internal_PACKET_HEADER.resize(0);
	internal_VERSION.resize(0);
}

#ifndef NO_GODOTREMOTE_SERVER
void init_server_utils() {
	GET_PS_SET(compress_buffer_size_mb, GodotRemote::ps_server_jpg_buffer_mb_size_name);
	compress_buffer.resize((1024 * 1024) * compress_buffer_size_mb);
}

void deinit_server_utils() {
	compress_buffer.resize(0);
}
#endif

void _log(const Variant &val, LogLevel lvl) {
#ifdef DEBUG_ENABLED
	if (lvl >= current_loglevel && lvl < LogLevel::LL_None) {
		if (lvl == LogLevel::LL_Error) {
			print_error("[GodotRemote Error] " + str(val));
		} else if (lvl == LogLevel::LL_Warning) {
			print_error("[GodotRemote Warning] " + str(val));
		} else {
			print_line("[GodotRemote] " + str(val));
		}
	}
#endif
}

String str_arr(const Array arr, const bool force_full, const int max_shown_items, String separator) {
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
		res += String::num_int64(int64_t(arr.size()) - s) + " more items...";
	}

	return res + " ]";
};

String str_arr(const Dictionary arr, const bool force_full, const int max_shown_items, String separator) {
	String res = "{ ";
	int s = arr.size();
	bool is_long = false;
	if (s > max_shown_items && !force_full) {
		s = max_shown_items;
		is_long = true;
	}

	for (int i = 0; i < s; i++) {
		res += str(arr.get_key_at_index(i)) + " : " + str(arr.get_value_at_index(i));
		if (i != s - 1 || is_long) {
			res += separator;
		}
	}

	if (is_long) {
		res += String::num_int64(int64_t(arr.size()) - s) + " more items...";
	}

	return res + " }";
};

String str_arr(const uint8_t *data, const int size, const bool force_full, const int max_shown_items, String separator) {
	String res = "[ ";
	int s = size;
	bool is_long = false;
	if (s > max_shown_items && !force_full) {
		s = max_shown_items;
		is_long = true;
	}

	for (int i = 0; i < s; i++) {
		res += String::num_uint64(data[i]);
		if (i != s - 1 || is_long) {
			res += separator;
		}
	}

	if (is_long) {
		res += String::num_int64(int64_t(size) - s) + " more bytes...";
	}

	return res + " ]";
};

#ifndef NO_GODOTREMOTE_SERVER
Error compress_jpg(PoolByteArray &ret, const PoolByteArray &img_data, int width, int height, int bytes_for_color, int quality, int subsampling) {
	PoolByteArray res;
	ERR_FAIL_COND_V(img_data.empty(), Error::ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(quality < 1 || quality > 100, Error::ERR_INVALID_PARAMETER);

	jpge::params params;
	params.m_quality = quality;
	params.m_subsampling = (jpge::subsampling_t)subsampling;

	ERR_FAIL_COND_V(!params.check(), Error::ERR_INVALID_PARAMETER);
	auto rb = compress_buffer.read();
	auto ri = img_data.read();
	int size = compress_buffer.size();

	TimeCountInit();

	ERR_FAIL_COND_V_MSG(!jpge::compress_image_to_jpeg_file_in_memory(
								(void *)rb.ptr(),
								size,
								width,
								height,
								bytes_for_color,
								(const unsigned char *)ri.ptr(),
								params),
			Error::FAILED, "Can't compress image.");

	TimeCount("Compress img");

	ri.release();

	res.resize(size);
	auto wr = res.write();
	memcpy(wr.ptr(), rb.ptr(), size);

	TimeCount("Combine arrays");

	_log("JPG size: " + str(res.size()), LogLevel::LL_Debug);

	rb.release();
	ret = res;
	return Error::OK;
}
#endif

Error compress_bytes(const PoolByteArray &bytes, PoolByteArray &res, int type) {
	Error err = res.resize(bytes.size());
	ERR_FAIL_COND_V_MSG(err, err, "Can't resize output array");

	auto r = bytes.read();
	auto w = res.write();
	int size = Compression::compress(w.ptr(), r.ptr(), bytes.size(), (Compression::Mode)type);

	r.release();
	w.release();

	if (size) {
		res.resize(size);
	} else {
		ERR_PRINT("Can't resize output array after compression");
		err = Error::FAILED;
		res = PoolByteArray();
	}

	return err;
}

Error decompress_bytes(const PoolByteArray &bytes, int output_size, PoolByteArray &res, int type) {
	Error err = res.resize(output_size);
	ERR_FAIL_COND_V_MSG(err, err, "Can't resize output array");

	auto r = bytes.read();
	auto w = res.write();
	int size = Compression::decompress(w.ptr(), output_size, r.ptr(), bytes.size(), (Compression::Mode)type);

	r.release();
	w.release();

	if (output_size == -1) {
		ERR_PRINT("Can't decompress bytes");
		err = Error::FAILED;
		res = PoolByteArray();
	} else if (output_size != size) {
		ERR_PRINT("Desired size not equal to real size");
		err = Error::FAILED;
		res = PoolByteArray();
	}

	return err;
}

String str(const Variant &val) {
	Variant::Type type = val.get_type();
	switch (type) {
		case Variant::NIL: {
			return "NULL";
		}
		case Variant::BOOL: {
			return val ? "True" : "False";
		}
		case Variant::INT: {
			return String::num_int64(val);
		}
		case Variant::REAL: {
			return String::num_real(val);
		}
		case Variant::STRING: {
			return val;
		}
		case Variant::VECTOR2: {
			Vector2 v2 = val;
			return String("V2(") + v2 + ")";
		}
		case Variant::RECT2: {
			Rect2 r = val;
			return String("R2((") + String::num_real(r.position.x) + ", " + String::num_real(r.position.y) + "), (" + String::num_real(r.size.x) + ", " + String::num_real(r.size.y) + "))";
		}
		case Variant::VECTOR3: {
			Vector3 v3 = val;
			return String("V3(") + v3 + ")";
		}
		case Variant::TRANSFORM: {
			Transform t3d = val;
			return String("T3D(") + t3d + ")";
		}
		case Variant::TRANSFORM2D: {
			Transform2D t2d = val;
			return String("T2D(") + t2d + ")";
		}
		case Variant::PLANE: {
			Plane pln = val;
			return String("P(") + pln + ")";
		}
		case Variant::QUAT: {
			Quat q = val;
			return String("Q(") + q + ")";
		}
		case Variant::AABB: {
			AABB ab = val;
			return String("AABB(") + ab + ")";
		}
		case Variant::BASIS: {
			Basis bs = val;
			return String("B(") + bs + ")";
		}
		case Variant::COLOR: {
			Color c = val;
			return String("C(") + c + ")";
		}
		case Variant::NODE_PATH: {
			NodePath np = val;
			return String("NP: ") + np + ")";
		}
		case Variant::_RID: {
			RID rid = val;
			return String("RID:") + String::num_int64(rid.get_id());
		}
		case Variant::OBJECT: {
			Object *obj = val;
			if (obj)
				return obj->to_string();
			else {
				return String("[NULL]");
			}
		}
		case Variant::DICTIONARY: {
			return str_arr((Dictionary)val);
		}
		case Variant::ARRAY: {
			return str_arr((Array)val);
		}
		case Variant::POOL_BYTE_ARRAY: {
			return str_arr((PoolByteArray)val);
		}
		case Variant::POOL_INT_ARRAY: {
			return str_arr((PoolIntArray)val);
		}
		case Variant::POOL_REAL_ARRAY: {
			return str_arr((PoolRealArray)val);
		}
		case Variant::POOL_STRING_ARRAY: {
			return str_arr((PoolStringArray)val);
		}
		case Variant::POOL_VECTOR2_ARRAY: {
			return str_arr((PoolVector2Array)val);
		}
		case Variant::POOL_VECTOR3_ARRAY: {
			return str_arr((PoolVector3Array)val);
		}
		case Variant::POOL_COLOR_ARRAY: {
			return str_arr((PoolColorArray)val);
		}
		default: {
			break;
		}
	}
	return String("|? ") + Variant::get_type_name(type) + " ?|";
}

bool validate_packet(const uint8_t *data) {
	if (data[0] == internal_PACKET_HEADER[0] && data[1] == internal_PACKET_HEADER[1] && data[2] == internal_PACKET_HEADER[2] && data[3] == internal_PACKET_HEADER[3])
		return true;
	return false;
}

bool validate_version(const PoolByteArray &data) {
	if (data.size() < 2)
		return false;
	if (data[0] == internal_VERSION[0] && data[1] == internal_VERSION[1])
		return true;
	return false;
}

bool validate_version(const uint8_t *data) {
	if (data[0] == internal_VERSION[0] && data[1] == internal_VERSION[1])
		return true;
	return false;
}

bool compare_pool_byte_arrays(const PoolByteArray &a, const PoolByteArray &b) {
	if (a.size() != b.size())
		return false;
	auto r_a = a.read();
	auto r_b = b.read();
	for (int i = 0; i < a.size(); i++) {
		if (r_a[i] != r_b[i])
			return false;
	}

	return true;
}

void set_gravity(const Vector3 &p_gravity) {
	auto *id = (InputDefault *)Input::get_singleton();
	if (id)
		id->set_gravity(p_gravity);
}

void set_accelerometer(const Vector3 &p_accel) {
	auto *id = (InputDefault *)Input::get_singleton();
	if (id)
		id->set_accelerometer(p_accel);
}

void set_magnetometer(const Vector3 &p_magnetometer) {
	auto *id = (InputDefault *)Input::get_singleton();
	if (id)
		id->set_magnetometer(p_magnetometer);
}

void set_gyroscope(const Vector3 &p_gyroscope) {
	auto *id = (InputDefault *)Input::get_singleton();
	if (id)
		id->set_gyroscope(p_gyroscope);
}

} // namespace GRUtils