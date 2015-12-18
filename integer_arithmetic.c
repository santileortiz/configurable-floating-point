#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "integer_arithmetic.h"

void int_raw_print (uint64_t * num, int size) {
    int i=0;

    printf("0x");
    for (i=size-1; i>=0; i--) {
        printf ("%016lX", num[i]);
    }
}

void int_print (uint64_t * num, int size) {
    int i=0;
    uint64_t tmp [size];

    if (num[size-1]&(1LL<<(WSIZE-1))){
        printf("-");
        twos_complement(num, tmp, size);
        int_raw_print (tmp, size);
        return;
    }

    int_raw_print (num, size);
}

void int_rand (uint64_t * num, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        num[i] = rand();
        num[i] |= (uint64_t)rand()<<32;
    }
}

void int_positive_rand (uint64_t * num, int size) {
    int_rand (num, size);
    // pone el bit de signo en 0
    num [NWORDS-1] &= ~(1LL<<(WSIZE-1));
}

void int_from_string (char *str, uint64_t *num, int size) {
    uint64_t temp_int [size];
    uint64_t l_num [NWORDS];
    uint64_t long_temp [2*NWORDS];
    uint64_t temp_10 [NWORDS];
    uint64_t temp [size];
    int chars_left = 0, i=0, current_word=0;
    char c, overflow, buff [17], str_is_negative = 0;
    if (str == NULL) {
        zero (num, size);
    } else {
        if (str[0] == '-') {
            str_is_negative = 1;
            str++;
        }

        if (str[0] == '0' && str[1] == 'x') {
            // ------------
            // Hexadecimal
            // ------------
            str += 2;

            // Cuenta la cantidad de digitos hexadecimales
            while (str[i] != '\0') {
                i++;
            }

            // Convierte de derecha a izquierda todas las palabras que tienen
            // exactamente 16 digitos
            chars_left = i;
            buff [16] = '\0';
            while (chars_left > 16) {
                chars_left -= 16;
                for (i=15; i>=0; i--) {
                    buff [i] = str [chars_left + i];
                }
                sscanf (buff, "%"SCNx64, &num[current_word]);
                current_word ++;
            }

            // Lee la ultima palabra que puede tener menos de 16 digitos
            for (i=0; i<16-chars_left; i++) {
                buff [i] = ' ';
            }

            while (chars_left > 0) {
                chars_left--;
                buff [i+chars_left] = str [chars_left];
            }

            sscanf (buff, "%"SCNx64, &num[current_word]);
            current_word ++;

            // Llena las demas palabras con 0's
            while (current_word<size) {
                num [current_word] = 0;
                current_word++;
            }
        }
        else {
            // -------
            // Decimal
            // -------
            // TODO: Si el string representa un entero de mas de size palabras
            //       el comportamiento es indefinido!
            copy_num (num, l_num, size);
            int_from_string ("0xA", temp_10, NWORDS);
            overflow = 0;
            zero (temp, size);
            zero (temp_int, size);
            zero (l_num, NWORDS);
            c = str[0];
            while (c != '\0' && !overflow) {
                temp_int [0] = c - '0';
                // num *=10;
                int_multiply (l_num, temp_10, long_temp, NWORDS);
                copy_num (long_temp, l_num, size);
                overflow = uint_add (l_num, temp_int, temp, size);
                copy_num (temp, l_num, size);

                //temp_int = str[i];
                i++;
                c = str[i];
            }
            copy_num (l_num, num, size);
        }

        if (str_is_negative){
            twos_complement (num, temp_int, size);
            swap (num, temp_int, size);
        }
    }
}

void zero (uint64_t * num, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        num[i] = 0;
    }
}

void one (uint64_t * num, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        num[i] = 0;
    }

    num [0] = 1;
}

