#include <video.h>
#include <string.h>
#include <mm/mm.h>
#include <multiboot.h>
#include <proc/atomic.h>

#if defined(ARCH_X86)
#include <dev/vga/print.h>
#include <io/serial.h>
#endif

int output_serial = 0; //!< If 0, write to VGA, else, write to serial port pointed to by `output_serial`

/**
 * Prints a single character.
 * 
 * @param c the input character
 */
void kput(char c)
{
#if defined(ARCH_X86)
	if(output_serial) {
		serial_write((uint16_t)output_serial, c);
	}
	else vga_put(c);
#endif
}

/**
 * Prints a single wide character.
 * 
 * @param c the input character
 */
void kwput(int c)
{
#if defined(ARCH_X86)
	if(output_serial) serial_write((uint16_t)output_serial, (char)c);
	else vga_put((char)c);
#endif
}

/**
 * Prints a string of characters.
 * 
 * @param str the input string
 * @see kput
 */
void kprint(char *str)
{
	while(*str) kput(*str++);
}

/**
 * Prints a string of wide characters.
 * 
 * @param str the input string
 * @see kwput
 */
void kwprint(uint16_t *str)
{
	while(*str) kwput(*str++);
}




/**
 * Converts an interger into a string and places the output into `out`
 * Used by `print` to make the code cleaner.
 * 
 * @param num the number to print
 * @param base the base in which to print the number
 * @param u wether the number is unsigned
 * @param pad how many characters of padding to give the number
 * @param padzero wether or not to pad the number with zeros instead of spaces
 * @param possign wether or not to place a positive sign in front of positive numbers
 * @param posspace wether or not to place a space in front of positive numbers
 * @param _case upper or lowercase (for bases above 10)
 * @param out the output string/buffer
 * @return the number of characters that have been put in `out`
 * @see print
 */
static int print_int(uint32_t num, uint8_t base, uint8_t u, uint8_t pad, uint8_t padzero, uint8_t possign, uint8_t posspace, uint8_t _case, char *out)
{
	int onum = (int)num;
	if(onum < 0) if(!u) num = (~num) + 1;
	char *nums;
	if(_case) nums = "0123456789ABCDEF";
	else      nums = "0123456789abcdef";
	
	char ans[16] = { '0', 0, };
	int i = 0;
	while(num)
	{
		ans[i++] = nums[num % base];
		num /= base;
	}
	if(i == 0) i++;
	
	if(!u)
	{
		if(onum >= 0)
		{
			if(possign)
			{
				*out++ = '+';
			}
			else if(posspace)
				*out++ = ' ';
		}
		else if(onum < 0)
			*out++ = '-';
	}
	
	int p = pad - i;
	if(p > 0)
	{
		while(p--) *out++ = (padzero ? '0' : ' ');
		while(--i >= 0) *out++ = ans[i];
		return (int)((pad > strlen(ans)) ? pad : strlen(ans)) + ((((possign || posspace) && !u) && onum >= 0) || ((onum < 0) && !u));
	}
	else
	{
		while(--i >= 0) *out++ = ans[i];
		return (int)(strlen(ans) + ((((possign || posspace) && !u) && onum >= 0) || ((onum < 0) && !u)));
	}
}


/**
 * Convert a number in a string into an integer. Used by `print` to make code cleaner.
 * 
 * @param str input string
 * @param out pointer to end of number (generated by this function)
 * @return the number found
 * @see print
 */
static int get_dec(const char *str, ptr_t *out)
{
	int n = 0;
	while(*str >= '0' && *str <= '9')
	{
		n *= 10;
		n += (int)(*str - '0');
		str++;
	}
	*out = (ptr_t)str;
	
	return n;
}


#define va_arg __builtin_va_arg // Helps clean up the code a bit
/**
 * Takes a format string and a list of arguments as input, and produces a
 * string as output.
 * 
 * @param out the output string
 * @param format the format string
 * @param varg the list of arguments
 * @return the number of charactern placed in `out`
 */
