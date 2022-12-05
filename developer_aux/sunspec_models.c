#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sunspec_models.h"
// typedef char
//"required": ["name", "type", "size"],
typedef struct point
{
    char *name;
    point_type _type;
    char *(*get_value)();      //"type": ["integer", "string"]
    void (*set_value)(char *); //"type": ["integer", "string"]
    int count;
    int size;
    int sf; // "type": ["integer", "string"], "minimum": -10, "maximum": 10
    char *units;
    access_type _access;
    mandatory_type _mandatory;
    static_type _static;
    char *label;
    char *desc;
    char *detail;
    char *notes;
    // comments TODO
    // symbols TODO
    point *next;
} point;

//"required": ["name", "type"],
typedef struct group
{
    char *name;
    group_type type;
    int count;     //["integer", "string"], "default": 1
    point *points; //"type": "array",
    group *groups; //"type": "array",
    char *label;
    char *desc;
    char *detail;
    char *notes;
    // comments TODO
} group;

// "required": ["id", "group"],
typedef struct model
{
    uint16_t id;
    group *group;
    // char *label;
    // char *desc;
    // char *detail;
    // char *notes;
    // comments TODO
} model;

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
        char *dest = malloc(strlen(src));
        strcpy(dest, src);
        return dest;
    }
    return NULL;
}

// Aloca espaço e preenche os valores padrão
point *create_point(char *name, point_type type)
{
    point *p = (point *)malloc(sizeof(struct point));

    p->name = allocate_and_fill(name);
    p->_type = type;
    p->get_value = NULL;
    p->set_value = NULL;
    p->count = 0;
    p->size = 0;
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
}

void print_points(group *g)
{
    point *p = g->points;
    while (p != NULL)
    {
        printf("\t{\n\t\tdesc: %s, \n\t\tlabel: %s, \n\t\tmandatory: %s,\n\t\tname: %s,\n\t\tsize: %d, \n\t\tstatic: %s, \n\t\ttype: %s, \n\t\tvalue: %s, \n\t\tunits: %s, \n\t\taccess: %s, \n\t\tcount: %d, \n\t\tdetail: %s, \n\t\tnotes: %s, \n\t\tsf: %d\n\t}\n",
               p->desc, p->label, get_mandatory_type_name(p->_mandatory), p->name, p->size, get_static_type_name(p->_static), get_point_type_name(p->_type), p->get_value == NULL ? NULL : p->get_value(),
               p->units, get_access_type_name(p->_access), p->count, p->detail, p->notes, p->sf);

        p = p->next;
    }
}

void print_group(model *m)
{
    group *g = m->group;
    printf("\n\tname: %s, \n\ttype: %s, \n\tcount: %d, \n\tlabel: %s, \n\tdesc: %s, \n\tdetail: %s, \n\tnotes: %s\n\tpoints: [\n",
           g->name, get_group_type_name(g->type), g->count,
           g->label, g->desc, g->detail, g->notes);
    print_points(g);
    printf("\t]\n");
}

