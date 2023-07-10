#include <midi20_ci.h>
#include <stdlib.h>
#include <string.h>
#include "midi_spec.h"

#define MAN_ID_HI 0x00
#define MAN_ID_MD 0x21
#define MAN_ID_LO 0x4E

#define CI_MAX_SYSEX_SIZE 0x100

#pragma pack(push, 1)
typedef struct
{
    uint8_t sysex_start;
    uint8_t universal_sysex;
    uint8_t device_id;
    uint8_t sub_id1;
    uint8_t sub_id2;
    uint8_t ci_version;
    uint32_t source_muid;
    uint32_t destination_muid;
} MIDI_CI_HEADER_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t manufacturer[3];
    uint8_t family[2];
    uint8_t family_model[2];
    uint8_t software_revision[4];
    uint8_t capability_category;
    uint8_t max_sysex_size[4];
    uint8_t sysex_end;
} MIDI_CI_DISCOVERY_REQUEST_VERSION_1_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t manufacturer[3];
    uint8_t family[2];
    uint8_t family_model[2];
    uint8_t software_revision[4];
    uint8_t capability_category;
    uint8_t max_sysex_size[4];
    uint8_t output_path_id;
    uint8_t sysex_end;
} MIDI_CI_DISCOVERY_REQUEST_VERSION_2_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t manufacturer[3];
    uint8_t family[2];
    uint8_t family_model[2];
    uint8_t software_revision[4];
    uint8_t capability_category;
    uint8_t max_sysex_size[4];
    uint8_t sysex_end;
} MIDI_CI_DISCOVERY_REPLY_VERSION_1_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t manufacturer[3];
    uint8_t family[2];
    uint8_t family_model[2];
    uint8_t software_revision[4];
    uint8_t capability_category;
    uint8_t max_sysex_size[4];
    uint8_t output_path_id;
    uint8_t function_block;
    uint8_t sysex_end;
} MIDI_CI_DISCOVERY_REPLY_VERSION_2_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t status;
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_ENDPOINT_REQUEST_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t status;
    uint16_t length;
    char product_id[16];
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_ENDPOINT_REPLY_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t sysex_end;
} MIDI_CI_NAK_REPLY_VERSION_1_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t original_sub_id2;
    uint8_t nak_code;
    uint8_t nak_data;
    uint8_t nak_details[5];
    uint16_t length;
    char message[32];
    uint8_t sysex_end;
} MIDI_CI_NAK_REPLY_VERSION_2_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t original_sub_id2;
    uint8_t ack_code;
    uint8_t ack_data;
    uint8_t ack_details[5];
    uint16_t length;
    char message[32];
    uint8_t sysex_end;
} MIDI_CI_ACK_REPLY_VERSION_2_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t requets_supported;
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_1_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t requets_supported;
    uint8_t major_version;
    uint8_t minor_version;
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_2_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t requets_supported;
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_PROP_EX_CAPS_REPLY_VERSION_1_T;

typedef struct
{
    MIDI_CI_HEADER_T header;
    uint8_t requets_supported;
    uint8_t major_version;
    uint8_t minor_version;
    uint8_t sysex_end;
} MIDI_CI_INQUIRY_PROP_EX_CAPS_REPLY_VERSION_2_T;
#pragma pack(pop)

uint32_t _midi_ci_muid;
ci_process_callback _process_callback = NULL;

void midi20_ci_init()
{
    _midi_ci_muid = rand();
}

void midi20_ci_build_header(MIDI_CI_HEADER_T *prequest_header, MIDI_CI_HEADER_T *preply_header, uint8_t sub_id2)
{
    preply_header->sysex_start = MIDI_SYSEX_START;
    preply_header->universal_sysex = MIDI20_UNIVERSAL_SYSEX;
    preply_header->device_id = prequest_header->device_id;
    preply_header->sub_id1 = MIDI20_UNIVERSAL_SYSEX_SUBID1_CI;
    preply_header->sub_id2 = sub_id2;
    preply_header->ci_version = prequest_header->ci_version;
    preply_header->source_muid = _midi_ci_muid;
    preply_header->destination_muid = prequest_header->source_muid;
}

