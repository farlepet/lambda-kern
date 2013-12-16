#include <types.h>

static u8 bkgc = 0x00; //!< Background color to use in vga_put
static u8 forc = 0x07; //!< Foreground color to use in vga_put

int xres = 80; //!< The current width of the VGA screen
int yres = 25; //!< The current hight of the VGA screen

static int xpos = 0; //!< X position of the cursor
static int ypos = 0; //!< Y position of the cursor

static u8 *vidmem = (u8 *)0xB8000; //!< Pointer to default VGA memory location

/**
 * \brief Clears VGA text.
 * Clears the first plane of VGA memory, effectively clearing all text from
 * the screen.
 */
void vga_clear()
{
	int i = 0;
	for(; i < xres*yres*2; i++)
		*(vidmem + i) = 0x00;
}

/**
 * \brief Moves all VGA text up a line.
 * Copies all text after the first line to the previous line, then clears
 * the final line.
 */
static void scrollup()
{
	int i = 0;
	for(; i < (xres * (yres - 1) * 2); i++)
		vidmem[i] = vidmem[i + xres*2];
	
	for(; i < (xres * (yres) * 2); i++)
		vidmem[i] = 0x00;
}

/**
 * \brief Prints a single character to the VGA screen.
 * Checks if character is printable, if so it places it in VGA memory,
 * along with a color byte. If the character is not printable, it deals
 * with it accordingly
 * @param c the input character
 */
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

/**
 * \brief Prints a string of characters.
 * Prints every character in a character array , until it reaches
 * a NULL terminator.
 * @param str the string to print
 * @see vga_put
 */
void vga_print(char *str)
{
	int i = 0;
	while(str[i] != 0)
		vga_put((u8)str[i++]);
}

/**
 * \brief Prints a number using the specified base.
 * Prints a number using any base between 2 and 16, inclusive.
 * @param n number to be printed
 * @param base base to use when printing the number
 * @see vga_print
 */
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
	
	if(!i) i = 1;

	for(i--; i >= 0; i--)
		vga_put(ans[i]);
}