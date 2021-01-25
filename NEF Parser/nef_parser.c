/**************************************************************//**
*
* \file nef_parser.c
*
* \author Nicholas Shanahan
*
* \date December 2020
*
* \details
*	Application to parse Nikon Electronic File (NEF) image files.
* 
*   Development Resources:
*       - https://www.itu.int/itudoc/itu-t/com16/tiff-fx/docs/tiff6.pdf
*       - https://www.exif.org/Exif2-2.PDF
*       - http://lclevy.free.fr/nef/#:~:text=Overview,full%20RAW%20image%20lossless%20compressed.
*       - https://exiftool.org/TagNames/EXIF.html
*
*******************************************************************/

/******************************************************************
                        Includes
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "nef.h"
#include "tiff.h"
#include "exif.h"

/******************************************************************
                        Defines
*******************************************************************/
const char banner[] = "**********************************************\n"
                      "*           NEF Parser Tool (2020)           *\n"
                      "**********************************************\n\n";

// Additional verbosity for development debugging
#define NEF_VERBOSE_DEBUG 0

/******************************************************************
                        Macros
*******************************************************************/
//#define nef_debug_print(...) printf(__VA_ARGS__)
#define nef_debug_print(...)

// Convert bytes to double words
#define BYTES_TO_DWORDS(x) ((x) >> 2)

/******************************************************************
                        Global Variables
*******************************************************************/
static uint32_t makernote_offset = 0;
static uint32_t tiff_offset = 0;

