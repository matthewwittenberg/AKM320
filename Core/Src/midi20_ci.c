#include <midi20_ci.h>
#include <stdlib.h>
#include <string.h>
#include "midi_spec.h"

extern const uint8_t MAN_ID_HI;
extern const uint8_t MAN_ID_MD;
extern const uint8_t MAN_ID_LO;
extern const char* PRODUCT_NAME;
extern const uint16_t PRODUCT_FAMILY;
extern const uint16_t PRODUCT_MODEL;
extern const uint32_t PRODUCT_SOFTWARE_VERSION;

#define CI_MAX_SYSEX_SIZE 128

#pragma pack(push, 1)
typedef struct
{
    uint8_t sysex_start;
    uint8_t universal_sysex;
    uint8_t device_id;
    uint8_t sub_id1;
    uint8_t sub_id2;
    uint8_t ci_version;
    uint8_t source_muid[4];
    uint8_t destination_muid[4];
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
    uint8_t length[2];
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
    uint8_t length[2];
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
    uint8_t length[2];
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
char _ci_property_buffer[256];

uint32_t bytes_to_uint32(uint8_t *pbytes)
{
    uint32_t value = 0;

    value |= (pbytes[0] << 0) & 0x0000007F;
    value |= (pbytes[1] << 7) & 0x00003F80;
    value |= (pbytes[2] << 14) & 0x001FC000;
    value |= (pbytes[3] << 21) & 0x0FE00000;

    return value;
}

void uint32_to_bytes(uint32_t value, uint8_t *pbytes)
{
    pbytes[0] = (value >> 0) & 0x7F;
    pbytes[1] = (value >> 7) & 0x7F;
    pbytes[2] = (value >> 14) & 0x7F;
    pbytes[3] = (value >> 21) & 0x7F;
}

uint16_t bytes_to_uint16(uint8_t *pbytes)
{
    uint16_t value = 0;

    value |= (pbytes[0] << 0) & 0x0000007F;
    value |= (pbytes[1] << 7) & 0x00003F80;

    return value;
}

void uint16_to_bytes(uint16_t value, uint8_t *pbytes)
{
    pbytes[0] = (value >> 0) & 0x7F;
    pbytes[1] = (value >> 7) & 0x7F;
}

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
    uint32_to_bytes(_midi_ci_muid, preply_header->source_muid);
    memcpy(preply_header->destination_muid, prequest_header->source_muid, 4);
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
    uint16_to_bytes(PRODUCT_FAMILY, reply.family);
    uint16_to_bytes(PRODUCT_MODEL, reply.family_model);
    uint32_to_bytes(PRODUCT_SOFTWARE_VERSION, reply.software_revision);
    reply.capability_category = MIDI20_CI_CATEGORY_PROPERTY_EXCHANGE;
    uint32_to_bytes(CI_MAX_SYSEX_SIZE, reply.max_sysex_size);
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
    uint16_to_bytes(PRODUCT_FAMILY, reply.family);
    uint16_to_bytes(PRODUCT_MODEL, reply.family_model);
    uint32_to_bytes(PRODUCT_SOFTWARE_VERSION, reply.software_revision);
    reply.capability_category = MIDI20_CI_CATEGORY_PROPERTY_EXCHANGE;
    uint32_to_bytes(CI_MAX_SYSEX_SIZE, reply.max_sysex_size);
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
    uint16_to_bytes(16, reply.length);
    strcpy(reply.product_id, PRODUCT_NAME);
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
    uint16_to_bytes(32, reply.length);
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
    uint16_to_bytes(32, reply.length);
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

        // request id
        preply[index] = request_id;
        index++;

        // header length
        uint16_to_bytes(header_length, &preply[index]);
        index += 2;

        // header data
        memcpy(&preply[index], pheader_data, header_length);
        index += header_length;

        // chunks in message
        uint16_to_bytes(total_chunks, &preply[index]);
        index += 2;

        // chunk
        uint16_to_bytes(chunk, &preply[index]);
        index += 2;

        // property length
        uint16_to_bytes(property_length, &preply[index]);
        index += 2;

        // property data
        memcpy(&preply[index], pproperty_data, property_length);
        index += property_length;

        // sysex end
        preply[index++] = MIDI_SYSEX_END;

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
    uint16_t chunks_in_message;
    uint16_t chunk_number;

    // request id
    request_id = pmessage[index];
    index += 1;

    // header data length
    header_data_length = bytes_to_uint16(&pmessage[index]);
    index += 2;

    // header data
    pheader_data = (char*)&pmessage[index];
    index += header_data_length;

    // chunks in message
    chunks_in_message = bytes_to_uint16(&pmessage[index]);
    index += 2;

    // chunk
    chunk_number = bytes_to_uint16(&pmessage[index]);
    index += 2;

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
        uint16_t prop_length = 0;
        uint16_t total_chunks = 1;
        uint16_t chunk = 0;
        MIDI20_CI_RESULT_T result;

        while(chunk < total_chunks)
        {
            memset(_ci_property_buffer, 0, sizeof(_ci_property_buffer));
            result = midi20_ci_get_prop_manufacturer(pheader_data, header_data_length, _ci_property_buffer, &prop_length, chunk, &total_chunks);

            if(result == MIDI20_CI_RESULT_SUCCESS)
            {
                midi20_ci_reply_prop_get(pheader, request_id, "{\"status\":200}", _ci_property_buffer, chunk+1, total_chunks);
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
