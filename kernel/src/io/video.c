#include <video.h>

#ifdef ARCH_X86
#include <kernel/arch/x86/dev/vga/print.h>
#endif

/**
 * \brief Prints a single character.
 * Uses the current architecture's put function
 * @param c the input character
 */
void kput(char c)
{
#ifdef ARCH_X86
	vga_put((u8)c);
#endif
}

/**
 * \brief Prints a string of characters.
 * Uses the current architecture's print function
 * @param str the input string
 */
void kprint(char *str)
{
#ifdef ARCH_X86
	vga_print(str);
#endif
}

/**
 * \brief Prints a number using the specified base.
 * Prints a number using any base between 2 and 16, inclusive.
 * @param n number to be printed
 * @param base base to use when printing the number
 * @see kput
 */
void kprintnum(u32 num, int base)
{
	char nums[16] = "0123456789ABCDEF";
	char ans[16] = { '0' };
	int i = 0;
	while(num)
	{
		ans[i++] = nums[num % base];
		num /= base;
	}
	
	for(i--; i >= 0; i--)
		kput(ans[i]);
}