// Translation table used to decrypt lens data fields
uint8_t xlat[2][256] = {
    { 0xc1, 0xbf, 0x6d, 0x0d, 0x59, 0xc5, 0x13, 0x9d, 0x83, 0x61, 0x6b, 0x4f, 0xc7, 0x7f, 0x3d, 0x3d,
      0x53, 0x59, 0xe3, 0xc7, 0xe9, 0x2f, 0x95, 0xa7, 0x95, 0x1f, 0xdf, 0x7f, 0x2b, 0x29, 0xc7, 0x0d,
      0xdf, 0x07, 0xef, 0x71, 0x89, 0x3d, 0x13, 0x3d, 0x3b, 0x13, 0xfb, 0x0d, 0x89, 0xc1, 0x65, 0x1f,
      0xb3, 0x0d, 0x6b, 0x29, 0xe3, 0xfb, 0xef, 0xa3, 0x6b, 0x47, 0x7f, 0x95, 0x35, 0xa7, 0x47, 0x4f,
      0xc7, 0xf1, 0x59, 0x95, 0x35, 0x11, 0x29, 0x61, 0xf1, 0x3d, 0xb3, 0x2b, 0x0d, 0x43, 0x89, 0xc1,
      0x9d, 0x9d, 0x89, 0x65, 0xf1, 0xe9, 0xdf, 0xbf, 0x3d, 0x7f, 0x53, 0x97, 0xe5, 0xe9, 0x95, 0x17,
      0x1d, 0x3d, 0x8b, 0xfb, 0xc7, 0xe3, 0x67, 0xa7, 0x07, 0xf1, 0x71, 0xa7, 0x53, 0xb5, 0x29, 0x89,
      0xe5, 0x2b, 0xa7, 0x17, 0x29, 0xe9, 0x4f, 0xc5, 0x65, 0x6d, 0x6b, 0xef, 0x0d, 0x89, 0x49, 0x2f,
      0xb3, 0x43, 0x53, 0x65, 0x1d, 0x49, 0xa3, 0x13, 0x89, 0x59, 0xef, 0x6b, 0xef, 0x65, 0x1d, 0x0b,
      0x59, 0x13, 0xe3, 0x4f, 0x9d, 0xb3, 0x29, 0x43, 0x2b, 0x07, 0x1d, 0x95, 0x59, 0x59, 0x47, 0xfb,
      0xe5, 0xe9, 0x61, 0x47, 0x2f, 0x35, 0x7f, 0x17, 0x7f, 0xef, 0x7f, 0x95, 0x95, 0x71, 0xd3, 0xa3,
      0x0b, 0x71, 0xa3, 0xad, 0x0b, 0x3b, 0xb5, 0xfb, 0xa3, 0xbf, 0x4f, 0x83, 0x1d, 0xad, 0xe9, 0x2f,
      0x71, 0x65, 0xa3, 0xe5, 0x07, 0x35, 0x3d, 0x0d, 0xb5, 0xe9, 0xe5, 0x47, 0x3b, 0x9d, 0xef, 0x35,
      0xa3, 0xbf, 0xb3, 0xdf, 0x53, 0xd3, 0x97, 0x53, 0x49, 0x71, 0x07, 0x35, 0x61, 0x71, 0x2f, 0x43,
      0x2f, 0x11, 0xdf, 0x17, 0x97, 0xfb, 0x95, 0x3b, 0x7f, 0x6b, 0xd3, 0x25, 0xbf, 0xad, 0xc7, 0xc5,
      0xc5, 0xb5, 0x8b, 0xef, 0x2f, 0xd3, 0x07, 0x6b, 0x25, 0x49, 0x95, 0x25, 0x49, 0x6d, 0x71, 0xc7 },
    { 0xa7, 0xbc, 0xc9, 0xad, 0x91, 0xdf, 0x85, 0xe5, 0xd4, 0x78, 0xd5, 0x17, 0x46, 0x7c, 0x29, 0x4c,
      0x4d, 0x03, 0xe9, 0x25, 0x68, 0x11, 0x86, 0xb3, 0xbd, 0xf7, 0x6f, 0x61, 0x22, 0xa2, 0x26, 0x34,
      0x2a, 0xbe, 0x1e, 0x46, 0x14, 0x68, 0x9d, 0x44, 0x18, 0xc2, 0x40, 0xf4, 0x7e, 0x5f, 0x1b, 0xad,
      0x0b, 0x94, 0xb6, 0x67, 0xb4, 0x0b, 0xe1, 0xea, 0x95, 0x9c, 0x66, 0xdc, 0xe7, 0x5d, 0x6c, 0x05,
      0xda, 0xd5, 0xdf, 0x7a, 0xef, 0xf6, 0xdb, 0x1f, 0x82, 0x4c, 0xc0, 0x68, 0x47, 0xa1, 0xbd, 0xee,
      0x39, 0x50, 0x56, 0x4a, 0xdd, 0xdf, 0xa5, 0xf8, 0xc6, 0xda, 0xca, 0x90, 0xca, 0x01, 0x42, 0x9d,
      0x8b, 0x0c, 0x73, 0x43, 0x75, 0x05, 0x94, 0xde, 0x24, 0xb3, 0x80, 0x34, 0xe5, 0x2c, 0xdc, 0x9b,
      0x3f, 0xca, 0x33, 0x45, 0xd0, 0xdb, 0x5f, 0xf5, 0x52, 0xc3, 0x21, 0xda, 0xe2, 0x22, 0x72, 0x6b,
      0x3e, 0xd0, 0x5b, 0xa8, 0x87, 0x8c, 0x06, 0x5d, 0x0f, 0xdd, 0x09, 0x19, 0x93, 0xd0, 0xb9, 0xfc,
      0x8b, 0x0f, 0x84, 0x60, 0x33, 0x1c, 0x9b, 0x45, 0xf1, 0xf0, 0xa3, 0x94, 0x3a, 0x12, 0x77, 0x33,
      0x4d, 0x44, 0x78, 0x28, 0x3c, 0x9e, 0xfd, 0x65, 0x57, 0x16, 0x94, 0x6b, 0xfb, 0x59, 0xd0, 0xc8,
      0x22, 0x36, 0xdb, 0xd2, 0x63, 0x98, 0x43, 0xa1, 0x04, 0x87, 0x86, 0xf7, 0xa6, 0x26, 0xbb, 0xd6,
      0x59, 0x4d, 0xbf, 0x6a, 0x2e, 0xaa, 0x2b, 0xef, 0xe6, 0x78, 0xb6, 0x4e, 0xe0, 0x2f, 0xdc, 0x7c,
      0xbe, 0x57, 0x19, 0x32, 0x7e, 0x2a, 0xd0, 0xb8, 0xba, 0x29, 0x00, 0x3c, 0x52, 0x7d, 0xa8, 0x49,
      0x3b, 0x2d, 0xeb, 0x25, 0x49, 0xfa, 0xa3, 0xaa, 0x39, 0xa7, 0xc5, 0xa7, 0x50, 0x11, 0x36, 0xfb,
      0xc6, 0x67, 0x4a, 0xf5, 0xa5, 0x12, 0x65, 0x7e, 0xb0, 0xdf, 0xaf, 0x4e, 0xb3, 0x61, 0x7f, 0x2f }
};

