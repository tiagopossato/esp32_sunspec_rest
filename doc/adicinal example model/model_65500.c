#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "sht31_reader.h"

#include "app_sunspec_models.h"
#include "sunspec_models.h"

/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 65500 ************/
/**********************************************************/

#define COMPRESSOR_MAX_VALUE 1
#define COMPRESSOR_MIN_VALUE 0

#define MODE_AUTO 1
#define MODE_MANUAL 0

#define MODE_MAX_VALUE MODE_AUTO
#define MODE_MIN_VALUE MODE_MANUAL

// REMEMBER THAT SCALE FACTOR IS -1
#define SETPOINT_MAX_VALUE 200
#define SETPOINT_MIN_VALUE -50
#define HISTERESIS_MAX_VALUE 100
#define HISTERESIS_MIN_VALUE 5

static model *m65500;

static bool compressor;
static bool mode;
static float setpoint;
static float histeresis;

char *get_m65500_p_id_value()
{
    return "65500";
}

char *get_m65500_p_length_value()
{
    return "6";
}

char *get_m65500_p_compressor_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%u", (uint16_t)(compressor));
    return tmp;
};

bool set_m65500_p_compressor_value(char *value, cJSON *error)
{
    char *errMessage;

    // converte o valor recebido em uint16_t
    uint16_t tmp = (uint16_t)strtoul(value, NULL, 10);

    // verifica o modo de operação, se estiver em modo automático(1), a escrita é proibida
    if (mode == MODE_AUTO)
    {
        asprintf(&errMessage, "Control is operating in AUTOMATIC mode. Turn to MANUAL mode before.");
        new_error(error, "MODEL_65500-ERR02",
                  errMessage, "Operation not allowed",
                  false, NULL);
        free(errMessage);
        return false;
    }

    // verifica se está dentro dos limites, como é um valor uint, nunca será menor que 0
    if (tmp > COMPRESSOR_MAX_VALUE)
    {
        asprintf(&errMessage, "Compressor should be between %d and %d.", COMPRESSOR_MIN_VALUE, COMPRESSOR_MAX_VALUE);
        new_error(error, "MODEL_65500-ERR01",
                  errMessage, "Value is out of range.",
                  false, NULL);
        free(errMessage);
        return false;
    }

    // associa o valor
    compressor = tmp;
    return true;
};

char *get_m65500_p_mode_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%u", (uint16_t)(mode));
    return tmp;
};

bool set_m65500_p_mode_value(char *value, cJSON *error)
{
    // converte o valor recebido em uint16_t
    uint16_t tmp = (uint16_t)strtoul(value, NULL, 10);

    // verifica se está dentro dos limites, como é um valor uint, nunca será menor que 0
    if (tmp > MODE_MAX_VALUE)
    {
        char *errMessage;
        asprintf(&errMessage, "Operational mode be between %d and %d.", COMPRESSOR_MIN_VALUE, COMPRESSOR_MAX_VALUE);
        new_error(error, "MODEL_65500-ERR01",
                  errMessage, "Value is out of range.",
                  false, NULL);
        free(errMessage);

        return false;
    }
    // associa o valor
    mode = tmp;
    return true;
};

char *get_m65500_p_setpoint_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%d", (int)(setpoint * 10));
    return tmp;
};

bool set_m65500_p_setpoint_value(char *value, cJSON *error)
{
    // converte o valor recebido para um inteiro, uma vez que o tipo é int16
    int16_t tmp = (int16_t)strtol(value, NULL, 10);

    // verifica se está dentro dos limites
    if (tmp < SETPOINT_MIN_VALUE || tmp > SETPOINT_MAX_VALUE)
    {
        char *errMessage;
        asprintf(&errMessage, "SetPoint should be between %d and %d.", SETPOINT_MIN_VALUE, SETPOINT_MAX_VALUE);
        new_error(error, "MODEL_65500-ERR01",
                  errMessage, "Value is out of range.",
                  false, NULL);
        free(errMessage);

        return false;
    }
    // divide o valor recebido por 10, uma vez que o sf é -1
    setpoint = (float)(tmp / 10.0f);
    return true;
};

char *get_m65500_p_histeresis_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%d", (int)(histeresis * 10));
    return tmp;
}

bool set_m65500_p_histeresis_value(char *value, cJSON *error)
{
    // converte o valor recebido para um inteiro, uma vez que o tipo é int16
    int16_t tmp = (int16_t)strtol(value, NULL, 10);

    // verifica se está dentro dos limites
    if (tmp < HISTERESIS_MAX_VALUE || tmp > HISTERESIS_MIN_VALUE)
    {
        // bool new_error(cJSON *root, char *errCode,
        //                char *errMessage, char *errReason,
        //                bool debug, char *TBD)
        char *errMessage;
        asprintf(&errMessage, "Histeresis should be between %d and %d.", SETPOINT_MIN_VALUE, SETPOINT_MAX_VALUE);
        new_error(error, "MODEL_65500-ERR01",
                  errMessage, "Value is out of range.",
                  false, NULL);
        free(errMessage);

        return false;
    }
    // divide o valor recebido por 10, uma vez que o sf é -1
    histeresis = (float)(tmp / 10.0f);
    return true;
}

