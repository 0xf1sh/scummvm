MODULE := engines/pegasus

MODULE_OBJS = \
	console.o \
	credits.o \
	detection.o \
	graphics.o \
	menu.o \
	overview.o \
	pegasus.o \
	video.o \
	Game_Shell/CGameState.o \
	Game_Shell/CInventory.o \
	Game_Shell/CItem.o \
	Game_Shell/CItemList.o \
	MMShell/Base_Classes/MMFunctionPtr.o \
	MMShell/Notification/MMNotification.o \
	MMShell/Notification/MMNotificationManager.o \
	MMShell/Notification/MMNotificationReceiver.o \
	MMShell/Sounds/MMSound.o \
	MMShell/Utilities/MMResourceFile.o \
	MMShell/Utilities/MMTimeValue.o \
	MMShell/Utilities/MMUtilities.o \
	neighborhood/door.o \
	neighborhood/exit.o \
	neighborhood/extra.o \
	neighborhood/hotspotinfo.o \
	neighborhood/neighborhood.o \
	neighborhood/spot.o \
	neighborhood/turn.o \
	neighborhood/view.o \
	neighborhood/zoom.o


# This module can be built as a plugin
ifeq ($(ENABLE_PEGASUS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
