#include "calc.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void calc_init(Calc *calc)
{
    if (!calc)
    {
        return;
    }
    calc->current_value = 0.0;
    calc->stored_value = 0.0;
    calc->pending_operation = OPERATION_NONE;
    calc->expression[0] = '\0';
    calc->expression_index = 0;
    calc->error = false;
    calc->just_calculated = false;
}

void calc_clear(Calc *calc)
{
    if (!calc)
    {
        return;
    }
    calc_init(calc);
}

void calc_input_digit(Calc *calc, int digit)
{
    if (!calc || calc->error || digit < 0 || digit > 9)
    {
        return;
    }

    if (calc->just_calculated)
    {
        calc->current_value = 0.0;
        calc->expression[0] = '\0';
        calc->expression_index = 0;
        calc->just_calculated = false;
    }

    calc->current_value = calc->current_value * 10.0 + (double)digit;

    if (calc->expression_index < sizeof(calc->expression) - 2)
    {
        calc->expression[calc->expression_index++] = (char)('0' + digit);
        calc->expression[calc->expression_index] = '\0';
    }
}

void calc_input_decimal(Calc *calc)
{
    if (!calc || calc->error || calc->just_calculated)
    {
        return;
    }

    // Check if decimal point already exists
    char temp[32];
    snprintf(temp, sizeof(temp), "%.10g", calc->current_value);
    if (strchr(temp, '.'))
    {
        return;
    }

    if (calc->expression_index < sizeof(calc->expression) - 2)
    {
        calc->expression[calc->expression_index++] = '.';
        calc->expression[calc->expression_index] = '\0';
    }
}

void calc_input_operation(Calc *calc, Operation opepration)
{
    if (!calc || calc->error || opepration == OPERATION_NONE)
    {
        return;
    }

    if (calc->pending_operation != OPERATION_NONE && !calc->just_calculated)
    {
        calc_calculate(calc);
    }

    calc->stored_value = calc->current_value;
    calc->pending_operation = opepration;
    calc->just_calculated = true;

    const char *operation_symbols[] = {
        "+", "-", "*", "/", "^", "sqrt", "sin", "cos", "tan"};
    if (calc->expression_index < sizeof(calc->expression) - 8)
    {
        calc->expression[calc->expression_index++] = ' ';
        const char *symbol = operation_symbols[opepration];
        size_t symbol_len = strlen(symbol);
        memcpy(calc->expression + calc->expression_index, symbol, symbol_len);
        calc->expression_index += symbol_len;
        calc->expression[calc->expression_index++] = ' ';
        calc->expression[calc->expression_index] = '\0';
    }
}

void calc_calculate(Calc *calc)
{
    if (!calc || calc->error || calc->pending_operation == OPERATION_NONE)
    {
        return;
    }

    double result = 0.0;
    bool error = false;

    switch (calc->pending_operation)
    {
    case OPERATION_ADD:
        result = calc->stored_value + calc->current_value;
        break;
    case OPERATION_SUBTRACT:
        result = calc->stored_value - calc->current_value;
        break;
    case OPERATION_MULTIPLY:
        result = calc->stored_value * calc->current_value;
        break;
    case OPERATION_DIVIDE:
        if (calc->current_value == 0.0)
        {
            error = true;
        }
        else
        {
            result = calc->stored_value / calc->current_value;
        }
        break;
    case OPERATION_POWER:
        result = pow(calc->stored_value, calc->current_value);
        break;
    case OPERATION_SQRT:
        if (calc->current_value >= 0.0)
        {
            result = sqrt(calc->current_value);
        }
        else
        {
            error = true;
        }
        break;
    case OPERATION_SIN:
        result = sin(calc->current_value);
        break;
    case OPERATION_COS:
        result = cos(calc->current_value);
        break;
    case OPERATION_TAN:
        result = tan(calc->current_value);
        break;
    default:
        error = true;
        break;
    }

    if (error)
    {
        calc->error = true;
        snprintf(calc->expression, sizeof(calc->expression), "Error");
        calc->expression_index = strlen(calc->expression);
    }
    else
    {
        calc->current_value = result;
        calc->stored_value = 0.0;
        calc->pending_operation = OPERATION_NONE;
        calc->just_calculated = true;

        char temp[32];
        snprintf(temp, sizeof(temp), "%.10g", result);
        size_t temp_len = strlen(temp);

        if (calc->expression_index + temp_len < sizeof(calc->expression) - 1)
        {
            memcpy(calc->expression + calc->expression_index, temp, temp_len + 1);
            calc->expression_index += temp_len;
        }
    }
}

void calc_toggle_sign(Calc *calc)
{
    if (!calc || calc->error)
    {
        return;
    }
    calc->current_value = -calc->current_value;
    calc->just_calculated = false;
}

void calc_backspace(Calc *calc)
{
    if (!calc || calc->error || calc->expression_index == 0 || calc->just_calculated)
    {
        return;
    }

    calc->expression_index--;
    calc->expression[calc->expression_index] = '\0';
    calc->current_value = floor(calc->current_value / 10.0);
}

const char *calc_get_expression(const Calc *calc)
{
    return calc ? calc->expression : "";
}

double calc_get_current_value(const Calc *calc)
{
    return calc ? calc->current_value : 0.0;
}

bool calc_get_error(const Calc *calc)
{
    return calc ? calc->error : true;
}