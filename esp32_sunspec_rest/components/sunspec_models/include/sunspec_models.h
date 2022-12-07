#pragma once

#include <stdbool.h>
#include "cJSON.h"

typedef enum
{
    pt_int16,
    pt_int32,
    pt_int64,
    pt_raw16,
    pt_uint16,
    pt_uint32,
    pt_uint64,
    pt_acc16,
    pt_acc32,
    pt_acc64,
    pt_bitfield16,
    pt_bitfield32,
    pt_bitfield64,
    pt_enum16,
    pt_enum32,
    pt_float32,
    pt_float64,
    pt_string,
    pt_sf,
    pt_pad,
    pt_ipaddr,
    pt_ipv6addr,
    pt_eui48,
    pt_sunssf,
    pt_count
} point_type;

typedef enum
{
    gt_group,
    gt_sync
} group_type;

typedef enum
{
    at_R,
    at_RW
} access_type;

typedef enum
{
    mt_M,
    mt_O
} mandatory_type;

typedef enum
{
    st_D,
    st_S
} static_type;

// struturas opacas definidas abaixo
typedef struct point point;
typedef struct group group;
typedef struct model model;

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
    //group *groups; //"type": "array", TODO: implementar
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
    model *next;
} model;

typedef struct SunSpec
{
    model *first;
} SunSpec;

point *create_point(char *name, point_type type, int size);
group *create_group(char *name, point_type type);
model *create_model(uint16_t id);

char *allocate_and_fill(char *src);

char *get_point_type_name(point_type);
char *get_group_type_name(group_type);
char *get_access_type_name(access_type);
char *get_mandatory_type_name(mandatory_type);
char *get_static_type_name(static_type);

/**
 * Recebe um objeto cJSON e insere os valores do point
*/
void point_to_cjson(cJSON *root, point *p);

void group_to_cjson(cJSON *root, group *g);

void model_to_cjson(cJSON *root, model *m, bool summary);

void sunspec_to_cjson(cJSON *root, SunSpec *suns, bool summary);

/**
 * busca um modelo pelo id
 * retorna true e o modelo se encontrado
 * retorna false e o modelo nulo se não encontrado
*/
bool get_model_cjson_by_id(cJSON *root, SunSpec *suns, uint16_t model_id);

bool get_model_cjson_points_by_name(cJSON *root, SunSpec *suns, uint16_t model_id, char *point_name_list[]);