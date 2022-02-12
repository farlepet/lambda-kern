#include <lambda/export.h>
#include <video.h>
#include <string.h>
#include <mm/mm.h>
#include <proc/atomic.h>
#include <proc/mtask.h>

hal_io_char_dev_t *kput_char_dev = NULL;

/**
 * Prints a single character.
 * 
 * @param c the input character
 */
void kput(char c) {
    if(kput_char_dev) {
        hal_io_char_dev_putc(kput_char_dev, c);
    }
}

/**
 * Prints a single wide character.
 * 
 * @param c the input character
 */
void kwput(int c) {
    if(kput_char_dev) {
        hal_io_char_dev_putc(kput_char_dev, c);
    }
#if 0
    if(boot_options.output_serial) {
        serial_write((uint16_t)boot_options.output_serial, (char)c);
    }
    else vga_put((char)c);
#endif
}

/**
 * Prints a string of characters.
 * 
 * @param str the input string
 * @see kput
 */
void kprint(char *str) {
    while(*str) kput(*str++);
}

/**
 * Prints a string of wide characters.
 * 
 * @param str the input string
 * @see kwput
 */
void kwprint(uint16_t *str) {
    while(*str) kwput(*str++);
}



typedef unsigned long long arg_type_t;
typedef signed long long   sarg_type_t;

#if (PLATFORM_BITS < PLATFORM_BITS_64)
/**
 * @brief Divides two `arg_type_t` values, and stores the remainder. Used mainly
 * to allow use of 64-bit numbers on 32-bit platforms.
 * 
 * @note This is not an effecient method, so 64-bit numbers should be used
 * sparingly in debug statements on 32-bit platforms.
 * 
 * @param a Dividend
 * @param b Divsior
 * @return arg_type_t Result
 */
