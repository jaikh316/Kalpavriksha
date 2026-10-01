#include <stdio.h>
#include <ctype.h>
int main() {
    char inputExp[1000];
    long long result = 0;
    long long term = 0;
    long long num = 0;
    char ope = '+';

    int i = 0;
    int needNum = 1; // 1 means we are expecting a number and 0 means expecting an operator
    int sign = 1;

    fgets(inputExp, sizeof(inputExp), stdin);

    while(inputExp[i] != '\0' && inputExp[i] != '\n') {
        if(isspace(inputExp[i])) {
            i++;
            continue;
        }
        if((inputExp[i] == '+' || inputExp[i] == '-') && needNum) {
            if(inputExp[i] == '-') {
                sign = -sign;
            }
            i++;
            continue;
        }

        if(isdigit(inputExp[i])) {
            if(!needNum) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            num = 0;
            while(isdigit(inputExp[i])) {
                num = num * 10 + (inputExp[i] - '0');
                i++;
            }
            num = num * sign;
            sign = 1;
            needNum = 0;

            if(ope == '+') {
                result += term;
                term = num;
            }
            else if(ope == '-') {
                result += term;
                term = -num;
            }
            else if(ope == '*') {
                term = term * num;
            }
            else if(ope == '/') {
                if(num == 0) {
                    printf("Error: Division by zero.\n");
                    return 0;
                }
                term = term / num;
            }
        }
        else if(inputExp[i] == '+' || inputExp[i] == '-' || inputExp[i] == '*' || inputExp[i] == '/') {

            if(needNum) {
                printf("Error: Invalid expression.\n");
                return 0;
            }
            ope = inputExp[i];
            needNum = 1;
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
    result += term;
    printf("%lld\n", result);
    return 0;
}

