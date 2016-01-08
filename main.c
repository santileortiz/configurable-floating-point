#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "integer_arithmetic.h"
#include "floating_point.h"

void set_E_T (char S, uint64_t *E, uint64_t *T, arbitrary_float *f)
{
    f -> S = S;
    f -> E = E;
    f -> T = T;
}

int main (void) {
    arbitrary_float a, b, ans;
    char * str;

    init_fp_lib (70, 127);

    convert_from_decimal_character_sequence ("2.34243", &a);
    convert_from_decimal_character_sequence ("-9.897345", &b);

    printf ("\na = ");
    float_print (a);
    printf ("b = ");
    float_print (b);

    init_fp_number (&ans);
    printf ("\na+b = ");
    add (a, b, &ans);
    float_print (ans);

    printf ("\na-b = ");
    subtract (a, b, &ans);
    float_print (ans);

    printf ("\nb-a = ");
    subtract (b, a, &ans);
    float_print (ans);

    printf ("\na*b = ");
    multiply (a, b, &ans);
    float_print (ans);

    printf ("\na/b = ");
    divide (a, b, &ans);
    float_print (ans);

    printf ("\nb/a = ");
    divide (b, a, &ans);
    float_print (ans);

    free_all_fp_lib ();
}