static arg_type_t _arg_udiv(arg_type_t a, arg_type_t b, arg_type_t *rem) {
    /* This Basic implementation is based off the simple 128-bit division
     * algorithm found here: https://danlark.org/2020/06/14/128-bit-division/ */

    if(a < 0x100000000ULL) {
        /* Do fast 32-bit arithmetic when possible */
        *rem = (uint32_t)a % (uint32_t)b;
        return (uint32_t)a / (uint32_t)b;
    }

    if(b > a) {
        *rem = a;
        return 0;
    }

    arg_type_t q = 0; /* Quotient */

    /* Get difference in position of most-significant bits between dividend and
     * divisor
     * NOTE: (63 - clz(a)) - (63 - clz(b)) == clz(b) - clz(a) */
    int shift = __builtin_clzll(b | 1) - __builtin_clzll(a | 1);

    b <<= shift;

    while(shift >= 0) {
        q <<= 1;
        if(a >= b) {
            a -= b;
            q |= 1;
        }
        b >>= 1;
        shift--;
    }

    *rem = a;
    return q;
}
#endif /* (PLATFORM_BITS < PLATFORM_BITS_64) */

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
static int _print_int(arg_type_t num, uint8_t base, uint8_t u, uint8_t pad, uint8_t padzero, uint8_t possign, uint8_t posspace, uint8_t _case, char *out) {
    sarg_type_t onum = (sarg_type_t)num;
    if(onum < 0 && !u) num = (~num) + 1;
    char *nums;
    if(_case) nums = "0123456789ABCDEF";
    else      nums = "0123456789abcdef";

    /* Maximum characters occupied by base 8 representation of 64-bit number:
     *   64 / log(8) = 23.333 */
    char ans[25] = { '0', 0, };
    int i = 0;
    while(num) {
#if (PLATFORM_BITS < PLATFORM_BITS_64)
        arg_type_t rem;
        num = _arg_udiv(num, base, &rem);
        ans[i++] = nums[rem];
#else
        ans[i++] = nums[num % base];
        num = num / base;
#endif
    }
    if(i == 0) i++;

    if(!u) {
        if(onum >= 0) {
            if(possign) {
                *out++ = '+';
            }
            else if(posspace) {
                *out++ = ' ';
            }
        }
        else {
            *out++ = '-';
        }
    }

    int p = pad - i;
    if(p > 0) {
        while(p--) *out++ = (padzero ? '0' : ' ');
        while(--i >= 0) *out++ = ans[i];
        return (int)((pad > strlen(ans)) ? pad : strlen(ans)) + ((((possign || posspace) && !u) && onum >= 0) || ((onum < 0) && !u));
    } else {
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
static int _get_dec(const char *str, ptr_t *out) {
    int n = 0;
    while(*str >= '0' && *str <= '9') {
        n *= 10;
        n += (int)(*str - '0');
        str++;
    }
    *out = (ptr_t)str;
    
    return n;
}

#define va_arg __builtin_va_arg // Helps clean up the code a bit
static arg_type_t _get_arg(__builtin_va_list *varg, int size) {
    arg_type_t arg = 0;

    switch(size) {
        case -2:
            arg = va_arg(*varg, int) & 0xFF;
            break;
        case -1:
            arg = va_arg(*varg, int) & 0xFFFF;
            break;
        case  0:
            arg = va_arg(*varg, int) & 0xFFFFFFFF;
            break;
        case  1:
            arg = va_arg(*varg, long);
            break;
        case  2:
            arg = va_arg(*varg, long long);
            break;
    }

    return arg;
}

/**
 * Takes a format string and a list of arguments as input, and produces a
 * string as output.
 * 
 * @param out the output string
 * @param format the format string
 * @param varg the list of arguments
 * @return the number of charactern placed in `out`
 */
static int print(char *out, const char *format, __builtin_va_list varg) {
    uint8_t is_in_spec = 0;
    int8_t  size = 0;      // Size of the integer
    uint32_t width = 0;     // Width of the number at minimum
    uint32_t precision = 0; // Precision
    uint8_t showsign = 0;  // Show the sign on positive numbers
    uint8_t signspace = 0; // Place a space before positive numbers
    uint8_t leftalign = 0; // Align to the left
    uint8_t padzeros = 0;  // Use zeros instead of spaces for padding
    
    int nchars = 0;    // Number of chars printed so far
    
    ptr_t      temp;
    arg_type_t arg;
    
    for(; *format != 0; format++) {
        if(!is_in_spec) {
            if(*format == FMT_SPEC) {
                is_in_spec = 1;
                continue;
            }
            *out++ = *format;
            nchars++;
            continue;
        }
        
        switch(*format) {
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
            case '9': width = (uint8_t)_get_dec(format, &temp);
                      format = (char *)(temp - 1);
                      break;
            
            case '.': format++;
                      precision = (uint8_t)_get_dec(format, &temp);
                      format = (char *)(temp - 1);
                      break;
            
            
        // Numbers, strings, etc...
            
            case 'u':
            case 'd':
            case 'i': arg = _get_arg(&varg, size);
                      temp = (ptr_t)_print_int(arg, 10, (*format != 'u'), width, padzeros, showsign, signspace, 0, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G': /* TODO: Floating point */
                      (void)va_arg(varg, double);
                      ZERO_ALL_VID();
                      break;
                      
            case 'x':
            case 'X': arg = _get_arg(&varg, size);
                      temp = (ptr_t)_print_int(arg, 16, 1, width, padzeros, showsign, signspace, (*format == 'X'), out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;
            
            case 'o': arg = _get_arg(&varg, size);
                      temp = (ptr_t)_print_int(arg, 8, 1, width, padzeros, showsign, signspace, 0, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
                      break;
                     
            case 's': if(!precision) precision = UINT_MAX;
                      if(size > 0) {
                          temp = (ptr_t)va_arg(varg, int16_t *);
                          if(temp == 0) { temp = (ptr_t)L"(null)"; }
                          else if(!mm_check_addr((void *)temp)) { temp = (ptr_t)L"(badaddr)"; }
                          nchars += wcslen((int16_t *)temp);
                          while(*(int16_t *)temp && precision--) *out++ = (char)*(int16_t *)temp++;
                      } else {
                          temp = (ptr_t)va_arg(varg, char *);
                          if(temp == 0) { temp = (ptr_t)"(null)"; }
                          else if(!mm_check_addr((void *)temp)) { temp = (ptr_t)"(badaddr)"; }
                          nchars += strlen((char *)temp);
                          while(*(char *)temp && precision--) *out++ = *(char *)temp++;
                      }
                      ZERO_ALL_VID();
                      break;
                    
            case 'c': arg = (arg_type_t)va_arg(varg, int);
                      if(size > 0) *out++ = (char)arg; /* TODO: wchar */
                      else         *out++ = (char)arg;
                      nchars++;
                      ZERO_ALL_VID();
                      break;
                    
            case 'p': if(!width) width = sizeof(void *) * 2;
                      arg = _get_arg(&varg, size);
                      temp = (ptr_t)_print_int(arg, 16, 1, width, 1, 0, 0, 1 /* Should this be upper or lower case? */, out);
                      nchars += temp;
                      out += temp;
                      ZERO_ALL_VID();
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
int sprintf(char *out, const char *format, ...) {
    __builtin_va_list varg;
    __builtin_va_start(varg, format);
    int ret = print(out, format, varg);
    __builtin_va_end(varg);
    return ret;
}
EXPORT_FUNC(sprintf);


static lock_t print_lock;

/**
 * Uses `print` to convert the format string and any number of arguments to
 * a string then prints that string to the screen.
 * 
 * @param format format string
 * @param ... argument list
 * @return the number of characters printed
 * @see print
 */
int kprintf(const char *format, ...) {
    __builtin_va_list varg;
    __builtin_va_start(varg, format);
    char temp[1024];
    int ret = print(temp, format, varg);

    if(mtask_get_curr_thread() &&
       interrupts_enabled()) lock_for(&print_lock, 100);
    kprint(temp);
    if(mtask_get_curr_thread() &&
       interrupts_enabled()) unlock(&print_lock);

    __builtin_va_end(varg);
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
int kprintv(const char *format, __builtin_va_list varg) {
    char temp[1024];
    int ret = print(temp, format, varg);

    if(mtask_get_curr_thread() &&
       interrupts_enabled()) lock_for(&print_lock, 100);
    kprint(temp);
    if(mtask_get_curr_thread() &&
       interrupts_enabled()) unlock(&print_lock);

    return ret;
}
