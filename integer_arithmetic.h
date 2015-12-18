#include <inttypes.h>
#define NWORDS 7
#define WSIZE 64
#define is_even(a) (!(a[0]&1))
#define is_negative(a, size) (a[size-1]&(1LL<<63))
#define get_bit(num,r) ((num[r/64]&(1LL<<(r%64))))

#define new_int(n, size) uint64_t n[size]; zero (n, size);

void init_int_lib (int size);
// Para todas las funciones size es la cantidad de palabras para las que
// debe correr.
void int_print (uint64_t * num, int size);
void int_raw_print (uint64_t * num, int size);

void int_rand (uint64_t * num, int size);
void int_positive_rand (uint64_t * num, int size);

void int_from_string (char *str, uint64_t *num, int size);
void zero (uint64_t * num, int size);
void one (uint64_t * num, int size);

void int_add (uint64_t *a, uint64_t *b, uint64_t *res, int size);
int uint_add (uint64_t *a, uint64_t *b, uint64_t *res, int size);
void int_increment (uint64_t *a, int size);
void int_decrement (uint64_t *a, int size);

void twos_complement (uint64_t *a, uint64_t *res, int size);
void int_subtract (uint64_t *a, uint64_t *b, uint64_t *res, int size);

void int_multiply (uint64_t *a, uint64_t *b, uint64_t *res, int size);
void uint_divide (uint64_t *a, uint64_t *b, uint64_t *q, uint64_t *r, int size);
void rshift_by_one (uint64_t *a, uint64_t *res, int size);
void lshift_by_one (uint64_t *a, uint64_t *res, int size);

void copy_num (uint64_t *a, uint64_t *b, int size);
void swap (uint64_t *a, uint64_t *b, int size);

int lt (uint64_t *a, uint64_t *b, int size);
int leq (uint64_t *a, uint64_t *b, int size);
int neq (uint64_t *a, uint64_t *b, int size);

int is_zero (uint64_t *num, int size);
int is_one (uint64_t *num);

void extended_binary (uint64_t *a, uint64_t *b, uint64_t *d, uint64_t *s, uint64_t *t);

