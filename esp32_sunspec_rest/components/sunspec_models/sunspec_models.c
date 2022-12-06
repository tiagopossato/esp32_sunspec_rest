#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sunspec_models.h"

void print_points(group *);
void print_group(model *);

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

char *allocate_and_fill(char *src)
{
    if (src != NULL)
    {
        char *dest = malloc(strlen(src) + 1); //+1 para o \0
        strcpy(dest, src);
        return dest;
    }
    return NULL;
}

// Aloca espaço e preenche os valores padrão
point *create_point(char *name, point_type type, int size)
{
    point *p = (point *)calloc(1, sizeof(struct point));

    p->name = allocate_and_fill(name);
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
    g->name = allocate_and_fill(name);
    g->type = type;
    g->count = 1; //["integer", "string"], "default": 1
    g->points = NULL;
    g->groups = NULL;
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
    // char *name;
    // required
    cJSON_AddStringToObject(root, "name", p->name);
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
    // int size;
    // required
    cJSON_AddNumberToObject(root, "size", p->size);

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

    // access_type _access;
    // if (p->_access == at_RW)
    // {
    cJSON_AddStringToObject(root, "access", get_access_type_name(p->_access));
    // }
    // mandatory_type _mandatory;
    // if (p->_mandatory == mt_M)
    // {
    cJSON_AddStringToObject(root, "mandatory", get_mandatory_type_name(p->_mandatory));
    // }
    // static_type _static;
    // if (p->_static == st_S)
    // {
    cJSON_AddStringToObject(root, "static", get_static_type_name(p->_static));
    // }
    // char *label;
    if (p->label != NULL)
    {
        cJSON_AddStringToObject(root, "label", p->label);
    }
    // char *desc;
    if (p->desc != NULL)
    {
        cJSON_AddStringToObject(root, "desc", p->desc);
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

void print_points(group *g)
{
    point *p = g->points;
    cJSON *root;
    root = cJSON_CreateObject();
    while (p != NULL)
    {
        point_to_cjson(root, p);
        char *my_json_string = cJSON_Print(root);
        printf("%s\n", my_json_string);
        p = p->next;
        if (p != NULL)
            root = cJSON_CreateObject();
    }
    if (root != NULL)
    {
        cJSON_Delete(root);
    }
}

void print_group(model *m)
{
    group *g = m->group;
    printf("\n\tname: %s, \n\ttype: %s, \n\tcount: %d, \n\tlabel: %s",
           g->name == NULL ? "_" : g->name, get_group_type_name(g->type), g->count, g->label == NULL ? "_" : g->label);

    printf("\n\tdesc: %s, \n\tdetail: %s, \n\tnotes: %s\n\tpoints: [\n",
           g->desc == NULL ? "_" : g->desc, g->detail == NULL ? "_" : g->detail, g->notes == NULL ? "_" : g->notes);
    print_points(g);
    printf("\t]\n");
}

void print_model(model *m)
{
    printf("{\n\tid: %d, \n\tgroup:{", m->id);
    print_group(m);

    printf("}\n");
}