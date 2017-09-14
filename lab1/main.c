/* You shold implement 'maxargs' and 'interp' in myimpl.c.
 * Please don't modify this file.
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "prog1.h"

int maxargs(A_stm stm);
void interp(A_stm stm);

int main(int argc, char **argv)
{
	int args;
	int test_num;

	assert(argc == 2);
	test_num = atoi(argv[1]);

	switch(test_num)
	{
		case 0:
			printf("prog\n");
			args = maxargs(prog());
			printf("args: %d\n",args);
			interp(prog());

			printf("prog_prog\n");
			args = maxargs(prog_prog());
			printf("args: %d\n",args);
			interp(prog_prog());
			break;
		case 1: 
			printf("prog_prog\n");
			args = maxargs(prog_prog());
			printf("args: %d\n",args);
			interp(prog_prog());

			printf("prog\n");
			args = maxargs(prog());
			printf("args: %d\n",args);
			interp(prog());
			break;
		default:
			printf("unexpected case\n");
			exit(-1);
	}
	printf("right_prog\n");
	args = maxargs(right_prog());
	printf("args: %d\n",args);
	interp(right_prog());

	return 0;
}
