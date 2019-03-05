#include "graphics.h"

static uint16_t hres;
static uint16_t vres;
static uint8_t bits_per_pixel;
static void *video_mem;
static size_t vram_base;
static size_t vram_size;
static uint8_t RedScreeMask;
static uint8_t GreenScreeMask;
static uint8_t BlueScreeMask;
static uint8_t RedLSB;
static uint8_t GreenLSB;
static uint8_t BlueLSB;

uint8_t calculate_mask(int component) {
  uint8_t mask = 0x00;

  // Calculates the mask for each color component depending on the size (in bits) that it occupies 

  switch (component) {
    case RED_COMPONENT:
      for (int i = 0; i < RedScreeMask; i++) {
        mask = (mask << 1) | 0x01;
      }
      break;

    case GREEN_COMPONENT:
      for (int i = 0; i < GreenScreeMask; i++) {
        mask = (mask << 1) | 0x01;
      }
      break;

    case BLUE_COMPONENT:
      for (int i = 0; i < BlueScreeMask; i++) {
        mask = (mask << 1) | 0x01;
      }
      break;

    default:
      break;
  }
  return mask;
}

int get_mode_info(uint16_t mode, vbe_mode_info_t *mode_info) {
  mmap_t memory_map;
  struct reg86u reg86;

  // Iniciates low memory area until successful

  video_mem = lm_init(true);
  while (video_mem == NULL) {
    video_mem = lm_init(true);
  }

  // Alocates low memory until successful

  video_mem = lm_alloc(sizeof(vbe_mode_info_t), &memory_map);
  while (video_mem == NULL) {
    video_mem = lm_alloc(sizeof(vbe_mode_info_t), &memory_map);
  }
  
  // Sets the struct to zero

  memset(&reg86, 0, sizeof(reg86));

  phys_bytes buf = memory_map.phys;

  reg86.u.b.intno = VBE_MODE;
  reg86.u.w.ax = VBE_MODE_INFO;
  reg86.u.w.es = PB2BASE(buf);
  reg86.u.w.di = PB2OFF(buf);
  reg86.u.w.cx = mode;

  if (sys_int86(&reg86) != OK) {
    return 1;
  }

  if (reg86.u.b.ah != VBE_CALL_SUCCESS)
    ;

  *mode_info = *(vbe_mode_info_t *) memory_map.virt;

  lm_free(&memory_map);

  return 0;
}

void *(vg_init)(uint16_t mode) {

  struct reg86u reg;

  int R;
  struct minix_mem_range mr;

  vbe_mode_info_t mode_info;

  if (get_mode_info(mode, &mode_info) != 0) {
    panic("get_mode_info unsuccessful");
  }

  /* Allow memory mapping */

  hres = mode_info.XResolution;
  vres = mode_info.YResolution;
  bits_per_pixel = mode_info.BitsPerPixel;
  vram_base = mode_info.PhysBasePtr;
  RedScreeMask = mode_info.RedMaskSize;
  GreenScreeMask = mode_info.GreenMaskSize;
  BlueScreeMask = mode_info.BlueMaskSize;
  RedLSB = mode_info.LinRedFieldPosition;
  GreenLSB = mode_info.LinGreenFieldPosition;
  BlueLSB = mode_info.LinBlueFieldPosition;

  if (bits_per_pixel % 4 == 0) {
    vram_size = hres * vres * bits_per_pixel;

  }
  else {
    vram_size = hres * vres * (bits_per_pixel + 1);
  }

  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (R = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", R);

  /* Map memory */

  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED)
    panic("couldn't map video memory");

  memset(&reg, 0, sizeof(reg));
  reg.u.w.ax = 0x4F02;
  reg.u.w.bx = mode | BIT(14);
  reg.u.b.intno = 0x10;

  if (sys_int86(&reg) != OK) {
    return 0;
  }

  return video_mem;
}

void paint_pixel(int x, int y, u_int32_t color) {
  if (x < hres && y < vres && x >= 0 && y >= 0) {
  int bytes = (int) ceil((float) (bits_per_pixel) / 8.0);

    uint8_t *ptr = (uint8_t *) video_mem + (x + y * hres) * bytes;

    for (int i = 0; i < bytes; i++) {
      ptr[i] = (uint8_t)(color >> (8 * i));
    }
  }
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t length, u_int32_t color) {

  for (uint16_t i = 0; i < length; i++) {
    paint_pixel(x + i, y, color);
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, u_int32_t color) {

  for (uint16_t j = 0; j < height; j++) {
    vg_draw_hline(x, y + j, width, color);
  }

  return 0;
}

void draw_pattern(uint8_t no_rectangles, uint32_t first, uint8_t step) {
  uint32_t color = first;
  int dif_x = 0;
  int dif_y = 0;

  for (int i = 0; ((hres - i) % (no_rectangles)) != 0; i++) {
    dif_x = i;
  }

  for (int j = 0; ((vres - j) % (no_rectangles)) != 0; j++) {
    dif_y = j;
  }

  uint16_t x_increment = (uint16_t)((hres - dif_x) / no_rectangles);
  uint16_t y_increment = (uint16_t)((vres - dif_y) / no_rectangles);

  if (bits_per_pixel == 8) {
    uint16_t col = 0;
    uint16_t row = 0;

    for (int i = 0; i < no_rectangles; i++) {
      col = 0;
      for (int j = 0; j < no_rectangles; j++) {

        color = (uint32_t)((first + (i * no_rectangles + j) * step) % (1 << bits_per_pixel));

        vg_draw_rectangle(col, row, x_increment, y_increment, color);

        col += x_increment;
      }
      row += y_increment;
    }
  }
  else {
    uint16_t x = 0;
    uint16_t y = 0;

    uint8_t Red_Mask = calculate_mask(RED_COMPONENT);
    uint8_t Green_Mask = calculate_mask(GREEN_COMPONENT);
    uint8_t Blue_Mask = calculate_mask(BLUE_COMPONENT);

    uint8_t R_first = (uint8_t)(color >> RedLSB) & Red_Mask;
    uint8_t G_first = (uint8_t)(color >> GreenLSB) & Green_Mask;
    uint8_t B_first = (uint8_t)(color >> BlueLSB) & Blue_Mask;
    uint8_t R, G, B;

    for (int row = 0; row < no_rectangles; row++) {
      x = 0;
      for (int col = 0; col < no_rectangles; col++) {
        R = (uint8_t)(R_first + col * step) % (1 << RedScreeMask);
        G = (uint8_t)(G_first + row * step) % (1 << GreenScreeMask);
        B = (uint8_t)(B_first + (col + row) * step) % (1 << BlueScreeMask);

        color = 0;
        color |= R;
        color = (color << GreenScreeMask);
        color |= G;
        color = (color << BlueScreeMask);
        color |= B;

        vg_draw_rectangle(x, y, x_increment, y_increment, color);

        x += x_increment;
      }
      y += y_increment;
    }
  }
}

void draw_pixmap(char *map, int x, int y, int width, int height) {

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      paint_pixel(x + col, y + row, (uint32_t) map[row * width + col]);
    }
  }
}
