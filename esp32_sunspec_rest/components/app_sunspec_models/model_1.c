#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_sunspec_models.h"
#include "sunspec_models.h"


/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 01, RETIRAR DAQUI*/
/**********************************************************/
uint16_t device_address;

char *get_m1_p_id_value()
{
    return "1";
}

char *get_m1_p_length_value()
{
    return "68";
}

char *get_m1_p_manufacturer_value()
{
    return "\"UTFPR Pato Branco\"";
}

char *get_m1_p_device_adress_value()
{
    static char val[6];
    sprintf(val, "%u", device_address);
    return val;
}

// recebe uma string contendo o valor e converte para um inteiro sem sinal.
void set_m1_p_device_adress_value(char *value)
{
    device_address = (uint16_t)strtoul(value, NULL, 10);
}

model *init_model_1()
{
    // aloca espaço na memoria para a estrutura do modelo
    model *m1 = create_model(1);
    if (m1 == NULL)
    {
        return NULL;
    }
    
    // aloca espaço para a estrutura grupo
    m1->group = create_group("common", gt_group);
    // alloca e preenche a descrição
    m1->group->desc = allocate_and_fill("All SunSpec compliant devices must include this as the first model");
    // alloca e preenche a label
    m1->group->label = allocate_and_fill("Common");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeiro
    //              assim já é possível ir associando o ponteiro *next

    point *device_addres = create_point("DA", pt_uint16, 1);
    device_addres->_access = at_RW;
    device_addres->desc = allocate_and_fill("Modbus device address");
    device_addres->label = allocate_and_fill("Device Address");
    device_addres->get_value = get_m1_p_device_adress_value;
    device_addres->set_value = set_m1_p_device_adress_value;
    device_addres->next = NULL; // TODO: apontar para o ponto correto
    set_m1_p_device_adress_value("1");

    point *manufacturer = create_point("Mn", pt_string, 16);
    manufacturer->desc = allocate_and_fill("Well known value registered with SunSpec for compliance");
    manufacturer->label = allocate_and_fill("Manufacturer");
    manufacturer->_mandatory = mt_M;
    manufacturer->_static = st_S;
    manufacturer->get_value = get_m1_p_manufacturer_value;
    manufacturer->next = device_addres;

    point *md_length = create_point("L", pt_uint16, 1);
    md_length->desc = allocate_and_fill("Model length");
    md_length->label = allocate_and_fill("Model Length");
    md_length->_mandatory = mt_M;
    md_length->_static = st_S;
    md_length->get_value = get_m1_p_length_value;
    md_length->next = manufacturer;

    point *model_id = create_point("ID", pt_uint16, 1);
    model_id->desc = allocate_and_fill("Model identifier");
    model_id->label = allocate_and_fill("Model ID");
    model_id->_mandatory = mt_M,
    model_id->_static = st_S;
    model_id->get_value = get_m1_p_id_value;
    model_id->next = md_length;

    // associa o primeiro ponto ao ponto id
    m1->group->points = model_id;

    return m1;
}