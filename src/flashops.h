#ifndef __FLASHOPS_H__
#define __FLASHOPS_H__

#include <stdint.h>
#include <stdio.h>
#include "msemu.h"

int df_init(ms_opts *options);
int df_deinit(ms_opts *options);
int cf_init(ms_opts *options);
int cf_deinit(ms_opts *options);

/**
 * Interpret commands intended for 28SF040 flash, aka Mailstation dataflash
 *
 * ms              - Struct of ms_hw used by main emulation
 * translated_addr - Address in range of dataflash, 0x00000:0x7FFFF
 * val             - Command or value to send to dataflash
 */
int df_write(unsigned int absolute_addr, uint8_t val);

uint8_t df_read(unsigned int absolute_addr);

int cf_write(unsigned int absolute_addr, uint8_t val);

uint8_t cf_read(unsigned int absolute_addr);

#endif
