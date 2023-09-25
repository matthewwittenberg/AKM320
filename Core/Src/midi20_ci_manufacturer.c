#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "midi_spec.h"
#include "midi20_ci.h"

const uint8_t MAN_ID_HI = 0x00;
const uint8_t MAN_ID_MD = 0x21;
const uint8_t MAN_ID_LO = 0x4E;
const char* PRODUCT_NAME = "MIDIPLUS AKM320";
const uint16_t PRODUCT_FAMILY = 0;
const uint16_t PRODUCT_MODEL = 0;
const uint32_t PRODUCT_SOFTWARE_VERSION = 1;

MIDI20_CI_RESULT_T midi20_ci_get_prop_manufacturer(
    const char *pheader_data, uint16_t header_data_length,
    char *pproperty_data, uint16_t *pproperty_data_length,
    uint8_t chunk, uint8_t *ptotal_chunks)
{
    if(strnstr(pheader_data, "ResourceList", header_data_length) != NULL)
    {
        strcpy(pproperty_data,
            "[{\"resource\":\"DeviceInfo\"}," \
            "{\"resource\":\"ChannelList\"}," \
            "{\"resource\":\"CMList\"}]");
        *pproperty_data_length = strlen(pproperty_data);
        *ptotal_chunks = 1;
    }
    else if(strnstr(pheader_data, "DeviceInfo", header_data_length) != NULL)
    {
        strcpy(pproperty_data,
            "{\"manufacturerId\":[78,33,0]," \
            "\"manufacturer\":\"MIDIPLUS\"," \
            "\"familyId\":[0,0]," \
            "\"family\":\"\"," \
            "\"modelId\":[0,0]," \
            "\"model\":\"AKM320\"," \
            "\"versionId\":[0,0,1,0]," \
            "\"version\":\"1.0\"}");
        *pproperty_data_length = strlen(pproperty_data);
        *ptotal_chunks = 1;
    }
    else if(strnstr(pheader_data, "ChannelList", header_data_length) != NULL)
    {
        strcpy(pproperty_data,
            "[{\"title\":\"keys\"," \
            "\"channel\":1," \
            "\"familyId\":\"\"," \
            "\"links\":[{\"resource\":\"CMList\",\"resId\":\"all\"}]}]");
        *pproperty_data_length = strlen(pproperty_data);
        *ptotal_chunks = 1;
    }
    else if(strnstr(pheader_data, "CMList", header_data_length) != NULL)
    {
        if(chunk == 0)
        {
            strcpy(pproperty_data,
                "[{\"name\":\"Octave\"," \
                "\"priority\":1," \
                "\"controlType\":\"cc\"," \
                "\"controlIdx\":[80]," \
                "\"default\":10},");
        }
        else
        {
            strcpy(pproperty_data,
                "{\"name\":\"Transpose\"," \
                "\"priority\":1," \
                "\"controlType\":\"cc\"," \
                "\"controlIdx\":[81]," \
                "\"default\":53}]");
        }

        *pproperty_data_length = strlen(pproperty_data);
        *ptotal_chunks = 2;
    }

    return MIDI20_CI_RESULT_SUCCESS;
}



