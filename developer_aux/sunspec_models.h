#pragma once
#include <stdbool.h>

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


char *get_point_type_name(point_type);
char *get_group_type_name(group_type);
char *get_access_type_name(access_type);
char *get_mandatory_type_name(mandatory_type);
char *get_static_type_name(static_type);


// struturas opacas definidas dentro do arquivo de cada modelo
typedef struct point point;
typedef struct group group;
typedef struct model model;

void print_model(model *);

model * init_model_1();

model * init_model_307();