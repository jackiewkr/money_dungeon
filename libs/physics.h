#ifndef PHYSICS_H_
#define PHYSICS_H_

/**
physics.h
---
File used to store all the functins to do with physics calculations,
collisions, etc.
*/

#include <math.h>

#include "structures.h"

#define TM_SZ (300) /* 20x15 tilemap size, equal to window width/height / 32 */
#define THROW_FACTOR (2) /* how much to divide the mouse velocity by */

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
void get_velocity_from_mouse( struct Position *, struct Position *,
			      struct Position * );

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
unsigned int do_step_collision( struct Position *, struct Position *,
				struct Level *, unsigned int * );

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
void calculate_position( struct Position *, struct Position *,
			 struct Position *,
			 struct Level *, float );

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
void calc_arc_from_comp( struct Position *, float*, float*, unsigned int );

/*
void do_rebound()
---
Setup a given projectile to rebound from a wall
---
struct Projectile * p: projectile to rebound
*/
void do_rebound( struct Projectile * );
/*
void do_bounce()
---
Setup a given projectile to bounce up from a jump pad with a given velocity
---
struct Projectile * p: projectile to bounce
int b_vel: velocity of the bounce
*/
void do_bounce( struct Projectile *, int );

#endif //PHYSICS_H_
