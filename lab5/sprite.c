#include "sprite.h"

Sprite *create_sprite(const char *pic[], int x, int y, int xspeed, int yspeed) {
//allocate space for the "object"
	Sprite *sp = (Sprite *) malloc(sizeof(Sprite));
	if (sp == NULL)
		return NULL;
// read the sprite pixmap
	sp->map = read_xpm(pic, &(sp->width), &(sp->height));
	if (sp->map == NULL) {
		free(sp);
		return NULL;
	}
	sp->x = x;
	sp->y = y;
	sp->xspeed = xspeed;
	sp->yspeed = yspeed;
	return sp;
}

void destroy_sprite(Sprite *sp) {
	if (sp == NULL)
		return;
	if (sp->map)
		free(sp->map);
	free(sp);
	sp = NULL; // XXX: pointer is passed by value
// should do this @ the caller
}

void move_xpm(Sprite* sp, int speed) {
	erase_Sprite(sp);

	if((sp->xspeed) != 0)
	{
		sp->x += speed;
	}
	else
	{
		sp->y += speed;
	}

	printf("Posição x = %u \n", sp->x);
	printf("Posição y = %u \n\n", sp->y);
	

	draw_Sprite(sp);
}

void draw_Sprite(Sprite * sp){
	int x = sp->x;
	int y = sp->y;

	uint32_t pixel_color = 0;

	for(int height = 0; height < sp->height; height++){

		for(int width = 0; width < sp->width; width++){
				paint_pixel(x + width, y + height, sp->map[pixel_color]);
				pixel_color++;
			}
  }
}

void erase_Sprite(Sprite * sp){
	int x = sp->x;
	int y = sp->y;

	for(int height = 0; height < sp->height; height++){

		for(int width = 0; width < sp->width; width++){
				paint_pixel(x + width, y + height, sp->map[BLACK]);
			}
  }
}

