/* Linker script for SLUS_010.13 PSX game */
SECTIONS
{
    /* General ROM position */
    __romPos = 0;

    /* GP pointer for small data */
    _gp = 0x8003EC14;

    /* ------------------ HEADER ------------------ */
    header_ROM_START = __romPos;
    header_VRAM = ADDR(.header);

    .header : AT(header_ROM_START) SUBALIGN(2)
    {
        FILL(0x00000000);
        header_DATA_START = .;
        build/asm/header.o(.data);
        header_DATA_END = .;
        header_DATA_SIZE = ABSOLUTE(header_DATA_END - header_DATA_START);
    }

    __romPos += SIZEOF(.header);
    header_ROM_END = __romPos;
    header_VRAM_END = .;

    /* ------------------ MAIN SECTION ------------------ */
    main_ROM_START = __romPos;
    main_VRAM = 0x80010000;

    .main main_VRAM : AT(main_ROM_START) SUBALIGN(2)
    {
        FILL(0x00000000);

        /* Constructor/initialization table */
        D_80010000 = .;
        PROVIDE(D_80010000 = .);

        /* RODATA */
        main_RODATA_START = .;
        build/src/main.o(.rodata);
        . = ALIGN(., 16);
        main_RODATA_END = .;
        main_RODATA_SIZE = ABSOLUTE(main_RODATA_END - main_RODATA_START);

        /* TEXT */
        main_TEXT_START = .;
        build/src/main.o(.text);
        . = ALIGN(., 16);
        main_TEXT_END = .;
        main_TEXT_SIZE = ABSOLUTE(main_TEXT_END - main_TEXT_START);

        /* DATA */
        main_DATA_START = .;
        build/src/main.o(.data);
        build/asm/data/initialized.data.o(.data);
        main_DATA_END = .;
        main_DATA_SIZE = ABSOLUTE(main_DATA_END - main_DATA_START);
    }

    __romPos += SIZEOF(.main);
    main_ROM_END = __romPos;
    main_VRAM_END = .;

    /* ------------------ SMALL DATA (.sdata) ------------------ */
    .sdata 0x8003EC14 : AT(__romPos) SUBALIGN(2)
    {
        main_SDATA_START = .;
        build/asm/data/gp_data.sdata.o(.sdata);
        main_SDATA_END = .;
        main_SDATA_SIZE = ABSOLUTE(main_SDATA_END - main_SDATA_START);
    }

    __romPos += SIZEOF(.sdata);

    /* ------------------ SBSS (Small Uninitialized Data) ------------------ */
    .sbss 0x8003EC20 (NOLOAD) : SUBALIGN(2)
    {
        main_SBSS_START = .;
        build/asm/data/sbss.sbss.o(.sbss);
        main_SBSS_END = .;
        main_SBSS_SIZE = ABSOLUTE(main_SBSS_END - main_SBSS_START);
    }

    /* ------------------ BSS (Uninitialized Data) ------------------ */
    .bss 0x8003EC88 (NOLOAD) : SUBALIGN(2)
    {
        main_BSS_START = .;
        build/src/main.o(.bss);
        build/asm/data/bss.bss.o(.bss);
        . = ALIGN(., 16);
        main_BSS_END = .;
        main_BSS_SIZE = ABSOLUTE(main_BSS_END - main_BSS_START);
    }

    /* ------------------ DISCARD ALL OTHER ------------------ */
    /DISCARD/ :
    {
        *(*);
    }
}
