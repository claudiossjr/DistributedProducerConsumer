#include <stdio.h>
#include <stdlib.h>

void Wait(int seconds)
{
  long time = 1000000000 * seconds, i;
  for (i = 0; i < time; i ++)
  {

  }
}

int main(int argc, char ** argv)
{
  printf("%s\n","Initial" );
  Wait(3);
  printf("%s\n","End" );
  return 0;
}
