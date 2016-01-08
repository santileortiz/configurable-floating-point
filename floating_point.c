#include "integer_arithmetic.h"
#include "floating_point.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Esta es la cantidad maxima de numeros que podemos manejar
#define MAX_USER_FP_NUMBERS 20 
#define MAX_LOCAL_FP_NUMBERS 20 
// Este arreglo contiene todos los numeros de punto flotante que se han creado
arbitrary_float *user_fp_numbers [MAX_USER_FP_NUMBERS];int next_user_number=0;
arbitrary_float *local_fp_numbers [MAX_LOCAL_FP_NUMBERS];int next_local_number=0;

// Numero de bits de precision
int p; 

// Valor maximo y minimo del exponente
uint64_t *emax, *emin, *bias; 

// Cantidad de palabras de 64 bits de E y T
int T_size, E_size; 


void init_fp_lib (int init_p, int init_w)
{
    int max_value, i;
    uint64_t *temp, *temp_1;

    p = init_p;

    T_size = 1;
    while (init_p >= T_size*64) {
        T_size++;
    }

    E_size = 1;
    while (init_w >= E_size*64) {
        E_size++;
    }

    // emax = 2^(w-1)-1
    temp = malloc (sizeof(uint64_t)*E_size);
    temp_1 = malloc (sizeof(uint64_t)*E_size);
    emax = malloc (sizeof(uint64_t)*E_size);

    one (emax, E_size);
    for (i=0; i< init_w-1; i++) {
        lshift_by_one (emax, temp, E_size);
        copy_num (temp, emax, E_size);
    }
    one (temp_1, E_size);
    int_subtract (emax, temp_1, temp, E_size);
    copy_num (temp, emax, E_size);

    // emin = 1-emax
    emin = malloc (sizeof(uint64_t)*E_size);
    int_subtract (temp_1, emax, temp, E_size);
    copy_num (temp, emin, E_size);

    // bias = emax
    bias = malloc (sizeof (uint64_t)*E_size);
    copy_num (emax, bias, E_size);

    free (temp);
    free (temp_1);
}

void free_fp_array (arbitrary_float **arr, int num)
{
    for (int i=0; i<num; i++) {
        free(arr [i] -> E);
        free(arr [i] -> T);
    }
}

void free_all_locals ()
{
    free_fp_array (local_fp_numbers, next_local_number);
    for (int i=0; i<next_user_number; i++) {
        free (local_fp_numbers[i]);
    }

    next_local_number = 0;
}

void free_all_fp_lib ()
{
    int i;
    free (emax);
    free (emin);
    free (bias);

    free_fp_array (user_fp_numbers, next_user_number);
    next_user_number = 0;
}

// Esto inicializa un numero flotante en el arreglo e incrementa num en 1 si
// este pointer no estaba declarado antes.
void init_fp_number_in_array(arbitrary_float *f, arbitrary_float **arr, int *num)
{
    int i, is_f_in_fp_numbers=0;
    for (i=0; i<*num; i++) {
        if (arr[i] == f) {
            is_f_in_fp_numbers = 1;
        }
    }

    if (!is_f_in_fp_numbers) {
        arr [*num] = f;
        *num = *num+1;
        f -> E = malloc (sizeof(uint64_t)*E_size);
        f -> T = malloc (sizeof(uint64_t)*T_size);
    }

    copy_num (bias, f->E, E_size);
    zero (f->T, T_size);
    f->S = 0;
}

void init_fp_number (arbitrary_float *f) 
{
    init_fp_number_in_array (f, user_fp_numbers, &next_user_number);
}

arbitrary_float* local_init_fp_number () 
{
    arbitrary_float *f = malloc (sizeof(arbitrary_float));
    init_fp_number_in_array (f, local_fp_numbers, &next_local_number);
    return f;
}

int count_non_zero_bits (uint64_t *a, int size) {
    int res = size*64, i;
    uint64_t temp;
    for  (i=size-1; !a[i]; i--) {
        res -= 64;
    }
    temp = a[i]>>1;
    for (i=1; temp; i++){
        temp = temp>>1;
    }
    res -= 64-i;
    return res;
}

