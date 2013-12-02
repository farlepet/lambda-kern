#include <types.h>

static u8 bkgc = 0x00;
static u8 forc = 0x07;

int xres = 80;
int yres = 25;

static int xpos = 0;
static int ypos = 0;

static u8 *vidmem = (u8 *)0xB8000;

void vga_clear()
{
	int i = 0;
	for(; i < xres*yres*2; i++)
		*(vidmem + i) = 0x00;
}

static void scrollup()
{
	int i = 0;
	for(; i < (xres * (yres - 1) * 2); i++)
		vidmem[i] = vidmem[i + xres*2];
	
	for(; i < (xres * (yres) * 2); i++)
		vidmem[i] = 0x00;
}

void vga_put(char c)
{
	switch(c)
	{
		case 0x00:
			return;

		case '\t':
			xpos = (xpos + 8) & ~(8);
			break;

		case '\n':
			xpos = 0;
			ypos++;
			break;

		default:
			*(vidmem + (xpos+ypos*xres)*2)     = (u8)c;
			*(vidmem + (xpos+ypos*xres)*2 + 1) = ((bkgc << 4) | forc);
			xpos++;
			break;
	}
	if(xpos >= xres) { xpos = 0; ypos++; }
	if(ypos >= yres) { ypos = yres - 1; scrollup(); }
}

void vga_print(char *str)
{
	int i = 0;
	while(str[i] != 0)
		vga_put((u8)str[i++]);
}

void vga_printnum(u32 n, int base)
{
	char nums[16] = "0123456789ABCDEF";
	char ans[16] = { '0' };
	int i = 0;
	while(n)
	{
		ans[i++] = nums[n % base];
		n /= base;
	}

	for(i--; i >= 0; i--)
		vga_put(ans[i]);
}