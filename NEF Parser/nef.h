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
#define MAKERNOTE_MAGIC   "Nikon"
// Lens data is encrypted is version is 201 or greater
#define LENS_DATA_0201    201

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
    NIKON_TAG_WHITE_BALANCE     = 0x0004,
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
    uint16_t byte_order;
    uint16_t tiff_magic;
    uint32_t ifd0_offset;
};
#pragma pack(pop)

/******************************************************************
                        Function Prototypes
*******************************************************************/

#endif /* end nef.h */
