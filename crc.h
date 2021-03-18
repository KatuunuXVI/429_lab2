#include

int twoThePower(int exp) {
    return exp == 0 ? 1 : 2 * two_the_power(exp-1);
}

int byteValue(int bits[], int len) {
    int i;
    int value = 0;
    for(i = 1; i <= len; i++) {
        value += bits[len-i] * twoThePower(len-i)
    }
}

int checkBits(int dividend, int divisor, int quotient, int remainder) {
    int divV = byteValue()
}