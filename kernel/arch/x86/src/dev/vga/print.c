#include <stddef.h>
#include <stdint.h>

#include <arch/dev/vga/print.h>

#include <err/error.h>


static uint8_t bkgc = 0x00; //!< Background color to use in vga_put
static uint8_t forc = 0x07; //!< Foreground color to use in vga_put

static int xres = 80; //!< The current width of the VGA screen
static int yres = 25; //!< The current hight of the VGA screen

static int xpos = 0; //!< X position of the cursor
static int ypos = 0; //!< Y position of the cursor

static uint8_t *vidmem = (uint8_t *)0xB8000; //!< Pointer to default VGA memory location


/**
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

static char buff[256];
static int is_esc = 0;   //!< Whether or not we are in an escape sequence
static int buff_loc = 0; //!< Where in the ANSI escape buffer we are
void ansi_escape(void);
/**
 * Checks if character is printable, if so it places it in VGA memory,
 * along with a color byte. If the character is not printable, it deals
 * with it accordingly
 * 
 * @param c the input character
 */
void vga_put(char c)
{
    if(is_esc != 0) {
        if(is_esc == 3) {
            if(c == '[') {
                is_esc = 1;
                return;
            } else {
                is_esc = 0;
            }
        } else {
            buff[buff_loc++] = c;
            if(is_esc == 2) {
                //if(c == ';') is_esc = 1;
            } else if(is_ansi(c)) {
                ansi_escape();
                return;
            } else {
                return;
            }
        }
    }
    
    switch(c) {
        case 0x00:  return;

        case '\t':  xpos = (xpos + 8) & ~(8);
                    break;

        case '\n':  xpos = 0;
                    ypos++;
                    break;

        case '\e':  is_esc = 3;
                    buff_loc = 0;
                    return;

        default:    *(vidmem + (xpos+ypos*xres)*2)     = (uint8_t)c;
                    *(vidmem + (xpos+ypos*xres)*2 + 1) = (uint8_t)((bkgc << 4) | forc);
                    xpos++;
                    break;
    }
    if(xpos >= xres) { xpos = 0; ypos++; }
    if(ypos >= yres) { ypos = yres - 1; scrollup(); }
}

/**
 * Prints every character in a character array , until it reaches
 * a NULL terminator.
 *
 * @param str the string to print
 * @see vga_put
 */
void vga_print(char *str)
{
    int i = 0;
    while(str[i] != 0)
        vga_put(str[i++]);
}

/**
 * Prints a number using any base between 2 and 16, inclusive.
 *
 * @param n number to be printed
 * @param base base to use when printing the number
 * @see vga_print
 */
void vga_printnum(uint32_t n, uint32_t base) {
    if(base == 0) {
        return;
    }

    const char nums[16] = "0123456789ABCDEF";
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










/**
 * Convert a string of base-10 digits into an integer.
 *
 * @param str string containing the digits
 * @param out optional pointer to end of digits, set by this function
 */
static int get_dec(char *str, char **out)
{
    int n = 0;
    while(*str >= '0' && *str <= '9')
    {
        n *= 10;
        n += (int)(*str - '0');
        str++;
    }
    if(out) *out = str;
    
    return n;
}


static uint8_t ansi_to_vga[16] = //!< Convert an ansi color to a VGA color
{
    0, 4,  2,  6,  1, 5,  3,  7,
    8, 12, 10, 14, 9, 13, 11, 15
};

/**
 * Handle a "\e[XXm" escape code
 */
static void m_escape()
{
    int n = get_dec(buff, NULL);

    switch(n)
    {
        case  0:    bkgc = 0;
                    forc = 7;
                    break;

        case  1:    forc |= 8;
                    break;

        case  2:    forc &= ~8;
                    break;

        case  3:    // En Italic
        case  4:    // En Underline
        case  5:    // En Blink
        case  6:    // En Fast Blink
                    break;

        case  7:    n = forc;
                    forc = bkgc;
                    bkgc = (uint8_t)n;
                    break;

        case  8:    // En Hidden
        case  9:    // En Strike-through
        case 20:    // En Fraktur
                    break;

        case 21:    forc &= ~8;
                    break;

        case 22:    bkgc = 0;
                    forc = 7;
                    break;

        case 23:    // Dis Italic & Fraktur
        case 24:    // Dis Underline
        case 25:    // Dis Blink
        case 26:    // Reserved
                    break;

        case 27:    n    = forc;
                    forc = bkgc;
                    bkgc = n;
                    break;

        case 28:    // Dis Hide
        case 29:    // Dis Strike-through
                    break;

        case 39:    forc = 7;
                    break;

        case 49:    bkgc = 0;
                    break;

        default:    if(n >= 30  && n <= 37)  forc = ansi_to_vga[n - 30];
                    if(n >= 40  && n <= 47)  bkgc = ansi_to_vga[n - 40];
                    if(n >= 90  && n <= 97)  forc = ansi_to_vga[n - 90  + 8];
                    if(n >= 100 && n <= 107) bkgc = ansi_to_vga[n - 100 + 8];
                    break;
    }
}

/**
 * Handles an ANSI escape code;
 */
void ansi_escape()
{
    is_esc = 2;
    switch(buff[buff_loc - 1])
    {
        case 'm':   m_escape();
                    break;
    }
}
