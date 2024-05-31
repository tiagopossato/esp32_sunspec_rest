#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "sht31_reader.h"
#include "bmp280_reader.h"

#include "app_sunspec_models.h"
#include "sunspec_models.h"

/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 307 ************/
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
    sprintf(tmp, "%d", (int)(get_bmp_temperature() * 10));
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

char *get_m307_p_wind_direction_value()
{
    return "-127";
}

char *get_m307_p_rainfall_value()
{
    return "-127";
}

char *get_m307_p_snow_depth_value()
{
    return "-127";
}

char *get_m307_p_precipitation_type_value()
{
    return "-127";
}

char *get_m307_p_electric_field_value()
{
    return "-127";
}

char *get_m307_p_surface_wetness_value()
{
    return "-127";
}

char *get_m307_p_soil_wetness_value()
{
    return "-127";
}

char *get_m307_p_wind_speed_value()
{
    return "-127";
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
    asprintf(&m307->group->desc, "Base Meteorological Model");
    // alloca e preenche a label
    asprintf(&m307->group->label, "Base Met");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeiro
    //              assim já é possível ir associando o ponteiro *next

    // point *wind_direction = create_point("WndDir", pt_int16, 1);
    // asprintf(&wind_direction->label, "Wind Direction");
    // asprintf(&wind_direction->units, "deg");
    // wind_direction->get_value = get_m307_p_wind_direction_value;
    // wind_direction->next = NULL;

    // point *rainfall = create_point("Rain", pt_int16, 1);
    // asprintf(&rainfall->label, "Rainfall");
    // asprintf(&rainfall->units, "mm");
    // rainfall->get_value = get_m307_p_rainfall_value;
    // rainfall->next = wind_direction;

    // point *snow_depth = create_point("Snw", pt_int16, 1);
    // asprintf(&snow_depth->label, "Snow Depth");
    // asprintf(&snow_depth->units, "mm");
    // snow_depth->get_value = get_m307_p_snow_depth_value;
    // snow_depth->next = rainfall;

    // point *precipitation_type = create_point("PPT", pt_int16, 1);
    // asprintf(&precipitation_type->label, "Precipitation Type");
    // asprintf(&precipitation_type->desc, "Precipitation Type (WMO 4680 SYNOP code reference)");
    // precipitation_type->get_value = get_m307_p_precipitation_type_value;
    // precipitation_type->next = snow_depth;

    // point *electric_field = create_point("ElecFld", pt_int16, 1);
    // asprintf(&electric_field->label, "Electric Field");
    // asprintf(&electric_field->units, "Vm");
    // electric_field->get_value = get_m307_p_electric_field_value;
    // electric_field->next = precipitation_type;

    // point *surface_wetness = create_point("SurWet", pt_int16, 1);
    // asprintf(&surface_wetness->label, "Surface Wetness");
    // asprintf(&surface_wetness->units, "kO");
    // surface_wetness->get_value = get_m307_p_surface_wetness_value;
    // surface_wetness->next = electric_field;

    // point *soil_wetness = create_point("SoilWet", pt_int16, 1);
    // asprintf(&soil_wetness->label, "Soil Wetness");
    // asprintf(&soil_wetness->units, "Pct");
    // soil_wetness->get_value = get_m307_p_soil_wetness_value;
    // soil_wetness->next = surface_wetness;

    // point *wind_speed = create_point("WndSpd", pt_int16, 1);
    // asprintf(&wind_speed->label, "Wind Speed");
    // asprintf(&wind_speed->units, "mps");
    // wind_speed->get_value = get_m307_p_wind_speed_value;
    // wind_speed->next = surface_wetness;

    point *pressure = create_point("Pres", pt_int16, 1);
    asprintf(&pressure->label, "Barometric Pressure");
    asprintf(&pressure->units, "HPa");
    pressure->get_value = get_m307_p_pressure_value;
    pressure->next = NULL;

    // point *humidity = create_point("RH", pt_int16, 1);
    // asprintf(&humidity->label, "Relative Humidity");
    // asprintf(&humidity->units, "Pct");
    // humidity->get_value = get_m307_p_humidity_value;
    // humidity->next = pressure;

    point *temperature = create_point("TmpAmb", pt_int16, 1);
    asprintf(&temperature->label, "Ambient Temperature");
    temperature->sf = -1;
    asprintf(&temperature->units, "C");
    temperature->get_value = get_m307_p_temperature_value;
    temperature->next = pressure;

    point *md_length = create_point("L", pt_uint16, 1);
    asprintf(&md_length->desc, "Model length");
    asprintf(&md_length->label, "Model Length");
    md_length->_mandatory = mt_M;
    md_length->_static = st_S;
    md_length->get_value = get_m307_p_length_value;
    md_length->next = temperature;

    point *model_id = create_point("ID", pt_uint16, 1);
    asprintf(&model_id->desc, "Model identifier");
    asprintf(&model_id->label, "Model ID");
    model_id->_mandatory = mt_M,
    model_id->_static = st_S;
    model_id->get_value = get_m307_p_id_value;
    model_id->next = md_length;

    m307->group->count = 4;

    // associa o primeiro ponto ao ponto id
    m307->group->points = model_id;

    return m307;
}