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
    return m;
}

void print_points(group *g)
{
    point *p = g->points;
    while (p != NULL)
    {
        printf("\t{\n\t\tdesc: %s, \n\t\tlabel: %s, \n\t\tmandatory: %s,\n\t\tname: %s,\n\t\tsize: %d, \n\t\tstatic: %s, \n\t\ttype: %s, \n\t\tvalue: %s, \n\t\tunits: %s, \n\t\taccess: %s, \n\t\tcount: %d, \n\t\tdetail: %s, \n\t\tnotes: %s, \n\t\tsf: %d\n\t}\n",
               p->desc == NULL ? "_" : p->desc, p->label == NULL ? "_" : p->label, get_mandatory_type_name(p->_mandatory), p->name == NULL ? "_" : p->name, p->size, get_static_type_name(p->_static), get_point_type_name(p->_type), p->get_value == NULL ? NULL : p->get_value(),
               p->units == NULL ? "_" : p->units, get_access_type_name(p->_access), p->count, p->detail == NULL ? "_" : p->detail, p->notes == NULL ? "_" : p->notes, p->sf);

        p = p->next;
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