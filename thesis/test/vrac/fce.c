// floating constant evaluation
//#include <stdio.h>

const double	f = 2.4;

int	main(int ac, char **av, char **ae)
{
  printf("%f\n", f);
  printf("recu %d param\n", ac);
  for (int i = 0; av[i]; ++i)
    printf("arg %d:%s\n", i, av[i]);
  for (int i = 0; ae[i]; ++i)
    printf("env %d:%s\n", i, ae[i]);
}