/******************************************************************
                        Function Prototypes
*******************************************************************/
static void decrypt(uint8_t* data, uint32_t size, char* serial_number, uint32_t shutter_count);
static char* nikon_lens_id_lookup(uint8_t* key);
static float get_tiff_rational(struct ifd_entry_t* entry, void* buffer);
static char* get_makernote_string(struct ifd_entry_t* entry, void* buffer);

/******************************************************************
*
* \brief Decrypt Nikon lens data information.
*
* \details
*   Algorithm credited to Phil Harvey, creator of the EXIF Tool.
*   See https://github.com/exiftool/exiftool/blob/master/lib/Image/ExifTool/Nikon.pm.
*
* \param[in] data          : Pointer to encrypted data.
* \param[in] size          : Size of the data (in bytes) to be decrypted.
* \param[in] serial_number : Camera serial number. Used an encryption key.
* \param[in] shutter_count : Camera shutter count. Used an encryption key.
* \param[out] None
*
* \return None
*
*******************************************************************/
static void decrypt(uint8_t* data, uint32_t size, char* serial_number, uint32_t shutter_count)
{
    uint8_t key = 0;
    uint8_t ci, cj, ck;

    if ((NULL != data) && (size != 0))
    {
        // Serial number is used as a key
        uint64_t serial = strtoull(serial_number, NULL, 10);
        serial &= 0xFF;

        for (unsigned i = 0; i < 4; ++i)
        {
            // Shutter count is used as an encryption key
            key ^= (shutter_count >> (i * 8)) & 0xFF;
        }

        ci = xlat[0][serial];
        cj = xlat[1][key];
        ck = 0x60;

        for (unsigned i = 0; i < size; ++i)
        {
            cj = (cj + ci * ck) & 0xFF;
            ck = (ck + 1) & 0xFF;
            data[i] ^= cj;
        }
    }
}

/******************************************************************
*
* \details Helper function to look up Nikon lens ID in table.
*
* \param[in] key : Lens ID key to be matched.
* \param[out] None
*
* \return
*   Return lens ID information as a string if a match is found.
*   Otherwise, return NULL.
*
*******************************************************************/
static char* nikon_lens_id_lookup(uint8_t* key)
{
    char* id = NULL;
    // Calculate entries in look up table
    unsigned int entries = sizeof(nikon_lens_id_table) / sizeof(nikon_lens_id_table[0]);

    for (unsigned i = 0; i < entries; ++i)
    {
        if (memcmp(key, nikon_lens_id_table[i].tag, sizeof(nikon_lens_id_table[i].tag)) == 0)
        {
            id = nikon_lens_id_table[i].id;
            break;
        }
    }

    return id;
}

/******************************************************************
*
* \details Helper function get value of EXIF rational entries.
*
* \param[in] entry  : EXIF entry to be processed.
* \param[in] buffer : Pointer to image file buffer.
* \param[out] None
*
* \return
*   Return rational value of entry.
*
*******************************************************************/
static float get_tiff_rational(struct ifd_entry_t* entry, void* buffer)
{
    float rational = 0;

    if ((NULL != entry) && (NULL != buffer))
    {
        if (TIFF_TYPE_RATIONAL == entry->type)
        {
            uint32_t* data = (uint32_t*)buffer;
            unsigned offset = BYTES_TO_DWORDS(entry->value);
            float numerator = (float)data[offset];
            float denominator = (float)data[++offset];
            rational = numerator / denominator;
        }
        else
        {
            fprintf(stderr, "Error: Entry type is not RATIONAL.\n");
        }
    }
    else
    {
        fprintf(stderr, "Error: One or more NULL input arguments.\n");
    }

    return rational;
}

