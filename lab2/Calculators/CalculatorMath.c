#include "CalculatorMath.h"

int32_t calculator_add(int16_t left, int16_t right)
{
    return (int32_t)left + (int32_t)right;
}

int16_t calculator_subtract(int16_t left, int16_t right)
{
    int32_t difference = (int32_t)left - (int32_t)right;

    if (difference < -32768L || difference > 32767L) {
        return 0;
    }
    return (int16_t)difference;
}

int32_t calculator_multiply(int16_t left, int16_t right)
{
    return (int32_t)left * (int32_t)right;
}

int16_t calculator_divide(int16_t left, int16_t right)
{
    int32_t quotient;

    if (right == 0 || (left == -32768L && right == -1)) {
        return 0;
    }
    quotient = (left < 0 ? -(int32_t)left : (int32_t)left)
        / (right < 0 ? -(int32_t)right : (int32_t)right);
    if ((left < 0) != (right < 0)) {
        quotient = -quotient;
    }
    return (int16_t)quotient;
}
