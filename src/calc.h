#include <stdbool.h>
#include <stddef.h>

typedef enum Operation_enum
{
    OPERATION_NONE = 0,
    OPERATION_ADD,
    OPERATION_SUBTRACT,
    OPERATION_MULTIPLY,
    OPERATION_DIVIDE,
    OPERATION_POWER,
    OPERATION_SQRT,
    OPERATION_SIN,
    OPERATION_COS,
    OPERATION_TAN
} Operation;

typedef struct Calc_struct
{
    double current_value;
    double stored_value;
    Operation pending_operation;
    char expression[256];
    size_t expression_index;
    bool error;
    bool just_calculated;
} Calc;

void calc_init(Calc *calc);
void calc_clear(Calc *calc);
void calc_input_digit(Calc *calc, int digit);
void calc_input_decimal(Calc *calc);
void calc_input_operation(Calc *calc, Operation operation);
void calc_calculate(Calc *calc);
void calc_toggle_sign(Calc *calc);
void calc_backspace(Calc *calc);
const char *calc_get_expression(const Calc *calc);
double calc_get_current_value(const Calc *calc);
bool calc_has_error(const Calc *calc);
