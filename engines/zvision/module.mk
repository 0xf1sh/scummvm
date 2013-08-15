MODULE := engines/zvision
 
MODULE_OBJS := \
	action_node.o \
	actions.o \
	clock.o \
	console.o \
	control.o \
	cursor.o \
	cursor_manager.o \
	detection.o \
	events.o \
	lzss_read_stream.o \
	mouse_event.o \
	render_manager.o \
	render_table.o \
	scr_file_handling.o \
	script_manager.o \
	scripts.o \
	single_value_container.o \
	utility.o \
	video.o \
	zvision.o \
	zfs_archive.o \
	zork_avi_decoder.o \
	zork_raw.o
 
MODULE_DIRS += \
	engines/zvision
 
# This module can be built as a plugin
ifeq ($(ENABLE_ZVISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk