#ifndef SENSIRION_COMMON_H
#define SENSIRION_COMMON_H

#include "sensirion_config.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t sensirion_common_bytes_to_uint16_t(const uint8_t* bytes);
uint32_t sensirion_common_bytes_to_uint32_t(const uint8_t* bytes);

#ifdef __cplusplus
}
#endif

#endif /* SENSIRION_COMMON_H */
