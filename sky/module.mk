MODULE := sky

MODULE_OBJS = \
	sky/autoroute.o \
	sky/cd_intro.o \
	sky/compact.o \
	sky/debug.o \
	sky/disk.o \
	sky/grid.o \
	sky/intro.o \
	sky/logic.o \
	sky/mouse.o \
	sky/rnc_deco.o \
	sky/screen.o \
	sky/sky.o \
	sky/sound.o \
	sky/text.o \
	sky/introimg.cpp \
	sky/music/adlibchannel.o \
	sky/music/adlibmusic.o \
	sky/music/gmchannel.o \
	sky/music/gmmusic.o \
	sky/music/musicbase.o \

# Include common rules 
include common.rules
