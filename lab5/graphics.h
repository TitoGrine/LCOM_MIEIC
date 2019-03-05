#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

#include "i8042.h"
#include "math.h"


typedef struct{
	char  vbe_signature[4];
	BCD   vbe_version[2];
	uint32_t oem_string_ptr;
	uint32_t capabilities;
	uint32_t mode_list_ptr;
	uint16_t total_memory;
	uint16_t oem_software_rev;
	uint32_t oem_vendor_name_ptr;
	uint32_t oem_product_name_ptr;
	uint32_t oem_product_rev_ptr;
	uint8_t  reserved[222];
	uint8_t  oem_data[256];
} __attribute__((packed)) vbe_info_t;


uint8_t calculate_mask(int component);

int get_mode_info(uint16_t mode, vbe_mode_info_t *mode_info);

void paint_pixel(int x, int y, u_int32_t color);

void paint_line(int x, int y, int length, u_int32_t color);

void paint_rectangle(int x, int y, int width, int height, u_int32_t color);

void draw_pattern(uint8_t no_rectangles,uint32_t first,uint8_t step);

void draw_pixmap(char* map, int x, int y, int width, int height);

#endif
