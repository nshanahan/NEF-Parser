/**************************************************************//**
*
* \file exif.h
*
* \author Nicholas Shanahan
*
* \date December 2020
*
* \details
*	EXIF format specification defintions.
*   See https://exiftool.org/TagNames/EXIF.html.
*
*******************************************************************/\

#ifndef EXIF_H_
#define EXIF_H_

/******************************************************************
                        Typdefs
*******************************************************************/
// EXIF Tag Identifiers
typedef enum
{
    EXIF_TAG_INTEROP_INDEX              = 0x0001,
    EXIF_TAG_INTEROP_VERSION            = 0x0002,
    EXIF_TAG_PROCESSING_SW              = 0x000B,
    EXIF_TAG_SUBFILE_TYPE               = 0x00FE,
    EXIF_TAG_OLD_SUBFILE_TYPE           = 0x00FF,
    EXIF_TAG_IMAGE_WIDTH                = 0x0100,
    EXIF_TAG_IMAGE_HEIGHT               = 0x0101,
    EXIF_TAG_BITS_PER_SAMPLE            = 0x0102,
    EXIF_TAG_COMPRESSION                = 0x0103,
    EXIF_TAG_PHOTOMETRIC_INTERPRETATION = 0x0106,
    EXIF_TAG_THRESHOLDING               = 0x0107,
    EXIF_TAG_CELL_WIDTH                 = 0x0108,
    EXIF_TAG_CELL_LENGTH                = 0x0109,
    EXIF_TAG_FILL_ORDER                 = 0x010A,
    EXIF_TAG_DOCUMENT_NAME              = 0x010D,
    EXIF_TAG_IMAGE_DESCRIPTION          = 0x010E,
    EXIF_TAG_MAKE                       = 0x010F,
    EXIF_TAG_MODEL                      = 0x0110,
    EXIF_TAG_STRIP_OFFSETS              = 0x0111,
    EXIF_TAG_ORIENTATION                = 0x0112,
    EXIF_TAG_SAMPLES_PER_PIXEL          = 0x0115,
    EXIF_TAG_ROWS_PER_STRIP             = 0x0116,
    EXIF_TAG_STRIP_BYTE_COUNTS          = 0x0117,
    EXIF_TAG_MIN_SAMPLE_VALUE           = 0x0118,
    EXIF_TAG_MAX_SAMPLE_VALUE           = 0x0119,
    EXIF_TAG_X_RESOLUTION               = 0x011A,
    EXIF_TAG_Y_RESOLUTION               = 0x011B,
    EXIF_TAG_SUBIFD_OFFSET              = 0x014A,
    EXIF_TAG_EXPOSURE_TIME              = 0x829A,
    EXIF_TAG_FNUMBER                    = 0x829D,
    EXIF_TAG_EXIF_OFFSET                = 0x8769,
    EXIF_TAG_DATE_TIME_ORIGINAL         = 0x9003,
    EXIF_TAG_SHUTTER_SPEED              = 0x9201,
    EXIF_TAG_APERTURE                   = 0x9202,
    EXIF_TAG_METERING_MODE              = 0x9207,
    EXIF_TAG_FOCAL_LENGTH               = 0x920A,
    EXIF_TAG_MAKERNOTE                  = 0x927C
} exif_tag_t;

#endif
