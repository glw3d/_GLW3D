 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Methods for importing NURBS curves and surfaces from an IGES file

*******************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nurbs_internal.h"
#include "nurbs_curve.h"
#include "nurbs_surface.h"

#include "domino_nurbs.h" /* TAKE THIS OUT */

/* Parameters readed from the global section */
typedef struct 
{
    char parameter_delimiter_character;         /* 1 */
    char record_delimiter_character;            /* 2 */
    char product_identification[256];           /* 3 */
    char filename[256];                         /* 4 */
    char native_system_id[256];                 /* 5 */
    char preprocessor_version[256];             /* 6 */
    int num_bits_integer;                       /* 7 */
    int max_power_single;                       /* 8 */
    int number_significat_bits_single;          /* 9 */
    int max_power_double;                       /* 10 */
    int number_significat_bits_double;          /* 11 */
    char product_identification_receiving[256]; /* 12 */
    float model_space_scale;                   /* 13 */
    int units_flag;                             /* 14 */
    char units_name[256];                       /* 15 */
    int num_line_weights_gradation;             /* 16 */
    float max_line_weight;                     /* 17 */
    char file_data_time_generation[256];        /* 18 */
    float granularity;                         /* 19 */
    float max_ccordinate_value;                /* 20 */
    char authors_name[256];                     /* 21 */
    char authors_organization[256];             /* 22 */
    int file_version;                           /* 23 */
    int drafting_standard;                      /* 24 */
    char file_data_time_modification[256];      /* 25 */
    char application_protocol[256];             /* 26 */

}GlobalSectionFields;


typedef struct 
{
    int entity_type_number; /* 126 nurbs_curve; 128 nurbs_surface */
    int data_line;          /* line in the IGES file with the parameter data */
    int form;               /* 0 is for rational B-spline parameters */
    char label[9];          /* Entity label */
    int subscript;          /* Entity subscript number */
    NurbsCurve* curve;      /* if it is a curve, data is stored here */
    NurbsSurface* surface;  /* if it is a surface, data is stored here*/
}Entity;


/******************************************************************************/

typedef struct _DynamicStack
{
    struct _DynamicStack *next; /* Pointer to the next element of the stack */
    struct _DynamicStack *from; /* Pointer to the previous element */
    void *data;                 /* Pointer to the data */
} DynamicStack;

/** Creates a token */
static DynamicStack* dynamic_stack_create_token(void)
{
    DynamicStack* token;
    _check_(token = (DynamicStack*)_malloc_(sizeof(DynamicStack)));
    token->data = nullptr;
    token->next = nullptr;

    return token;
}

/**
 * Releases the memory used by the stack structure, but NOT the data
 * To clear the data uses the optional function pointer
 */
static void dynamic_stack_free
    ( DynamicStack *root, void (*data_free_handler)(DynamicStack* node))
{
    DynamicStack* token = root;
    DynamicStack* next;

    while (token != nullptr){
        next = (DynamicStack*)token->next;
        if (data_free_handler != nullptr){
            (*data_free_handler)(token);
        }
        free(token);
        token = next;
    }
}

/******************************************************************************/

/* Safe version of the strcat routine */
static void strcat_safe(char* dest, char* source, int dest_size)
{
    int size = strlen(dest);
    char* ps = source;
    char* pd = dest + size;

    size += 1;
    while (*ps != '\n' && size < dest_size){
        *pd++ = *ps++;
        dest++;
        size++;
    }
    *dest = '\0';
}

/** Eliminates spaces, tabs and line breaks at the beginning and the end
  * of the string */
