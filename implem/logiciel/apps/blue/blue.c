
#include "cep_platform.h"
#include <stdint.h>

volatile uint32_t *img  = (volatile uint32_t *)VRAM_OFFSET; /* Memory Video */


#define DISPLAY_WIDTH  1920 /* display width resolution */
#define DISPLAY_HEIGHT 1080  /* display height resolution */
#define N_OBJECTS 7        /* displayed objects (aliens, laser, spaceship) */

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


int main() {

    clear_screen(0xFF0000);

    while(1);

}
