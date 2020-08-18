
#include "suricata-plugin.h"

#include "decode.h"
#include "source-testimony.h"
#include "runmode-testimony.h"
#include "util-device.h"

static char *source_name = "testimony-plugin";

void InitCapturePlugin(const char *args, int plugin_slot, int receive_slot, int decode_slot)
{
    LiveBuildDeviceList("testimony");
    RunModeIdsTestimonyRegister(plugin_slot);
    TmModuleReceiveTestimonyRegister(receive_slot);
    TmModuleDecodeTestimonyRegister(decode_slot);
    
    LiveRegisterDeviceName(args);
    ConfSetFinal("testimony.socket-path", args);
}

void SCPluginInit(void)
{
    SCLogNotice("SCPluginInit");
    SCCapturePlugin *plugin = SCCalloc(1, sizeof(SCCapturePlugin));
    if (plugin == NULL) {
        FatalError(SC_ERR_MEM_ALLOC, "Failed to allocate memory for capture plugin");
    }
    plugin->name = source_name;
    plugin->Init = InitCapturePlugin;
    plugin->GetDefaultMode = RunModeTestimonyGetDefaultMode;
    SCPluginRegisterCapture(plugin);
}

const SCPlugin PluginSpec = {
    .name = "testimony-plugin",
    .author = "Vadym Malakhatko <v.malakhatko@sirinsoftware.com>",
    .license = "GPLv2",
    .Init = SCPluginInit,
};