// Convierte un entero con _size_ palabras de 64 bits a float
// Nota: f ya debe estar inicializado, de lo contrario falla
void int_to_float (uint64_t *integer, int size, arbitrary_float *f)
{
    uint64_t i, temp, length_in_bits = size*64;
    new_int (l_int, size);
    new_int (l_temp, size);

    new_int (temp_1, E_size);
    new_int (temp_e, E_size);

    if (is_negative(integer, size)) {
        twos_complement (integer, l_int, size);
        f->S = 1;
    } else {
        f->S = 0;
        copy_num (integer, l_int, size);
    }

    length_in_bits = count_non_zero_bits (l_int, size);
    // el exponente es length_in_bits-1, este procedimiento falla si
    // la cantidad de bits es mayor al maximo representable por un uint64
    zero (temp_e, E_size);
    temp_e[0] = length_in_bits-1;
    int_add (f->E, temp_e, temp_1, E_size);
    copy_num (temp_1, f->E, E_size);

    one (temp_1, E_size);
    if (length_in_bits>p) {
        //shift a la derecha
        for (i=length_in_bits-p; i>0; i--) {
            rshift_by_one (l_int, l_temp, size);
            copy_num (l_temp, l_int, size);
        }
    }
    else if (length_in_bits<p) {
        // shift a la izquierda
        for (i=p-length_in_bits; i>0; i--) {
            lshift_by_one (l_int, l_temp, size);
            copy_num (l_temp, l_int, size);
        }
    }

    copy_num (l_int, f->T, E_size);
}

void float_raw_print (char *name, arbitrary_float f) {
    printf ("%s_E: ", name); int_raw_print (f.E, E_size); printf ("\n");
    if (f.S) {
        printf ("%s_T: -", name); int_raw_print (f.T, T_size); printf ("\n");
    } else {
        printf ("%s_T: ", name); int_raw_print (f.T, T_size); printf ("\n");
    }
}

void float_raw_print_m_e (char *name, arbitrary_float f) {
    uint64_t l_E[E_size];
    uint64_t l_T[T_size];
    uint64_t tmp[T_size];

    zero (l_E, E_size);
    int_subtract (f.E, bias, l_E, E_size);

    copy_num (f.T, l_T, T_size);
    while (!is_negative(l_T, T_size)) {
        lshift_by_one(l_T, tmp, T_size);
        copy_num (tmp, l_T, T_size);
    }

    printf ("\n%s_e: ", name); int_print (l_E, E_size); printf ("\n");
    if (f.S) {
        printf ("%s_M: -", name); int_raw_print (l_T, T_size); printf ("\n");
    } else {
        printf ("%s_M: ", name); int_raw_print (l_T, T_size); printf ("\n");
    }
}

void fp_copy (arbitrary_float a, arbitrary_float *b) {
    b->S = a.S;
    copy_num (a.T, b->T, T_size);
    copy_num (a.E, b->E, E_size);
}

// Esta funcion regresa la parte entera de a siempre y cuando quepa en un numero
// de 64 bits. Esto es mas que suficiente para la funcion float_print()
uint64_t get_int_10 (arbitrary_float a) {
    uint64_t a_T [T_size];
    uint64_t tmp1 [T_size];
    uint64_t tmp2 [T_size];
    uint64_t e [E_size];
    int i;

    zero (a_T, T_size);
    copy_num (a.T, a_T, T_size);
    int_subtract (a.E, bias, e, E_size);
    if ((p%64)>e[0]+1) {
        for (i=p%64-e[0]-1; i; i--) {
            rshift_by_one(a_T, tmp2, T_size);
            copy_num (tmp2, a_T, T_size);
        }
    } else {
        // NOTE: Creo que este caso nunca sucede, revisar
        for (i=e[0]+1-(p%64); i; i--) {
            lshift_by_one(a_T, tmp2, T_size);
            copy_num (tmp2, a_T, T_size);
        }
    }

    // return a_T[ceilling(p/64-1)] y hace 0 esta posicion
    if (p%64) {
        return a_T[p/64];
    } else {
        return a_T[p/64-1];
    }
}

// Nota: Esta funcion no es general, solo saca la parte fraccionaria si la
// cantidad de numeros en la parte entera es menor a 2^64. Esto es mas que
// suficiente para imprimir el numero.
void fp_get_fractional_part (arbitrary_float a, arbitrary_float *ans) {
    uint64_t a_T [T_size];
    uint64_t tmp1 [T_size];
    uint64_t tmp2 [T_size];
    uint64_t e [E_size];
    uint64_t res=0;
    int i;
    uint64_t mask=0;

    zero (a_T, T_size);
    copy_num (a.T, a_T, T_size);
    int_subtract (a.E, bias, e, E_size);
    for (i=e[0]+1; i; i--) {
        lshift_by_one(a_T, tmp2, T_size);
        copy_num (tmp2, a_T, T_size);
    }

    mask = (1LL<<(p%64))-1;
    a_T[p/64] &= mask;
    copy_num (a_T, ans->T, T_size);

    e[0]++;
    int_subtract (a.E, e, ans->E, E_size);
}