static char* str_trim(char *str)
{
  size_t length;
  char *p1;
  char *p0;

    if (str == nullptr)
        return str;

  length = strlen(str);

  // cut the right
  for (p1 = &str[length - 1]
        ; (*p1 == ' ' || *p1 == '\t' || *p1 == '\n' || *p1 == '\r')
          && p1 >= &str[0];)
    *p1-- = '\0';

  // cut the left
  for (p1 = &str[0]
        ; (*p1 == ' ' || *p1 == '\t' || *p1 == '\n' || *p1 == '\r')
            && p1 <= &str[length]; p1++);

  // move the uncut content to the beginning of the string
  for (p0 = str; *p1 != '\0'; p0++, p1++)
    *p0 = *p1;

    *p0 ='\0';

    return str;
}

/* Gets the letter code in the line */
static char getLetterCode(const char* line)
{
    if (line != nullptr){
        return line[72];
    }
    else{
        return '\0';
    }
}

/* Reads a string data type in Hollerith format, e.g. 3Habcd */
static char* readHollerithString(char* input_string, char** text)
{
    char* pt = input_string;
    char* end;

    int hlen, slen;

    /* Look for the 'H' character */
    while (*pt != '\0' && *pt != 'H' && *pt != 'h'){
        pt++;
    }

    if (*pt == '\0'){
        /* The string does not contains an H */
        *text = pt;
        return pt;
    }

    *pt = '\0';
    sscanf(input_string, "%i", &hlen);
    pt++;
    slen = strlen(pt);

    if (slen > hlen){
        pt[hlen] = '\0';
        end = pt + hlen + 1;
    }
    else{
        end = pt + hlen;
    }

    *text = pt;

    return end;
}


/* Extracts one data  */
static char* parseData(char* pt
        , char* value_buffer
        , int* end_of_param
        , int* end_of_record
        , int* str_len
        , const char param_delimiter
        , const char record_delimiter
){
    char* p1 = pt;

    *end_of_record = 0;

    if (*str_len > 0){
        /* We are already reading a string in Hollerith notation */
        while(*p1 != '\0' && *p1 != '\n' && *str_len > 0){
            (*str_len)--;
            p1++;
        }
    }
    else{
        /* Find the next delimiter or Hollerith or end of line */
        while(*p1 != param_delimiter && *p1 != record_delimiter 
                && *p1 != '\0' && *p1 != '\n' && *p1 != 'H' && *p1 != 'h'){
            p1++;
        }
    }

    if (*p1 == '\0'){
        /* End of line. Continue with the next one? */
        strcat_safe(value_buffer, pt, 256);
        *end_of_param = 0;
        return p1;
    }
    else if (*p1 == 'H' || *p1 == 'h'){
        /* String in Hollerith notation */
        if (sscanf(pt, "%i", str_len) == 0){
            *str_len = 0;
        }
        *end_of_param = 0;
        return p1+1;
    }
    else if (*p1 == record_delimiter){
        *end_of_record = 1;
    }
    
    if (*p1 != param_delimiter && *p1 != record_delimiter){
        /* Uh!? something is terribly wrong! */
        while(*p1 != param_delimiter && *p1 != record_delimiter 
                && *p1 != '\0' && *p1 != '\n'){
            p1++;
        }
        if (*p1 == '\0'){
            *end_of_param = 0;
            return p1;
        }
    }

    *p1 = '\0';
    p1++;
    *end_of_param = 1;

    /* Copy the string */
    strcat_safe(value_buffer, pt, 256);

    return p1;
}


/* Get the next raw parameter value */
static char* getValue(FILE* fd  /* File handler */
        , char* *status         /* If status == NULL we are at the end of file */
        , char* pt              /* Jumping pointer through the parameters */
        , char* line_buffer     /* Buffer used to read lines     */
        , char* value_buffer    /* Returned pointer value */
        , int* end_of_record    /* Indicates if the end of record character is found */
        , const char parameter_delimiter_character  /* Delimiter */
        , const char record_delimiter_character     /* Delimiter */
        , const int line_length /* Where the line should be cut */
){
    int end_of_parameter = 0;
    int str_len = 0;    /* Size of the string taken from the Hollerith */

    *end_of_record = 0;

    /* Initialize value */
    value_buffer[0] = '\0';
    /* Cut the line to get only the data we are interested */
    line_buffer[line_length+1] = '\0';

    while (end_of_parameter == 0 && pt != nullptr && *status != NULL){
        pt =  parseData
              ( pt, &(value_buffer[0])
              , &end_of_parameter, end_of_record
              , &str_len
              , parameter_delimiter_character
              , record_delimiter_character
              );

        if (pt[0] == '\0'){
            /* End of line */
            *status = fgets(line_buffer, 127, fd);
            if (*status == NULL){
                /* End of file (something is terribly wrong!) */
                return nullptr;
            }
            pt = line_buffer;
        }
    }

    return pt;
}

