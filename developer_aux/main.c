#include <stdio.h>
#include "sunspec_models.h"
#include <stdbool.h>

model *model_1;
model *model_307;

void main(void)
{
    model_1 = init_model_1();
    if (model_1 != NULL)
    {
        print_model(model_1);
    }
    else
    {
        printf("Modelo 1 iniciado com sucesso\n");
    }

    model_307 = init_model_307();
    if (model_307 != NULL)
    {
        print_model(model_307);
    }
    else
    {
        printf("Modelo 307 iniciado com sucesso\n");
    }
}