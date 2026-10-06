#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#define MAX_LENGTH 1000

int readNumber(const char inputExp[], int *i, long long *number, int sign);
int evaluateExpression(const char inputExp[], long long *result);
int main() {
    char inputExp[MAX_LENGTH];
    long long result = 0;

    if(fgets(inputExp, sizeof(inputExp), stdin) == NULL) {
        printf("Error: Invalid expression.\n");
        return 0;
    }

    if (!evaluateExpression(inputExp, &result)) {
        return 0;
    }
    printf("%lld\n", result);
    return 0;
}
int readNumber(const char inputExp[], int *i, long long *number, int sign) {
    long long currvalue = 0;
    int digit;
    while(isdigit((unsigned char)inputExp[*i])) {
        digit = inputExp[*i] - '0';
        if(sign == 1) {
            if (currvalue > (LLONG_MAX - digit) / 10) {
                return 0;
            }
        }
        else {
            // Neg nums can go up to LLONG_MAX + 1
            if(currvalue > ((unsigned long long)LLONG_MAX + 1 - digit) / 10) {
                return 0;
            }
        }
        currvalue = currvalue * 10 + digit;
        (*i)++;
    }
    if(sign == -1) {
        *number = -(long long)currvalue;
    }
    else {
        *number = currvalue;
    }

    return 1;
}
int evaluateExpression(const char inputExp[], long long *result) {
    long long term = 0;
    long long number = 0;
    char operation = '+';

    int i = 0;
    int needNum = 1; // 1 means we are expecting a number and 0 means expecting an operator
    int sign = 1;
    int signUsedAlready = 0;

    while(inputExp[i] != '\0' && inputExp[i] != '\n') {
        if(isspace((unsigned char)inputExp[i])) {
            i++;
            continue;
        }
        if((inputExp[i] == '+' || inputExp[i] == '-') && needNum) {
            // by this, --5, +-5, -+5, ++5 will be invalid expressions
            if(signUsedAlready) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            if(inputExp[i] == '-') {
                sign = -1;
            }
            signUsedAlready = 1;
            i++;
            continue;
        }

        if(isdigit((unsigned char)inputExp[i])) {
            if(!needNum) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            if(!readNumber(inputExp, &i, &number, sign)) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            sign = 1;
            signUsedAlready = 0;
            needNum = 0;

            if(operation == '+') {
                *result += term;
                term = number;
            }
            else if(operation == '-') {
                *result += term;
                term = -number;
            }
            else if(operation == '*') {
                if (number != 0 &&
                    ((term > 0 && number > 0 && term > LLONG_MAX / number) ||
                     (term < 0 && number < 0 && term < LLONG_MAX / number) || (term > 0 && number < 0 && number < LLONG_MIN / term) ||
                     (term < 0 && number > 0 && term < LLONG_MIN / number))) {

                    printf("Error: Invalid expression.\n");
                    return 0;
                }
                term = term * number;
            }
            else if(operation == '/') {
                if(number == 0) {
                    printf("Error: Division by zero.\n");
                    return 0;
                }
                if(term == LLONG_MIN && number == -1) {
                    printf("Error: Invalid expression.\n");
                    return 0;
                }
                term = term / number;
            }
        }
        else if(inputExp[i] == '+' || inputExp[i] == '-' || inputExp[i] == '*' || inputExp[i] == '/') {
            if(needNum) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            operation = inputExp[i];
            needNum = 1;
            sign = 1;
            signUsedAlready = 0;
            i++;
        }
        else {
            printf("Error: Invalid expression.\n");
            return 0;
        }
    }
    if(needNum) {
        printf("Error: Invalid expression.\n");
        return 0;
    }
    if((term > 0 && *result > LLONG_MAX - term) || (term < 0 && *result < LLONG_MIN - term)) {
        printf("Error: Invalid expression.\n");
        return 0;
    }
    *result += term;
    return 1;
}