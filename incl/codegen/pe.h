#pragma once

#include "pe_defines.h"

/*
 * TODO:
 *   Ensure that everything is endian agnostic
 *   Figure out a way to construct the structures correctly
 *   Add comments to the DOS_Header members
 *   More work on the interface
 *     NOTE: I don't have to maintain byte compatibility either, so I can change the organization if needed
 *   Make sure these work with PE 32+ format
 *   Trim down DOS_Header.SNIPPET to the minimal version I need
 */

namespace spero::codegen::pe {

    /*
     * Struct for laying out dos header information
     *
     * TODO: What does this do ???
     */
    struct DOS_Header {
        const char signature[2] = "MZ";
        short lastsize;
        short nblocks;
        short nreloc;
        short hdr_size;
        short min_alloc;
        short max_alloc;
        short ss;
        short sp;
        short checksum;
        short ip;
        short cs;
        short reloc_pos;
        short noverlay;
        short reserved1[4];
        short oem_id;
        short oem_info;
        short reserved2[10];
        long e_lfanew;
        const char SNIPPET[12*16] = {           // Code snippet for DOS running (may be in little endian)
            0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68,
            0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F,
            0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20,
            0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x40, 0x6A, 0x9B, 0xAF, 0x04, 0x0B, 0xF5, 0xFC, 0x04, 0x0B, 0xF5, 0xFC, 0x04, 0x0B, 0xF5, 0xFC,
            0x0D, 0x73, 0x66, 0xFC, 0x0E, 0x0B, 0xF5, 0xFC, 0xBE, 0x54, 0xF4, 0xFD, 0x07, 0x0B, 0xF5, 0xFC,
            0xBE, 0x54, 0xF6, 0xFD, 0x05, 0x0B, 0xF5, 0xFC, 0xBE, 0x54, 0xF0, 0xFD, 0x16, 0x0B, 0xF5, 0xFC,
            0xBE, 0x54, 0xF1, 0xFD, 0x09, 0x0B, 0xF5, 0xFC, 0xB0, 0x97, 0x1A, 0xFC, 0x06, 0x0B, 0xF5, 0xFC,
            0x04, 0x0B, 0xF4, 0xFC, 0x2F, 0x0B, 0xF5, 0xFC, 0xC0, 0x54, 0xFC, 0xFD, 0x05, 0x0B, 0xF5, 0xFC,
            0xC0, 0x54, 0x0A, 0xFC, 0x05, 0x0B, 0xF5, 0xFC, 0xC0, 0x54, 0xF7, 0xFD, 0x05, 0x0B, 0xF5, 0xFC,
            0x52, 0x69, 0x63, 0x68, 0x04, 0x0B, 0xF5, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        DOS_Header(const char[62]);
    };


    /*
     * Struct defining some information common in COFF object files and executables
     */
    struct FileHeader {
        Machine machine;                    // The machine this file was compiled for
        short n_sections;                   // The number of sections described at the end of the pe headers
        long time_stamp;                    // Time since 1970 epoch when this header was created
        long symtable_ptr;                  // Offset of the COFF symbol table (only used in OBJ files and PE files with COFF debug info)
                                            // PE files support multiple debug formats (debuggers refer to IMAGE_DIRECTORY_ENTRY_DEBUG)
        long n_symbols;                     // Number of symbols in the symbol table
        short opt_header_size;              // How long the optional header that follows this is
        long characteristics;               // Field of bit flags that specify some file characteristics
    };


    /*
     * Short struct defining the layout of data directory entries
     *
     * TODO: What does this do ???
     */
    struct ImageDataDir {
        long virtual_addr;
        long size;
    };


