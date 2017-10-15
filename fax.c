#include <stdio.h>

main(int argc, char** argv)
{
   int i;
   printf("there are %d arguments\n",argc);
   for(i = 0; i < argc; i++)
   {
       printf("%s\n",argv[i]);
   }
}
