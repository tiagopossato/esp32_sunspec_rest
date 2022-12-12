#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"

#include "sunspec_models.h"

static SunSpec *sunspec;

void get_points_by_name(cJSON *root, model *m, char *point_name_csv);

SunSpec *init_sunspec()
{
    sunspec = (SunSpec *)malloc(sizeof(SunSpec));
    sunspec->first = NULL;
    return sunspec;
}

SunSpec *get_sunspec()
{
    return sunspec;
}

void add_model(SunSpec *sunspec, model *m)
{
    model *last = sunspec->first;
    if (last == NULL)
    {
        sunspec->first = m;
    }
    else
    {
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = m;
    }
}

char *get_point_type_name(point_type t)
{
    switch (t)
    {

    case pt_int16:
        return "int16";
    case pt_int32:
        return "int32";
    case pt_int64:
        return "int64";
    case pt_raw16:
        return "raw16";
    case pt_uint16:
        return "uint16";
    case pt_uint32:
        return "uint32";
    case pt_uint64:
        return "uint64";
    case pt_acc16:
        return "acc16";
    case pt_acc32:
        return "acc32";
    case pt_acc64:
        return "acc64";
    case pt_bitfield16:
        return "bitfield16";
    case pt_bitfield32:
        return "bitfield32";
    case pt_bitfield64:
        return "bitfield64";
    case pt_enum16:
        return "enum16";
    case pt_enum32:
        return "enum32";
    case pt_float32:
        return "float32";
    case pt_float64:
        return "float64";
    case pt_string:
        return "string";
    case pt_sf:
        return "sf";
    case pt_pad:
        return "pad";
    case pt_ipaddr:
        return "ipaddr";
    case pt_ipv6addr:
        return "ipv6addr";
    case pt_eui48:
        return "eui48";
    case pt_sunssf:
        return "sunssf";
    case pt_count:
        return "count";

    default:
        return "unknown";
    }
}

char *get_group_type_name(group_type t)
{
    switch (t)
    {
    case gt_group:
        return "group";
    case gt_sync:
        return "sync";
    default:
        return "unknown";
    }
}

char *get_access_type_name(access_type t)
{
    switch (t)
    {
    case at_R:
        return "R";
    case at_RW:
        return "RW";
    default:
        return "unknown";
    }
}

char *get_mandatory_type_name(mandatory_type t)
{
    switch (t)
    {
    case mt_M:
        return "M";
    case mt_O:
        return "O";
    default:
        return "unknown";
    }
}

char *get_static_type_name(static_type t)
{
    switch (t)
    {
    case st_D:
        return "D";
    case st_S:
        return "S";
    default:
        return "unknown";
    }
}

// Aloca espaço e preenche os valores padrão
point *create_point(char *name, point_type type, int size)
{
    point *p = (point *)calloc(1, sizeof(struct point));

    asprintf(&p->name, name);
    p->_type = type;
    p->get_value = NULL;
    p->set_value = NULL;
    p->count = 0;
    p->size = size;
    p->sf = 0;
    p->units = NULL;
    p->_access = at_R;
    p->_mandatory = mt_O;
    p->_static = st_D;
    p->label = NULL;
    p->desc = NULL;
    p->detail = NULL;
    p->notes = NULL;
    p->next = NULL;

    return p;
}

group *create_group(char *name, point_type type)
{
    group *g = (group *)calloc(1, sizeof(struct group));
    asprintf(&g->name, name);
    g->type = type;
    g->count = 1; //["integer", "string"], "default": 1
    g->points = NULL;
    // g->groups = NULL;
    g->label = NULL;
    g->desc = NULL;
    g->detail = NULL;
    g->notes = NULL;

    return g;
}

model *create_model(uint16_t id)
{
    model *m = (model *)malloc(sizeof(struct model));
    m->id = id;
    m->count = 1;
    m->group = NULL;
    m->next = NULL;

    return m;
}

