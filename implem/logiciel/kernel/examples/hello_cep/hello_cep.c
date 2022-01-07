#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "arch/riscv/trap.h"
#include "arch/riscv/csr.h"
#include "arch/riscv/encoding.h"
#include "arch/riscv/machine.h"
#include "cep_platform.h"
#include "paysage_720.h"
#include "paysage_1080.h"

// Mask to enable irq 1 of PLIC 
#define PLIC_IRQ_1                 0x2

// Flickering velocity
#define SLOW 0x500000
#define FAST 0x200000

// Peripherals Registers 
volatile uint32_t *reg_switches        = (uint32_t*)REG_PIN_ADDR; 
volatile uint32_t *reg_push_button_ctl = (uint32_t*)REG_PUSHBUTTON_CTL_ADDR; 

// VRAM buffer
volatile uint32_t *vram                = (uint32_t*)(VRAM_OFFSET); 

// VRAM control registers
volatile uint32_t *reg_hdmi_MODE       = (uint32_t*)FRAME_BUFFER_CTRL_MODE_REG; 
volatile uint32_t *reg_hdmi_ADDR       = (uint32_t*)FRAME_BUFFER_CTRL_ADDR_REG; 


void sleep(uint64_t max) {
    volatile uint64_t j;
    for (uint64_t i=0; i <= max*0x8000000; i++) {
        j = i;
    }
}


void display_status(){
    long read = 0;
    
    read = read_csr_enum(csr_mstatus);
    printf("csr mstatus     : 0x%lx\n", read);
    read = read_csr_enum(csr_mie);
	printf("csr mie         : 0x%lx\n", read);
    read = read_csr_enum(csr_mip);
	printf("csr mip         : 0x%lx\n", read);
	
    printf("PLIC Pending1   : 0x%lx\n", *(volatile uint32_t*)PLIC_PENDING_1);
	printf("PLIC Enable1    : 0x%lx\n", *(volatile uint32_t*)PLIC_ENABLE_1);
    
    // Deprecated : gestion des priorités dans le PLIC a été rétirée de la plateforme cep de qemu
	//printf("PLIC Threshold  : 0x%lx\n", *(volatile uint32_t*)PLIC_THRESHOLD);
	//printf("PLIC Priority 1 : 0x%lx\n", *(volatile uint32_t*)PLIC_IRQ_PRIORITY_1);
    
    printf("\n\n");
}


void display_img(uint32_t* img) {
    memcpy((void*)vram, &img[2], img[0]*img[1]*4);
}


void ack_irq() {
	volatile uint32_t read;
    
    printf("Ack irq\n");

    // Read PLIC Claim register
    read = *(volatile uint32_t*)PLIC_IRQ_CLAIM;
	//printf("PLIC Claim      : 0x%lx\n", read);
    
    // Write to PLIC Claim register to ack irq at PLIC Level
    *(volatile uint32_t*)PLIC_IRQ_CLAIM = read;
    
    // Read switches register to ack the irq at FB periph level
    volatile uint32_t i = *reg_switches;
}


void trap_h(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc) {
    
    static uint32_t mode = 1080;
    printf("\n\nEntering trap handler\n\n");
    
    if (mode == 720) {
        mode = 1080;
        *reg_hdmi_MODE = HDMI_MODE_1080p_60Hz;
        display_img(image_1080);
    } else {
        mode = 720;
        *reg_hdmi_MODE = HDMI_MODE_720p_60Hz;
        display_img(image_720);
    }

    display_status();
    ack_irq();
    printf("Sleep a bit\n");
    sleep(3);
    printf("\nExiting trap handler\n\n");
}


void enable_irqs() {
    
    // Enable Machine Interrupts globally
    write_csr_enum(csr_mstatus, 0x8);
    // Enable Machine External Interrupt 
    write_csr_enum(csr_mie, 0xFFFFFFFF);

    // Configuring the PLIC to enable irq 1
    *(volatile uint32_t*)PLIC_ENABLE_1       = PLIC_IRQ_1;

    // Deprecated : gestion des priorités dans le PLIC a été rétirée de la plateforme cep de qemu
    //*(volatile uint32_t*)PLIC_IRQ_PRIORITY_1 = 0x2;
    //*(volatile uint32_t*)PLIC_THRESHOLD      = 0x1;

    // Setting the push button in interrupt mode
    *reg_push_button_ctl = REG_PUSHBUTTON_MODE_INT;
}

int main(int argc, char **argv)
{
    //display_status();
   
    // Set trap handler
    set_trap_fn(trap_h);
    enable_irqs();
  
    display_img(image_1080);
    while(1) {
	    printf("main loop\n");
    }
}


