#ifndef __FLASHOPS_H__
#define __FLASHOPS_H__

#include <stdint.h>
#include <stdio.h>
#include "msemu.h"

int df_init(uint8_t **df_buf, ms_opts *options);
int df_deinit(uint8_t **df_buf, ms_opts *options);
int cf_init(uint8_t **cf_buf, ms_opts *options);
int cf_deinit(uint8_t **cf_buf, ms_opts *options);

/**
 * Interpret commands intended for 28SF040 flash, aka Mailstation dataflash
 *
 * ms              - Struct of ms_hw used by main emulation
 * translated_addr - Address in range of dataflash, 0x00000:0x7FFFF
 * val             - Command or value to send to dataflash
 */
int df_write(uint8_t *df_buf, unsigned int absolute_addr, uint8_t val);

uint8_t df_read(uint8_t *df_buf, unsigned int absolute_addr);

int cf_write(uint8_t *cf_buf, unsigned int absolute_addr, uint8_t val);

uint8_t cf_read(uint8_t *cf_buf, unsigned int absolute_addr);

#endif