// Suma enteros a y b de multiprecision asumiendo que los numeros
// son siempre positivos
int uint_add (uint64_t *a, uint64_t *b, uint64_t *res, int size) {
    int i=0;
    int prev_carry = 0, next_carry=0;
    uint64_t tmp;
    zero (res, size);
    for (i=0; i<size; i++) {
        tmp = a[i]+b[i];
        if ( tmp < a[i] || tmp < b[i]){
            next_carry = 1;
        } else {
            next_carry = 0;
        }

        if (prev_carry) {
            res[i] = tmp + prev_carry;
            if ( res[i] == tmp || res[i] < tmp ){
                next_carry = 1;
            }
        } else {
            res [i] = tmp;
        }

        prev_carry = next_carry;
    }

    return prev_carry;
}

// Imprime un mensaje de error si el resultado de la suma es un numero mas
// grande que el maximo posible para la cantidad de palabras elegidas, 
// es decir que hubo un overflow.
void int_add (uint64_t *a, uint64_t *b, uint64_t *res, int size) {
    int i=0;
    int prev_carry = 0, next_carry=0;
    uint64_t tmp;
    zero (res, size);
    for (i=0; i<size; i++) {
        tmp = a[i]+b[i];
        if ( tmp < a[i] || tmp < b[i]){
            next_carry = 1;
        } else {
            next_carry = 0;
        }

        if (prev_carry) {
            res[i] = tmp + prev_carry;
            if ( res[i] == tmp || res[i] < tmp ){
                next_carry = 1;
            }
        } else {
            res [i] = tmp;
        }

        prev_carry = next_carry;
    }

    if (!((a[size-1]&(1LL<<63))^(b[size-1]&(1LL<<63)))) { 
        // los signos de a y b son iguales
        if ((a[size-1]&(1LL<<63))^(res[size-1]&(1LL<<63))) { 
            // el signo resultante es contrario al de a y b
            printf ("Error: Esta suma/resta da un resultado fuera del rango"
                    "maximo para la cantidad de palabras elegida, elija mas"
                    "palabras por favor\n");
        }
    }
}

void int_increment (uint64_t *a, int size) {
    uint64_t temp_1 [size];
    uint64_t temp [size];
    one (temp_1, size);
    int_add (a, temp_1, temp, size);
    copy_num (temp, a, size);
}

void int_decrement (uint64_t *a, int size) {
    uint64_t temp_1 [size];
    uint64_t temp [size];
    one (temp_1, size);
    int_subtract (a, temp_1, temp, size);
    copy_num (temp, a, size);
}

void int_subtract (uint64_t *a, uint64_t *b, uint64_t *res, int size) {
    int i=0;
    uint64_t tmp[size];

    twos_complement (b, tmp, size);
    int_add (a, tmp, res, size);
}

// res debe ser una variable de tamaño 2*size
void int_multiply (uint64_t *a, uint64_t *b, uint64_t *res, int size) {
    int i,j;
    uint64_t u=0, v=0, c=0, abl=0, abh=0, temp=0;
    uint64_t a0b0,a1b1,a1b0,a0b1;
    zero (res, 2*size);
    for (i=0; i<size; i++) {
        u = 0;
        for (j=0; j<size; j++) {
            // (abh,abl) <- a[i]b[j]
            abh = 0;
            abl = 0;
            a0b0 = (0xFFFFFFFF&a[i])*(0xFFFFFFFF&b[j]);
            a0b1 = (0xFFFFFFFF&a[i])*(b[j]>>32);
            a1b0 = (a[i]>>32)*(0xFFFFFFFF&b[j]);
            a1b1 = (a[i]>>32)*(b[j]>>32);

            temp = a0b1 + a1b0;
            if (temp < a0b1 || temp < a1b0) {
                abh = (1LL<<32);
            }

            abh += a1b1 + (temp>>32);

            abl = a0b0 + (temp<<32);
            if (abl < a0b0 || abl<(temp<<32)) {
                abh += 1;
            }

            c = 0;
            // (u,v) <- res [i+j] + a[i]b[j] + u
            // v <- res [i+j] + abl + u
            temp = abl + u;
            if (temp<abl || temp<u) {
                c += 1;
            }

            v = res [i+j] + temp;
            if (v<res[i+j] || v<temp) {
                c += 1;
            }

            // u <- abh + c
            u = abh + c;

            res [i+j] = v;
        }
        res [i+size] = u;
    }
}

