ECOS_GLOBAL_CFLAGS = -mcpu=arm7tdmi -Wall -Wpointer-arith -Wstrict-prototypes -Winline -Wundef -Woverloaded-virtual -g -O2 -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -fvtable-gc -finit-priority
ECOS_GLOBAL_LDFLAGS = -mcpu=arm7tdmi -Wl,--gc-sections -Wl,-static -g -nostdlib
ECOS_COMMAND_PREFIX = arm-elf-
