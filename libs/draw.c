/**
draw.c
---
File used to store all the functions related to drawing on the screen and other
micellaneous stuff such as events. Uses allegro5.

Functions that should not be accessed outside of this file are given the
keyword 'static'.
*/

#include "draw.h"

/* time amount to be incremented by with each frame when drawing projectile */
#define TIME_INC (0.0875)

/*
static void draw_tile()
---
Draws a given 16x16 section of the tileset bitmap at a given (x,y) position.
'ty' is used to select which tileset is used (fg, bg, dec, alt_fg, etc...)
---
ALLEGRO_BITMAP * ts: tileset bitmap, stores all tilesets.
int tile: which tile to use (0-15 for valid, any other val for blank)
int x, y: position to draw at
unsigned int ty: tileset to use (fg, bg, dec, alt_fg, etc...)
---
Returns nothing
*/
static void draw_tile( ALLEGRO_BITMAP* ts, int tile, int x, int y,
		       unsigned int ty )
{
	int tx;
	
        if ( tile != 16 ) {
                tx = tile * 8;
		ty *= 8;
                al_draw_scaled_bitmap(ts, tx, ty, 8, 8, x, y, 32, 32, 0);
	}
}

/*
int chtoi()
---
Converts a given character to integer.
---
char ch: character to convert
---
Returns an integer
*/
static int chtoi( char ch )
{
        if (ch >= '0' && ch <= '9')
                return ch - '0';
        if (ch >= 'A' && ch <= 'F')
                return ch - 'A' + 10;
	else
		return 16;
}

/*
void draw_tilemap()
---
Draws a given tilemap onto the screen
---
char map[]: tilemap to draw
int rows, cols: no. of rows and columns the tilemap has
ALLEGRO_BITMAP * ts: bitmap to use, passed to draw_tile()
unsigned int ty: tileset to use from bitmap, passed to draw_tile()
---
Returns nothing
*/
static void draw_tilemap( char map[], int rows, int cols, ALLEGRO_BITMAP* ts,
		   unsigned int ty)
{
	int ch;
	int index = 0;
	
        for (int col = 0; col < cols; col++) {
                for (int row = 0; row < rows; row++) {
			
			/* turns hex string to int */
                        ch = chtoi( map[index++] );
                        draw_tile( ts, ch, row*32, col*32, ty );
		}
	}       
}

static void draw_target( ALLEGRO_BITMAP * target, float x, float y )
{
        al_draw_scaled_bitmap( target, 0, 0, 15, 13, x, y, 30, 26, 0 );
}

/*
void draw_character
[as in player character]
---
*/
static void draw_character( ALLEGRO_BITMAP * player, ALLEGRO_BITMAP * bow,
		     struct Position * mouse, struct Level * l )
{
        /* calculate angle */
        float angle = atan2f( (mouse->y - l->start_pos.y),
			    (mouse->x - l->start_pos.x) );

	/* angle offset correction */
	angle += M_PI/2;
	
	int right = 1;
	int x_off = 15;
	int y_off = -40;
	int cx = 11;
	int cy = 25;
	int flags = 0;
	/* check if character facing left */
        if ( mouse->x < (l->start_pos.x+13) ) {
		right = 0;
		flags = ALLEGRO_FLIP_HORIZONTAL;
		cx = 21;
		cy = 25;
		x_off = -15;
		y_off = -40;

		angle = atan2f( (mouse->y - (l->start_pos.y + y_off)),
			      (mouse->x - (l->start_pos.x + x_off)) );
		/* angle offset correction */
	        angle -= (3*(M_PI)/2);
        }
	
	int dx = l->start_pos.x + x_off;
	int dy = l->start_pos.y + y_off;

	al_draw_scaled_rotated_bitmap(bow, cx, cy, dx, dy, 2, 2, angle, flags);
	al_draw_scaled_bitmap(player, 0, 0, 26, 40,
			      l->start_pos.x-26, l->start_pos.y-80, 52, 80, flags);
}

