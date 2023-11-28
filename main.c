/**
Main file for the C assessment game. 
Enter the Money-Dungeon: a game where the player traverses the Money-Dungeon in
search for riches.

Files
---
main.c - stores entrypoint of game and file i/o for loading level data
physics.c - stores physics calculations (i.e. projectiles, collisions)
structures.c - stores structs and functions for initializing, modifying or
               freeing them
draw.c - stores functions relating to drawing (drawing, events, etc.)
*/

#include "libs/physics.h"
#include "libs/structures.h"
#include "libs/draw.h"

#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#define FPS (60)
#define WIN_WIDTH (640)
#define WIN_HEIGHT (480)
#define MAX_SCORE (10000) //max score for a level done with only 1 throw
#define LAST_LEVEL (8)

/* Done for debugging purposes, usd with make debug_buid*/
#ifndef S_LEVEL
#define S_LEVEL (1)
#endif

float n_strtof( char * str )
{
        return (float)strtol( str, NULL, 10 );
}

void load_tilemap( char * fg_loc, struct Tilemap * tm )
{
	char ch;
	int index = 0;
	FILE *fg_pntr = fopen(fg_loc, "r");
	while ( (ch = fgetc(fg_pntr)) != EOF) {
                tm->map[index] = ch;

		index++;
	}
	fclose(fg_pntr);
}

void parse_level_file( char * level_loc, struct Level * l, int num )
{
        FILE* file_pntr = fopen(level_loc, "r");
	int l_index = 0;
	int t_index = 0;
	size_t len = 0;
	ssize_t read;/* length of gotten line */
	char* line = NULL;/* null pointer */
        char* token;
	char* delim = ",";
	/* for level parsing */
	float startx, starty, targetx, targety,
	      windx, windy, gravx, gravy, f_tok;
	/* for object parsing */
        struct Object o;
	float posx,posy,dimx,dimy,type;
	/* read each line in the file*/
        while ( (read = getline(&line, &len, file_pntr)) != -1 ) {
                t_index = 0;
		/* split each line into tokens */
		token = strtok(line, delim);
		while (token != NULL) {
			/* turn string token into float */
			f_tok = n_strtof( token );

			/* check which line of the file it is */
                        switch (l_index) {
                        case 0:                         //line 0, start pos
				if (t_index++ == 0)
					startx = f_tok;
				else
					starty = f_tok;
				break;
                        case 1:                         //line 1, target pos
				if (t_index++ == 0)
					targetx = f_tok;
				else
					targety = f_tok;
				break;
			case 2:                         //line 2, wind vel
				if (t_index++ == 0)
					windx = f_tok;
				else
					windy = f_tok;
				break;
                        case 3:                         //line 3, grav vel
				if (t_index++ == 0)
					gravx = f_tok;
				else {
					gravy = f_tok;
				        initialize_level( l, startx, starty,
							  targetx, targety,
							  windx, windy,
							  gravx, gravy, num);
				}
				break;
                        default:                        //else, must be object
                                /* check which token it is */
				switch (t_index++) {
                                case 0:                         //x pos
					posx = f_tok;
					break;
                                case 1:                         //y pos
					posy = f_tok;
					break;
				case 2:                         //width
					dimx = f_tok;
					break;
				case 3:                         //height
					dimy = f_tok;
					break;
				case 4:                         //type
					type = f_tok;
					initialize_object(&o, posx, posy,
							  dimx, dimy,
							  type, 0, 0, 0);
					add_object_to_level(l, o);
					break;
				default:
					break;
				}
			        break;
			}
			/* get next token */
			token = strtok(NULL, delim);
		}
		l_index++;	
	}
}

/*
void load_level()
---
Loads a given level according to its level number and initializes the 
level accordingly
---
struct Level * l: level to store loaded data in
int num: number of the level
*/
void load_level( struct Level * l, int num )
{
	char level_loc[32];
	char fg_loc[32];
	char bg_loc[32];
	char dec_loc[32];
	int index = 0;
	
	
	/* if num is invalid, default to level 1 */
	if (num > LAST_LEVEL || num < 1)
		num = 1;

	/* edit file locs */
	snprintf(level_loc, 32, "levels/%i/level.txt", num);
	snprintf(fg_loc, 32, "levels/%i/fg.txt", num);
	snprintf(bg_loc, 32, "levels/%i/bg.txt", num);
	snprintf(dec_loc, 32, "levels/%i/dec.txt", num);
	
        /* load tilemaps */
        load_tilemap( fg_loc, &l->fg );
	load_tilemap( bg_loc, &l->bg );
	load_tilemap( dec_loc, &l->dec );

	/* parse level.txt */
        parse_level_file( level_loc, l, num );
}

/*
unsigned int is_win()
---
Checks if a given position is within the collider for the target
---
struct Level * l: level with the target's position
struct Position * pos: position to use
---
Returns an unsigned int, 1 on success, 0 on fail
*/
unsigned int is_win( struct Level * l, struct Position * pos )
{
        if ( (pos->x >= l->target_pos.x && pos->x <= l->target_pos.x+50) &&
	     (pos->y >= l->target_pos.y-50 && pos->y <= l->target_pos.y ) )
		return 1;
	return 0;
}

/*
unsigned int calculate_score()
---
Calculates a score for the level just completed based on the number of arrows
fired.
---
int arrows_fired: no. of arrows fired
---
Returns an  unsigned int representing the score calculated.
*/
unsigned int calculate_score( int arrows_fired )
{
        unsigned int score = 0;
	if ( arrows_fired == 1 )
		return MAX_SCORE;
	else if ( arrows_fired < 5 )
		return MAX_SCORE / 2;
	else
		return MAX_SCORE / 5;
}

