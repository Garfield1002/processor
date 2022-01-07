#include "sprite.h"
#include "cep_platform.h"

#include <stdint.h>

#ifdef ENV_QEMU
#include <string.h>
#include <stdio.h>
#endif

/*
 * definition of functions' prototype
 * ---------------------------------------------------------------------------
 */
int read_pixel(int x, int y);
void write_pixel(int pixel, int x, int y);
void clear_screen(uint32_t color);
void initialize();
void display_pattern_line(uint32_t m,int x,int y,uint32_t color);
void display_pattern(uint32_t pattern[8], int x, int y, uint32_t color);
void display_sprite(Object *object);
void timer_set(uint32_t period, uint32_t );
void timer_set_and_wait(uint32_t period, uint32_t time);
void timer_wait(void);
void led_set(uint32_t value);
uint32_t push_button_get(void);

void display_timer();

/*
 * definition of macros
 * ---------------------------------------------------------------------------
 */
#define DISPLAY_WIDTH  1920 /* display width resolution */
#define DISPLAY_HEIGHT 1080  /* display height resolution */
#define N_OBJECTS 7        /* displayed objects (aliens, laser, spaceship) */

// #SCALING
#define DISPLAY_SCALE  4

#define MAX_X 59
/*
 * definition of global variables
 * ---------------------------------------------------------------------------
 */

/* sprite objects */
Object object[N_OBJECTS] = {
	{1, 3, 1, 18, 32, 0, 0, sprite_sship,  0x0000FF}, /* blue spaceship */
	{0, 1, 1, 18, 0,  0, 0, sprite_laser,  0xFFFFFF}, /* white laser */
	{1, 4, 1, 10, 1, -1, 0, sprite_alien1, 0x00FF00}, /* green alien */
	{1, 4, 1, 18, 1, -1, 0, sprite_alien2, 0xFF0000}, /* red alien */
	{1, 4, 1, 26, 1, -1, 0, sprite_alien3, 0xFF00FF}, /* magenta alien */
	{1, 4, 1, 14, 3, -1, 0, sprite_alien4, 0xFFFF00}, /* yellow alien */
	{1, 4, 1, 22, 3, -1, 0, sprite_alien5, 0x00FFFF}  /* cyan alien */
};
typedef struct etat {
	int dx;
	int dy;
	int etat_suivant;
	} etat;
etat etats[5] = {
	{0,1,1},
	{0,1,2},
	{1,0,3},
	{0,1,4},
	{-1,0,1}
	};

/* pointers to the peripherals base address */
volatile uint32_t *img  = (volatile uint32_t *)VRAM_OFFSET; /* Memory Video */
volatile uint32_t *led  = (volatile uint32_t *)REG_LEDS_ADDR; /* IP led */
volatile uint32_t *push = (volatile uint32_t *)REG_PIN_ADDR; /* IP push */

volatile uint64_t *timer     = (volatile uint64_t *)CLINT_TIMER; 
volatile uint32_t *timer_hi =  (volatile uint32_t *)CLINT_TIMER_HI; 
volatile uint32_t *timer_lo =  (volatile uint32_t *)CLINT_TIMER_LOW; 
volatile uint64_t *timer_cmp = (volatile uint64_t *)CLINT_TIMER_CMP;
volatile uint32_t *timer_cmp_hi = (volatile uint32_t *)CLINT_TIMER_CMP_HI;
volatile uint32_t *timer_cmp_lo = (volatile uint32_t *)CLINT_TIMER_CMP_LO;

// Timer options
#ifdef ENV_QEMU 
#define TIMER_FREQ 10000000 // 10MHz
#define RATIO 500  
#else
#define TIMER_FREQ 100000000 // 100MHz
#define RATIO 200 
#endif


/*
 * main program
 * ---------------------------------------------------------------------------
 */
void main(void)
{
	/* declaration of local variables */
	uint32_t i, dx, dy;
	uint32_t push_state, led_state, alien_state, edge_reached;
	uint32_t n_aliens;
	Object *spaceship, *laser;

init:
	/* initialization stage */
	push_state = 0;           /* no button pressed at beginning */
	led_state = 0;            /* initial value displayed on leds */
	alien_state = 0;          /* state of alien in a line */
	edge_reached = 0;         /* no edge reached at beginning */
	n_aliens = N_OBJECTS - 2; /* number of displayed aliens */
	spaceship = &object[0];   /* spaceship is the first declared object */
	laser = &object[1];       /* laser is the second declared object */
	timer_set(TIMER_FREQ, 1);
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
				/* reinitialize the object deadline to period */
				object[i].deadline = object[i].period;
				/* determine new position and manage screen edges */
				object[i].x += object[i].dx;
				if (object[i].x < 0)
					object[i].x = 0;
				if (object[i].x > MAX_X)
					object[i].x = MAX_X;
				object[i].y += object[i].dy;
				/* test if an edge of the screen was reached by an alien */
				if (i >= 2 && (object[i].x == 0 || object[i].x == MAX_X))
					edge_reached = 1;
				/* store background of the next position */
				if (i > 1 && object[i].y >= spaceship->y){
					clear_screen(0xFF); /* blue screen */
					while(1);
                    timer_set_and_wait(TIMER_FREQ, 1000);
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
                        			timer_set_and_wait(TIMER_FREQ, 1000);
						clear_screen(0xFF0000); /* red screen */
					} else {
						display_sprite(&object[i]);
						display_sprite(laser);
					}
				}
			}
		}

		/* when an alien reaches a screen edge, the group of aliens is moved */
		if (edge_reached) {
			for (i = 2; i < N_OBJECTS; i++) {
				object[i].dx = etats[alien_state].dx;
				object[i].dy = etats[alien_state].dy;
			}
			alien_state = etats[alien_state].etat_suivant;
					
		}
		
		/* laser disappears when it reaches the screen top */
		if (laser->alive && laser->y == 0) {
			laser->alive = 0;
			display_sprite(laser);
		}
		
		/* manage push buttons */
        push_state = push_button_get();
        // if we won, press fire to restart
        if (   (n_aliens == 0) 
            && (push_state & 0x4)) {
            goto init;
        }
		if (   (spaceship->deadline == 1)
            || (n_aliens == 0)) {
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
    // #SCALING
    //return img[y * DISPLAY_WIDTH + x];
	const unsigned int real_y = y * DISPLAY_WIDTH * DISPLAY_SCALE;
	const unsigned int real_x = x * DISPLAY_SCALE;
	return img[real_y + real_x];
}

