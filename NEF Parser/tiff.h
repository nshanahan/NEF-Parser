/**************************************************************//**
*
* \file tiff.h
*
* \author Nicholas Shanahan
*
* \date December 2020
*
* \details
*	TIFF format specification defintions.
*   See https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
*   for details regarding the TIFF specification.
*
*******************************************************************/

#ifndef TIFF_H_
#define TIFF_H_

/******************************************************************
						Includes
*******************************************************************/
#include <stdint.h>

/******************************************************************
						Defines
*******************************************************************/
#define TIFF_MAGIC			0x2A
#define TIFF_LITTLE_ENDIAN	0x4949 //"II"
#define TIFF_BIG_ENDIAN		0x4D4D //"MM"

/******************************************************************
						Structures
*******************************************************************/
// See Section 2 of TIFF Specification
struct tiff_header_t
{
	uint16_t byte_order;  // "MM" indicates big endian and "II" indicates little endian.
	uint16_t tiff_magic;  // TIFF magic number (0x2a)
	uint32_t ifd0_offset; // Offset of 0th IFD in TIFF file
};

// See Section 2 of TIFF Specification
struct ifd_entry_t
{
	uint16_t tag;   // Identification tag
	uint16_t type;  // Field type
	uint32_t count; // Number of values in the entry
	uint32_t value; // Value offset
};

// See Section 2 of TIFF Specification
#pragma pack(push, 1)
struct ifd_t
{
	uint16_t entries;           // Entry count
	struct ifd_entry_t entry[]; // Array of IFD entries
};
#pragma pack(pop)

/******************************************************************
						Typedefs
*******************************************************************/
// See https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
typedef enum
{
	TIFF_TYPE_BYTE		= 1,
	TIFF_TYPE_ASCII		= 2,
	TIFF_TYPE_SHORT		= 3,
	TIFF_TYPE_LONG		= 4,
	TIFF_TYPE_RATIONAL	= 5,
	TIFF_TYPE_SBYTE		= 6,
	TIFF_TYPE_UNDEFINED	= 7,
	TIFF_TYPE_SSHORT	= 8,
	TIFF_TYPE_SLONG		= 9,
	TIFF_TYPE_SRATIONAL	= 10,
	TIFF_TYPE_FLOAT		= 11,
	TIFF_TYPE_DOUBLE	= 12
} tiff_type_t;

/******************************************************************
						Function Prototypes
*******************************************************************/

#endif