void point_to_cjson(cJSON *root, point *p)
{
    if (root == NULL)
    {
        return;
    }
    // access_type _access;
    if (p->_access == at_RW)
    {
        cJSON_AddStringToObject(root, "access", get_access_type_name(p->_access));
    }
    // char *desc;
    if (p->desc != NULL)
    {
        cJSON_AddStringToObject(root, "desc", p->desc);
    }
    // char *label;
    if (p->label != NULL)
    {
        cJSON_AddStringToObject(root, "label", p->label);
    }
    // mandatory_type _mandatory;
    if (p->_mandatory == mt_M)
    {
        cJSON_AddStringToObject(root, "mandatory", get_mandatory_type_name(p->_mandatory));
    }
    // char *name;
    // required
    cJSON_AddStringToObject(root, "name", p->name);
    // int size;
    // required
    cJSON_AddNumberToObject(root, "size", p->size);
    // static_type _static;
    if (p->_static == st_S)
    {
        cJSON_AddStringToObject(root, "static", get_static_type_name(p->_static));
    }
    // point_type _type;
    // required
    cJSON_AddStringToObject(root, "type", get_point_type_name(p->_type));

    // char *(*get_value)();      //"type": ["integer", "string"]
    if (p->get_value != NULL)
    {
        // TODO: tratar corretamente todos os casos
        switch (p->_type)
        {
        case pt_int16:
        case pt_int32:
        case pt_int64:
        case pt_raw16:
        case pt_uint16:
        case pt_uint32:
        case pt_uint64:
        case pt_float32:
        case pt_float64:
        case pt_sf:
        case pt_count:
            cJSON_AddNumberToObject(root, "value", strtod(p->get_value(), NULL));
            break;
        case pt_acc16:
        case pt_acc32:
        case pt_acc64:
        case pt_bitfield16:
        case pt_bitfield32:
        case pt_bitfield64:
        case pt_enum16:
        case pt_enum32:
        case pt_string:
        case pt_pad:
        case pt_ipaddr:
        case pt_ipv6addr:
        case pt_eui48:
        case pt_sunssf:
            cJSON_AddStringToObject(root, "value", p->get_value());
            break;
        }
    }
    // int count;
    if (p->count != 0)
    {
        cJSON_AddNumberToObject(root, "count", p->count);
    }

    // int sf; // "type": ["integer", "string"], "minimum": -10, "maximum": 10
    if (p->sf != 0)
    {
        cJSON_AddNumberToObject(root, "sf", p->sf);
    }
    // char *units;
    if (p->units != NULL)
    {
        cJSON_AddStringToObject(root, "units", p->units);
    }

    // char *detail;
    if (p->detail != NULL)
    {
        cJSON_AddStringToObject(root, "detail", p->detail);
    }
    // char *notes;
    if (p->notes != NULL)
    {
        cJSON_AddStringToObject(root, "notes", p->notes);
    }
}

void group_to_cjson(cJSON *root, group *g)
{
    if (root == NULL)
    {
        return;
    }
    // char *desc;
    if (g->desc != NULL)
    {
        cJSON_AddStringToObject(root, "desc", g->desc);
    }
    // char *label;
    if (g->label != NULL)
    {
        cJSON_AddStringToObject(root, "label", g->label);
    }
    // char *name;
    // required
    cJSON_AddStringToObject(root, "name", g->name);

    // point_type _type;
    // required
    cJSON_AddStringToObject(root, "type", get_group_type_name(g->type));
    // int count;     //["integer", "string"], "default": 1
    if (g->count != 1)
    {
        cJSON_AddNumberToObject(root, "count", g->count);
    }

    // char *detail;
    if (g->detail != NULL)
    {
        cJSON_AddStringToObject(root, "detail", g->detail);
    }
    // char *notes;
    if (g->notes != NULL)
    {
        cJSON_AddStringToObject(root, "notes", g->notes);
    }

    // point *points; //"type": "array",
    cJSON *points = cJSON_AddArrayToObject(root, "points");
    cJSON *json_point;
    point *p = g->points;

    while (p != NULL)
    {
        json_point = cJSON_CreateObject();
        point_to_cjson(json_point, p);
        cJSON_AddItemToArray(points, json_point);
        p = p->next;
    }

    // group *groups; //"type": "array", TODO: implementar
}

