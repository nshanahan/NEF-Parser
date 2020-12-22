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
#define TIFF_MAGIC		   0x2A
#define TIFF_LITTLE_ENDIAN 0x4949 //"II"
#define TIFF_BIG_ENDIAN	   0x4D4D //"MM"

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

/******************************************************************
						Function Prototypes
*******************************************************************/

#endif