static void draw_projectile_path( struct Position * mouse, struct Level * l )
{
	//get start pos of throw from level

	//defensive copy of start position
	struct Position pos;
        initialize_position( &pos, l->start_pos.x, l->start_pos.y );
	
        if ( mouse->x < (l->start_pos.x+13) ) {
		pos.x -= 50;
		pos.y -=50;
	} else {
                pos.x += 50;
		pos.y -=50;
	}

	//calculate component velocity from mouse pos
	struct Position velocity;
	get_velocity_from_mouse( &velocity, &pos, mouse );

	unsigned int index = 0;
	float time = 0;
	float step_x, step_y;
	unsigned int type = 0;//ignored for this, rebounds and jumps ignored
	
	//starting position
	struct Position s_pos;
	initialize_position( &s_pos, pos.x, pos.y );

        struct Position prev_pos;
	initialize_position( &prev_pos, pos.x, pos.y );

	unsigned int last_coll = 4;
        while ( last_coll > 0 ) {
                //calculate new position and increment time
		calculate_position( &velocity, &pos, &s_pos, l, time );
		//calculate how far the throw is allowed to go
		last_coll = do_step_collision( &pos, &prev_pos, l, &type );
                
		step_x = ( pos.x - prev_pos.x ) / 4;
		step_y = ( pos.y - prev_pos.y ) / 4;

                //fix position of throw to stop it from being inside a wall
		pos.x = prev_pos.x + step_x * last_coll;
		pos.y = prev_pos.y + step_y * last_coll;
		
		time = time + TIME_INC;

                if(index%4==0){
			al_draw_line( prev_pos.x, prev_pos.y, pos.x,
				      pos.y, al_map_rgb_f( 1, 0, 0 ), 2 );
		}
		prev_pos.x = pos.x;
		prev_pos.y = pos.y;
		index++;
	}
}

static void draw_object( struct Object * o )
{
	/* switch case structure to provide future-proofing if more object
        types are added */
	ALLEGRO_COLOR col;
	switch (o->type)
	{
	case 1:
                col = al_map_rgb_f(1, 0, 0);
		break;
        case 2:
		col = al_map_rgb_f(0,0,1);
	        break;
        case 3:
		col = al_map_rgb_f(0,1,0);
		break;
	case 4:
		col = al_map_rgb_f(0,0.5,0.3);
	default:
		col = al_map_rgb_f(1,1,0);
		break;
	}

	/* draws rectange with given colour */
	al_draw_filled_rectangle( (o->pos.x), (o->pos.y - o->dims.y),
				  (o->pos.x + o->dims.x), (o->pos.y), col );
	
}

static void draw_grid() {
	for (int r=0;r<15;r++) {
                al_draw_line( 0, r*32, 640, r*32, al_map_rgb_f(1,1,1), 2);
	}

	for (int c=0;c<20;c++) {
                al_draw_line( c*32, 0, c*32, 480, al_map_rgb_f(1,1,1), 2);
	}
}

/*
void draw_water()
---
Draws a semi-transparent water effect on the bottom of the screen.
---
ALLEGRO_BITMAP * water: bitmap to use
int frame: what frame the animation is on (assumes FPS is 60)
*/
static void draw_water( ALLEGRO_BITMAP * water, int frame )
{
	float startx = -64;
	float starty = 480-32;
	float frame_off = 64 * frame/60.0;
	
	for (int i = 0; i < 12; i++) {
                al_draw_tinted_bitmap( water, al_map_rgba_f(1,1,1,0.5),
				       (startx + frame_off + i*63 ), starty,
				       100);
	}
}


static void draw_projectile( ALLEGRO_BITMAP * proj, struct Projectile * p,
			     struct Level * l ) {
	float step_x, step_y;
	unsigned int type = 0;
	if ( !(p->p_coll > 0) ) {
                p->active = 0;
	} else {
		//calculate new position and increment time
		calculate_position( &p->vel, &p->pos, &p->s_pos, l, p->time );
		//calculate how far the throw is allowed to go
		p->p_coll = do_step_collision( &p->pos, &p->p_pos, l, &type );

		step_x = ( p->pos.x - p->p_pos.x ) / 4;
		step_y = ( p->pos.y - p->p_pos.y ) / 4;

		//fix position of throw to stop it from being inside a wall
		p->pos.x = p->p_pos.x + step_x * p->p_coll;
		p->pos.y = p->p_pos.y + step_y * p->p_coll;

		//check type of collision
		switch (type) {
		case 1:
			//rebound
			do_rebound( p );
			break;
		case 2:
			//jump pad
			do_bounce( p, -250 );
			break;
                case 3:
			//jump pad but smaller
                        do_bounce( p, -100 );
			break;
                case 4:
			//jump pad mid
                        do_bounce( p, -200 );
			break;
		default:
			p->time = p->time + TIME_INC;
			break;
		}
		float y_comp = (p->pos.y - p->p_pos.y);
		float x_comp = (p->pos.x - p->p_pos.x);
                float angle = atan(y_comp/x_comp);

		if (x_comp < 0)
		        al_draw_scaled_rotated_bitmap(proj, 9, 6, p->pos.x, p->pos.y, 2, 2, angle, ALLEGRO_FLIP_HORIZONTAL);
		else
			al_draw_scaled_rotated_bitmap(proj, 9, 6, p->pos.x, p->pos.y, 2, 2, angle, 0);

		p->p_pos.x = p->pos.x;
		p->p_pos.y = p->pos.y;
	}
}

