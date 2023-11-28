#ifndef DRAW_H_
#define DRAW_H_

/**
draw.h
---
Header file for draw.h, used to store all accessible functions related to
drawing on the screen or to do with allegro
*/

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdlib.h>

#include "structures.h"
#include "physics.h"

/*
void load_bitmaps()
---
This function loads al the bitmap objects into struct Bitmap
---
struct Bitmap * b: struct to load into
*/
void load_bitmaps( struct Bitmap * );

/*
void free_bitmaps()
---
This function frees all the bitmap objects into struct Bitmap
---
struct Bitmap * b: struct to free
*/
void free_bitmaps( struct Bitmap * );

/*
void mask_bitmaps()
---
This function masks all the bitmaps in struct Bitmap
---
struct Bitmap *: bitmaps to mask
---
Returns nothing
*/
void mask_bitmaps(struct Bitmap *);

/*
void draw_screen()
---
This function handles drawing everything on the screen.
---
struct Level * l: level to draw

---
Returns nothing
*/
void draw_screen( struct Level *, struct Position *, struct Proj_arr *,
		  struct Bitmap *, int,
		  unsigned int);

/*
void draw_menu()
---
This function handles drawing the start menu.
---
struct Bitmap * b: holds all bitmap objects for the game
*/
void draw_menu( struct Bitmap * );

/*
void draw_end_menu()
---
This function handles drawing the endscreen.
---
struct Bitmap * b: holds all bitmap objects for the game
unsigned int score: score for the game
*/
void draw_end_menu( struct Bitmap *, unsigned int );

#endif //DRAW_H_