model *init_model_65500()
{
    // aloca espaço na memoria para a estrutura do modelo
    m65500 = create_model(65500);
    if (m65500 == NULL)
    {
        return NULL;
    }

    // aloca espaço para a estrutura grupo
    m65500->group = create_group("refrigerator", gt_group);
    // alloca e preenche a descrição
    asprintf(&m65500->group->desc, "Refrigerator Model");
    // alloca e preenche a label
    asprintf(&m65500->group->label, "Refrigerator");

    // {
    //     "desc": "Operational mode",
    //     "label": "Operational mode",
    //     "access": "RW",
    //     "mandatory": "M",
    //     "name": "OpMode",
    //     "size": 1,
    //     "type": "uint16"
    // }
    point *mode_point = create_point("OpMode", pt_uint16, 1);
    asprintf(&mode_point->label, "Operational mode");
    asprintf(&mode_point->desc, "Operational mode");
    mode_point->_mandatory = mt_M;
    mode_point->_access = at_RW;
    mode_point->get_value = get_m65500_p_mode_value;
    mode_point->set_value = set_m65500_p_mode_value;
    mode_point->next = NULL;

    // {
    //     "desc": "Temperature Histeresis",
    //     "label": "Histeresis",
    //     "access": "RW",
    //     "mandatory": "M",
    //     "name": "Hist",
    //     "sf": -1,
    //     "size": 1,
    //     "type": "int16",
    //     "units": "C"
    // }
    point *point_histeresis = create_point("Hist", pt_int16, 1);
    asprintf(&point_histeresis->label, "Histeresis");
    asprintf(&point_histeresis->desc, "Temperature Histeresis");
    asprintf(&point_histeresis->units, "C");
    point_histeresis->_mandatory = mt_M;
    point_histeresis->_access = at_RW;
    point_histeresis->sf = -1;
    point_histeresis->get_value = get_m65500_p_histeresis_value;
    point_histeresis->set_value = set_m65500_p_histeresis_value;
    point_histeresis->next = mode_point;

    // {
    //     "desc": "Temperature SetPoint",
    //     "label": "SetPoint",
    //     "access": "RW",
    //     "mandatory": "M",
    //     "name": "SetPt",
    //     "sf": -1,
    //     "size": 1,
    //     "type": "int16",
    //     "units": "C"
    // },
    point *setpoint_point = create_point("SetPt", pt_int16, 1);
    asprintf(&setpoint_point->label, "SetPoint");
    asprintf(&setpoint_point->desc, "Temperature SetPoint");
    asprintf(&setpoint_point->units, "C");
    setpoint_point->_mandatory = mt_M;
    setpoint_point->_access = at_RW;
    setpoint_point->sf = -1;
    setpoint_point->get_value = get_m65500_p_setpoint_value;
    setpoint_point->set_value = set_m65500_p_setpoint_value;
    setpoint_point->next = point_histeresis;

    // {
    //     "desc": "Compressor output",
    //     "label": "Compressor",
    //     "access": "RW",
    //     "mandatory": "M",
    //     "name": "Comp",
    //     "size": 1,
    //     "type": "uint16"
    // },
    point *compressor_point = create_point("Comp", pt_uint16, 1);
    asprintf(&compressor_point->label, "Compressor");
    asprintf(&compressor_point->desc, "Compressor output");
    compressor_point->_mandatory = mt_M;
    compressor_point->_access = at_RW;
    compressor_point->get_value = get_m65500_p_compressor_value;
    compressor_point->set_value = set_m65500_p_compressor_value;
    compressor_point->next = setpoint_point;

    point *md_length = create_point("L", pt_uint16, 1);
    asprintf(&md_length->desc, "Model length");
    asprintf(&md_length->label, "Model Length");
    md_length->_mandatory = mt_M;
    md_length->_static = st_S;
    md_length->get_value = get_m65500_p_length_value;
    md_length->next = compressor_point;

    point *model_id = create_point("ID", pt_uint16, 1);
    asprintf(&model_id->desc, "Model identifier");
    asprintf(&model_id->label, "Model ID");
    model_id->_mandatory = mt_M,
    model_id->_static = st_S;
    model_id->get_value = get_m65500_p_id_value;
    model_id->next = md_length;

    m65500->group->count = 6;

    // associa o primeiro ponto ao ponto id
    m65500->group->points = model_id;

    return m65500;
}