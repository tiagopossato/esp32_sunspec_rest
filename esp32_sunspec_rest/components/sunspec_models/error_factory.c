#include "cJSON.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "sunspec_models.h"

bool new_error(cJSON *root, char *errCode,
               char *errMessage, char *errReason,
               bool debug, char *TBD)
{
    if ((root == NULL) ||
        (strlen(errCode) == 0) ||
        (strlen(errMessage) == 0) ||
        (strlen(errReason) == 0))
    {
        return false;
    }
    
    cJSON_AddStringToObject(root, "errCode", errCode);
    cJSON_AddStringToObject(root, "errMessage", errMessage);
    cJSON_AddStringToObject(root, "errReason", errReason);
    cJSON_AddBoolToObject(root, "debug", debug);
    if (debug)
    {
        cJSON_AddStringToObject(root, "TBD", TBD);
    }
    return true;
}