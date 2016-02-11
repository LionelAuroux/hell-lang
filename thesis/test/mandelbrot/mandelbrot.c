/*

	Bonne valeur d'utilisation:

	./a.out 0.0 0.0 3 3 700 700 > mandel.ppm

*/


#include <stdio.h>
#include <stdlib.h>

typedef double tReal;
 
#include "mandelbrot-common.h"
#include "mandelbrot-palette.h"

int isInSet(tComplex C, int limit, tReal moduleMax)
{
  int i;
  tReal mod;
#ifdef WITHCOMPLEXSUPPORT
  tComplex Z = 0.0fi + 0.0f;
#else
  tComplex Z = {0.0f, 0.0f};
#endif
  for (i = 0; i < limit; i++)
    {
      Z = cAdd (cMul (Z, Z), C);
      mod = module (Z);
      if (moduleMax < mod) break;
    }
  return i;
} /* isInSet */

int main(int argc, char * argv[])
{
  tComplex Center, C1, C2;
  tReal reInc, imInc, reSize, imSize;
  int IMGXSIZE, IMGYSIZE, i, j, k;
  int ITERLIMIT = 256;
  if (argc <= 6)
    {
      fprintf(stderr, "%s [Cr] i[Ci] re[Size] i[Size] imgXsize imgYsize\n", argv[0]);
      fprintf(stderr, "C = Center, B = bottom rigth\n");
      exit(0);
    }
  cSet(&Center, atof (argv[1]), atof (argv[2]));
  reSize     = atof (argv[3]);  imSize   = atof (argv[4]);
  IMGXSIZE   = atoi (argv[5]);
  IMGYSIZE   = atoi (argv[6]);
#ifdef WITHCOMPLEXSUPPORT
  fprintf(stderr, "Mandelbrot set (complex support): ");  pComplex(Center, stderr); 
#else 
  fprintf(stderr, "Mandelbrot set (real support): ");  pComplex(Center, stderr); 
#endif
  fprintf(stderr, "Size     : %2.2f %2.2f ", reSize, imSize);
  fprintf(stderr, "Img Size : %dx%d\n",IMGXSIZE, IMGYSIZE);

  reInc = reSize / IMGXSIZE;
  imInc = imSize / IMGYSIZE;
  cSet(&C1, RE(Center) - (reSize / 2.0f), IMAG(Center) - (imSize / 2.0f));
  printf("P3\n");  
  printf("%d %d\n", IMGXSIZE, IMGYSIZE);  
  printf("%d\n", 256);  
  for (i = 0; i < IMGYSIZE; i++)
    {
      C2 = C1;
      for (j = 0; j < IMGXSIZE; ++j)
	{
	  k = COLORMAPSIZE*isInSet(C2, ITERLIMIT, 4.0f)/ITERLIMIT;
	  printf("%d %d %d\n", thePalette[k].r, thePalette[k].v, thePalette[k].b);
  	  RE(C2) += reInc;
	}
      IMAG(C1) += imInc;
      printf("\n");
    }
  return 0;
}