void float_print (arbitrary_float a) {
    int k=0, i=0;
    uint64_t tmp_10 [T_size];
    uint64_t a_T [T_size];
    uint64_t digit=0;
    arbitrary_float *f_10, *f_10_k, *tmp, *a_l;
    a_l = local_init_fp_number ();
    tmp = local_init_fp_number ();
    f_10_k = local_init_fp_number ();
    f_10 = local_init_fp_number ();
    int_from_string ("0xA", tmp_10, T_size);
    int_to_float (tmp_10, T_size, f_10);

    fp_one (f_10_k);
    fp_one (tmp);

    if (lt (a.E, bias, E_size)) {
        k++;
        while (fp_abs_lt (a, *f_10_k)) {
            divide (*f_10_k, *f_10, tmp);
            fp_copy (*tmp, f_10_k);
            k--;
        }

    } else {
        while (fp_abs_lt (*tmp, a)) {
            fp_copy (*tmp, f_10_k);
            multiply (*f_10_k, *f_10, tmp);
            k++;
        }
    }

    divide (a, *f_10_k, tmp);
    fp_copy (*tmp, a_l);

    if (a.S) {
        printf ("-0.");
    } else {
        printf ("0.");
    }

    digit = get_int_10 (*a_l);
    fp_get_fractional_part (*a_l, tmp);
    fp_copy (*tmp, a_l);
    printf ("%lu", digit);

    // Esto siempre imprime los 10 primeros digitos del numero
    for (i=0; i<10; i++) {
        multiply (*a_l, *f_10, tmp);
        fp_copy (*tmp, a_l);

        digit = get_int_10 (*a_l);
        fp_get_fractional_part (*a_l, tmp);
        fp_copy (*tmp, a_l);
        printf ("%lu", digit);
    }

    printf ("e%i", k);

    printf ("\n");
    free_all_locals ();
}

int is_subnormal (arbitrary_float a) 
{
    return is_zero (a.E, E_size);
}

void convert_from_decimal_character_sequence (char *str, arbitrary_float *f)
{
    uint64_t temp[3*T_size];
    char count_fractional_digits=0;
    char buffer[100];
    int i=0, fractional_digits=0;
    arbitrary_float *M, *D;
    init_fp_number (f);

    M = local_init_fp_number ();
    D = local_init_fp_number ();

    if (strlen (str) < 100) {
        while (str[i] != '\0') {
            if (count_fractional_digits) {
                fractional_digits++;
            }

            if (str[i] == '.') {
                count_fractional_digits = 1;
            } else {
                buffer [i-count_fractional_digits] = str[i];
            }

            i++;
        }
        buffer [i-count_fractional_digits] = '\0';

        int_from_string (buffer, temp, 3*T_size);
        int_to_float (temp, 3*T_size, M);

        if (fractional_digits) {
            buffer [0] = '1';
            for (i=1; i<=fractional_digits; i++) {
                buffer[i] = '0';
            }
            buffer [i] = '\0';

            int_from_string (buffer, temp, 3*T_size);
            int_to_float (temp, 3*T_size, D);

            divide (*M, *D, f);
        } else {
            fp_copy (*M, f);
        }

    } else {
        printf ("Error: numero demasiado grande");
    }

    free_all_locals ();
}

int fp_abs_lt (arbitrary_float a, arbitrary_float b) {
    if (lt (a.E, b.E, E_size) ) {
        return 1;
    } else if ( (!neq (a.E, b.E, E_size)) && lt(a.T, b.T, T_size)) {
        return 1;
    }
    return 0;
}

int fp_abs_eq (arbitrary_float a, arbitrary_float b) {
    if (neq(a.E, b.E, E_size) || neq(a.T, b.T, T_size)) {
        return 0;
    }
    return 1;
}

void fp_zero (arbitrary_float *f) {
    f->S = 0;
    zero (f->E, E_size);
    zero (f->T, T_size);
}

void fp_one (arbitrary_float *f) {
    int i;
    uint64_t tmp[T_size];

    f->S = 0;
    copy_num (bias, f->E, E_size);
    zero (f->T, T_size);
    f->T[0] = 1;
    for ( i=0; i<p-1; i++) {
        lshift_by_one (f->T, tmp, T_size);
        copy_num (tmp, f->T, T_size);
    }
}