/* Reads the start section of an ASCII IGES file */
static void readStartSectionASCII(FILE* fd
        , char* *status
        , char* line_buffer)
{
    char letter_code;

    letter_code = getLetterCode(line_buffer);
    /* Advance until we find the start section */
    while(*status != NULL && letter_code != 'S'){        
        *status = fgets(line_buffer, 127, fd);
        letter_code = getLetterCode(line_buffer);
    }

    while(*status != NULL && letter_code == 'S'){
        /* Not much to do */
        *status = fgets(line_buffer, 127, fd);
        letter_code = getLetterCode(line_buffer);
    }
}


/* Reads the global section from an ASCII IGES file */
static void readGlobalSectionASCII(FILE* fd
    , char* *status
    , char* line_buffer
    , GlobalSectionFields* global
){
    char* pt = line_buffer;
    char* ph;
    char letter_code;
    char value_buffer[256];
    char pdc, rdc;
    int end_of_record;

    letter_code = getLetterCode(line_buffer);
    /* Advance until we find the global section */
    while(*status != NULL && letter_code != 'G'){    
        *status = fgets(line_buffer, 127, fd);
        letter_code = getLetterCode(line_buffer);
    }

    /* Set default parameters */
    global->parameter_delimiter_character = ',';
    global->record_delimiter_character = ';';

    if (*status == NULL){
        return;
    }

    /* The first parameter should be taken to know the separator character */
    pt = readHollerithString(line_buffer, &ph);
    if (ph[0] != '\0'){
        global->parameter_delimiter_character = ph[0];
    }

    /* And the second one, which is the record delimiter character */
    pt = readHollerithString(pt, &ph);
    if (ph[0] != '\0'){
        global->record_delimiter_character = ph[0];
    }

    pdc = global->parameter_delimiter_character;
    rdc = global->record_delimiter_character;

    /* Read the rest of the global section */
    pt = getValue(fd, status, pt, line_buffer
        , &(global->product_identification[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->filename[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->native_system_id[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->preprocessor_version[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->num_bits_integer);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->max_power_single);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->number_significat_bits_single);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->max_power_double);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->number_significat_bits_double);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->product_identification_receiving[0]), &end_of_record
        , pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%g", &global->model_space_scale);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->units_flag);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->units_name[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->num_line_weights_gradation);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%g", &global->max_line_weight);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->file_data_time_generation[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%g", &global->granularity);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%g", &global->max_ccordinate_value);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->authors_name[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->authors_organization[0]), &end_of_record, pdc, rdc, 71);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%g", &global->model_space_scale);

    pt = getValue(fd, status, pt, line_buffer
        , &(value_buffer[0]), &end_of_record, pdc, rdc, 71);
    sscanf(value_buffer, "%i", &global->file_version);

    pt = getValue(fd, status, pt, line_buffer
        , &(global->file_data_time_modification[0]), &end_of_record, pdc, rdc, 71);

    // Seems that some files do not have the last one
    if (end_of_record == 1) return;
    pt = getValue(fd, status, pt, line_buffer
          , &(global->application_protocol[0]), &end_of_record, pdc, rdc, 71);
}


