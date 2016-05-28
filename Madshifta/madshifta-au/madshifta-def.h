#ifndef __MADSHIFTA_DEF_H
#define __MADSHIFTA_DEF_H


#include "dfxplugin-prefix.h"


#define PLUGIN_NAME_STRING	"MadShifta"
#define PLUGIN_ID	'TBMS'
#define PLUGIN_VERSION_MAJOR	1
#define PLUGIN_VERSION_MINOR	0
#define PLUGIN_VERSION_BUGFIX	4
#define PLUGIN_ENTRY_POINT	"MadShiftaEntry"
#define TARGET_PLUGIN_USES_MIDI	1
#define TARGET_PLUGIN_IS_INSTRUMENT	0
#define TARGET_PLUGIN_USES_DSPCORE	1
#define TARGET_PLUGIN_HAS_GUI	1

#define PLUGIN_CREATOR_NAME_STRING	"Tobybear"
#define PLUGIN_CREATOR_ID	'Toby'
#define PLUGIN_COLLECTION_NAME	"Tobybear vs. Bram @ Smartelectronix learning toolz"
#define PLUGIN_ICON_FILE_NAME	"smartelectronix.icns"
#define PLUGIN_HOMEPAGE_URL	"http://tobybear.de/"

// only necessary if using a custom GUI
#define PLUGIN_EDITOR_ENTRY_POINT	"MadShiftaEditorEntry"

// optional
#define PLUGIN_DESCRIPTION_STRING	"ultrafast and lofi pitchshifter"
#define PLUGIN_EDITOR_DESCRIPTION_STRING	"boxy red interface for MadShifta"


#define PLUGIN_BUNDLE_IDENTIFIER	"de.tobybear." PLUGIN_NAME_STRING DFX_BUNDLE_ID_SUFFIX


#define DFXGUI_USE_CONTEXTUAL_MENU	0	/* XXX for now, until I finish the implementation */


#endif