void model_to_cjson(cJSON *root, model *m, bool summary)
{
    if (root == NULL)
    {
        return;
    }
    if (summary)
    {
        cJSON_AddStringToObject(root, "name", m->group->name);
        cJSON_AddNumberToObject(root, "id", m->id);
        cJSON_AddNumberToObject(root, "count", m->count); //number of instances of the model
        return;
    }
    // uint16_t id;
    cJSON_AddNumberToObject(root, "id", m->id);

    group_to_cjson(root, m->group);
}

void sunspec_to_cjson(cJSON *root, SunSpec *suns, bool summary)
{
    if (root == NULL)
    {
        return;
    }
    cJSON *models = cJSON_AddArrayToObject(root, "models");

    cJSON *json_model;
    model *m = suns->first;

    while (m != NULL)
    {
        json_model = cJSON_CreateObject();
        model_to_cjson(json_model, m, summary);
        cJSON_AddItemToArray(models, json_model);
        m = m->next;
    }
}

bool get_model_cjson_by_id(cJSON *root, SunSpec *suns, uint16_t model_id)
{
    if (root == NULL)
    {
        return false;
    }
    model *m = suns->first;
    if (m == NULL)
    {
        return false;
    }
    cJSON *models = cJSON_AddArrayToObject(root, "models");
    cJSON *json_model;

    while (m != NULL)
    {
        if (m->id == model_id)
        {
            // model_to_cjson(root, m, false);
            json_model = cJSON_CreateObject();
            model_to_cjson(json_model, m, false);
            cJSON_AddItemToArray(models, json_model);
            return true;
        }
        m = m->next;
    }
    return false;
}

void get_points_by_name(cJSON *root, model *m, char *point_name_csv)
{
    point *p;
    char *token;
    const char separator[2] = ",";

    // ESP_LOGI("TAG", "point_name_csv: %s", point_name_csv);
    // take first token
    token = strtok(point_name_csv, separator);

    while (token != NULL)
    {
        // take first point
        p = m->group->points;
        while (p != NULL)
        {
            if (strcmp(p->name, token) == 0)
            {
                if (p->get_value != NULL)
                {
                    // TODO: tratar corretamente todos os casos
                    switch (p->_type)
                    {
                    case pt_int16:
                    case pt_int32:
                    case pt_int64:
                    case pt_raw16:
                    case pt_uint16:
                    case pt_uint32:
                    case pt_uint64:
                    case pt_float32:
                    case pt_float64:
                    case pt_sf:
                    case pt_count:
                        cJSON_AddNumberToObject(root, p->name, strtod(p->get_value(), NULL));
                        break;
                    case pt_acc16:
                    case pt_acc32:
                    case pt_acc64:
                    case pt_bitfield16:
                    case pt_bitfield32:
                    case pt_bitfield64:
                    case pt_enum16:
                    case pt_enum32:
                    case pt_string:
                    case pt_pad:
                    case pt_ipaddr:
                    case pt_ipv6addr:
                    case pt_eui48:
                    case pt_sunssf:
                        cJSON_AddStringToObject(root, p->name, p->get_value());
                        break;
                    }
                }
            }
            // take next point
            p = p->next;
        }
        // take next token
        token = strtok(NULL, separator);
    }
}

bool get_model_cjson_points_by_name(cJSON *root, SunSpec *suns, uint16_t model_id, char *point_name_csv)
{
    if (root == NULL)
    {
        return false;
    }
    model *m = suns->first;
    if (m == NULL)
    {
        return false;
    }
    cJSON *models = cJSON_AddArrayToObject(root, "models");
    cJSON *json_model;

    while (m != NULL)
    {
        if (m->id == model_id)
        {
            json_model = cJSON_CreateObject();
            cJSON_AddStringToObject(json_model, "name", m->group->name);
            cJSON_AddNumberToObject(json_model, "id", m->id);
            get_points_by_name(json_model, m, point_name_csv);
            cJSON_AddItemToArray(models, json_model);
            return true;
        }
        m = m->next;
    }
    return false;
}