void add (arbitrary_float a, arbitrary_float b, arbitrary_float *ans)
{
    int i;
    arbitrary_float x, y;
    uint64_t sticky=0;
    uint64_t my [T_size+1];
    uint64_t mx [T_size+1];
    uint64_t temp_T [T_size+1];
    uint64_t temp_T1[T_size+1];
    uint64_t temp_T2[T_size+1];
    char Sz = a.S^b.S, B;
    uint64_t delta [E_size];
    uint64_t temp [E_size];


    if (fp_abs_lt (a,b)) {
        ans->S = b.S;
    }

    if (lt(a.E, b.E, E_size)) {
        x = b;
        y = a;
        //copy_num (b.E, x.E, E_size);
        //copy_num (a.E, y.E, E_size);
    } else {
        x = a;
        y = b;
        //copy_num (a.E, x.E, E_size);
        //copy_num (b.E, y.E, E_size);
    }

    // En este punto sabemos que x->E >= y->E sin importar a y b

    // Calculando el signo del resultado
    if (neq (x.E, y.E, E_size)) {
        // |x|>|y| (porque x->E >= y->E)
        ans->S = x.S;
    } else {
        // x->E == y->E
        if (lt (y.T, x.T, T_size)) {
            // |x|<|y|
            ans->S = x.S;
        } else if (neq(y.T, x.T, T_size)) {
            // |x|>|y|
            ans->S = y.S;
        } else {
            // |x|=|y|
            fp_zero (ans);
            if (Sz) {
                ans->S = 0;
            } else {
                ans->S = x.S;
            }
            return;
        }
    }

    int_subtract (x.E, y.E, delta, E_size);
    i = count_non_zero_bits (delta, E_size);

    // Tentativamente ans->E = x.E
    copy_num (x.E, ans->E, E_size);

    // Crea copias locales de mx y my, y abre espacio para los 3 bits de guarda
    copy_num (x.T, mx, T_size); mx[T_size] = 0;
    lshift_by_one (mx, temp_T, T_size+1);
    lshift_by_one (temp_T, mx, T_size+1);
    lshift_by_one (mx, temp_T, T_size+1);
    copy_num (temp_T, mx, T_size+1);

    copy_num (y.T, my, T_size); my[T_size] = 0;
    lshift_by_one (my, temp_T, T_size+1);
    lshift_by_one (temp_T, my, T_size+1);
    lshift_by_one (my, temp_T, T_size+1);
    copy_num (temp_T, my, T_size+1);

    if (i<=64 && delta[0] < p) {
        while (delta[0]) {
            if (my[0]&0x1) {
                sticky = 1;
            }

            rshift_by_one (my, temp_T, T_size);
            copy_num (temp_T, my, T_size);
            delta[0]--;
        }
    }
    else {
        // Como el caso en que my=0 ya se debio manejar sabemos que se descarto
        // por lo menos un 1.
        sticky = 1;
    }

    my[0]|sticky;
    if (Sz) {
        int_subtract (mx, my, temp_T, T_size+1);
    } else {
        int_add (mx, my, temp_T, T_size+1);
    }

    //NOTA: estoy restando los 3 bits que agregue
    i = count_non_zero_bits (temp_T, T_size+1)-3;

    if (i>p) {
        // Hubo un carry en la ultima posicion de la suma
            
        // ver p339 de "Handbook of Floating Point Arithmetic"
        B = temp_T[0]&0x8 && ((temp_T[0]&0x10)||(temp_T[0]&0x7));
        rshift_by_one (temp_T, temp_T1, T_size+1);
        rshift_by_one (temp_T1, temp_T, T_size+1);
        rshift_by_one (temp_T, temp_T1, T_size+1);
        rshift_by_one (temp_T1, temp_T, T_size+1);
        copy_num (temp_T, ans->T, T_size);
        if (B) {
            int_increment (ans->T, T_size);
        }

        int_increment (ans->E, E_size);
    }
    else if (i==p) {
        // No hubo carry en la ultima posicion de la suma
            
        B = temp_T[0]&0x4 && ((temp_T[0]&0x8)||(temp_T[0]&0x3));
        rshift_by_one (temp_T, temp_T1, T_size+1);
        rshift_by_one (temp_T1, temp_T, T_size+1);
        rshift_by_one (temp_T, temp_T1, T_size+1);
        copy_num (temp_T1, ans->T, T_size);

        if (B) {
            int_increment (ans->T, T_size);
        }
    } else {
        // Hay que normalizar el resultado
        rshift_by_one (temp_T, temp_T1, T_size+1);
        rshift_by_one (temp_T1, temp_T, T_size+1);
        rshift_by_one (temp_T, temp_T1, T_size+1);
        copy_num (temp_T1, ans->T, T_size);

        while ((!get_bit (ans->T, p))||is_zero(ans->E, E_size)) {
            lshift_by_one (ans->T, temp_T, T_size);
            copy_num (temp_T, ans->T, T_size);
            int_decrement (ans->E, E_size);
        }
    }
}

