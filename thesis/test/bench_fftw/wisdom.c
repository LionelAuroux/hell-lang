#include <stdio.h>
#include <fftw3.h>

#define N 64

int     main()
{
     fftw_plan  plan;
     char       *wisdom_string;
     FILE       *input_file;

     /* wisdom file */
     input_file = fopen("sample.wisdom", "r");
     if (input_file && !fftw_import_wisdom_from_file(input_file))
          printf("Error reading wisdom!\n");
     fclose(input_file); 

        // fftw_real  -> complex
     fftw_complex       *in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
     fftw_complex       *out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

     /* create a plan for N possibly creating and/or using wisdom */
     plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_MEASURE);
     
     printf("Execute plan\n");
     fftw_execute(plan);

     /* always destroy plans when you are done */
     fftw_destroy_plan(plan);
     fftw_free(in);
     fftw_free(out);
     /* write the wisdom to a string */
     
     wisdom_string = fftw_export_wisdom_to_string();
     if (wisdom_string != NULL)
     {
          printf("Accumulated wisdom:\n %s",wisdom_string);
          FILE  *output_file = fopen("sample.wisdom", "w");
          fftw_export_wisdom_to_file(output_file);
          fclose(output_file);
          fftw_free(wisdom_string);
     }
}