/* Arranges the entities by the parameter line */
static void arrangeEntities(Entity* entities, const int num_entities)
{
    int i;
    int flag = 1;
    Entity buffer;

    while (flag == 1){
        flag = 0;
        for (i = 0; i < num_entities-1; i++){
            if (entities[i].data_line > entities[i+1].data_line){
                /* Swap */
                flag = 1;
                buffer = entities[i];
                entities[i] = entities[i+1];
                entities[i+1] = buffer;
            }
        }
    }
}


/* Reads the directory section */
static void readDirectorySectionASCII(FILE* fd
    , char* *status
    , char* line_buffer
    , Entity** entities
    , int *num_entities
){
    int i, j, k, entity_type;
    char* line;
    char* field[20];
    char letter_code;
    DynamicStack* root;
    DynamicStack* token;

    letter_code = getLetterCode(line_buffer);
    /* Advance until we find the directory section */
    while(*status != NULL && letter_code != 'D'){        
        *status = fgets(line_buffer, 127, fd);
        letter_code = getLetterCode(line_buffer);
    }

    if (*status == NULL){
        /* THERE IS NO DIRECToRY SECTION!!! */
        return;
    }

    *num_entities = 0;
    root = dynamic_stack_create_token();
    token = root;
    letter_code = getLetterCode(line_buffer);

    while(*status != NULL && letter_code == 'D'){
        /* Create a new token */
        _check_(token->data = _malloc_(sizeof(char) * 20 * 9));
        line = (char*)token->data;
        token->next = dynamic_stack_create_token();
        token = token->next;

        k = 0;
        /* It should be two lines for each directory entry */
        for (i = 0; i < 2 && letter_code == 'D'; i++){
            /* There are always 10 fields of 8 characteres 
             * and a total of 80 characters per line */
            for (j = 0; j < 10; j++){
                strncpy(&(line[k * 9]), &(line_buffer[j * 8]), 8);
                line[k * 9 + 8] = '\0';
                k++;
            }

            /* Read the next line */
            *status = fgets(line_buffer, 127, fd);
            letter_code = getLetterCode(line_buffer);
        }

        /* Check if it is a NURBS entity (entities 126 and 128) */
        sscanf(&(line[0]), "%i", &entity_type);
        if (entity_type == 126 || entity_type == 128){
            (*num_entities)++;
        }
    }

    /* Now we know how many entities we have */
    if (*num_entities <= 0){
        *entities = nullptr;
    }
    else{
        token = root;
        _check_(*entities = (Entity*)_malloc_(sizeof(Entity) * (*num_entities)));

        j = 0;
        while (token != nullptr){
            if (token->data != nullptr){
                line = (char*)token->data;
                for (i = 0; i < 20; i++){
                    field[i] = &(line[9*i]);
                }

                /* Check (again) if it is a nurbs surface or nurbs curve */
                sscanf(field[0], "%i", &entity_type);
                if (entity_type == 126 || entity_type == 128){
                    (*entities)[j].curve = nullptr;
                    (*entities)[j].surface = nullptr;
                    (*entities)[j].entity_type_number = entity_type;
                    sscanf(field[1], "%i", &((*entities)[j].data_line));
                    sscanf(field[14], "%i", &((*entities)[j].form));
                    strcpy((*entities)[j].label, field[17]);
                    sscanf(field[18], "%i", &((*entities)[j].subscript));
                    j++;
                }
            }
            token = token->next;
        }
    }

    /* Arrange the entities by the parameter line,  
     * so we can find them in the correct order */
    arrangeEntities(*entities, *num_entities);

    /* Clear the data memory */
    token = root;
    while (token != nullptr){
        if (token->data != nullptr){
            free(token->data);
        }
        token = token->next;
    }

    /* Release the stack */
    dynamic_stack_free(root, nullptr);
}

