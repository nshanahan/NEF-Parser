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
*******************************************************************/

/******************************************************************
                        Includes
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nef.h"
#include "tiff.h"
#include "exif.h"

/******************************************************************
                        Defines
*******************************************************************/
const char banner[] = "**********************************************\n"
                      "*           NEF Parser Tool (2020)           *\n"
                      "**********************************************\n\n";

/******************************************************************
                        Macros
*******************************************************************/
//#define nef_debug_print(...) printf(__VA_ARGS__)
#define nef_debug_print(...)

/******************************************************************
                        Function Prototypes
*******************************************************************/
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
                    uint16_t ifd0_entries = *((uint16_t*)&buffer[nef_header->ifd0_offset]);
                    nef_debug_print("IFD0 Entries = %d\n", ifd0_entries);
                    offset = sizeof(nef_header_t) + sizeof(uint16_t);
                    struct ifd_entry_t* entry = (struct ifd_entry_t*)&buffer[offset];
                    uint32_t subifd_offset = 0;
                    uint32_t exif_offset = 0;

                    for (unsigned i = 0; i < ifd0_entries; ++i)
                    {
                        switch (entry->tag)
                        {
                        case EXIF_TAG_EXIF_OFFSET:
                            exif_offset = entry->value;
                            break;
                        case EXIF_TAG_MODEL:
                        {
                            // Limit variable scope to this case
                            char* model = (char*)&buffer[entry->value];
                            model[--entry->count] = '\0';
                            printf("Camera Model = %s\n", model);
                            break;
                        }
                        case EXIF_TAG_SUBIFD_OFFSET:
                            // Entry word count determines if value is an offset or the actual value
                            subifd_offset = (entry->count > 2) ? *((uint32_t*)&buffer[entry->value]) : entry->value;
                            nef_debug_print("Sub-IFD Offset = 0x%08X\n", subifd_offset);
                            break;
                        default:
                            break;
                        }

                        entry++;
                    }

                    // Sub-IFD stores the image as a lossy jpeg
                    // Calculate number of sub-IFD entries
                    uint16_t subifd_entries = *((uint16_t*)&buffer[subifd_offset]);
                    nef_debug_print("Sub-IFD Entries = %d\n", subifd_entries);
                    offset = subifd_offset + sizeof(uint16_t);
                    entry = (struct ifd_entry_t*)&buffer[offset];

                    for (unsigned i = 0; i < subifd_entries; ++i)
                    {
                        //TODO: Anything useful to do here?
                        entry++;
                    }

                    // Next IFD offset is located after the last IFD entry
                    offset = sizeof(nef_header_t) + sizeof(uint16_t) + (ifd0_entries * sizeof(struct ifd_entry_t));
                    uint32_t next_ifd_offset = *((uint32_t*)&buffer[offset]);

                    if (next_ifd_offset == 0)
                    {
                        nef_debug_print("No other IFD discovered.\n");
                    }

                    uint16_t exif_entries = *((uint16_t*)&buffer[exif_offset]);
                    nef_debug_print("EXIF IFD Entries = %d\n", exif_entries);
                    offset = exif_offset + sizeof(uint16_t);
                    entry = (struct ifd_entry_t*)&buffer[offset];
                    uint32_t makernote_offset = 0;

                    for (unsigned i = 0; i < exif_entries; ++i)
                    {
                        if (entry->tag == EXIF_TAG_MAKERNOTE)
                        {
                            makernote_offset = entry->value;
                        }

                        entry++;
                    }

                    struct makernote_header_t* makernote_header = (struct makernote_header_t*)&buffer[makernote_offset];
                    // Magic value string is zero terminated. Replace with null terminator.
                    makernote_header->magic_value[5] = '\0';
                    nef_debug_print("Makernote Magic Value = %s\n", makernote_header->magic_value);

                    if (strcmp(makernote_header->magic_value, MAKERNOTE_MAGIC) == 0)
                    {
                        offset = makernote_offset + sizeof(struct makernote_header_t);
                        uint16_t makernote_entries = *((uint16_t*)&buffer[offset]);
                        nef_debug_print("Makernote IFD Entries = %d\n", makernote_entries);
                        offset = makernote_offset + sizeof(struct makernote_header_t) + sizeof(uint16_t);
                        entry = (struct ifd_entry_t*)&buffer[offset];

                        for (unsigned i = 0; i < makernote_entries; ++i)
                        {
                            switch (entry->tag)
                            {
                            case NIKON_TAG_MAKERNOTE_VERSION:
                            {
                                // Makernote version is an undefined type and must be
                                // handled differently than other EXIF string types.
                                unsigned size = entry->count + 1;
                                char* makernote_version = malloc(size);

                                if (NULL != makernote_version)
                                {
                                    strncpy_s(makernote_version, size, (char*)&entry->value, entry->count);
                                    makernote_version[--size] = '\0';
                                    nef_debug_print("Makernote Version = \"%s\"\n", makernote_version);
                                }

                                break;
                            }
                            case NIKON_TAG_SHUTTER_COUNT:
                                printf("Shutter Count = %u\n", entry->value);
                                break;
                            default:
                                break;
                            }

                            entry++;
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