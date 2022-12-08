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
    m307->group->desc = allocate_and_fill("Base Meteorological Model");
    // alloca e preenche a label
    m307->group->label = allocate_and_fill("Base Met");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeir0
    //              assim já é possível ir associando o ponteiro *next

    point *wind_direction = create_point("WndDir", pt_int16, 1);
    wind_direction->label = allocate_and_fill("Wind Direction");
    wind_direction->units = allocate_and_fill("deg");
    wind_direction->get_value = get_m307_p_wind_direction_value;
    wind_direction->next = NULL;

    point *rainfall = create_point("Rain", pt_int16, 1);
    rainfall->label = allocate_and_fill("Rainfall");
    rainfall->units = allocate_and_fill("mm");
    rainfall->get_value = get_m307_p_rainfall_value;
    rainfall->next = wind_direction;

    point *snow_depth = create_point("Snw", pt_int16, 1);
    snow_depth->label = allocate_and_fill("Snow Depth");
    snow_depth->units = allocate_and_fill("mm");
    snow_depth->get_value = get_m307_p_snow_depth_value;
    snow_depth->next = rainfall;

    point *precipitation_type = create_point("PPT", pt_int16, 1);
    precipitation_type->label = allocate_and_fill("Precipitation Type");
    precipitation_type->desc = allocate_and_fill("Precipitation Type (WMO 4680 SYNOP code reference)");
    precipitation_type->get_value = get_m307_p_precipitation_type_value;
    precipitation_type->next = snow_depth;

    point *electric_field = create_point("ElecFld", pt_int16, 1);
    electric_field->label = allocate_and_fill("Electric Field");
    electric_field->units = allocate_and_fill("Vm");
    electric_field->get_value = get_m307_p_electric_field_value;
    electric_field->next = precipitation_type;

    point *surface_wetness = create_point("SurWet", pt_int16, 1);
    surface_wetness->label = allocate_and_fill("Surface Wetness");
    surface_wetness->units = allocate_and_fill("kO");
    surface_wetness->get_value = get_m307_p_surface_wetness_value;
    surface_wetness->next = electric_field;

    point *soil_wetness = create_point("SoilWet", pt_int16, 1);
    soil_wetness->label = allocate_and_fill("Soil Wetness");
    soil_wetness->units = allocate_and_fill("Pct");
    soil_wetness->get_value = get_m307_p_soil_wetness_value;
    soil_wetness->next = surface_wetness;

    point *wind_speed = create_point("WndSpd", pt_int16, 1);
    wind_speed->label = allocate_and_fill("Wind Speed");
    wind_speed->units = allocate_and_fill("mps");
    wind_speed->get_value = get_m307_p_wind_speed_value;
    wind_speed->next = surface_wetness;

    point *pressure = create_point("Pres", pt_int16, 1);
    pressure->label = allocate_and_fill("Barometric Pressure");
    pressure->units = allocate_and_fill("HPa");
    pressure->get_value = get_m307_p_pressure_value;
    pressure->next = wind_speed;

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

    m307->group->count = 13;

    // associa o primeiro ponto ao ponto id
    m307->group->points = model_id;

    return m307;
}