/* Reads all data parameters for a parametric curve */
static void parseCurveEntity
  ( Entity* entity
  , FILE* fd, char* *status
  , char* line_buffer
  , const char parameter_delimiter_character
  , const char record_delimiter_character
  )
{
    char* pt = &(line_buffer[0]);
    char value[128];
    int entity_type_number;
    int k, m, n, a, i;
    float fvalue;
    int end_of_record;

    /* Entity type number should be the same as in the directory */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &entity_type_number);

    if (entity_type_number != entity->entity_type_number){
        _handle_error_("type numbers are not the same!"); 
    }

    /* Upper Index of sum */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &k);

    /* Degree of basis function */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &m);

    n = 1+k-m;
    a = n+2*m;

    entity->curve = nurbs_curve_alloc(nullptr, k+1, m);

    /* Get the label and id */
    str_trim(entity->label);
    strcpy(entity->curve->label, entity->label);
    entity->curve->id = entity->subscript;

    /* Read property 0 - nonplanar, 1 - planar */
    pt = getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
          , parameter_delimiter_character, record_delimiter_character, 64);

    /* Read property 0 - open curve, 1 - closed curve */
    pt = getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Read property 0 - rational 1 - polynomial */
    pt = getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Read property 0 - nonperiodic 1 - periodic */
    pt = getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Read the knots */
    for (i = 0; i <= a; i++){
        pt =  getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
               , parameter_delimiter_character, record_delimiter_character, 64);

        sscanf(value, "%g", &fvalue);
        entity->curve->knot[i] = (NurbsFloat)fvalue;
    }

    /* Read the weights */
    for (i = 0; i <= k; i++){
        pt = getValue( fd, status, pt, line_buffer, &(value[0]), &end_of_record
              , parameter_delimiter_character, record_delimiter_character, 64);
        sscanf(value, "%g", &fvalue);
        entity->curve->cp[i].w = (NurbsFloat)fvalue;
    }

    /* Read the control point coordinates x y z */
    for (i = 0; i <= k; i++){
        pt =  getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
              , parameter_delimiter_character, record_delimiter_character, 64
              );
        sscanf(value, "%g", &fvalue);
        entity->curve->cp[i].x = (NurbsFloat)fvalue;
        pt =  getValue
              ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
              , parameter_delimiter_character, record_delimiter_character, 64
              );
        sscanf(value, "%g", &fvalue);
        entity->curve->cp[i].y = (NurbsFloat)fvalue;
        pt =  getValue
              ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
              , parameter_delimiter_character, record_delimiter_character, 64
              );
        sscanf(value, "%g", &fvalue);
        entity->curve->cp[i].z = (NurbsFloat)fvalue;
    }

    /* Read x norm */
    pt = getValue
        ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64
        );
    /* Read y norm */
    pt = getValue
        ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64
        );
    /* Read z norm */
    pt =  getValue
        ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64
        );
}