void print_model(model *m)
{
    printf("{\n\tid: %d, \n\tgroup:{", m->id);
    print_group(m);

    printf("}\n");
}

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
    model *m1 = (model *)malloc(sizeof(struct model));
    if (m1 == NULL)
    {
        return NULL;
    }
    // associa o id do modelo
    m1->id = 1;

    // aloca espaço para a estrutura grupo
    m1->group = (group *)malloc(sizeof(struct group));

    // associa o tipo do grupo
    m1->group->type = gt_group;

    // alloca e preenche a descrição
    m1->group->desc = allocate_and_fill("All SunSpec compliant devices must include this as the first model");
    // alloca e preenche a label
    m1->group->label = allocate_and_fill("Common");

    // alloca e preenche o nome
    m1->group->name = allocate_and_fill("common");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeiro
    //              assim já é possível ir associando o ponteiro *next

    point *device_addres = create_point("DA", pt_uint16);
    device_addres->_access = at_RW;
    device_addres->desc = allocate_and_fill("Modbus device address");
    device_addres->label = allocate_and_fill("Device Address");
    device_addres->size = 1;
    device_addres->get_value = get_m1_p_device_adress_value;
    device_addres->set_value = set_m1_p_device_adress_value;
    device_addres->next = NULL; // TODO: apontar para o ponto correto
    set_m1_p_device_adress_value("1");

    point *manufacturer = create_point("Mn", pt_string);
    manufacturer->desc = allocate_and_fill("Well known value registered with SunSpec for compliance");
    manufacturer->label = allocate_and_fill("Manufacturer");
    manufacturer->_mandatory = mt_M;
    manufacturer->size = 16;
    manufacturer->_static = st_S;
    manufacturer->get_value = get_m1_p_manufacturer_value;
    manufacturer->next = device_addres;

    point *md_length = create_point("L", pt_uint16);
    md_length->desc = allocate_and_fill("Model length");
    md_length->label = allocate_and_fill("Model Length");
    md_length->_mandatory = mt_M;
    md_length->size = 1;
    md_length->_static = st_S;
    md_length->get_value = get_m1_p_length_value;
    md_length->next = manufacturer;

    point *model_id = create_point("ID", pt_uint16);
    model_id->desc = allocate_and_fill("Model identifier");
    model_id->label = allocate_and_fill("Model ID");
    model_id->_mandatory = mt_M,
    model_id->size = 1,
    model_id->_static = st_S;
    model_id->get_value = get_m1_p_id_value;
    model_id->next = md_length;

    // associa o primeiro ponto ao ponto id
    m1->group->points = model_id;

    return m1;
}

/**********************************************************/
/*********** FUNÇÕES ESPECÍFICAS DO MODELO 307, RETIRAR DAQUI*/
/**********************************************************/

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
    return "235";
}

model *init_model_307()
{
    // aloca espaço na memoria para a estrutura do modelo
    model *m307 = (model *)malloc(sizeof(struct model));
    if (m307 == NULL)
    {
        return NULL;
    }
    // associa o id do modelo
    m307->id = 307;

    // aloca espaço para a estrutura grupo
    m307->group = (group *)malloc(sizeof(struct group));

    // associa o tipo do grupo
    m307->group->type = gt_group;

    // alloca e preenche a descrição
    m307->group->desc = allocate_and_fill("Base Meteorological Model");
    // alloca e preenche a label
    m307->group->label = allocate_and_fill("Base Met");

    // alloca e preenche o nome
    m307->group->name = allocate_and_fill("base_met");

    // cria os pontos do modelo.
    // IMPORTANTE: para facilitar a programação, iniciar do último para o primeir0
    //              assim já é possível ir associando o ponteiro *next

    point *temperature = create_point("TmpAmb", pt_int16);
    temperature->label = allocate_and_fill("Ambient Temperature");
    temperature->sf = -1;
    temperature->size = 1;
    temperature->units = allocate_and_fill("C");
    temperature->get_value = get_m307_p_temperature_value;
    temperature->next = NULL;

    point *md_length = create_point("L", pt_uint16);
    md_length->desc = allocate_and_fill("Model length");
    md_length->label = allocate_and_fill("Model Length");
    md_length->_mandatory = mt_M;
    md_length->size = 1;
    md_length->_static = st_S;
    md_length->get_value = get_m307_p_length_value;
    md_length->next = temperature;

    point *model_id = create_point("ID", pt_uint16);
    model_id->desc = allocate_and_fill("Model identifier");
    model_id->label = allocate_and_fill("Model ID");
    model_id->_mandatory = mt_M,
    model_id->size = 1,
    model_id->_static = st_S;
    model_id->get_value = get_m307_p_id_value;
    model_id->next = md_length;

    // associa o primeiro ponto ao ponto id
    m307->group->points = model_id;

    return m307;
}