/*
void draw_projectiles()
---
Draws all active projectiles in Proj_arr (i.e. any currently in flight)
---
struct Proj_arr * proj_arr: projectile array to draw
ALLEGRO_BITMAP * proj: bitmap of arrow to draw
struct Level * l: level to use
*/
static void draw_projectiles( struct Proj_arr * proj_arr,
			      ALLEGRO_BITMAP * proj,
			      struct Level * l )
{
        for (int p = 0; p < proj_arr->size; p++) {
		if( proj_arr->arr[p].active ){
			draw_projectile(proj, &proj_arr->arr[p], l);
		}
	}
}

/*
void load_bitmaps()
---
This function loads all the bitmap objects into struct Bitmap
---
struct Bitmap * b: struct to load into
*/
void load_bitmaps( struct Bitmap * b )
{
	b->ts = al_load_bitmap("tilesets/all.bmp");
	b->plyr = al_load_bitmap("tilesets/player.bmp");
	b->plyr_bow = al_load_bitmap("tilesets/bow.bmp");
	b->proj = al_load_bitmap("tilesets/proj.bmp");
	b->water = al_load_bitmap("tilesets/water.bmp");
	b->font = al_load_bitmap("tilesets/font.bmp");
	b->bg = al_load_bitmap("tilesets/menu_bg.bmp");
	b->target = al_load_bitmap("tilesets/target.bmp");
}

/*
void free_bitmaps()
---
This function frees all the bitmap objects into struct Bitmap
---
struct Bitmap * b: struct to free
*/
void free_bitmaps( struct Bitmap * b )
{
        al_destroy_bitmap(b->ts);
	al_destroy_bitmap(b->plyr);
	al_destroy_bitmap(b->plyr_bow);
	al_destroy_bitmap(b->proj);
	al_destroy_bitmap(b->water);
	al_destroy_bitmap(b->font);
	al_destroy_bitmap(b->bg);
	al_destroy_bitmap(b->target);
}

/*
void mask_bitmaps()
---
Masks the bitmaps in struct bitmap to allow transparent bitmaps.
---
struct Bitmap * b: bitmaps to mask
*/
void mask_bitmaps( struct Bitmap * b)
{
        al_convert_mask_to_alpha( b->ts, al_map_rgb_f(1,0,1) );
        al_convert_mask_to_alpha( b->plyr, al_map_rgb_f(1,0,1) );
        al_convert_mask_to_alpha( b->plyr_bow, al_map_rgb_f(1,0,1) );
        al_convert_mask_to_alpha( b->proj, al_map_rgb_f(1,0,1) );
	al_convert_mask_to_alpha( b->water, al_map_rgb_f(1,0,1) );
	al_convert_mask_to_alpha( b->font, al_map_rgb_f(1,0,1) );
	al_convert_mask_to_alpha( b->target, al_map_rgb_f(1,0,1) );
}

/*
void draw_char()
[as in ASCII character]
---
This function draws a given character onto the screen at co-ords (x,y) using
the given font bmp. As the 'char' data type stores each character as its ASCII
value, parsing which character is simple enough if the font bitmap is in
the same order.
---
char ch: character to draw
int x, y: position to draw at
ALLEGRO_BITMAP * font: font bitmap to use
---
Returns nothing
*/
static void draw_char( char ch, int x, int y, ALLEGRO_BITMAP * font )
{
	int ty = 0;
	int tx = (ch - 32) * 8; //32 is start of visible chars in ASCII
        al_draw_scaled_bitmap( font, tx, ty, 8, 8, x, y, 16, 16, 0);
}