/* Reads all data parameters */
static void parseSurfaceEntity(Entity* entity
    , FILE* fd, char* *status
    , char* line_buffer
    , const char parameter_delimiter_character
    , const char record_delimiter_character)
{
    char* pt = &(line_buffer[0]);
    char value[128];
    int entity_type_number;
    int k1, k2, m1, m2, i, j;
    float fvalue;
    int end_of_record;

    /* Entity type number should be the same as in the directory */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &entity_type_number);
    if (entity_type_number != entity->entity_type_number){
        _handle_error_("type numbers are not the same!"); 
    }

    /* Upper index of first sum */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &k1);

    /* Upper index of second sum */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &k2);

    /* Degree of first basis function */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &m1);

    /* Degree of second basis function */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    sscanf(value, "%i", &m2);

    /* property: 0 - closed in the first parametric direction, 1 - not closed */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* property: 0 - closed in the second parametric direction, 1 - not closed */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* property: 0 - rational, 1 - polynomial */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* property: 0 - non periodic in the first parametric direction, 1 - periodic */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* property: 0 - non periodic in the second parametric direction, 1 - periodic */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    entity->surface = nurbs_surface_alloc(nullptr, 1+k1, 1+k2, m1, m2);

    /* Set the label and id */
    str_trim(entity->label);
    strcpy(entity->surface->label, entity->label);
    entity->surface->id = entity->subscript;

    /* Read the first knots sequence */
    for (i = 0; i < entity->surface->knot_length_u; i++){
        pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
            , parameter_delimiter_character, record_delimiter_character, 64);
        sscanf(value, "%g", &fvalue);
        entity->surface->knot_u[i] = (NurbsFloat)fvalue;
    }

    /* Read the second knots sequence */
    for (i = 0; i < entity->surface->knot_length_v; i++){
        pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
            , parameter_delimiter_character, record_delimiter_character, 64);
        sscanf(value, "%g", &fvalue);
        entity->surface->knot_v[i] = (NurbsFloat)fvalue;
    }

    /* Read the weights */
    for (i = 0; i < entity->surface->cp_length_u; i++){
        for (j = 0; j < entity->surface->cp_length_v; j++){
          pt = getValue
                ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
                , parameter_delimiter_character
                , record_delimiter_character, 64
                );
          sscanf(value, "%g", &fvalue);
          entity->surface->cp[i][j].w = (NurbsFloat)fvalue;
        }
    }

    /* Read the control points coordinates x y z */
    for (i = 0; i < entity->surface->cp_length_v; i++){
        for (j = 0; j < entity->surface->cp_length_u; j++){
            pt = getValue
                ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
                , parameter_delimiter_character
                , record_delimiter_character, 64
                );

            sscanf(value, "%g", &fvalue);
            entity->surface->cp[j][i].x = (NurbsFloat)fvalue;
            pt = getValue
                ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
                , parameter_delimiter_character
                , record_delimiter_character, 64
                );

            sscanf(value, "%g", &fvalue);
            entity->surface->cp[j][i].y = (NurbsFloat)fvalue;
            pt = getValue
                ( fd, status, pt, line_buffer, &(value[0]), &end_of_record
                , parameter_delimiter_character
                , record_delimiter_character, 64
                );

            sscanf(value, "%g", &fvalue);
            entity->surface->cp[j][i].z = (NurbsFloat)fvalue;
        }
    }

    /* Starting value for the first parametric direction */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Ending value for the first parametric direction */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Starting value for the second parametric direction */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);

    /* Ending value for the second parametric direction */
    pt = getValue(fd, status, pt, line_buffer, &(value[0]), &end_of_record
        , parameter_delimiter_character, record_delimiter_character, 64);
}


/* Copies a string */
static char* str_copy_label( const char* label )
{
    char* dest = nullptr;

    if (label == nullptr){
        return nullptr;
    }

    dest = (char*)malloc( sizeof(char) * (strlen(label) + 1) );
    strcpy(dest, label);

    return dest;
}


/* Reads the NURBS entities in the parameter section */
static void readParameterSectionASCII(FILE* fd
    , char** status
    , char* line_buffer
    , Entity* entities
    , const int num_entities
    , const char parameter_delimiter_character
    , const char record_delimiter_character
){
    char letter_code;       /* Letter code of the line */
    int line_number;        /* Line number readed from the IGES line */
    int entity_number = 0;  /* Entity indexer */

    letter_code = getLetterCode(line_buffer);
    /* Advance until we find the parameter section */
    while(*status != NULL && letter_code != 'P'){        
        *status = fgets(line_buffer, 127, fd);
        letter_code = getLetterCode(line_buffer);
    }

    while(*status != NULL && letter_code == 'P' && strlen(line_buffer) >= 80){        
        /* The line number is at the last characters */
        sscanf(&(line_buffer[74]), "%i", &line_number);
        /* Cut at column 65; it must be a space */
        line_buffer[65] = '\0';

        /* If the line number is one with a nurbs, extract the data */
        if (line_number == entities[entity_number].data_line){
            if (entities[entity_number].entity_type_number == 128){
                parseSurfaceEntity
                    ( &(entities[entity_number]), fd, status, line_buffer
                    , parameter_delimiter_character
                    , record_delimiter_character
                    );

                entity_number++;

            }
            else if (entities[entity_number].entity_type_number == 126){
                parseCurveEntity
                    ( &(entities[entity_number]), fd, status, line_buffer
                    , parameter_delimiter_character
                    , record_delimiter_character
                    );

                entity_number++;
            }
        }

        /* Read the next line */
        *status = fgets(line_buffer, 82, fd);
        letter_code = getLetterCode(line_buffer);
    }
}


