# cortex-m4 thumb PIE
CPU_CFLAGS = -mcpu=cortex-m4 -mthumb -ffunction-sections -fdata-sections -mno-unaligned-access -fpic -mpic-register=r9 -msingle-pic-base -mno-pic-data-is-text-relative
CPU_LDFLAGS = -nostartfiles -Wl,--gc-sections -static

CPU_MEM_RANGES_NEEDS_EXP_ROUNDING=1
