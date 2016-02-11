#include <stdio.h>
#include <stdlib.h>

int __attribute__((noinline)) static_f2c(int i) 
{
  return (i-32)*5/9;
}

int __attribute__((noinline)) static_c2f(int i)
{
  return (i*9/5)+32;
}