/*
void draw_text()
---
This function draws a given string onto the screen starting at co-ords (x,y).
Each character is blitted onto the screen as a section of the font bitmap.
---
char * text: text to draw
size_t size: size of the string given
int x, y: co-ords of starting character
ALLEGRO_BITMAP * font: font bmp to use
---
Returns nothing
*/
static void draw_text( char * text, size_t size, int x, int y,
		       ALLEGRO_BITMAP * font )
{
	for ( int i = 0; i < size; i++ ) {
                draw_char(text[i], x + (16*i), y, font);
	}
}

/*
void draw_stats()
---
Draws the current score and wind direction
---
struct Level * l: level to use
unsigned int score: current score
ALLEGRO_BITMAP * font: font tileset to use
*/
static void draw_stats( struct Level * l, unsigned int score,
			ALLEGRO_BITMAP * font )
{
	/* wind direction */
	float mag, angle;
	calc_arc_from_comp( &l->wind, &mag, &angle, 0 );
	char buf[32];
        int size = snprintf( buf, 32, "Wind: %i (@%i)\n",
			     (int)mag, (int)angle );
	draw_text(buf, size, 0, 0, font);

	/* score */
	char buf_b[32];
	size = snprintf( buf_b, 32, "$%i", score );
	draw_text( buf_b, size, (640 - 16*size), 0, font );
}

/*
void draw_screen()
---
This function handles drawing everything on the screen.
---
struct Level * l: level to draw
ALLEGRO_BITMAP * fg: foreground tileset (stored as bitmap)
ALLEGRO_BITMAP * bg: background tileset (stored as bitmap)
---
Returns nothing
*/
void draw_screen( struct Level * l, struct Position * mouse,
		  struct Proj_arr * proj_arr, struct Bitmap * b,
		  int frame, unsigned int score )
{
	/* if alt tilemap */
	int offset = 0;
	if ( l->level > 7 )
		offset = 3;
	
        /* clear screen */
        al_clear_to_color( al_map_rgb_f( 0, 0, 0 ) );

	/* background tilemap */
	draw_tilemap( l->bg.map, l->bg.rows, l->bg.cols,
		      b->ts, ( 0 + offset) );
	
	draw_target( b->target, l->target_pos.x, l->target_pos.y );
	draw_character(b->plyr, b->plyr_bow, mouse, l );

	/* if alt tilemap */
        if ( l->level > 7 )
		draw_water( b->water, frame);

	draw_projectile_path(mouse, l);
	draw_projectiles( proj_arr, b->proj, l );


	/* foreground tileset */
	draw_tilemap( l->fg.map, l->fg.rows, l->fg.cols,
		      b->ts, ( 1 + offset ) );

	/* draw overlayed decorations tilemap */
	draw_tilemap( l->dec.map, l->dec.rows, l->dec.cols,
		      b->ts, ( 2 + offset) );

	/* if debug, draw each object's collider and its type, along with
           a tile grid */
	#if DEBUG
	for (int i = 0;i<l->index;i++)
                draw_object(&l->object_arr[i]);
        draw_grid();
	#endif

	draw_stats( l, score, b->font );
	
	al_flip_display();
}

/*
void draw_menu()
---
This function handles drawing the start menu.
---
struct Bitmap * b: holds all bitmap objects for the game
*/
void draw_menu( struct Bitmap * b )
{
        /* clear screen */
	al_clear_to_color( al_map_rgb_f( 0,0,0 ) );

	/* draw menu's bg */
	al_draw_scaled_bitmap( b->bg, 0, 0, 160, 120, 0, 0, 640, 480, 0 );

	/* draw text onto screen */
	draw_text( "Press enter to start", 20, 160, 208, b->font );
	al_flip_display();
}

/*
void draw_end_menu()
---
This function handles drawing the endscreen.
---
struct Bitmap * b: holds all bitmap objects for the game
unsigned int score: score for the game
*/
void draw_end_menu( struct Bitmap * b, unsigned int score )
{
        /* clear screen */
	al_clear_to_color( al_map_rgb_f( 0,0,0 ) );

	/* draw menu's bg */
	al_draw_scaled_bitmap( b->bg, 0, 0, 160, 120, 0, 0, 640, 480, 0 );

	/* draw text onto screen */
	char buf[32];
	int size = snprintf(buf, 32, "Score: $%i", score);
	draw_text( buf, size, (240 - 8 * size ), 208, b->font );
	draw_text( "Congratulations! You won!", 25, 128, 192, b->font );
	al_flip_display();
}