void subtract (arbitrary_float a, arbitrary_float b, arbitrary_float *ans)
{
    arbitrary_float l_b = b;
    l_b.S ^= 1;
    add (a, l_b, ans);
}

void multiply (arbitrary_float a, arbitrary_float b, arbitrary_float *ans)
{
    uint64_t temp [E_size];
    uint64_t temp_l1 [2*T_size];
    uint64_t temp_l2 [2*T_size];
    uint64_t sticky;
    int i, l;

    ans -> S = a.S^b.S;

    // ans->E = a.E+b.E-bias
    int_subtract (a.E, bias, temp, E_size);
    int_add (temp, b.E, ans->E, E_size);

    int_multiply (a.T, b.T, temp_l1, T_size);

    for (i=0; i<p-1; i++) {
        if (temp_l1[0]&0x1) {
            sticky = 1;
        }
        rshift_by_one (temp_l1, temp_l2, 2*T_size);
        copy_num (temp_l2, temp_l1, 2*T_size);
    }

    if (is_subnormal (a) && is_subnormal (b)) {
        zero (ans->E, E_size);
        if ((temp_l1[0]&0x1)&&sticky) {
            rshift_by_one (temp_l1, temp_l2, T_size);
            copy_num (temp_l2, temp_l1, T_size);
            int_increment (temp_l1, T_size);
            copy_num (temp_l1, ans->T, T_size);
        } else {
            rshift_by_one (temp_l1, temp_l2, T_size);
            copy_num (temp_l2, temp_l1, T_size);
            copy_num (temp_l1, ans->T, T_size);
        }
    } else if (is_subnormal (a)) {
        l = count_non_zero_bits (a.T, T_size);
        i = count_non_zero_bits (temp_l1, 2*T_size);
        if (i==l) {
            int_decrement (ans->E,E_size);
            lshift_by_one (temp_l1, temp_l2, 2*T_size);
            copy_num (temp_l2, ans->T, T_size);
        }
        else {
            copy_num (temp_l2, ans->T, T_size);
        }
    } else if (is_subnormal (b)) {
        l = count_non_zero_bits (b.T, T_size);
        i = count_non_zero_bits (temp_l1, 2*T_size);
        if (i==l) {
            int_decrement (ans->E, E_size);
            lshift_by_one (temp_l1, temp_l2, 2*T_size);
            copy_num (temp_l2, ans->T, T_size);
        }
        else {
            copy_num (temp_l1, ans->T, T_size);
        }
    } else {
        l = count_non_zero_bits (temp_l1, T_size);
        if (l>p) {
            int_increment (ans->E, E_size);
            rshift_by_one (temp_l1, temp_l2, 2*T_size);
            copy_num (temp_l2, ans->T, T_size);
        } else {
            copy_num (temp_l1, ans->T, T_size);
        }
    }
}

void divide (arbitrary_float a, arbitrary_float b, arbitrary_float *ans)
{
    uint64_t temp [E_size];
    uint64_t temp_l1 [2*T_size];
    uint64_t temp_l2 [2*T_size];
    uint64_t a_l [2*T_size];
    uint64_t b_l [2*T_size];
    uint64_t q [2*T_size];
    uint64_t r [2*T_size];
    uint64_t sticky;
    int i, l;

    ans -> S = a.S^b.S;

    // ans->E = a.E-b.E+bias
    int_subtract (a.E, b.E, temp, E_size);
    int_add (temp, bias, ans->E, E_size);
    if (is_negative(ans->E, E_size)) {
        // El resultado es subnormal
        zero(ans->E, E_size);
    } else {
        zero (a_l, 2*T_size);
        copy_num (a.T, a_l, T_size);
        for (i=0; i<p; i++) {
            lshift_by_one (a_l, temp_l1, 2*T_size);
            copy_num (temp_l1, a_l, 2*T_size);
        }

        zero (b_l, 2*T_size);
        copy_num (b.T, b_l, T_size);
        uint_divide (a_l, b_l, q, r, 2*T_size);

        l = count_non_zero_bits (q, 2*T_size);
        if (l==p) {
            int_decrement (ans->E, E_size);
            copy_num (q, ans->T, T_size);
        } else {
            rshift_by_one (q, ans->T, T_size);
        }
    }
}


