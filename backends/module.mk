MODULE := backends

MODULE_OBJS := \
	backends/fs/fs.o \
	backends/fs/posix/posix-fs.o \
	backends/fs/morphos/abox-fs.o \
	backends/fs/windows/windows-fs.o \
	backends/midi/alsa.o \
	backends/midi/coreaudio.o \
	backends/midi/morphos.o \
	backends/midi/null.o \
	backends/midi/quicktime.o \
	backends/midi/seq.o \
	backends/midi/windows.o

MODULE_DIRS += \
	backends \
	backends/fs/posix \
	backends/fs/morphos \
	backends/fs/windows

# Include common rules 
include $(srcdir)/common.rules
