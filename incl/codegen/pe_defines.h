#pragma once

/*
 * Constants, etc. for PE Headers
 *
 * Taken from creedpe.c from https://github.com/intellectualheaven/ceed/blob/master/src/ceedpe.c
 */

namespace spero::codegen::pe {
    /*
     * Various constants and default values
     */
    constexpr int NUM_DIR_ENTRIES = 16;
    constexpr int IMAGE_DIR_ENTRY_IMPORT = 1;
    constexpr short IMAGE_FILE_32BIT_MACINE = 0x0100;
    constexpr long DEFAULT_ALIGNMENT = 512;

    /*
     * Constants for specifying the compilation target machine
     */
    enum Machine : short {
        INTEL_386 = 0x14c,
        x64 = 0x8664,
        MIPS_R3000 = 0x162,
        MIPS_R10000 = 0x168,
        MIPS_WCIv2 = 0x169,
        ALPHA_AXP_old = 0x183,
        ALPHA_AXP = 0x184,
        HITACHI_SH3 = 0x1a2,
        HITACHI_SH3_DSP = 0x1a3,
        HITACHI_SH4 = 0x1a6,
        HITACHI_SH5 = 0x1a8,
        ARM_le = 0x1c0,                 // little endian
        THUMB = 0x1c2,
        MATSUSHITA_AM33 = 0x1d3,
        POWERPC_le = 0x1f0,
        POWERPC_fps = 0x1f1,            // floating-point support
        INTEL_IA64 = 0x200,
        MIPS16 = 0x266,
        MOTOROLA_68000 = 0x268,
        ALPHA_AXP_64 = 0x284,
        MIPS_FPU = 0x366,
        MIPS16_FPU = 0x466,
        EFI_BYTE = 0xebc,
        AMD64 = 0x8664,
        MITSUBISHI_M32R_le = 0x9041,
        CLR_MSIL = 0xc0ee,
    };

    /*
     * Bit flags for the `characteristics` field of FileHeader
     *
     * TODO: Think of changing to a different backing size (I fear `short` is too small)
     */
    enum FileCharacter : short {
        Executable = 0x02,
        Non_Relocatable = 0x200,
        DLL = 0x2000
    };

    /*
     *
     */
    enum Magic : short {
        ROM = 0x107,
        HDR32 = 0x10b,
        HDR64 = 0x20b
    };

    /*
     * Default values for the `image_base` field
     */
    constexpr long IMAGE_BASE_DLL    = 0x10000000;
    constexpr long IMAGE_BASE_APP    = 0x00400000;
    constexpr long IMAGE_BASE_CE_APP = 0x00010000;

    /*
     * Constant values for various subsystems
     */
    enum Subsystem : short {
        UNKNOWN = 0,
        NATIVE,
        WINDOWS_GUI,
        WINDOWS_CUI,
        OS2_CUI,
        POSIX_CUI,
        WINDOWS_CE_CUI,
        EFI_APP,
        EFI_BOOT_DRIVER,
        EFI_RUN_DRIVER,
        EFI_ROM,
        XBOX,
        WINDOWS_BOOT_APP
    };

    /*
     * Bit fields for dll characteristics member of OptHeader
     */
    enum DLL_Characteristics : short {
        FIRST_LOAD = 0x0001,                // Call when first loaded
        THREAD_TERM = 0x0002,               // Call when thread terminates
        THREAD_STRT = 0x0004,               // Call when thread starts up
        DLL_EXIT = 0x0008,                  // Call when DLL exits
        RESERVED_5 = 0x1000,
        RESERVED_6 = 0x4000,
        DYNAMIC_BASE = 0x0040,              // Can be relocated at load time
        FORCE_INTEGRITY = 0x0080,           // Force code integrity checks
        NX_COMPAT = 0x0100,                 // Image is compatible with DEP
        NO_ISOLATION = 0x0200,              // Image is isolation aware, but shouldn't be isolated
        NO_SEH = 0x0400,                    // Image does not use structured exception handling
        NO_BIND = 0x0800,                   // Do not bind the image
        WDM_DRIVER = 0x2000,                // Image is a WDM driver
        TERMINAL_SERVER_AWARE = 0x8000      // Image is terminal server aware
    };

    /*
     * Entries in the `data_directory` of OptHeader
     */
    enum ImageDirectoryEntry : short {
        EXPORT = 0,
        IMPORT,
        RESOURCE,
        EXCEPTION,
        SECURITY,
        BASERELOC,
        DEBUG,
        ARCHITECTURE,
        GLOBALPTR,
        TLS,
        LOAD_CONFIG,
        BOUND_IMPORT,
        IAT,
        DELAY_IMPORT,
        COM_DESCRIPTOR,
        RESERVED
    };

    /*
     * Bit fields for section flags in SectionHeader
     */
    enum Image_Scn : long {
        TYPE_NO_PAD            = 0x8  // Reserved.
        CNT_CODE               = 0x20  // Section contains code.
        CNT_INITIALIZED_DATA   = 0x40  // Section contains initialized data.
        CNT_UNINITIALIZED_DATA = 0x80  // Section contains uninitialized data.
        LNK_OTHER              = 0x100  // Reserved.
        LNK_INFO               = 0x200  // Section contains comments or some  other type of information.
        LNK_REMOVE             = 0x800  // Section contents will not become part of image.
        LNK_COMDAT             = 0x1000  // Section contents comdat.
        NO_DEFER_SPEC_EXC      = 0x4000  // Reset speculative exceptions handling bits in the TLB entries for this section.
        GPREL                  = 0x8000  // Section content can be accessed relative to GP
        MEM_FARDATA            = 0x8000
        MEM_PURGEABLE          = 0x20000
        MEM_16BIT              = 0x20000
        MEM_LOCKED             = 0x40000
        MEM_PRELOAD            = 0x80000
        ALIGN_1BYTES           = 0x100000  //
        ALIGN_2BYTES           = 0x200000  //
        ALIGN_4BYTES           = 0x300000  //
        ALIGN_8BYTES           = 0x400000  //
        ALIGN_16BYTES          = 0x500000  // Default alignment if no others are specified.
        ALIGN_32BYTES          = 0x600000  //
        ALIGN_64BYTES          = 0x700000  //
        ALIGN_128BYTES         = 0x800000  //
        ALIGN_256BYTES         = 0x900000  //
        ALIGN_512BYTES         = 0xA00000  //
        ALIGN_1024BYTES        = 0xB00000  //
        ALIGN_2048BYTES        = 0xC00000  //
        ALIGN_4096BYTES        = 0xD00000  //
        ALIGN_8192BYTES        = 0xE00000  //
        ALIGN_MASK             = 0xF00000
        LNK_NRELOC_OVFL        = 0x1000000  // Section contains extended relocations.
        MEM_DISCARDABLE        = 0x2000000  // Section can be discarded.
        MEM_NOT_CACHED         = 0x4000000  // Section is not cachable.
        MEM_NOT_PAGED          = 0x08000000  // Section is not pageable.
        MEM_SHARED             = 0x10000000  // Section is shareable.
        MEM_EXECUTE            = 0x20000000  // Section is executable.
        MEM_READ               = 0x40000000  // Section is readable.
        MEM_WRITE              = 0x80000000  // Section is writeable.
    };
    
}