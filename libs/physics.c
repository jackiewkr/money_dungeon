/**
physics.c
---
File used to store all the functins to do with physics calculations, collisions,
etc.

Functions that should not be accessed outside of this file are given the
keyword 'static'.
*/

#include "physics.h"


/*
void get_velocity_from_mouse()
---
Calculates the component velocities by subtracting the mouse pos from the
start pos. The final components are divided by THROW_FACTOR to ensure the
throw does not go off the screen easily.
---
struct Position * v: struct to store the component velocities in
struct Position * s_pos: starting xy pos of the throw
struct Position * mouse: current xy pos of the mouse
*/
void get_velocity_from_mouse( struct Position * v, struct Position * s_pos,
			      struct Position * mouse )
{
	float vel_x, vel_y;
        /* subtract differences and divide by throw_factor*/
        vel_x = ( mouse->x - s_pos->x ) / THROW_FACTOR;
	vel_y = ( mouse->y - s_pos->y ) / THROW_FACTOR;

	/* cast to struct */
	v->x = vel_x;
	v->y = vel_y;
	
}

/*
unsigned int is_oob()
---
Checks if a projectile at a given position is out of bounds of the level. This
is in a separate function from is_collide() in order to be called if a level
has no objects to iterate through (only for testing purposes)
---
struct Position * pos; pos of the projectile
---
Returns an unsigned int, 1 if oob, 0 if not
*/
static unsigned int is_oob( struct Position * pos )
{
        if ( (pos->x <= 0 || pos->x >= 640) ||
	     (pos->y <= 0 || pos->y >= 480) )
		return 1;
	return 0;
}

/*
unsigned int is_collide()
---
Checks if a projectile at position (pos) collides with any of the objects in
the level. this is done by iterating through all the objects on object_arr,
figuring out bounds and checking if the projectile is within any of the bounds.
Also checks if the projectile is out of bounds of the window
---
struct Position * pos: position the projectile is at ( stored as float )
struct Level * l: level the projectile is in, stores the object_arr
                  of the level
---
Returns an unsigned int, -1 if no collision, type of object if collision,
0 is out of bounds.

This is because objects can have type 0, which leads to confusion when parsing
the result.
*/
static unsigned int is_collide( struct Position * pos, struct Level * l )
{
	float min_x, min_y, max_x, max_y;
	unsigned int return_type = -1;
        /* iterate through object_arr */
	unsigned int brk = 0;
	int i = 0;
	while ( !brk && i < l->index ) {
                /* figure out bounds of current object */
		min_x = l->object_arr[i].pos.x;
		max_x = l->object_arr[i].pos.x + l->object_arr[i].dims.x;

		min_y = l->object_arr[i].pos.y - l->object_arr[i].dims.y;
		max_y = l->object_arr[i].pos.y;

		/* check if pos is within obj bounds */
		if ( (pos->x >= min_x && pos->x <= max_x) &&
		     (pos->y >= min_y && pos->y <= max_y) ) {
                        /* is colliding */
			return_type = l->object_arr[i].type;
			brk = 1;
		}

		/* if projectile out of window bounds */
		if (is_oob(pos)) {
                        /* is out of bounds */
			return_type = 0;
			brk = 1;
		}
		i++;
	}

	return return_type;
}

/*
unsigned int do_step_collide()
---
Collision method based on the Super Mario 64 Quarter-Step Collision method.
Given the previous position and the current position, the function checks
collision at each quarter of the distance. If there is collision at any of the
steps, return the last step that did not collide. If there is a collision, the
type of object is also returned for future processing.
---
struct Position * curr_pos: current position of the projectile
struct Position * prev_pos: previous position of the projectile
struct Level * l: level to be passed to the is_collide() function
unsigned int * type: type of object the projectile collided with, if collision
---
Returns unsigned int from 0-4,
returns unsigned int * type [implicit].
*/
unsigned int do_step_collision( struct Position * curr_pos,
				struct Position * prev_pos,
				struct Level * l, unsigned int * type  )
{
        float step_x = ( curr_pos->x - prev_pos->x ) / 4;
	float step_y = ( curr_pos->y - prev_pos->y ) / 4;
	int coll_type;
	/* assume no collision will occur */
	unsigned int step = 4;
	
        struct Position step_pos;
	initialize_position( &step_pos, prev_pos->x, prev_pos->y );
	
	/* iterate throug the four steps */
	int i = 1;
	unsigned int brk = 0;
	
	while ( i < 5 && !brk ) {
                /* increment step */
		step_pos.x += step_x;
		step_pos.y += step_y;

		/* check for collision */
		coll_type = is_collide( &step_pos, l );
		if ( coll_type != -1 ) {
                        /* collision has occured */
			step = i - 1;
			*type = coll_type;
			brk = 1;
		}

		i++;
	}
	return step;
}

/*
void calculate_position()
---
Calculates the position along an arc at time t with velocity v and 
forces gravity and wind on the projectile (using SUVAT equations)
---
struct Position * v: velocity of the projectile
struct Position * pos: current position of the projectile
struct Position * s_pos: starting position of the projectile
struct Level * l: level the projectile is in, stores wind and gravity
float time: time of current throw
---
Returns new values for struct position * pos [implicit].
*/
void calculate_position( struct Position * v, struct Position * pos,
			 struct Position * s_pos,
			 struct Level * l, float time )
{
        pos->x = s_pos->x + ( v->x + l->wind.x + l->gravity.x ) * time;
	
	pos->y = s_pos->y + ( v->y - l->wind.y ) * time +
		 (( l->gravity.y ) * time * time) /2;
	
};

/*
void calc_arc_from_comp()
---
Calculates the magnitude and angle of a arc from a given x,y component velocity
---
struct Position * p: velocity x,y comps
float * mag: magnitude calculated
float * angle: angle calculated (either in rad or deg)
unsigned int is_rad: whether 'angle' is in rad or deg
---
Returns nothing explicitly
*/
void calc_arc_from_comp( struct Position * p, float * mag, float * angle,
			 unsigned int is_rad )
{
        *mag = sqrt( p->x*p->x + p->y*p->y );
	*angle = atan2( p->y, p->x );
	if (!is_rad)
		*angle *= 180/M_PI;
}

/*
void do_rebound()
---
Setup a given projectile to rebound from a wall
---
struct Projectile * p: projectile to rebound
*/
void do_rebound( struct Projectile * p )
{
	initialize_position(&p->s_pos, p->pos.x, p->pos.y);
	initialize_position(&p->p_pos, p->pos.x, p->pos.y);
	
	/* rebound with less velocity */
	p->vel.x = -(p->vel.x) / 1.5;

	/* restart as if new throw */
	p->time = 0;
	p->p_coll = 4;
}

/*
void do_bounce()
---
Setup a given projectile to bounce up from a jump pad with a given velocity
---
struct Projectile * p: projectile to bounce
int b_vel: velocity of the bounce
*/
void do_bounce( struct Projectile * p, int b_vel )
{
	initialize_position(&p->s_pos, p->pos.x, p->pos.y);
	initialize_position(&p->p_pos, p->pos.x, p->pos.y);
	
	/* bounce with given vel */
	p->vel.y = b_vel;

	/* restart as if new throw */
	p->time = 0;
	p->p_coll = 4;
}