/* Checks that all ids are different, if not reassign the ids */
static void check_surface_ids( NurbsSurface* surface_array, const int nurbs_length )
{
    int i, j;
    int repeated = 0;
    NurbsSurface* a;
    NurbsSurface* b;

    if (surface_array == nullptr){
        return;
    }

    for (i = 0; i < nurbs_length-1 && repeated == 0; i++){
        for (j = i+1; j < nurbs_length && repeated == 0; j++){
            a = &(surface_array[i]);
            b = &(surface_array[j]);
            if (a->id == b->id){
                repeated = 1;
            }
        }
    }

    if (repeated == 1){
        for (i = 0; i < nurbs_length; i++){
            surface_array[i].id = 100+i;
        }
    }
}


/* Checks that all ids are different, if not reassign the ids */
static void check_curves_ids( NurbsCurve* curve_array, const int nurbs_length )
{
    int i, j;
    int repeated = 0;
    NurbsCurve* a;
    NurbsCurve* b;

     if (curve_array == nullptr){
        return;
    }

   for (i = 0; i < nurbs_length-1 && repeated == 0; i++){
        for (j = i+1; j < nurbs_length && repeated == 0; j++){
            a = &(curve_array[i]);
            b = &(curve_array[j]);
            if (a->id == b->id){
                repeated = 1;
            }
        }
    }

    if (repeated == 1){
        for (i = 0; i < nurbs_length; i++){
            curve_array[i].id = 200+i;
        }
    }
}


