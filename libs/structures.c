/**
structures.c
---
File used to store all the functions directly relating to initializing,
modifying or freeing structures/unions.
*/

#include "structures.h"

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
unsigned int initialize_tilemap( struct Tilemap * tm,
				 unsigned int r, unsigned int c )
{
        tm->rows = r;
	tm->cols = c;
	tm->size = r * c;

	tm->map = malloc(sizeof( char ) * tm->size );

	if (tm->map == NULL)
		return 0;
	return 1;
}

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
unsigned int initialize_position ( struct Position * a, float x, float y )
{
	a->x = x;
	a->y = y;
	
	/* function cannot fail in normal use */
	return 1;
};

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

unsigned int initialize_object ( struct Object *  o, float pos_x, float pos_y,
			 float dims_x, float dims_y, unsigned int type,
			 unsigned int move, float end_x, float end_y )
{
        unsigned int a = initialize_position( &o->pos, pos_x, pos_y );
	unsigned int b = initialize_position( &o->dims, dims_x, dims_y );
	unsigned int c = initialize_position( &o->end_pos, end_x, end_y );
	
	o->type = type;
	o->move = move;

	/* function cannot fail in normal use */
	if ( a && b && c )
	        return 0;
	return 1;
};

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
unsigned int initialize_level ( struct Level * l,
				float start_x, float start_y,
			        float target_x, float target_y,
				float wind_x, float wind_y,
				float gravity_x, float gravity_y,
				unsigned int level_no )
{
	l->level = level_no;
	
	/* set positions up */
	unsigned int a = initialize_position( &l->start_pos,
					     start_x, start_y );
	unsigned int b = initialize_position( &l->target_pos,
					     target_x, target_y );
	unsigned int c = initialize_position( &l->wind,
					      wind_x, wind_y );
	unsigned int d = initialize_position( &l->gravity,
					      gravity_x, gravity_y );
	
	/* set variables for using dynamic array */
        l->size = OBJECT_ARR_SZ;
	l->index = 0;
	
	/* allocate space for dynamic array */
	l->object_arr = malloc( l->size * sizeof (struct Object) );

	/* check if it failed */
	if ( l->object_arr == NULL ) {
	        fprintf( stderr, "Failed to malloc in initialize_level!\n" );
	        return 0;
        }

	/* if initialize_position somehow fails */
	if ( a && b && c && d )
	        return 0;
	return 1;
};

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
unsigned int add_object_to_level ( struct Level * l, struct Object o )
{
	/* check if the array is full */
	if (l->size == l->index)
	{
                fprintf(stderr, "Too many objects in level!\n");
		return 0;
	}

	/* add object to array */
        initialize_object( &l->object_arr[l->index], o.pos.x, o.pos.y,
			   o.dims.x, o.dims.y, o.type, 0, 0, 0 );
	l->index++;

	return 1;
};

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
unsigned int free_level ( struct Level * l )
{
        free( l->object_arr );
	l->size = 0;
	l->index = 0;

	/* function cannot fail in normal use */
	return 1;
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
unsigned int initialize_proj_arr( struct Proj_arr * p ) {
        p->size = PROJ_ARR_SZ;
	p->index = 0;

	p->arr = malloc( sizeof( struct Projectile ) * p->size );

	if ( p->arr == NULL ) {
                fprintf(stderr, "Could not initialize projectile arr!\n");
		return 0;
	}
	return 1;
}

/*
unsigned int initialize_projectile()
---
Initializes a struct Projectile with given values
---
struct Projectile * p: struct to initialize
struct Position * v: velocity of projectile
struct Position * s_pos: starting position of projectile
---
Returns 1 on success, 0 on fail
*/
static unsigned int initialize_projectile( struct Projectile * p,
				    struct Position * v,
				    struct Position * s_pos )
{
        p->active = 1;
	/* all start at the same values */
	initialize_position( &p->s_pos, s_pos->x, s_pos->y );
	initialize_position( &p->p_pos, s_pos->x, s_pos->y );
	initialize_position( &p->pos, s_pos->x, s_pos->y );

	initialize_position( &p->vel, v->x, v->y );

	p->time = 0;
	p->g_time = 0;
	p->p_coll = 4;

	/* function cannot fail in normal use */
	return 1;
}

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
void add_to_proj_arr( struct Proj_arr * p, struct Position * m,
		      struct Position s_pos )
{
        if (p->index == p->size) {
                /* wrap around */
		p->index = 0;
	}
        /* adjust s_pos for which way the player is facing*/
	if ( m->x < (s_pos.x+13) ) {
		s_pos.x -= 50;
		s_pos.y -=50;
	} else {
                s_pos.x += 50;
		s_pos.y -=50;
	}
	
        /* calculate velocity */
	struct Position vel;
        get_velocity_from_mouse( &vel, &s_pos, m );
	
	initialize_projectile(&p->arr[p->index], &vel, &s_pos );
	p->index++;
}

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
void free_tilemap( struct Tilemap * tm )
{
        free(tm->map);
	tm->size = 0;
	tm->rows = 0;
	tm->cols = 0;
}

/*
void reset_proj_arr()
---
Resets a given proj_arr for use when changing levels
---
struct Proj_arr * proj_arr: struct to reset
*/
void reset_proj_arr( struct Proj_arr * proj_arr )
{
        for (int p = 0; p < proj_arr->size; p++) {
		proj_arr->arr[p].active = 0;
	}
	
	proj_arr->index = 0;
}
