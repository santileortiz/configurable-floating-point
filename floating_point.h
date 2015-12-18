#include <stdint.h>

typedef struct _arbitraty_float {
    char S;
    uint64_t * E;
    uint64_t * T;
} arbitrary_float;

void init_fp_lib (int p_new, int e);
void init_fp_number (arbitrary_float *f);
void fp_zero (arbitrary_float *f);
void fp_one (arbitrary_float *f);
void convert_from_decimal_character_sequence (char *str, arbitrary_float *f);
void int_to_float (uint64_t *integer, int size, arbitrary_float *f);

void float_print (arbitrary_float a);
void float_raw_print (char *name, arbitrary_float f);
void float_raw_print_m_e (char *name, arbitrary_float f);

int fp_abs_lt (arbitrary_float a, arbitrary_float b);
int fp_abs_eq (arbitrary_float a, arbitrary_float b);
void add (arbitrary_float a, arbitrary_float b, arbitrary_float *ans);
void subtract (arbitrary_float a, arbitrary_float b, arbitrary_float *ans);
void multiply (arbitrary_float a, arbitrary_float b, arbitrary_float *ans);
void divide (arbitrary_float a, arbitrary_float b, arbitrary_float *ans);
void free_all_fp_lib ();

