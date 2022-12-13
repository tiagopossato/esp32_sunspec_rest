#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "app_sunspec_models.h"
#include "sunspec_models.h"

static model *m1;

/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 01 *************/
/**********************************************************/
#define DEVICE_ADDRESS_MIN 1
#define DEVICE_ADDRESS_MAX 255

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
    return "UTFPR Pato Branco";
}

char *get_m1_p_device_adress_value()
{
    static char val[6];
    sprintf(val, "%u", device_address);
    return val;
}

char *get_m1_p_serial_number_value()
{

    return "WEFWR-ASD5FAS-2022";
}

// recebe uma string contendo o valor e converte para um inteiro sem sinal.
bool set_m1_p_device_adress_value(char *value, cJSON *error)
{
    // converte o valor recebido em uint16_t
    uint16_t tmp = (uint16_t)strtoul(value, NULL, 10);

    // verifica se está dentro dos limites
    if (tmp < DEVICE_ADDRESS_MIN || tmp > DEVICE_ADDRESS_MAX)
    {
        // bool new_error(cJSON *root, char *errCode,
        //                char *errMessage, char *errReason,
        //                bool debug, char *TBD)
        char *errMessage;
        asprintf(&errMessage, "Device address should be between %d and %d.", DEVICE_ADDRESS_MIN, DEVICE_ADDRESS_MAX);
        new_error(error, "MODEL_1-ERR01",
                  errMessage, "Value is out of range.",
                  false, NULL);
        free(errMessage);

        return false;
    }
    device_address = tmp;
    return true;
}

char *get_m1_p_version_value()
{

    return "0.0.1";
}

char *get_m1_p_options_value()
{

    return "1, 2, 5";
}

char *get_m1_p_model_value()
{
    return "2022";
}

model *init_model_1()
{
    // aloca espaço na memoria para a estrutura do modelo
    m1 = create_model(1);
    if (m1 == NULL)
    {
        return NULL;
    }

    // aloca espaço para a estrutura grupo
    m1->group = create_group("common", gt_group);
    // alloca e preenche a descrição
    asprintf(&m1->group->desc, "All SunSpec compliant devices must include this as the first model");
    // alloca e preenche a label
    asprintf(&m1->group->label, "Common");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeiro
    //              assim já é possível ir associando o ponteiro *next

    point *pad = create_point("Pad", pt_pad, 1);
    asprintf(&pad->desc, "Force even alignment");
    asprintf(&pad->label, "Pad");
    pad->_static = st_S;

    point *device_addres = create_point("DA", pt_uint16, 1);
    device_addres->_access = at_RW;
    asprintf(&device_addres->desc, "Modbus device address");
    asprintf(&device_addres->label, "Device Address");
    device_addres->get_value = get_m1_p_device_adress_value;
    device_addres->set_value = set_m1_p_device_adress_value;
    device_addres->next = pad; // TODO: apontar para o ponto correto
    (void)set_m1_p_device_adress_value("1", NULL);

    point *serial_number = create_point("SN", pt_string, 16);
    asprintf(&serial_number->desc, "Manufacturer specific value (32 chars)");
    asprintf(&serial_number->label, "Serial Number");
    serial_number->_mandatory = mt_M;
    serial_number->_static = st_S;
    serial_number->get_value = get_m1_p_serial_number_value;
    serial_number->next = device_addres;

    point *version = create_point("Vr", pt_string, 8);
    asprintf(&version->desc, "Manufacturer specific value (16 chars)");
    asprintf(&version->label, "Version");
    version->_static = st_S;
    version->get_value = get_m1_p_version_value;
    version->next = serial_number;

    point *options = create_point("Opt", pt_string, 8);
    asprintf(&options->desc, "Manufacturer specific value (16 chars)");
    asprintf(&options->label, "Options");
    options->_static = st_S;
    options->get_value = get_m1_p_options_value;
    options->next = version;

    point *model = create_point("Md", pt_string, 16);
    asprintf(&model->desc, "Manufacturer specific value (32 chars)");
    asprintf(&model->label, "Model");
    model->_mandatory = mt_M;
    model->_static = st_S;
    model->get_value = get_m1_p_model_value;
    model->next = options;

    point *manufacturer = create_point("Mn", pt_string, 16);
    asprintf(&manufacturer->desc, "Well known value registered with SunSpec for compliance");
    asprintf(&manufacturer->label, "Manufacturer");
    manufacturer->_mandatory = mt_M;
    manufacturer->_static = st_S;
    manufacturer->get_value = get_m1_p_manufacturer_value;
    manufacturer->next = model;

    point *md_length = create_point("L", pt_uint16, 1);
    asprintf(&md_length->desc, "Model length");
    asprintf(&md_length->label, "Model Length");
    md_length->_mandatory = mt_M;
    md_length->_static = st_S;
    md_length->get_value = get_m1_p_length_value;
    md_length->next = manufacturer;

    point *model_id = create_point("ID", pt_uint16, 1);
    asprintf(&model_id->desc, "Model identifier");
    asprintf(&model_id->label, "Model ID");
    model_id->_mandatory = mt_M,
    model_id->_static = st_S;
    model_id->get_value = get_m1_p_id_value;
    model_id->next = md_length;

    m1->group->count = 9;
    // associa o primeiro ponto ao ponto id
    m1->group->points = model_id;

    return m1;
}