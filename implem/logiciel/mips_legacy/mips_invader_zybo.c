#include "sprite.h"


/*
 * definition of functions' prototype
 * ---------------------------------------------------------------------------
 */
int read_pixel(int x, int y);
void write_pixel(int pixel, int x, int y);
void clear_screen(uint32 color);
void initialize();
void display_pattern_line(uint32 m,int x,int y,uint32 color);
void display_pattern(uint32 pattern[8], int x, int y, uint32 color);
void display_sprite(Object *object);
void timer_set(uint32 period, uint32 start_value);
void timer_wait(void);
void led_set(uint32 value);
uint32 push_button_get(void);


/*
 * definition of macros
 * ---------------------------------------------------------------------------
 */
#define DISPLAY_WIDTH  320 /* display width resolution */
#define DISPLAY_HEIGHT 240 /* display height resolution */
#define N_OBJECTS 7        /* displayed objects (aliens, laser, spaceship) */


/*
 * definition of global variables
 * ---------------------------------------------------------------------------
 */

/* sprite objects */
Object object[N_OBJECTS] = {
	{1, 3, 1, 20, 29, 0, 0, sprite_sship,  0x003F}, /* blue spaceship */
	{0, 1, 1, 20, 29, 0, 0, sprite_laser,  0xFFFF}, /* white laser */
	{1, 4, 1, 10, 1, -1, 0, sprite_alien1, 0x0FC0}, /* green alien */
	{1, 4, 1, 18, 1, -1, 0, sprite_alien2, 0xF000}, /* red alien */
	{1, 4, 1, 26, 1, -1, 0, sprite_alien3, 0xF03F}, /* magenta alien */
	{1, 4, 1, 14, 3, -1, 0, sprite_alien4, 0xFFC0}, /* yellow alien */
	{1, 4, 1, 22, 3, -1, 0, sprite_alien5, 0x0FFF}  /* cyan alien */
};

/* pointers to the peripherals base address */
volatile uint32 *img  = (volatile uint32 *)0x80080000; /* IP VGA */
volatile uint32 *led  = (volatile uint32 *)0x80004000; /* IP led */
volatile uint32 *push = (volatile uint32 *)0x80004004; /* IP push */
volatile uint32 *timer= (volatile uint32 *)0x80004010; /* IP timer */


/*
 * main program
 * ---------------------------------------------------------------------------
 */
void main(void)
{
	/* declaration of local variables */
	uint32 i, dx, dy;
	uint32 push_state, led_state, alien_state, edge_reached;
	uint32 n_aliens;
	Object *spaceship, *laser;

	/* initialization stage */
	push_state = 0;           /* no button pressed at beginning */
	led_state = 0;            /* initial value displayed on leds */
	alien_state = 0;          /* state of alien in a line */
	edge_reached = 0;         /* no edge reached at beginning */
	n_aliens = N_OBJECTS - 2; /* number of displayed aliens */
	spaceship = &object[0];   /* spaceship is the first declared object */
	laser = &object[1];       /* laser is the second declared object */
	timer_set(1200000, 200);
	initialize();

	/* display stage */
	while(1) {
		edge_reached=0;

		/* decrease deadline of alive objects */
		for (i = 0; i < N_OBJECTS; i++) {
			if (object[i].alive)
				object[i].deadline--;
		}

		/* display all alive objects */
		for (i = 0; i < N_OBJECTS; i++) {
			if (object[i].alive)
				display_sprite(&object[i]);
		}

		/* determine new positions of all alive objects */
		for (i = 0; i < N_OBJECTS; i++) {
			/* update object state when deadline is reached */
			if (object[i].alive && object[i].deadline == 0) {
				/* reinititialize the object deadline to period */
				object[i].deadline = object[i].period;
				/* determine new position and manage screen edges */
				object[i].x += object[i].dx;
				if (object[i].x < 0)
					object[i].x = 0;
				if (object[i].x > 39)
					object[i].x = 39;
				object[i].y += object[i].dy;
				/* test if an edge of the screen was reached by an alien */
				if (i >= 2 && (object[i].x == 0 || object[i].x == 39))
					edge_reached = 1;
				/* store background of the next position */
				if (i > 1 && object[i].y >= spaceship->y){
					clear_screen(0x00FF); /* blue screen */
					initialize();
				}
			}
		}

		/* test if alien is hit by an alive laser */
		if (laser->alive) {
			for(i = 2; i < N_OBJECTS; i++) {
				if (object[i].alive && laser->x == object[i].x &&
					laser->y == object[i].y) {
					n_aliens--;
					object[i].alive = 0;
					laser->alive = 0;
					if (n_aliens == 0) {
						/* no more aliens */
						spaceship->alive = 0;
						clear_screen(0xFF00); /* yellow screen */
					} else {
						display_sprite(&object[i]);
						display_sprite(laser);
					}
				}
			}
		}

		/* when an alien reaches a screen edge, the group of aliens is moved */
		if (edge_reached) {
			switch(alien_state) {
				case 0: /* from init position --> to down */
					for (i = 2; i < N_OBJECTS; i++) {
						object[i].dx = 0;
						object[i].dy = 1;
					}
					alien_state = 1;
					break;
				case 1: /* in left edge --> to right */
					for(i = 2; i < N_OBJECTS; i++) {
						object[i].dx = 1;
						object[i].dy = 0;
					}
					alien_state = 2;
					break;
				case 2: /* in right edge (from left side) --> to down */
					for (i = 2; i < N_OBJECTS; i++) {
						object[i].dx = 0;
						object[i].dy = 1;
					}
					alien_state = 3;
					break;
				case 3: /* in right edge (from up) --> to left */
					for (i = 2; i < N_OBJECTS; i++) {
						object[i].dx = -1;
						object[i].dy = 0;
					}
					alien_state = 1;
					break;
			};
		}

		/* laser disappears when it reaches the screen top */
		if (laser->alive && laser->y == 0) {
			laser->alive = 0;
			display_sprite(laser);
		}

		/* manage push buttons */
		if (spaceship->deadline == 1) {
			push_state = push_button_get();
			spaceship->dx = 0;
			if (push_state & 0x1)
				/* to right */
				spaceship->dx = 1;
			if (push_state & 0x2)
				/* to left */
				spaceship->dx = -1;
			if (push_state & 0x4) {
				/* fire a laser */
				if (!laser->alive) {
					laser->alive = 1;
					laser->dx = 0;
					laser->dy = -1;
					laser->x = spaceship->x;
					laser->y = spaceship->y - 1;
					laser->deadline = laser->period;
				}
			}
		}

		/* manage leds' state */
		led_set(led_state);
		led_state++;
		timer_wait();
	}
}


