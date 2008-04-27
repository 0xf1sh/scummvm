MODULE := engines/kyra

MODULE_OBJS := \
	animator_v1.o \
	animator_v2.o \
	animator_v3.o \
	debugger.o \
	detection.o \
	gui.o \
	gui_v1.o \
	gui_v2.o \
	gui_v3.o \
	items_v1.o \
	items_v2.o \
	items_v3.o \
	kyra.o \
	kyra_v1.o \
	kyra_v2.o \
	kyra_v3.o \
	resource.o \
	saveload.o \
	saveload_v1.o \
	saveload_v2.o \
	saveload_v3.o \
	scene.o \
	scene_v1.o \
	scene_v2.o \
	scene_v3.o \
	screen.o \
	screen_v1.o \
	screen_v2.o \
	screen_v3.o \
	script_v1.o \
	script_v2.o \
	script_v3.o \
	script.o \
	script_tim.o \
	seqplayer.o \
	sequences_v1.o \
	sequences_v2.o \
	sequences_v3.o \
	sound_adlib.o \
	sound_digital.o \
	sound_towns.o \
	sound.o \
	sound_v1.o \
	sprites.o \
	staticres.o \
	text.o \
	text_v1.o \
	text_v2.o \
	text_v3.o \
	timer.o \
	timer_v1.o \
	timer_v2.o \
	timer_v3.o \
	vqa.o \
	wsamovie.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
