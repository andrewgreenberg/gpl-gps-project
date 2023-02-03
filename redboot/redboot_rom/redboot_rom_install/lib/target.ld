STARTUP(vectors.o)
ENTRY(reset_vector)
INPUT(extras.o)
GROUP(libtarget.a libgcc.a libsupc++.a)




MEMORY
{
    sram : ORIGIN = 0, LENGTH = 0x2000
    ram : ORIGIN = 0x20000000, LENGTH = 0x40000
    rom : ORIGIN = 0x60000000, LENGTH = 0x40000
}

SECTIONS
{
    .debug_aranges 0 : { *(.debug_aranges) } .debug_pubnames 0 : { *(.debug_pubnames) } .debug_info 0 : { *(.debug_info) } .debug_abbrev 0 : { *(.debug_abbrev) } .debug_line 0 : { *(.debug_line) } .debug_frame 0 : { *(.debug_frame) } .debug_str 0 : { *(.debug_str) } .debug_loc 0 : { *(.debug_loc) } .debug_macinfo 0 : { *(.debug_macinfo) } .note.arm.ident 0 : { KEEP (*(.note.arm.ident)) }
    .fixed_vectors 0x20 : { . = .; KEEP (*(.fixed_vectors)) } > sram
    .rom_vectors 0x60000000 : { __rom_vectors_vma = ABSOLUTE(.); . = .; KEEP (*(.vectors)) } > rom __rom_vectors_lma = LOADADDR(.rom_vectors);
    .text ALIGN (0x4) : { _stext = ABSOLUTE(.); PROVIDE (__stext = ABSOLUTE(.)); *(.text*) *(.gnu.warning) *(.gnu.linkonce.t.*) *(.init) *(.glue_7) *(.glue_7t) } > rom _etext = .; PROVIDE (__etext = .);
    .fini ALIGN (0x4) : { . = .; *(.fini) } > rom
    .rodata ALIGN (0x4) : { . = .; *(.rodata*) *(.gnu.linkonce.r.*) } > rom
    .rodata1 ALIGN (0x4) : { . = .; *(.rodata1) } > rom
    .fixup ALIGN (0x4) : { . = .; *(.fixup) } > rom
    .gcc_except_table ALIGN (0x4) : { . = .; *(.gcc_except_table) } > rom
    .data 0x20000000 : AT ((LOADADDR (.gcc_except_table) + SIZEOF (.gcc_except_table) + 4 - 1) & ~ (4 - 1)) { __ram_data_start = ABSOLUTE (.); *(.data*) *(.data1) *(.gnu.linkonce.d.*) . = ALIGN (4); KEEP(*( SORT (.ecos.table.*))) ; . = ALIGN (4); __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); *(.dynamic) *(.sdata*) *(.gnu.linkonce.s.*) . = ALIGN (4); *(.2ram.*) } > ram __rom_data_start = LOADADDR (.data); __ram_data_end = .; PROVIDE (__ram_data_end = .); _edata = .; PROVIDE (edata = .); PROVIDE (__rom_data_end = LOADADDR (.data) + SIZEOF(.data));
    .bss ALIGN (0x4) : { __bss_start = ABSOLUTE (.); *(.scommon) *(.dynsbss) *(.sbss*) *(.gnu.linkonce.sb.*) *(.dynbss) *(.bss*) *(.gnu.linkonce.b.*) *(COMMON) __bss_end = ABSOLUTE (.); } > ram
    __heap1 = ALIGN (0x8);
    . = ALIGN(4); _end = .; PROVIDE (end = .);
}
