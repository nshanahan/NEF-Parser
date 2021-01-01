/**************************************************************//**
*
* \file nef.h
*
* \author Nicholas Shanahan
*
* \date December 2020
*
* \details
*	Nikon Electronic File (NEF) format definitions.
*
*******************************************************************/

#ifndef NEF_H_
#define NEF_H_

/******************************************************************
                        Includes
*******************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "tiff.h"

/******************************************************************
                        Defines
*******************************************************************/
#define MAKERNOTE_MAGIC     "Nikon"
// Lens data is encrypted is version is 201 or greater
#define LENS_DATA_0201      201
#define MAX_LENS_ID_LENGTH  96
#define MAX_LENS_ID_ENTRIES 256

/******************************************************************
                        Typedefs
*******************************************************************/
// NEF header is a standard TIFF header
typedef struct tiff_header_t nef_header_t;

// Nikon Makernote tag values
typedef enum
{
    NIKON_TAG_MAKERNOTE_VERSION = 0x0001,
    NIKON_TAG_ISO               = 0x0002,
    NIKON_TAG_QUALITY           = 0x0004,
    NIKON_TAG_WHITE_BALANCE     = 0x0005,
    NIKON_TAG_FOCUS_MODE        = 0x0007,
    NIKON_TAG_FLASH_SETTING     = 0x0008,
    NIKON_TAG_SERIAL_NUMBER     = 0x001D,
    NIKON_TAG_ISO_INFO          = 0x0025,
    NIKON_TAG_LENS_TYPE         = 0x0083,
    NIKON_TAG_LENS              = 0x0084,
    NIKON_TAG_LENS_DATA         = 0x0098,
    NIKON_TAG_SHUTTER_COUNT     = 0x00A7,
} nikon_tag_t;

/******************************************************************
                        Structures
*******************************************************************/
// See Section 5: Makernote
// http://lclevy.free.fr/nef/#:~:text=Overview,full%20RAW%20image%20lossless%20compressed.
#pragma pack(push, 1)
struct makernote_header_t
{
    char magic_value[6]; // "Nikon"
    uint16_t version;
    uint16_t reserved;
    struct tiff_header_t tiff_hdr;
};
#pragma pack(pop)

// Lens ID entry containing composite tag and associated lens ID (model) string
struct lens_id_entry_t
{
    uint8_t tag[8];
    char id[MAX_LENS_ID_LENGTH];
};

/******************************************************************
                        Global Variables
*******************************************************************/
// See https://exiftool.org/TagNames/Nikon.html#LensID.
struct lens_id_entry_t nikon_lens_id_table[3] = {
    { {0xE3, 0x40, 0x76, 0xA6, 0x38, 0x40, 0xDF, 0x4E}, "Tamron SP 150-600mm f/5-6.3 Di VC USD G2" },
    { {0xAA, 0x48, 0x37, 0x5C, 0x24, 0x24, 0xC5, 0x4E}, "AF-S Nikkor 24-70mm f/2.8E ED VR" },
    { {0xAE, 0x3C, 0x80, 0xA0, 0x3C, 0x3C, 0xC9, 0x4E}, "AF-S Nikkor 200-500mm f/5.6E ED VR" },
    // TODO: Implement the rest of the table
};

#endif /* end nef.h */