/* Reads the NURBS curves and surfaces from a IGES file in ASCII format */
static void nurbs_import_iges_file
		( FILE* fd 			             /* File handler */
        , NurbsCurve** p_curve_array     /* Output array with the curves */
        , int* num_curves                /* Number of curves */
        , NurbsSurface** p_surface_array /* Output array with the surfaces */
        , int* num_surfaces              /* Number of surfaces */
){
    char line_buffer[128];      /* Buffer for reading text lines in the file */
    char* status;               /* If status == NULL we are at the end of file */
    GlobalSectionFields global; /* Parameter in the global section  */
    Entity* entities = nullptr; /* CAD entities (NURBS curves or surface) */
    int num_entities = 0;       /* Total number of CAD entities */
    int i, j;

    NurbsCurve* curve_array = nullptr;
    NurbsSurface* surface_array = nullptr;

    /* Lines must have 80 characters, but there are also \n \r,  
     * which increases the line length, so rows requires more that 80 chars */
    status = fgets(&(line_buffer[0]), 127, fd);
    /* Check that is not a flag section */
    if (getLetterCode(&(line_buffer[0])) == 'C'){
        _handle_error_("Cannot read compressed IGES files");
        return;
    }

    readStartSectionASCII(fd, &status, &(line_buffer[0]));

    rewind(fd);
    status = fgets(&(line_buffer[0]), 127, fd);

    readGlobalSectionASCII(fd, &status, &(line_buffer[0]), &global);

    rewind(fd);
    status = fgets(&(line_buffer[0]), 127, fd);

    readDirectorySectionASCII(fd, &status, &(line_buffer[0])
        , &entities, &num_entities);

    if (num_entities <= 0){
        /* The file is empty or it is not an IGES file or who knows... */
        _handle_error_("There are no entities in the file");
        return;
    }

    rewind(fd);
    status = fgets(&(line_buffer[0]), 127, fd);

    readParameterSectionASCII
        ( fd, &status, &(line_buffer[0])
        , entities, num_entities
        , global.parameter_delimiter_character
        , global.record_delimiter_character
        );

    /* Close file */
    fclose(fd);

    /* Store the curves */
    if (num_curves != nullptr && p_curve_array != nullptr){
        *num_curves = 0;
        for (i = 0; i < num_entities; i++){
            if (entities[i].curve != nullptr){
            *num_curves += 1;
            }
        }
        if (*num_curves > 0){
            _check_( curve_array = (NurbsCurve*)
                _malloc_(sizeof(NurbsCurve)* (*num_curves)));

            j = 0;
            for (i = 0; i < num_entities; i++){
                if (entities[i].curve != nullptr){
                    curve_array[j] = *(entities[i].curve);
                    free(entities[i].curve);
                    j++;
                }
            }
        }
    }

    /* Stores the surfaces */
    if (num_surfaces != nullptr && p_surface_array != nullptr){
        *num_surfaces = 0;
        for (i = 0; i < num_entities; i++){
            if (entities[i].surface != nullptr){
                *num_surfaces += 1;
            }
        }
        if (*num_surfaces > 0){
            _check_( surface_array = (NurbsSurface*)
                _malloc_(sizeof(NurbsSurface) * (*num_surfaces)));

            j = 0;
            for (i = 0; i < num_entities; i++){
                if (entities[i].surface != nullptr){
                    surface_array[j] = *(entities[i].surface);
                    free(entities[i].surface);
                    j++;
                }
            }
        }
    }

    /* Release memory */
    if (p_curve_array != nullptr){
        *p_curve_array = curve_array;
    }
    else{
        /* Release the unused entities */
        for (i = 0; i < num_entities; i++){
            if (entities[i].curve != nullptr){
                nurbs_curve_dispose( entities[i].curve );
                free( entities[i].curve );
            }
        }
    }
    if (p_surface_array != nullptr){
        *p_surface_array = surface_array;
    }
    else{
        /* Release the unused entities */
        for (i = 0; i < num_entities; i++){
            if (entities[i].surface != nullptr){
                nurbs_surface_dispose( entities[i].surface );
                free( entities[i].surface );
            }
        }
    }
    free(entities);

    /* Check that all ids are different */
    check_surface_ids( surface_array, *num_surfaces );
    check_curves_ids( curve_array, *num_surfaces );
}


/* Reads the NURBS curves and surfaces from a IGES file in ASCII format */
void nurbs_import_iges
		( const char* filename 			 /* IGES file name */
        , NurbsCurve** p_curve_array     /* Output array with the curves */
        , int* num_curves                /* Number of curves */
        , NurbsSurface** p_surface_array /* Output array with the surfaces */
        , int* num_surfaces              /* Number of surfaces */
){
    FILE* fd;

    /* Initialize */
    if (p_curve_array != nullptr){
        *p_curve_array = nullptr;
    }
    if (num_curves != nullptr){
        *num_curves = 0;
    }
    if (p_surface_array != nullptr){
        *p_surface_array = nullptr;
    }
    if (num_surfaces != nullptr){
        *num_surfaces = 0;
    }

    fd = fopen(filename, "r");

    if (fd == NULL){
        _handle_error_("Cannot open file %s", filename);
        return;
    }

    nurbs_import_iges_file(fd, p_curve_array, num_curves, p_surface_array, num_surfaces);
}


/* Imports only the nurbs surfaces */
NurbsSurface* nurbs_surface_import_iges
    ( const char *filename  /** file name */
    , int* num_surfaces     /** (out) number of nurbs read in the file */
    )
{
    NurbsSurface* surface_array = nullptr;
    FILE* fd;
    char* time_str = getlocaltime();

    *num_surfaces = 0;

    _trace_( "--- %s", time_str );
    _trace_( "Loading IGES file %s\n", filename );

    fd = fopen(filename, "r");

    if (fd == NULL){
        _handle_error_("Cannot open file %s", filename);
        return nullptr;
    }

    nurbs_import_iges_file(fd, nullptr, nullptr, &surface_array, num_surfaces);

    return surface_array;
}