void midi20_ci_process_discovery1(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_DISCOVERY_REQUEST_VERSION_1_T *prequest = (MIDI_CI_DISCOVERY_REQUEST_VERSION_1_T*)pmessage;

    MIDI_CI_DISCOVERY_REPLY_VERSION_1_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(&prequest->header, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_DISCOVERY_REPLY);
    reply.manufacturer[0] = MAN_ID_LO;
    reply.manufacturer[1] = MAN_ID_MD;
    reply.manufacturer[2] = MAN_ID_HI;
    reply.family[0] = 0;
    reply.family[1] = 0;
    reply.family_model[0] = 0;
    reply.family_model[1] = 0;
    reply.software_revision[0] = 0;
    reply.software_revision[1] = 0;
    reply.software_revision[2] = 0;
    reply.software_revision[3] = 0;
    reply.capability_category = MIDI20_CI_CATEGORY_PROPERTY_EXCHANGE;
    reply.max_sysex_size[0] = (CI_MAX_SYSEX_SIZE >> 0) & 0x7F;
    reply.max_sysex_size[1] = (CI_MAX_SYSEX_SIZE >> 7) & 0x7F;
    reply.max_sysex_size[2] = (CI_MAX_SYSEX_SIZE >> 14) & 0x7F;
    reply.max_sysex_size[3] = (CI_MAX_SYSEX_SIZE >> 21) & 0x7F;
    reply.sysex_end = MIDI_SYSEX_END;

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_process_discovery2(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_DISCOVERY_REQUEST_VERSION_2_T *prequest = (MIDI_CI_DISCOVERY_REQUEST_VERSION_2_T*)pmessage;

    MIDI_CI_DISCOVERY_REPLY_VERSION_2_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(&prequest->header, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_DISCOVERY_REPLY);
    reply.manufacturer[0] = MAN_ID_LO;
    reply.manufacturer[1] = MAN_ID_MD;
    reply.manufacturer[2] = MAN_ID_HI;
    reply.family[0] = 0;
    reply.family[1] = 0;
    reply.family_model[0] = 0;
    reply.family_model[1] = 0;
    reply.software_revision[0] = 0;
    reply.software_revision[1] = 0;
    reply.software_revision[2] = 0;
    reply.software_revision[3] = 0;
    reply.capability_category = MIDI20_CI_CATEGORY_PROPERTY_EXCHANGE;
    reply.max_sysex_size[0] = (CI_MAX_SYSEX_SIZE >> 0) & 0x7F;
    reply.max_sysex_size[1] = (CI_MAX_SYSEX_SIZE >> 7) & 0x7F;
    reply.max_sysex_size[2] = (CI_MAX_SYSEX_SIZE >> 14) & 0x7F;
    reply.max_sysex_size[3] = (CI_MAX_SYSEX_SIZE >> 21) & 0x7F;
    reply.output_path_id = prequest->output_path_id;
    reply.function_block = 0;
    reply.sysex_end = MIDI_SYSEX_END;

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_process_inquiry_endpoint(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_INQUIRY_ENDPOINT_REQUEST_T *prequest = (MIDI_CI_INQUIRY_ENDPOINT_REQUEST_T*)pmessage;

    MIDI_CI_INQUIRY_ENDPOINT_REPLY_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(&prequest->header, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_ENDPOINT_REPLY);
    reply.status = 0;
    reply.length = 16;
    strcpy(reply.product_id, "AKM320_CUSTOM");
    reply.sysex_end = MIDI_SYSEX_END;

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_nak1(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_HEADER_T *pheader = (MIDI_CI_HEADER_T*)pmessage;

    MIDI_CI_NAK_REPLY_VERSION_1_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(pheader, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_NAK);

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_nak2(uint8_t *pmessage, uint32_t length, uint8_t status_code, const char *ptext)
{
    MIDI_CI_HEADER_T *pheader = (MIDI_CI_HEADER_T*)pmessage;

    MIDI_CI_NAK_REPLY_VERSION_2_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(pheader, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_NAK);
    reply.original_sub_id2 = pheader->sub_id2;
    reply.nak_code = status_code;
    reply.length = 32;
    strcpy(reply.message, ptext);

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_ack(uint8_t *pmessage, uint32_t length, uint8_t status_code, const char *ptext)
{
    MIDI_CI_HEADER_T *pheader = (MIDI_CI_HEADER_T*)pmessage;

    MIDI_CI_ACK_REPLY_VERSION_2_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(pheader, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_ACK);
    reply.original_sub_id2 = pheader->sub_id2;
    reply.ack_code = status_code;
    reply.length = 32;
    strcpy(reply.message, ptext);

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_process_inquiry_prop_ex_caps1(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_1_T *prequest = (MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_1_T*)pmessage;

    MIDI_CI_INQUIRY_PROP_EX_CAPS_REPLY_VERSION_1_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(&prequest->header, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_CAPS_REPLY);
    reply.requets_supported = 1;
    reply.sysex_end = MIDI_SYSEX_END;

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_process_inquiry_prop_ex_caps2(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_2_T *prequest = (MIDI_CI_INQUIRY_PROP_EX_CAPS_REQUEST_VERSION_2_T*)pmessage;

    MIDI_CI_INQUIRY_PROP_EX_CAPS_REPLY_VERSION_2_T reply;
    memset(&reply, 0, sizeof(reply));
    midi20_ci_build_header(&prequest->header, &reply.header, MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_CAPS_REPLY);
    reply.requets_supported = 1;
    reply.major_version = 0;
    reply.minor_version = 0;
    reply.sysex_end = MIDI_SYSEX_END;

    if(_process_callback)
        _process_callback((uint8_t*)&reply, sizeof(reply));
}

void midi20_ci_reply_prop_get(MIDI_CI_HEADER_T *prequest_header, uint8_t request_id,
    const char *pheader_data, const char *pproperty_data,
    uint16_t chunk, uint16_t total_chunks)
{
    uint16_t header_length = strlen(pheader_data);
    uint16_t property_length = strlen(pproperty_data);
    uint8_t *preply = (uint8_t*)malloc(sizeof(MIDI_CI_HEADER_T) + header_length + property_length + 10);

    if(preply)
    {
        memset(preply, 0, sizeof(MIDI_CI_HEADER_T) + header_length + property_length + 10);
        midi20_ci_build_header(prequest_header, (MIDI_CI_HEADER_T*)preply, MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_GET_REPLY);
        uint32_t index = sizeof(MIDI_CI_HEADER_T);

        preply[index++] = request_id;                       // request_id
        preply[index++] = header_length;                    // header length
        preply[index++] = 0;
        memcpy(preply, pheader_data, header_length);        // header data
        index += header_length;
        preply[index++] = total_chunks;                     // chunks in message
        preply[index++] = total_chunks >> 8;
        preply[index++] = chunk;                            // chunk
        preply[index++] = chunk >> 8;
        preply[index++] = property_length & 0x7F;           // property length
        preply[index++] = (property_length >> 7) & 0x7F;
        memcpy(preply, pproperty_data, property_length);    // property data
        index += property_length;
        preply[index++] = MIDI_SYSEX_END;                   // sysex end

        if(_process_callback)
            _process_callback(preply, index);

        free(preply);
    }
}

__attribute__((weak)) MIDI20_CI_RESULT_T midi20_ci_get_prop_manufacturer(
    const char *pheader_data, uint16_t header_data_length,
    char *pproperty_data, uint16_t *pproperty_data_length,
    uint16_t chunk, uint16_t *ptotal_chunks)
{
    return MIDI20_CI_RESULT_UNKNOWN_PROPERTY;
}

void midi20_ci_process_inquiry_prop_ex_get(uint8_t *pmessage, uint32_t length)
{
    MIDI_CI_HEADER_T *pheader = (MIDI_CI_HEADER_T*)pmessage;
    uint32_t index = sizeof(MIDI_CI_HEADER_T);
    uint8_t request_id;
    uint16_t header_data_length;
    char *pheader_data;
//    uint16_t chunks_in_message;
//    uint16_t chunk_number;

    request_id = pmessage[index];
    index += 1;
    header_data_length = pmessage[index] | (pmessage[index+1] << 8);
    index += 2;
    pheader_data = (char*)&pmessage[index];
//    index += header_data_length;
//    chunks_in_message = pmessage[index] | (pmessage[index+1] << 8);
//    index += 2;
//    chunk_number = pmessage[index] | (pmessage[index] << 8);

    if(header_data_length == 0)
    {
        if(pheader->ci_version == 1)
            midi20_ci_nak1(pmessage, length);
        else
            midi20_ci_nak2(pmessage, length, MIDI20_CI_NAK_STATUS_CODE_MESSAGE_MALFORMED, "zero length header data");

        return;
    }

    // TODO: validate chunk

    if(strnstr(pheader_data, "resource", header_data_length) != NULL)
    {
        char prop_buffer[256];
        uint16_t prop_length = 0;
        uint16_t total_chunks = 1;
        uint16_t chunk = 0;
        MIDI20_CI_RESULT_T result;

        while(chunk < total_chunks)
        {
            result = midi20_ci_get_prop_manufacturer(pheader_data, header_data_length, prop_buffer, &prop_length, chunk, &total_chunks);

            if(result == MIDI20_CI_RESULT_SUCCESS)
            {
                midi20_ci_reply_prop_get(pheader, request_id, "{\"status\":200}", prop_buffer, chunk, total_chunks);
                chunk++;
            }
            else
            {
                if(pheader->ci_version == 1)
                    midi20_ci_nak1(pmessage, length);
                else
                    midi20_ci_nak2(pmessage, length, MIDI20_CI_NAK_STATUS_CODE_CI_MESSAGE_NOT_SUPPORTED, "unsupported resource");

                break;
            }
        }
    }
}

void midi20_ci_process_inquiry_prop_ex_set(uint8_t *pmessage, uint32_t length)
{
}

void midi20_ci_process(uint8_t *pmessage, uint32_t length, ci_process_callback callback)
{
    MIDI_CI_HEADER_T *pheader = (MIDI_CI_HEADER_T*)pmessage;
    _process_callback = callback;

    // min length requirement
    if(length < 15)
        return;

    // validate start
    if(pheader->sysex_start != MIDI_SYSEX_START)
        return;

    // validate universal sysex
    if(pheader->universal_sysex != MIDI20_UNIVERSAL_SYSEX)
        return;

    // verify this is a ci message
    if(pheader->sub_id1 != MIDI20_UNIVERSAL_SYSEX_SUBID1_CI)
        return;

    // process discovery message
    if(pheader->sub_id2 == MIDI20_UNIVERSAL_SYSEX_SUBID2_DISCOVERY)
    {
        if(pheader->ci_version == 1)
            midi20_ci_process_discovery1(pmessage, length);
        else
            midi20_ci_process_discovery2(pmessage, length);
        return;
    }
    // process inquiry endpoint message
    else if(pheader->sub_id2 == MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_ENDPOINT)
    {
        midi20_ci_process_inquiry_endpoint(pmessage, length);
        return;
    }
    // process inquiry property exchange
    if(pheader->sub_id2 == MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_CAPS)
    {
        if(pheader->ci_version == 1)
            midi20_ci_process_inquiry_prop_ex_caps1(pmessage, length);
        else
            midi20_ci_process_inquiry_prop_ex_caps2(pmessage, length);
        return;
    }
    // process inquiry property exchange get
    else if(pheader->sub_id2 == MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_GET)
    {
        midi20_ci_process_inquiry_prop_ex_get(pmessage, length);
        return;
    }
    // process inquiry property exchange set
    else if(pheader->sub_id2 == MIDI20_UNIVERSAL_SYSEX_SUBID2_INQUIRY_PROP_EX_SET)
    {
        midi20_ci_process_inquiry_prop_ex_set(pmessage, length);
        return;
    }
    // process unhandled message with nak
    else
    {
        if(pheader->ci_version == 1)
            midi20_ci_nak1(pmessage, length);
        else
            midi20_ci_nak2(pmessage, length, MIDI20_CI_NAK_STATUS_CODE_CI_MESSAGE_NOT_SUPPORTED, "message not supported");
        return;
    }
}