void twos_complement (uint64_t *a, uint64_t *res, int size) {
    int i=0;
    uint64_t one [size];
    uint64_t tmp [size];

    zero (one, size);
    one[0] = 1;

    for (i=0; i<size; i++) {
        tmp[i] = ~a[i];
    }

    int_add (one, tmp, res, size);
}

void rshift_by_one (uint64_t *a, uint64_t *res, int size) {
    int i=0;
    for (i=0; i<size-1; i++){
        res[i] = a[i]>>1;
        res[i] |= (a[i+1]&1)<<63;
    }

    res[size-1] = ((int64_t)a[size-1])>>1;
}

void lshift_by_one (uint64_t *a, uint64_t *res, int size) {
    int i=0;
    if (a[size-1]&(1LL<<63))
        printf ("Error: este shift se desborda \n");

    for (i=size-1; i>0; i--){
        res[i] = a[i]<<1;
        res[i] |= (a[i-1]&(1LL<<63))>>63;
    }
    res[0] = a[0]<<1;
}

void copy_num (uint64_t *a, uint64_t *b, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        b[i] = a[i];
    }
}
    
void swap (uint64_t *a, uint64_t *b, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        a[i] = a[i]^b[i];
        b[i] = a[i]^b[i];
        a[i] = a[i]^b[i];
    }
}

int is_zero (uint64_t *num, int size) {
    int i=0;
    for (i=0; i<size; i++) {
        if (num[i]!=0)
            return 0;
    }
    return 1;
}

int is_one (uint64_t *num) {
    int i=0;
    if (num[0] != 1)
        return 0;
    else {
        for (i=1; i<NWORDS; i++) {
            if (num[i]!=0)
                return 0;
        }
    }
    return 1;
}

int lt (uint64_t *a, uint64_t *b, int size) {
    int i=0;
    if ((a[size-1]&(1LL<<63))^(b[size-1]&(1LL<<63))){
        if ((is_negative(a, NWORDS)) && (!(is_negative(b, NWORDS)))){
            return 1;
        } else {
            return 0;
        }
    }

    for (i=size-1; i>=0; i--) {
        if (a[i] != b[i])
            return a[i]<b[i];
    }
    return 0;
}

int leq (uint64_t *a, uint64_t *b, int size) {
    int i=0;
    if ((a[size-1]&(1LL<<63))^(b[size-1]&(1LL<<63))){
        if ((is_negative(a, size)) && (!(is_negative(b, size)))){
            return 1;
        } else {
            return 0;
        }
    }

    for (i=size-1; i>=0; i--) {
        if (a[i] != b[i])
            return a[i]<b[i];
    }
    return 1;
}

int neq (uint64_t *a, uint64_t *b, int size) {
    int i=0;
    for (i=size-1; i>=0; i--) {
        if (a[i] != b[i])
            return 1;
    }
    return 0;
}

