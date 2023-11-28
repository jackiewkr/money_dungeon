#ifndef STRUCTURES_H_
#define STRUCTURES_H_

/**
structures.h
---
Header file used to store all structs needed for the program and
function declarations for publicly accessible function from structures.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <allegro5/allegro_image.h> /* needed for struct bitmap */
#include <allegro5/allegro.h> /* needed for struct bitmap */

#define OBJECT_ARR_SZ (32)
#define PROJ_ARR_SZ (32)

/**
struct Bitmap
---
Bitmap struct used to store all images needed for the game.
*/
struct Bitmap {
        ALLEGRO_BITMAP * ts;         //combined tileset for all levels
	ALLEGRO_BITMAP * plyr;       //player
	ALLEGRO_BITMAP * plyr_bow;   //bow
	ALLEGRO_BITMAP * proj;       //arrow
	ALLEGRO_BITMAP * water;      //water texture
	ALLEGRO_BITMAP * font;       //font tileset
	ALLEGRO_BITMAP * bg;         //start menu's bg
	ALLEGRO_BITMAP * target;     //target bmp
};

/**
struct Tilemap
---
Tilemap struct used to store data about a given tilemap (i.e. fg, bg, dec, etc)
---
unsigned int rows, cols: no of rows/cols
unsigned int size: size of tilemap (equal to rows*cols)
char * map: tilemap data (each tile is stored sequentially from left to right,
            top to bottom, tiles are stored as hex characters '0'-'9','A'-'F')
*/
struct Tilemap
{
        unsigned int rows;
	unsigned int cols;
	char * map;
	unsigned int size;
};

/**
struct Position
---
Position struct used to store anything that has x,y components (usually 
position)
*/
struct Position
{
	float x;
	float y;
};

/**
struct Object
---
Object struct used to store all relevant data about an object.
Objects can be colliders, jump pads, reboundable walls, etc.
---
struct Position pos: position of bottom left corner of object
struct Position dims: width and height of position
unsigned int type: what type the object is (used when calculating proj colls)
unsigned int move: whether the object moves or not (NOT IMPLEMENTED)
struct Position end_pos: where the object should move to (NOT IMPLEMENTED)
*/
struct Object
{
        struct Position pos;
	struct Position dims;
	unsigned int type;
	unsigned int move;
	struct Position end_pos;
};

/**
struct Level
---
Stores all data needed for a given level
---
struct Position start_pos: starting pos of player
struct Position target_pos: position of target
struct Position wind: x,y comps of force (wind)
struct Position gravity: x,y comps of force (gravity)
struct Object * object_arr: array of objects in level
struct Tilemap fg, bg, dec: tilemaps of level
unsigned int size: size of object_arr
unsigned int index: index of object_arr
unsigned int level: which level number the level is
*/
struct Level
{
        struct Position start_pos;
	struct Position target_pos;
	struct Position wind;
	struct Position gravity;
	struct Object * object_arr;
	struct Tilemap fg;
	struct Tilemap bg;
	struct Tilemap dec;
	unsigned int size;
	unsigned int index;
	unsigned int level;
};

/**
struct Projectile
---
Projectile struct used to store data about a given projectile. Allows
multiple projectiles to be active at the same time.
---
unsigned int active: whether the projectile is active
struct Position s_pos: starting pos of the projectile
struct Position p_pos: previous pos of the projectile
struct Position vel: velocity
float time: time of the proj
float g_time: global time of the projectile (used for despawning)
int p_coll: prevous 4-step collision. used to check when a projectile
	    can no longer move (i.e. collided with no rebound/jump)
*/
struct Projectile {
        unsigned int active;
	struct Position s_pos;
	struct Position p_pos;
	struct Position pos;
	struct Position vel;
	float time;
	float g_time;
	int p_coll;
};

/**
struct Proj_arr
---
Struct to store all active and non-active projectile currently in a level
---
struct Projectile * arr: array of Projectile objects
unsigned int size: size of arr
unsigned int index: current index of arr
*/
struct Proj_arr {
        struct Projectile *  arr;
	unsigned int size;
	unsigned int index;
};

/*
unsigned int initialize_proj_arr()
---
Initializes a given Proj_arr with default values
---
struct Proj_arr *: struct to initialize
---
Returns 1 on success, 0 on fail
*/
unsigned int initialize_proj_arr( struct Proj_arr * );

/*
unsigned int initialize_tilemap()
---
Initializes a given tilemap with given values
---
struct Tilemap * tm: struct to initialize
unsigned in r, c: rows and cols of tilemap
---
Returns 1 on success, 0 on fail
*/
unsigned int initialize_tilemap( struct Tilemap *,
				 unsigned int, unsigned int );
	
/*
unsigned int initialize_position()
---
Initializes a given position with given values
---
struct Position *: struct to initialize
float x,y: components of struct
---
Returns 1 on success, 0 on fail
*/
unsigned int initialize_position ( struct Position *, float, float );

/*
unsigned int initialize_object()
---
Initializes a given Object with given values
---
struct Object *: struct to initialize
float pos_x, pos_y: xy of bottom left corner of position
float dims_x, dims_y: width and height of object
unsigned int type: type of object (see struct Object for a full list)
unsigned int move: whether the objecct can move
float end_x, end_y: xy of there the object should move back and forth to
---
Returns 1 on success, 0 on fail
*/
unsigned int initialize_object ( struct Object *, float, float,
			         float, float, unsigned int,
				 unsigned int, float,
				 float );

/*
unsigned int initialize_level()
---
Initializes a given level with all necessary details about it, along
with allocating some memory for the dynamic array object_arr.
---
struct Level * l: level to be initialized
float start_x, start_y: player's starting position
float target_x, target_y: target's position
float wind_x, wind_y: force acting on the projectile
float gravity_x, gravity_y: gravity acting on the projectile
unsigned int level_no: level number
---
Returns unsigned int 1 on success, 0 on fail
*/
unsigned int initialize_level ( struct Level *, float, float, float, float,
				float, float, float, float, unsigned int );

/*
unsigned int add_object_to_level()
---
Adds a struct Object to the object_arr in a given struct Level.
---
struct Level * l: level to be added to
struct Object o: objecto to be added
---
Returns 1 on success, 0 on fail
*/
unsigned int add_object_to_level ( struct Level *, struct Object );

/*
unsigned int free_level()
---
Frees the memory allocated to the object array and resets size and
index of the object array
---
struct Level * l: level to be freed
---
Returns 1 on success, 0 on fail
*/
unsigned int free_level ( struct Level * );

/*
unsigned int add_to_proj_arr()
---
Initializes a new projectile in proj_arr with given values
---
struct Proj_arr * p: Proj_arr to add to
struct Position * m: mouse Position
struct Position s_pos: starting pos of projectile
---
Returns nothing
*/
void add_to_proj_arr( struct Proj_arr *, struct Position *,
			      struct Position );

/*
unsigned int free_level()
---
Frees the memory allocated to the tilemap and resets size and
index of the array
---
struct Tilemap * tm: tilemap to be freed
---
Returns 1 on success, 0 on fail
*/
void free_tilemap( struct Tilemap * );

/*
void reset_proj_arr()
---
Resets a given proj_arr for use when changing levels
---
struct Proj_arr * proj_arr: struct to reset
*/
void reset_proj_arr( struct Proj_arr * );


/* because physics.h requires structures.h, a compiler warning is thrown if
this is included at the top of the file */
#include "physics.h" /* needed for initializing projectiles */

#endif //STRUCTURES_H_