    /*
     * Struct defining the layout of the PE Optional Header
     * This header is actually required in executables.
     *
     * TODO: What does the opt header do ???
     */
    struct OptHeader {
        Magic signature;                    // A signature that identifies the image ???
        char major_linker_ver;              // Linker version number tracking
        char minor_linker_ver;
        long code_size;                     // The size of all code sections, in bytes
        long init_size;                     // The size of all initialized data sections, in bytes
        long uninit_size;                   // The size of all uninitialized data sections, in bytes
        long entrypoint_addr;               // Pointer to the etnry point function, relative to the image base address
        long code_base;                     // Pointer to the beginning of the code section
        long data_base;                     // Pointer to the beginning of the data section
        long image_base;                    // Preferred address of the image when loaded into memory
        long section_align;                 // The alignment of sections in memory, must be >= `file_align`.
                                            // NOTE: If this is < system page size, then "file_align = section_align"
        long file_align;                    // The alignment of the raw sections, in bytes. Should be a power of 2 in [512,64K]
        short major_os_ver;                 // Version numbering of the required OS
        short minor_os_ver;
        short major_image_ver;              // Image version number tracking
        short minor_image_ver;
        short major_subsys_ver;             // Subsystem version number tracking
        short minor_subsys_ver;
        const long win32_ver = 0;           // Reserved
        long image_size;                    // Size of the image, including headers, in bytes. Must be a multiple of `section_align`
        long headers_size;                  // Combined size of a set of items, rounded to a multiple of `file_align`
                                            // e_lfanew, 4 bytes, FileHeader, OptHeader, all SectionHeader
        long checksum;                      // File checksum. I don't think this is checked for applications
        short subsystem;                    // The subsystem that will be used to run the executable
        short dll_characteristics;          // DLL characteristics of the image
        long stack_reserve;                 // Number of bytes to reserve for the stack
        long stack_commit;                  // Number of bytes available at load time (rest of pages are loaded on demand)
        long heap_reserve;                  // Number of bytes to reserver for the heap
        long heap_commit;                   // Number of bytes available at load time (rest of pages are loaded on demand)
        long loader_flags;                  // Obsolete
        long n_entries;                     // Optional field that could be used to specify the number of DataDirectory entries
        ImageDataDir data_dir[NUM_DIR_ENTRIES];       // Doesn't have to be 16, but it always is in PE fils
                                            // Provides a method for locating various structures in useful for the setting up the execution environment
    };


    /*
     * Structure for organizing the information regarding
     * Section Header entries (immediately following the optional header).
     *
     * TODO: What do section headers do ???
     */
    struct SectionHeader {
        char name[8];                       // Name of the secion
        union {
            long physical_addr;             // TODO: Get value
            long virtual_size;              // Size of the section once loaded
        } misc;
        long virtual_addr;                  // Location of the section once loaded
        long raw_data_size;                 // Physical size of the section on disk
        long raw_data_ptr;                  // Physical location of section on disk
        long reloc_ptr;                     // Reserved. Normally 0
        long linenos_ptr;                   // Reserved. Used in object formats
        short n_relocations;                // Reserved
        short n_linenos;                    // Reserved
        long characteristics;               // Bit field for various section flags
    };

    
    /*
     * Struct defining the organization of a complete pe executable beginning
     *
     * TODO: Get this organized better
     */
    struct PE_ImageStart = {
        DOS_Header dos;
        const char signature[4] = { 0x50, 0x45, 0x00, 0x00 };
        FileHeader head;
        OptHeader opt;
        SectionHeader section[];           // TODO: Improve this aspect
    }


    /*
     * I don't have any idea what these structures are for, or there purpose
     * Maybe I'll see what they are in the future, so for now I'm commenting them
    struct ImageExportDirectory {
        long characteristics;
        long timedate_stamp;
        short major_ver;
        short minor_ver;
        long name;
        long base;
        long num_functions;
        long num_names;
        long* addrof_functions;
        long* addrof_names;
        long* addrof_name_ordinals;
    };

    struct ImageImportByName {
        short hint;
        char name[1];
    };

    // creedpe.c entry
    struct ImageThunkData32 {
        union {
            long forwarder_string;
            long function;
            long ordinal;
            long addr_of_data;
        };
    };

    struct ImageImportDescriptor {
        union {
            long characteristics;
            long orig_first_thunk;
        };
        long time_date_stamp;
        long forwarder_chain;
        long name;
        long first_thunk;
    };

    // wikibooks entry
    struct ImageImportDescriptor {
        long* original_thunk;
        long timedate_stamp;
        long forwarder_chain;
        long name;
        long* first_thunk;
    };

    struct ImageResourceDirectory {
        long characteristics;
        long timedate_stamp;
        short major_ver;
        short minor_ver;
        short num_named_entries;
        short num_id_entries;
    };

    struct ImageResourceDirectoryEntry {
        long nameid;
        long* data;
    };

    struct ImageResourceDataEntry {
        long* data;
        long size;
        long code_page;
        long reserved;
    };
    */
}


void example() {
    DOS_Header dos({
        0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
        0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00
    });

    PE_Header pe({
        FileHeader{},
        OptHeader{},
        new SectionHeader[5]{

        }
    });
}