/******************************************************************
*
* \details Helper function get value of Makernote string entries.
*
* \param[in] entry  : Makernote entry to be processed.
* \param[in] buffer : Pointer to image file buffer.
* \param[out] None
*
* \return
*   Return pointer to entry ASCII string.
*
*******************************************************************/
static char* get_makernote_string(struct ifd_entry_t* entry, void* buffer)
{
    char* str = NULL;
    
    if ((NULL != entry) && (NULL != buffer))
    {
        if (TIFF_TYPE_ASCII == entry->type)
        {
            if (entry->count > sizeof(uint32_t))
            {
                nef_debug_print("Count = %u\n", entry->count);
                uint8_t* data = (uint8_t*)buffer;
                // Offset is relative to the beginning of the Makernote TIFF header.
                // Unlike the other IFD structures, which use an absolute offset.
                uint32_t offset = makernote_offset + tiff_offset + entry->value;
                str = (char*)&data[offset];
            }
            else
            {
                str = (char*)&entry->value;
            }
        }
        else
        {
            fprintf(stderr, "Error: Entry type is not ASCII.\n");
        }
    }
    else
    {
        fprintf(stderr, "Error: One or more NULL input arguments.\n");
    }

    return str;
}

/* Main */
int main(int argc, char** argv)
{
    bool error = false;
    FILE* nef_file = NULL;
    uint8_t* buffer = NULL;
    long size = 0;
    uint32_t offset = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Error: Too few input arguments. Please specify a .NEF file to process.\n");
        error = true;
    }

    if (!error)
    {
        printf("%s", banner);
        char* extension;
        extension = strrchr(argv[1], '.');
        // Verify file extension is correct
        if (strcmp(++extension, "NEF") != 0)
        {
            fprintf(stderr, "Error: Unsupported file type .%s. Please specify a .NEF file to process.\n", extension);
            error = true;
        }
    }

    if (!error)
    {
        fopen_s(&nef_file, argv[1], "rb");

        if (nef_file == NULL)
        {
            fprintf(stderr, "Error: Failed to open %s.\n", argv[1]);
            error = true;
        }
        else
        {
            // Extract file name from path
            char* filename = strrchr(argv[1], '\\');

            if (NULL != filename)
            {
                printf("File = %s\n", ++filename);
            }
            else
            {
                printf("File = %s\n", argv[1]);
            }

            fseek(nef_file, 0, SEEK_END);
            size = ftell(nef_file);
            rewind(nef_file);
            nef_debug_print("NEF File Size = %d bytes\n", size);
            buffer = malloc(size);

            if (buffer == NULL)
            {
                fprintf(stderr, "Error: Insufficient memory to allocate buffer.\n");
                error = true;
            }
            else
            {
                // Read entire file into buffer
                fread_s(buffer, size, size, 1, nef_file);
                nef_header_t* nef_header = (nef_header_t*)buffer;

                // Validate NEF header
                if (nef_header->tiff_magic != TIFF_MAGIC ||
                    nef_header->byte_order != TIFF_LITTLE_ENDIAN)
                {
                    fprintf(stderr, "Error: Invalid NEF.\n");
                }
                else
                {
                    nef_debug_print("Valid NEF File.\n");
                    nef_debug_print("Processing IFD0 entries...\n");
                    struct ifd_t* ifd0 = (struct ifd_t*)&buffer[nef_header->ifd0_offset];
                    nef_debug_print("IFD0 Entries = %d\n", ifd0->entries);
                    uint32_t subifd_offset = 0;
                    uint32_t exif_offset = 0;

                    for (unsigned i = 0; i < ifd0->entries; ++i)
                    {
#if NEF_VERBOSE_DEBUG
                        printf("IFD0 Tag = 0x%04X\n", ifd0->entry[i].tag);
#endif                   
                        switch (ifd0->entry[i].tag)
                        {
                        case EXIF_TAG_EXIF_OFFSET:
                            exif_offset = ifd0->entry[i].value;
                            break;
                        case EXIF_TAG_MODEL:
                        {
                            char* model = (char*)&buffer[ifd0->entry[i].value];
                            printf("Camera Model = %s\n", model);
                            break;
                        }
                        case EXIF_TAG_SUBIFD_OFFSET:
                            // Entry word count determines if value is an offset or the actual value
                            subifd_offset = (ifd0->entry[i].count > 2) ? *((uint32_t*)&buffer[ifd0->entry[i].value]) : ifd0->entry[i].value;
                            nef_debug_print("Sub-IFD Offset = 0x%08X\n", subifd_offset);
                            break;
                        case EXIF_TAG_DATE_TIME_ORIGINAL:
                        {
                            char* timestamp = (char*)&buffer[ifd0->entry[i].value];
                            printf("Time Stamp = %s\n", timestamp);
                            break;
                        }
                        default:
                            break;
                        }
                    }

                    // Sub-IFD stores the image as a lossy jpeg
                    // Calculate number of sub-IFD entries
                    struct ifd_t* subifd = (struct ifd_t*)&buffer[subifd_offset];
                    nef_debug_print("Sub-IFD Entries = %d\n", subifd->entries);

                    for (unsigned i = 0; i < subifd->entries; ++i)
                    {
#if NEF_VERBOSE_DEBUG
                        //TODO: Anything useful to do here?
                        printf("Sub-IFD Tag = 0x%04X\n", subifd->entry[i].tag);
#endif
                    }

                    // Next IFD offset is located after the last IFD entry
                    offset = sizeof(nef_header_t) + sizeof(uint16_t) + (ifd0->entries * sizeof(struct ifd_entry_t));
                    uint32_t next_ifd_offset = *((uint32_t*)&buffer[offset]);

                    if (next_ifd_offset == 0)
                    {
                        nef_debug_print("No IFD1 discovered.\n");
                    }
                    else
                    {
                        nef_debug_print("Discovered an additional IFD.\n");
                        //TODO: Process IFD
                    }

                    nef_debug_print("Processing IFD0 EXIF data...\n");
                    struct ifd_t* exif = (struct ifd_t*)&buffer[exif_offset];
                    nef_debug_print("EXIF IFD Entries = %d\n", exif->entries);

                    for (unsigned i = 0; i < exif->entries; ++i)
                    {
#if NEF_VERBOSE_DEBUG
                        printf("EXIF Tag = 0x%04X\n", exif->entry[i].tag);
#endif
                        switch (exif->entry[i].tag)
                        {
                        case EXIF_TAG_MAKERNOTE:
                            makernote_offset = exif->entry[i].value;
                            break;
                        case EXIF_TAG_EXPOSURE_TIME:
                        {
                            float shutter = get_tiff_rational(&exif->entry[i], buffer);
                            // FIXME: Update to account for slow shutter speeds (>= 1s) 
                            printf("Shutter Speed = 1/%.0f second\n", 1 / shutter);
                            break;
                        }
                        case EXIF_TAG_FNUMBER:
                        {
                            float aperature = get_tiff_rational(&exif->entry[i], buffer);
                            printf("Aperature = f/%.1f\n", aperature);
                            break;
                        }
                        case EXIF_TAG_METERING_MODE:
                        {
                            printf("Metering Mode = ");
                            switch (exif->entry[i].value)
                            {
                            case 0:
                                printf("Unknown\n");
                                break;
                            case 1:
                                printf("Average\n");
                                break;
                            case 2:
                                printf("Center-Weighted\n");
                                break;
                            case 3:
                                printf("Spot\n");
                                break;
                            case 4:
                                printf("Multi-Spot\n");
                                break;
                            case 5:
                                printf("Multi-Segment\n");
                                break;
                            case 6:
                                printf("Partial\n");
                                break;
                            default:
                                printf("Other\n");
                                break;
                            }

                            break;
                        }
                        case EXIF_TAG_FOCAL_LENGTH:
                        {
                            float focal_length = get_tiff_rational(&exif->entry[i], buffer);
                            printf("Focal Length = %.2f mm\n", focal_length);
                        }
                        default:
                            break;
                        }
                    }

                    nef_debug_print("Processing Nikon Makernote...\n");
                    struct makernote_header_t* makernote_header = (struct makernote_header_t*)&buffer[makernote_offset];
                    nef_debug_print("Makernote Magic Value = %s\n", makernote_header->magic_value);

                    if (strcmp(makernote_header->magic_value, MAKERNOTE_MAGIC) == 0)
                    {
                        // Limit scope to Makernote processing
                        struct ifd_entry_t* lens_data = NULL;
                        uint8_t lens_type = 0;
                        uint32_t shutter_count = 0;
                        char* serial_number = 0;

                        offset = makernote_offset + sizeof(struct makernote_header_t);
                        nef_debug_print("Makernote IFD Offset = %d\n", makernote_header->tiff_hdr.ifd0_offset);
                        struct ifd_t* makernote = (struct ifd_t*)&buffer[offset];
                        nef_debug_print("Makernote IFD Entries = %d\n", makernote->entries);
                        tiff_offset = sizeof(struct makernote_header_t) - sizeof(struct tiff_header_t);

                        for (unsigned i = 0; i < makernote->entries; ++i)
                        {
#if NEF_VERBOSE_DEBUG
                            printf("Makernote Tag = 0x%04X\n", makernote->entry[i].tag);
#endif
                            switch (makernote->entry[i].tag)
                            {
                            case NIKON_TAG_MAKERNOTE_VERSION:
                            {
                                // Makernote version is an undefined type and must be
                                // handled differently than other EXIF string types.
                                unsigned size = makernote->entry[i].count + 1;
                                char* makernote_version = malloc(size);

                                if (NULL != makernote_version)
                                {
                                    strncpy_s(makernote_version, size, (char*)&makernote->entry[i].value, makernote->entry[i].count);
                                    makernote_version[--size] = '\0';
                                    nef_debug_print("Makernote Version = \"%s\"\n", makernote_version);
                                    free(makernote_version);
                                }

                                break;
                            }
                            case NIKON_TAG_SHUTTER_COUNT:
                                shutter_count = makernote->entry[i].value;
                                printf("Shutter Count = %u\n", shutter_count);
                                break;
                            case NIKON_TAG_FOCUS_MODE:
                            {
                                char* focus_mode = get_makernote_string(&makernote->entry[i], buffer);
                                printf("Focus Mode = %s\n", focus_mode);
                                break;
                            }
                            case NIKON_TAG_QUALITY:
                            {
                                char* quality = get_makernote_string(&makernote->entry[i], buffer);
                                printf("Quality = %s\n", quality);
                                break;
                            }
                            case NIKON_TAG_WHITE_BALANCE:
                            {
                                char* white_balance = get_makernote_string(&makernote->entry[i], buffer);
                                printf("White Balance = %s\n", white_balance);
                                break;
                            }
                            case NIKON_TAG_SERIAL_NUMBER:
                            {
                                serial_number = get_makernote_string(&makernote->entry[i], buffer);
                                printf("Camera Serial Number = %s\n", serial_number);
                                break;
                            }
                            case NIKON_TAG_ISO_INFO:
                            {
                                offset = makernote_offset + tiff_offset + makernote->entry[i].value;
                                // Calculate the ISO value
                                double raw = (double)buffer[offset];
                                uint32_t iso = 100 * pow(2, raw / 12 - 5);
                                unsigned remainder = iso % 10;
                                // Raw ISO value is stored as a single byte.
                                // Need to round up if value is not divisble by 10.
                                if (remainder != 0) iso += 10 - remainder;
                                printf("Image ISO = %u\n", iso);
                                break;
                            }
                            case NIKON_TAG_LENS_TYPE:
                            {
                                // Used as last bye of lens ID composite tag
                                lens_type = makernote->entry[i].value & 0xFF;
                                break;
                            }
                            case NIKON_TAG_LENS_DATA:
                            {
                                // Need shutter count and serial number before processing lens data
                                lens_data = &makernote->entry[i];
                                break;
                            }
                            default:
                                break;
                            }
                        }

                        if (NULL != lens_data)
                        {
                            char version[5];
                            offset = makernote_offset + tiff_offset + lens_data->value;
                            strncpy_s(version, sizeof(version), (char*)&buffer[offset], sizeof(version) - 1);
                            version[4] = '\0'; // Lens data version is not NULL terminated
                            uint32_t lens_data_version = atoi(version);
                            nef_debug_print("Lens Data Version = %u\n", lens_data_version);

                            // Lens data is encrypted if the version is 0201 or greater
                            if (lens_data_version >= LENS_DATA_0201)
                            {
                                nef_debug_print("Nikon lens data is encrypted. Decrypting data...\n");
                                // Encrypted data begins after version string
                                decrypt(&buffer[offset + 4], lens_data->count - 4, serial_number, shutter_count);
                            }

                            // Construct Lens ID composite tag
                            // See https://exiftool.org/TagNames/Nikon.html#LensData00
                            uint8_t lens_id[8];
                            memcpy_s(lens_id, sizeof(lens_id), &buffer[offset + LENS_ID_OFFSET], sizeof(lens_id) - 1);
                            lens_id[7] = lens_type;
                            char* lens_model = nikon_lens_id_lookup(lens_id);
                            printf("Camera Lens = ");

                            if (NULL != lens_model)
                            {
                                printf("%s\n", lens_model);
                            }
                            else
                            {
                                printf("Unknown Model.\n");
                            }
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error: Invalid Makernote.\n");
                    }

                    free(buffer);
                }
            }

            fclose(nef_file);
        }
    }

    return 0;
}