void extended_binary (uint64_t *a, uint64_t *b, uint64_t *d, uint64_t *s, uint64_t *t) {
    uint64_t a_l[NWORDS];
    uint64_t b_l[NWORDS];
    uint64_t tmp[NWORDS];
    int r=0;

    copy_num (a, a_l, NWORDS);
    copy_num (b, b_l, NWORDS);

    while ( (is_even(a_l)) && (is_even(b_l))){
        rshift_by_one (a_l, tmp, NWORDS);
        swap(a_l, tmp, NWORDS);
        rshift_by_one (b_l, tmp, NWORDS);
        swap(b_l, tmp, NWORDS);
        r++;
    }

    uint64_t x0 [NWORDS]; uint64_t y0 [NWORDS];
    uint64_t x1 [NWORDS]; uint64_t y1 [NWORDS];
    uint64_t x2 [NWORDS]; uint64_t y2 [NWORDS];

    copy_num (a_l, x0, NWORDS); copy_num (b_l, y0, NWORDS);
    one (x1, NWORDS); zero (x2, NWORDS);
    zero (y1, NWORDS); one (y2, NWORDS);

    while (is_even(x0)){
        rshift_by_one (x0, tmp, NWORDS);
        swap (x0, tmp, NWORDS);

        if ((is_even(x1)) && (is_even(x2))){
            rshift_by_one (x1, tmp, NWORDS);
            swap (x1, tmp, NWORDS);
            rshift_by_one (x2, tmp, NWORDS);
            swap (x2, tmp, NWORDS);
        } else {
            int_add (x1, b_l, tmp, NWORDS);
            rshift_by_one (tmp, x1, NWORDS);
            int_subtract (x2, a_l, tmp, NWORDS);
            rshift_by_one (tmp, x2, NWORDS);
        }
    }
    //printf ("x1=%d, x2=%d\n", x1, x2);
    //printf ("y1=%d, y2=%d\n", y1, y2);

    //while (x0 != y0) {
    while (neq (x0,y0, NWORDS)) {
        if (is_even(y0)){
            rshift_by_one (y0, tmp, NWORDS);
            swap (y0, tmp, NWORDS);
            if ((is_even(y1)) && (is_even(y2))){
                rshift_by_one (y1, tmp, NWORDS);
                swap (y1, tmp, NWORDS);
                rshift_by_one (y2, tmp, NWORDS);
                swap (y2, tmp, NWORDS);
            } else {
                int_add (y1, b_l, tmp, NWORDS);
                rshift_by_one (tmp, y1, NWORDS);

                int_subtract (y2, a_l, tmp, NWORDS);
                rshift_by_one (tmp, y2, NWORDS);
            }
        }else if (lt(y0,x0, NWORDS)) {
        //}else if (y0<x0) {
            swap (x0, y0, NWORDS);
            swap (x1, y1, NWORDS);
            swap (x2, y2, NWORDS);
        } else {
            int_subtract (y0, x0, tmp, NWORDS);
            swap (tmp, y0, NWORDS);

            int_subtract (y1, x1, tmp, NWORDS);
            swap (tmp, y1, NWORDS);

            int_subtract (y2, x2, tmp, NWORDS);
            swap (tmp, y2, NWORDS);
        }
    
        //printf ("-------\n");
        //printf ("x0= "); int_print (x0, NWORDS); printf ("\n");
        //printf ("x1= "); int_print (x1, NWORDS); printf ("\n");
        //printf ("x2= "); int_print (x2, NWORDS); printf ("\n");
        //printf ("y0= "); int_print (y0, NWORDS); printf ("\n");
        //printf ("y1= "); int_print (y1, NWORDS); printf ("\n");
        //printf ("y2= "); int_print (y2, NWORDS); printf ("\n");
    }


    for (; r>0; r--) {
        lshift_by_one (x0, tmp, NWORDS);
        swap (x0, tmp, NWORDS);
    }

    swap (d, x0, NWORDS);
    swap (s, y1, NWORDS);
    swap (t, y2, NWORDS);
}

