/* register_types.cpp */

#include "register_types.h"

#include "GRClient.h"
#include "GRDevice.h"
#include "GRNotifications.h"
#include "GRPacket.h"
#include "GRServer.h"
#include "GRUtils.h"
#include "GodotRemote.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/project_settings.h"

// clumsy settings to test
// outbound and ip.DstAddr >= 127.0.0.1 and ip.DstAddr <= 127.255.255.255 and (tcp.DstPort == 52341 or tcp.SrcPort == 52341)

void register_gd_godot_remote_types() {
	ClassDB::register_class<GodotRemote>();
	ClassDB::register_class<GRUtilsData>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GodotRemote", memnew(GodotRemote)));
	GRUtils::init();

	ClassDB::register_class<GRNotifications>();
	ClassDB::register_class<GRNotificationPanel>();
	ClassDB::register_class<GRNotificationPanelUpdatable>();
	ClassDB::register_class<GRNotificationStyle>();

	ClassDB::register_virtual_class<GRDevice>();

#ifndef NO_GODOTREMOTE_SERVER
	ClassDB::register_class<GRServer>();
	ClassDB::register_class<GRSViewport>();
	ClassDB::register_class<GRSViewportRenderer>();
#endif

#ifndef NO_GODOTREMOTE_CLIENT
	ClassDB::register_class<GRClient>();
	ClassDB::register_class<GRInputCollector>();
	ClassDB::register_class<GRTextureRect>();
#endif

	// Packets
	ClassDB::register_virtual_class<GRPacket>();
	ClassDB::register_class<GRPacketClientStreamAspect>();
	ClassDB::register_class<GRPacketClientStreamOrientation>();
	ClassDB::register_class<GRPacketCustomInputScene>();
	ClassDB::register_class<GRPacketImageData>();
	ClassDB::register_class<GRPacketInputData>();
	ClassDB::register_class<GRPacketMouseModeSync>();
	ClassDB::register_class<GRPacketServerSettings>();
	ClassDB::register_class<GRPacketSyncTime>();
	ClassDB::register_class<GRPacketCustomUserData>();

	ClassDB::register_class<GRPacketPing>();
	ClassDB::register_class<GRPacketPong>();

	// Input Data
	ClassDB::register_virtual_class<GRInputData>();
	ClassDB::register_class<GRInputDeviceSensorsData>();
	ClassDB::register_class<GRInputDataEvent>();

	ClassDB::register_class<GRIEDataWithModifiers>();
	ClassDB::register_class<GRIEDataMouse>();
	ClassDB::register_class<GRIEDataGesture>();

	ClassDB::register_class<GRIEDataAction>();
	ClassDB::register_class<GRIEDataJoypadButton>();
	ClassDB::register_class<GRIEDataJoypadMotion>();
	ClassDB::register_class<GRIEDataKey>();
	ClassDB::register_class<GRIEDataMagnifyGesture>();
	ClassDB::register_class<GRIEDataMIDI>();
	ClassDB::register_class<GRIEDataMouseButton>();
	ClassDB::register_class<GRIEDataMouseMotion>();
	ClassDB::register_class<GRIEDataPanGesture>();
	ClassDB::register_class<GRIEDataScreenDrag>();
	ClassDB::register_class<GRIEDataScreenTouch>();
}

void unregister_gd_godot_remote_types() {
	if (GodotRemote *instance = GodotRemote::get_singleton())
		memdelete(instance);
	GRUtils::deinit();
}