/*
unsigned int check_for_win_cond()
---
Checks each active projectile to see if it is within the target's collider.
If so, break and return 1.
---
struct Proj_arr * p_arr: struct to use
struct Level * l: level to use
---
Returns an unsigned int, 1 on win, 0 on no win
*/
unsigned int check_for_win_cond( struct Proj_arr * p_arr, struct Level * l )
{
	unsigned int win = 0;
	unsigned int done = 0;
        int index = 0;
	while ( !done ) {
		
                if ( index >= p_arr->size || win )
			done = 1;
		else {
			if ( p_arr->arr[index].active )
			        win = is_win( l, &p_arr->arr[index].pos );
		}
		index++;
	}
	return win;
}

/*
int main()
---
Entrypoint of the program. Handles the main loop of the game and execution of
everything else
*/
int main( )
{
        ALLEGRO_DISPLAY* display;
	ALLEGRO_EVENT_QUEUE* event_queue;
	ALLEGRO_TIMER* timer;
	ALLEGRO_EVENT event;
	ALLEGRO_MOUSE_STATE state;

	/* init allegro and install necessary addons */
	al_init();
        al_install_mouse();
	al_install_keyboard();
	al_init_image_addon();
	al_init_primitives_addon();

	/* load all bitmaps and put them in a struct for passing around funcs */
	struct Bitmap b;

	/* create display */
	display = al_create_display( WIN_WIDTH, WIN_HEIGHT );

	/* do timer */
	timer = al_create_timer( 1.0 / FPS );

	/* do queue */
        event_queue = al_create_event_queue( );
	al_register_event_source( event_queue,
				  al_get_keyboard_event_source() );
	al_register_event_source( event_queue,
				  al_get_display_event_source(display) );
	al_register_event_source( event_queue,
				  al_get_timer_event_source(timer) );
	al_register_event_source( event_queue, al_get_mouse_event_source() );

	/* start timer */
	al_start_timer(timer);

	unsigned int curr_level = S_LEVEL;       //current level
	unsigned int redraw = 0;                 //if screen needs redrawing
	unsigned int exit = 0;                   //if program needs closing
	unsigned int create_proj = 0;            //if proj needs creating
	unsigned int do_load = 1;                //if new level needs loading
	unsigned int arrows_fired;               //counter of arrows fired
	unsigned int score = 0;                  //current score
	unsigned int menu = 1;                   //whether start menu or game
	int frame = 0;                           //counter of frame (0-59)

	/* set up proj_arr */
	struct Proj_arr proj_arr;
	initialize_proj_arr( &proj_arr );

	struct Position mouse;
	struct Level l;
	
	while ( !exit ) {
                /* check if level needs loading */
		if ( do_load ) {
			initialize_tilemap( &l.fg, 20, 15 );
	                initialize_tilemap( &l.bg, 20, 15 );
			initialize_tilemap( &l.dec, 20, 15 );
                        load_level( &l, curr_level );

			/* reload bitmaps */
			load_bitmaps( &b );
			mask_bitmaps( &b );      
			
			/* reset proj_arr */
			reset_proj_arr( &proj_arr );

			/* reset level variables */
			do_load = 0;
			arrows_fired = 0;
		}

		
		/* get next event */
                al_wait_for_event( event_queue, &event);

                /* if display closed */
		if ( event.type == ALLEGRO_EVENT_DISPLAY_CLOSE )
                        exit = 1;

		/* if tick */
		if ( event.type == ALLEGRO_EVENT_TIMER )
                        redraw = 1;

		/* if lmb clicked */
		if ( event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
		     && (event.mouse.button == 1) )
			create_proj = 1;


		/* if key pressed */
		if ( event.type == ALLEGRO_EVENT_KEY_UP ) {
                        if (event.keyboard.keycode == ALLEGRO_KEY_Q)
                                exit = 1;
			if( event.keyboard.keycode == ALLEGRO_KEY_ENTER
			    && menu != 2)
				menu = 0;
		}


		/* called FPS times per second, main game loop */
		if ( redraw && al_is_event_queue_empty( event_queue ) ) {
                        redraw = 0;
			frame++;
			
			if (frame== 60)
				frame = 0;

			/* chck which screen needs to be drawn */
			if ( menu == 1 ) //menu screen
				draw_menu(&b);
			else if ( menu == 2 )
				draw_end_menu( &b, score );
			else { //game screen
		       
				/* update proj_arr if new projectile */
				if ( create_proj && !do_load ) {
					arrows_fired++;
					create_proj = 0;
					/* add projectile to proj_arr */
					add_to_proj_arr( &proj_arr, &mouse,
							 l.start_pos );
				}

				/* update mouse pos */
				al_get_mouse_state(&state);
				initialize_position( &mouse,
						     state.x, state.y );

				draw_screen( &l, &mouse, &proj_arr, &b,
					     frame, score );

				/* check for win condition */
				if ( check_for_win_cond( &proj_arr, &l )
				     && !do_load ) {
					score += calculate_score(arrows_fired);
					curr_level += 1;
					do_load = 1;

					/* free level, tilemaps and bitmaps */
					free_level( &l );
					free_tilemap( &l.fg );
					free_tilemap( &l.bg );
					free_tilemap( &l.dec );
					free_bitmaps( &b );

					/* check if entire game finished */
					if ( curr_level > LAST_LEVEL )
						menu = 2;
				}

			}
		}
	}
        printf("Closing...\n");

	/* free all dynamically allocated stuff */
	free_level( &l );
	free_tilemap( &l.fg );
	free_tilemap( &l.bg );
	free_tilemap( &l.dec );
	free_bitmaps( &b );
}