// Esta funcion de division entera asume que los numeros a y b son positivos
void uint_divide (uint64_t *a, uint64_t *b, uint64_t *q, uint64_t *r, int size) {
    uint64_t c[size+1];
    uint64_t r_l[size+1];
    uint64_t tmp1[size+1];
    uint64_t b_l[size+1];
    int j=0, i;

    zero (tmp1, size+1);
    zero (b_l, size+1);
    zero (q, size);
    if (lt(b, a, size)) {
        zero (r_l, size+1);
        copy_num (a, r_l, size);
        zero (b_l, size+1);
        copy_num (b, b_l, size);
        zero (c, size+1);
        c[0] = 1;

        while (!(b_l[size-1]&(1LL<<63))){
            lshift_by_one (b_l, tmp1, size+1);
            copy_num (tmp1, b_l, size);
            lshift_by_one (c, tmp1, size+1);
            copy_num (tmp1, c, size);
            j++;
        }

        for (i=64*size; i>=(64*size-j); i--) {
            if (leq (b_l, r_l, size+1)) {
                int_subtract (r_l, b_l, tmp1, size+1);
                copy_num (tmp1, r_l, size+1);
                int_add (q, c, tmp1, size);
                copy_num (tmp1, q, size);
            }
            rshift_by_one (c, tmp1, size);
            copy_num (tmp1, c, size);
            rshift_by_one (b_l, tmp1, size+1);
            copy_num (tmp1, b_l, size+1);
        }
        copy_num (r_l, r, size);
    } else {
        copy_num (a, r, size);
    }
}

// El lcm puede ser hasta el doble del tamaño de a y b, por lo que res
// debe tener tamaño 2*NWORDS.
void lcm (uint64_t *a, uint64_t *b, uint64_t *gcd, uint64_t *res) {
    uint64_t tmp1[NWORDS];
    uint64_t tmp_l[2*NWORDS];
    uint64_t adivd[NWORDS];
    uint64_t one[NWORDS];
    uint64_t a_l[NWORDS];
    uint64_t b_l[2*NWORDS];
    uint64_t gcd_l[NWORDS];
    int r = 0; int i=0;

    copy_num (a, a_l, NWORDS);
    copy_num (gcd, gcd_l, NWORDS);

    zero (one, NWORDS);
    one [0] = 1;
    zero (adivd, NWORDS);

    // Se mueve el GCD lo mas a la izquierda para comenzar a restar
    // se podria optimizar y moverlo solo hasta que deje de ser menor
    // que a_l.
    while (!(gcd_l[NWORDS-1]&(1<<30))){
        lshift_by_one (gcd_l, tmp1, NWORDS);
        swap (gcd_l, tmp1, NWORDS);
        lshift_by_one (one, tmp1, NWORDS);
        swap (one, tmp1, NWORDS);
        r++;
    }

    // Resta sucesivamente el gcd recorrido en caso de ser menor que
    // lo que se lleva, en caso contrario solo se recorre. Al final
    // la suma de los bits donde se pudo hacer la resta es igual a la
    // division a div d.
    for (i=32*NWORDS; i>=(32*NWORDS-r); i--) {
        if (leq (gcd_l, a_l, NWORDS)){
            int_subtract (a_l, gcd_l, tmp1, NWORDS);
            swap (a_l, tmp1, NWORDS);
            int_add (adivd, one, tmp1, NWORDS);
            swap (adivd, tmp1, NWORDS);
        }
        rshift_by_one (one, tmp1, NWORDS);
        swap (one,tmp1, NWORDS);
        rshift_by_one (gcd_l, tmp1, NWORDS);
        swap (gcd_l,tmp1, NWORDS);
    }

    
    zero (b_l, 2*NWORDS);
    copy_num (b, b_l, NWORDS);

    for (i=0; i<2*NWORDS; i++) {
        res[i] = 0;
    }

    // Se multiplica b por el calor calculado antes de a div d, y se tiene
    // en cuenta que esta puede ser una palabra grande.
    for (i=0; i<32*NWORDS-1; i++) {
        if (get_bit (adivd, i)) {
            int_add (res, b_l, tmp_l, 2*NWORDS);
            swap (res, tmp_l, 2*NWORDS);
        }
        lshift_by_one (b_l, tmp_l, 2*NWORDS);
        swap (b_l, tmp_l, 2*NWORDS);
    }
}

