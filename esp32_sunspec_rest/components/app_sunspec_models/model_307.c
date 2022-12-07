#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "sht31_reader.h"
#include "bmp280_reader.h"

#include "app_sunspec_models.h"
#include "sunspec_models.h"

/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 307, RETIRAR DAQUI*/
/**********************************************************/

static model *m307;

char *get_m307_p_id_value()
{
    return "307";
}
char *get_m307_p_length_value()
{
    return "13";
}

char *get_m307_p_temperature_value()
{
    static char tmp[6];
    sprintf(tmp, "%d", (int)(get_sht_temperature() * 10));
    return tmp;
}

char *get_m307_p_humidity_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%d", (int)(get_sht_humidity()));
    return tmp;
}

char *get_m307_p_pressure_value()
{
    static char tmp[6];
    snprintf(tmp, 6, "%d", (int)(get_bmp_pressure() / 100));
    return tmp;
}

model *init_model_307()
{
    // aloca espaço na memoria para a estrutura do modelo
    m307 = create_model(307);
    if (m307 == NULL)
    {
        return NULL;
    }

    // aloca espaço para a estrutura grupo
    m307->group = create_group("base_met", gt_group);
    // alloca e preenche a descrição
    m307->group->desc = allocate_and_fill("Base Meteorological Model");
    // alloca e preenche a label
    m307->group->label = allocate_and_fill("Base Met");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeir0
    //              assim já é possível ir associando o ponteiro *next

    point *pressure = create_point("Pres", pt_int16, 1);
    pressure->label = allocate_and_fill("Barometric Pressure");
    pressure->units = allocate_and_fill("HPa");
    pressure->get_value = get_m307_p_pressure_value;
    pressure->next = NULL;

    point *humidity = create_point("RH", pt_int16, 1);
    humidity->label = allocate_and_fill("Relative Humidity");
    humidity->units = allocate_and_fill("Pct");
    humidity->get_value = get_m307_p_humidity_value;
    humidity->next = pressure;

    point *temperature = create_point("TmpAmb", pt_int16, 1);
    temperature->label = allocate_and_fill("Ambient Temperature");
    temperature->sf = -1;
    temperature->units = allocate_and_fill("C");
    temperature->get_value = get_m307_p_temperature_value;
    temperature->next = humidity;

    point *md_length = create_point("L", pt_uint16, 1);
    md_length->desc = allocate_and_fill("Model length");
    md_length->label = allocate_and_fill("Model Length");
    md_length->_mandatory = mt_M;
    md_length->_static = st_S;
    md_length->get_value = get_m307_p_length_value;
    md_length->next = temperature;

    point *model_id = create_point("ID", pt_uint16, 1);
    model_id->desc = allocate_and_fill("Model identifier");
    model_id->label = allocate_and_fill("Model ID");
    model_id->_mandatory = mt_M,
    model_id->_static = st_S;
    model_id->get_value = get_m307_p_id_value;
    model_id->next = md_length;

    m307->group->count = 5;

    // associa o primeiro ponto ao ponto id
    m307->group->points = model_id;

    return m307;
}