static int print(char *out, const char *format, __builtin_va_list varg)
{
	uint8_t is_in_spec = 0;
	s8 size = 0;      // Size of the integer
	uint8_t width = 0;     // Width of the number at minimum
	uint8_t precision = 0; // Precision
	uint8_t showsign = 0;  // Show the sign on positive numbers
	uint8_t signspace = 0; // Place a space before positive numbers
	uint8_t leftalign = 0; // Align to the left
	uint8_t padzeros = 0;  // Use zeros instead of spaces for padding
	
	int nchars = 0;    // Number of chars printed so far
	
	ptr_t temp;
	
	for(; *format != 0; format++)
	{
		if(!is_in_spec)
		{
			if(*format == FMT_SPEC)
			{
				is_in_spec = 1;
				continue;
			}
			*out++ = *format;
			nchars++;
			continue;
		}
		
		switch(*format)
		{
			case FMT_SPEC: is_in_spec = 0;
						   *out++ = FMT_SPEC;
						   nchars++;
						   break;
			
			case 'l': if(size < 2) size++;
					  break;
		
			case 'L': size = 1;
					  break;
			
			case 'h': if(size > -2) size--;
					  break;
			
			case 'z':
			case 'j':
			case 't': size = 0;
					  break;
		
			case '+': showsign = 1;
					  break;
			
			case ' ': signspace = 1;
					  break;
			
			case '-': leftalign = 1;
					  break;
		
			case '0': padzeros = 1;
					  break;
			
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': width = (uint8_t)get_dec(format, &temp);
					  format = (char *)(temp - 1);
					  break;
			
			case '.': format++;
					  precision = (uint8_t)get_dec(format, &temp);
					  format = (char *)(temp - 1);
					  break;
			
			
		// Numbers, strings, etc...
			
			case 'd':
			case 'i': temp = (ptr_t)va_arg(varg, int);
					  if(size == -1) temp &= 0xFFFF;
					  if(size == -2) temp &= 0xFF;
					  if(size == 2){} // TODO: Handle this!
					  temp = (ptr_t)print_int(temp, 10, 0, width, padzeros, showsign, signspace, 0, out);
					  nchars += temp;
					  out += temp;
					  ZERO_ALL_VID();
					  break;
					  
			case 'u': temp = (ptr_t)va_arg(varg, uint32_t);
					  if(size == -1) temp &= 0xFFFF;
					  if(size == -2) temp &= 0xFF;
					  if(size == 2){} // TODO: Handle this!
					  temp = (ptr_t)print_int(temp, 10, 1, width, padzeros, showsign, signspace, 0, out);
					  nchars += temp;
					  out += temp;
					  ZERO_ALL_VID();
					  break;
			
			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'g':
			case 'G': (void)va_arg(varg, double);
					  ZERO_ALL_VID();
					  break;
					  
			case 'x':
			case 'X': temp = (ptr_t)va_arg(varg, uint32_t);
					  if(size == -1) temp &= 0xFFFF;
					  if(size == -2) temp &= 0xFF;
					  if(size == 2){} // TODO: Handle this!
					  temp = (ptr_t)print_int(temp, 16, 1, width, padzeros, showsign, signspace, (*format == 'X'), out);
					  nchars += temp;
					  out += temp;
					  ZERO_ALL_VID();
					  break;
			
			case 'o': temp = (ptr_t)va_arg(varg, uint32_t);
					  if(size == -1) temp &= 0xFFFF;
					  if(size == -2) temp &= 0xFF;
					  if(size == 2){} // TODO: Handle this!
					  temp = (ptr_t)print_int(temp, 8, 1, width, padzeros, showsign, signspace, 0, out);
					  nchars += temp;
					  out += temp;
					  ZERO_ALL_VID();
					  break;
					 
			case 's': if(size > 0)
					  {
						  temp = (ptr_t)va_arg(varg, s16 *);
						  if(temp == 0) { temp = (ptr_t)L"(null)"; }
						  else if(!mm_check_addr((void *)temp)) { temp = (ptr_t)L"(badaddr)"; }
						  nchars += wcslen((s16 *)temp);
						  while(*(s16 *)temp) *out++ = (char)*(s16 *)temp++;
					  }
					  else
					  {
						  temp = (ptr_t)va_arg(varg, char *);
						  if(temp == 0) { temp = (ptr_t)"(null)"; }
						  else if(!mm_check_addr((void *)temp)) { temp = (ptr_t)"(badaddr)"; }
						  nchars += strlen((char *)temp);
						  while(*(char *)temp) *out++ = *(char *)temp++;
					  }
					  ZERO_ALL_VID();
					  break;
					
			case 'c': temp = (ptr_t)va_arg(varg, int);
					  if(size > 0) *out++ = (char)temp;
					  else         *out++ = (char)temp;
					  nchars++;
					  ZERO_ALL_VID();
					  break;
					
			case 'p': temp = (ptr_t)va_arg(varg, ptr_t);
					  temp = (ptr_t)print_int(temp, 16, 1, width, padzeros, showsign, signspace, 1 /* Should this be upper or lower case? */, out);
					  nchars += temp;
					  out += temp;
					  break;
					
			case 'a':
			case 'A': (void)va_arg(varg, double);
					  ZERO_ALL_VID();
					  break;
					
			case 'n': (void)va_arg(varg, int);
					  ZERO_ALL_VID();
					  break;
		}
		
	}
	
	*(out) = 0;
	
	// These aren't used yet:
	(void)precision;
	(void)leftalign;
	
	return nchars;
}

/**
 * Uses `print` to convert the format string and any number of arguments to
 * an output string.
 * 
 * @param out output string
 * @param format format string
 * @param ... argument list
 * @return the number of characters placed in `out`
 * @see print
 */
int sprintf(char *out, const char *format, ...)
{
	__builtin_va_list varg;
	__builtin_va_start(varg, format);
	int ret = print(out, format, varg);
	__builtin_va_end(varg);
	return ret;
}


lock_t print_lock;

/**
 * Uses `print` to convert the format string and any number of arguments to
 * a string then prints that string to the screen.
 * 
 * @param format format string
 * @param ... argument list
 * @return the number of characters printed
 * @see print
 */
int kprintf(const char *format, ...)
{
	if(interrupts_enabled()) lock_for(&print_lock, 16);
	__builtin_va_list varg;
	__builtin_va_start(varg, format);
	char temp[1024];
	int i = 0;
	while(i < 1024) temp[i++] = ' ';
	int ret = print(temp, format, varg);
	kprint(temp);
	__builtin_va_end(varg);
	if(interrupts_enabled()) unlock(&print_lock);
	return ret;
}

/**
 * Uses `print` to convert the format string and any number of arguments in varg to
 * a string then prints that string to the screen.
 * 
 * @param format format string
 * @param varg argument list
 * @return the number of characters printed
 * @see print
 */
int kprintv(char *format, __builtin_va_list varg)
{
	if(interrupts_enabled()) lock_for(&print_lock, 16);
	char temp[1024];
	int i = 0;
	while(i < 1024) temp[i++] = ' ';
	int ret = print(temp, format, varg);
	kprint(temp);
	if(interrupts_enabled()) unlock(&print_lock);
	return ret;
}