/* function to write a pixel in a (x,y) position of video framebuffer */
void write_pixel(int pixel, int x, int y)
{
    img[y * DISPLAY_WIDTH + x] = pixel;
}

void write_pixel_scaling(int pixel, int x, int y)
{
    // #SCALING
    unsigned int i, j;

	for (i = 0; i < DISPLAY_SCALE; ++i) {
		for (j = 0; j < DISPLAY_SCALE; ++j) {
			const unsigned int real_y = (y * DISPLAY_SCALE + i) *
			                            DISPLAY_WIDTH;
			const unsigned int real_x = x * DISPLAY_SCALE + j;
			img[real_y + real_x] = pixel;
		}
	}
}
void *memset_32b(volatile uint32_t *dest, uint32_t c, uint32_t n)
{
    volatile uint32_t *p = dest;
    while (n-- > 0) {
        *(volatile uint32_t*)dest++ = c;
    }
    return (void*)p;
}
/* function to clear entire screen to the selected color */
void clear_screen(uint32_t color)
{
    memset_32b(img, color, DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

/* function to initialize all objects */
void initialize()
{
	uint32_t i, dx, dy;
	clear_screen(0x0); /* black screen */
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
#ifdef ENV_QEMU
				object[i].bg[dx][dy] = read_pixel(((object[i].x) << 3) + dx,
				                                  ((object[i].y) << 3) + dy);
#else
				object[i].bg[dx][dy] = 0;
#endif
	}
}

/* function to display the 8 pixels of a pattern line */
void display_pattern_line(uint32_t m, int x, int y, uint32_t color)
{
	int i;

	for (i = 0; i < 8; i++) {
		int new_color = ((m & 1) == 1) ? color : 0;
		m = m >> 1;
		write_pixel_scaling(new_color, x + i, y);
	}
}

/* function to display an 8x8 object considering the last background */
void display_pattern(uint32_t pattern[8], int x, int y, uint32_t color)
{
	int i;

	for(i = 0; i < 8; i++)
		display_pattern_line(pattern[i], x, y + i, color);
}

/* function to display an 8x8 object (spaceship, laser or alien) */
void display_sprite(Object *object)
{
	int dx, dy;

	if ((object->ax > -1 && object->ay > -1) &&
		(object->x != object->ax || object->y != object->ay || !object->alive))
	{
		for (dx = 0; dx < 8; dx++) {
			for (dy = 0; dy < 8; dy++) {
				write_pixel_scaling(object->bg[dx][dy],
				            ((object->ax) << 3) + dx, ((object->ay) << 3) + dy);
				if (!object->alive)
#ifdef ENV_QEMU
					object->bg[dx][dy] = read_pixel(((object->x) << 3) + dx,
					                                ((object->y) << 3) + dy);
#else
		    		object->bg[dx][dy] = 0;
#endif
			}
		}
	}

	object->ax = object->x;
	object->ay = object->y;

	if (object->alive)
		display_pattern(object->pattern, (object->x) << 3, (object->y) << 3,
		                object->color);
}

void display_timer()
{
#ifdef ENV_QEMU 
    printf("mtimehi : 0x%x\n", *timer_hi);
	printf("mtimelo : 0x%x\n", *timer_lo);
	printf("mtimecmphi : 0x%x\n", *timer_cmp_hi);
	printf("mtimecmplo : 0x%x\n", *timer_cmp_lo);
#else
#endif
}

/* function to set the timer to be reached in period*time/100 in the future */
void timer_set(uint32_t period, uint32_t time)
{
    //display_timer();
    uint64_t now = *timer;
    *timer_cmp = now + ((uint64_t)period/RATIO * time);
    //display_timer();
}

void timer_set_and_wait(uint32_t period, uint32_t time)
{
    uint64_t now = *timer;
    *timer_cmp = now + ((uint64_t)period/RATIO * time);
	while(*timer <=  *timer_cmp);
}

/* function to wait for timer zero value */
void timer_wait(void)
{
	while(*timer <=  *timer_cmp);
 
	timer_set(TIMER_FREQ, 4);
}

/* function to set the value displayed on leds */
void led_set(uint32_t value)
{
	*led = value;
}

/* function to get the state of push buttons */
uint32_t push_button_get(void)
{
	return (*push) >> 16;
}