/*
 * definition of functions
 * ---------------------------------------------------------------------------
 */

/* function to read a pixel from a (x,y) position of video framebuffer */
int read_pixel(int x, int y)
{
	return img[y * DISPLAY_WIDTH + x];
}

/* function to write a pixel in a (x, y) position of video framebuffer */
void write_pixel(int pixel, int x, int y)
{
	img[y * DISPLAY_WIDTH + x] = pixel;
}

/* function to clear entire screen to the selected color */
void clear_screen(uint32 color)
{
	uint32 x, y;
	for (x = 0; x < DISPLAY_WIDTH; x++)
		for (y = 0; y < DISPLAY_HEIGHT; y++)
			write_pixel(color, x, y);
}

/* function to initialize all objects */
void initialize()
{
	uint32 i, dx, dy;
	clear_screen(0); /* black screen */

	for(i = 0; i < N_OBJECTS; i++) {
		if (i == 1) {
			/* laser */
			object[i].alive = 0;
			object[i].period = 1;
		} else {
			/* spaceship or aliens */
			object[i].alive = 1;
			if (i == 0)
				/* spaceship */
				object[i].period = 3;
			else
				/* aliens */
				object[i].period = 4;
		}
		object[i].deadline = 1;
		if (i > 1) {
			/* aliens */
			if (i > 4) {
				/* alien4 or alien5 */
				object[i].y = 3; /* 3rd line */
				object[i].x = 6 + (i - 4) * 8 ;
			} else {
				/* alien1, alien2 or alien3 */
				object[i].y = 1; /* 1st line */
				object[i].x = 10 + (i - 2) * 8;
			}
			object[i].dx = -1;
			object[i].dy = 0;
		}
		object[i].ax = -1;
		object[i].ay = -1;

		/* initialization of object background considering the last one */
		for (dx = 0; dx < 8; dx++)
			for (dy = 0; dy < 8; dy++)
				object[i].bg[dx][dy] = read_pixel(((object[i].x) << 3) + dx,
				                                  ((object[i].y) << 3) + dy);
	}
}

/* function to display the 8 pixels of a pattern line */
void display_pattern_line(uint32 m, int x, int y, uint32 color)
{
	int i;

	for (i = 0; i < 8; i++) {
		int new_color = ((m & 1) == 1) ? color : 0;
		m = m >> 1;
		write_pixel(new_color, x + i, y);
	}
}

/* function to display the 8 lines of a pattern (spaceship, laser or alien) */
void display_pattern(uint32 pattern[8], int x, int y, uint32 color)
{
	int i;

	for(i = 0; i < 8; i++)
		display_pattern_line(pattern[i], x, y + i, color);
}

/* function to display an 8x8 object considering the last background */
void display_sprite(Object *object)
{
	int dx, dy;

	if ((object->ax > -1 && object->ay > -1) &&
		(object->x != object->ax || object->y != object->ay || !object->alive))
	{
		for (dx = 0; dx < 8; dx++) {
			for (dy = 0; dy < 8; dy++) {
				write_pixel(object->bg[dx][dy],
							((object->ax) << 3) + dx, ((object->ay) << 3) + dy);
				if (!object->alive)
					object->bg[dx][dy] = read_pixel(((object->x) << 3) + dx,
					                                ((object->y) << 3) + dy);
			}
		}
	}

	object->ax = object->x;
	object->ay = object->y;

	if (object->alive)
		display_pattern(object->pattern, (object->x) << 3, (object->y) << 3,
		                object->color);
}

/* function to set the period and the start value of timer */
void timer_set(uint32 period, uint32 start_value)
{
	*timer = period;
	*(timer + 1) = start_value;
}

/* function to wait for timer zero value */
void timer_wait(void)
{
	while(*timer != 0);
}

/* function to set the value displayed on leds */
void led_set(uint32 value)
{
	*led = value;
}

/* function to get the state of push buttons */
uint32 push_button_get(void)
{
	return (*push) >> 16;
}
