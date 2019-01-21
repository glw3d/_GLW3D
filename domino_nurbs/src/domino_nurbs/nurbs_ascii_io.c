 /***
    Author: Mario J. Martin <dominonurbs$gmail.com>

    Methods to read and write in ASCII file format

*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common/file_encoding.h"
#include "common/check_malloc.h"
#include "common/log.h"

#include "nurbs_internal.h"

#include "nurbs_curve.h"
#include "nurbs_surface.h"
#include "nurbs_controlbox.h"
#include "nurbs_io.h"

#define NURBS_FILE_MAX_LINE_LEN 255
#define BLOCK_SEPARATOR         "---"
#define KEYNAME_DESCRIPTOR      ':'

#define NURBS_GROUP_SEPARATOR   29
#define NURBS_RECORD_SEPARATOR  28
#define NURBS_KEY_ERROR         -1
#define NURBS_END_OF_FILE       4
#define CURRENT_FILE_VERSION    "ow_1.0"    /* Same signature as Overview */

/* Checks if the line has the specified keyname. The check is case insensitive.
 * Return 1 if the line match with the keyname or 0 if not. */
static int check_keyname( const char* line, const char* keyname )
{
    const char* pa = line;
    const char* pb = keyname;
    char ca, cb;
    int i = 0;
    
    if (line == nullptr || keyname ==  nullptr){
        return 0;
    }

    /* Find the next valid character */
    while (*pa == ' ' || *pa == '\t' || *pa == '\n' || *pa == '\r'){
        if (++i > NURBS_FILE_MAX_LINE_LEN){    /* This is a safe guard. */
            return 0;
        }
        pa++;
    }

    /* Compare the string, case insensitive */
    while (*pb != '\0'){
        if (*pa >= 'a' && *pa <= 'z'){
            ca = *pa + 'A' - 'a';
        }
        else{
            ca = *pa;
        }
        if (*pb >= 'a' && *pb <= 'z'){
            cb = *pb + 'A' - 'a';
        }
        else{
            cb = *pb;
        }

        if (ca != cb){
            return 0;
        }
        pa++;
        pb++;
    }

    return 1;
}


/* Gets the first keyname that encounters, ignoring the lower or upper case.
 * Returns the a pointer after ':', where the value ought to be found. 
 * If reaches the end of file, returns nullptr */
static char* get_keyname
    ( char* buffer
    , FILE* fh
    , int* keytype 
    )
{
    char* value;

    if (buffer == nullptr || fh == NULL){
        *keytype = NURBS_KEY_ERROR;
        return nullptr;
    }

    /* Read lines until a keyname is found */
    while( fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) != NULL){
        /* Check the end block separator */
        if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
            /* The end block sequence is found */
            *keytype = NURBS_GROUP_SEPARATOR;
            return nullptr;
        }

        /* Look for the ':' */
        value = strchr( buffer, KEYNAME_DESCRIPTOR );
        if (value != NULL){
            /* A keyname is found */
            *keytype = NURBS_RECORD_SEPARATOR;
            return value + 1;
        }
    }

    /* No keyname was found */
    *keytype = NURBS_END_OF_FILE;
    return nullptr;
}


/* Reads a string value */
static void read_label_safe( char* label, const char* value )
{
    const char* pb = value;
    char* p1 = label;
    int i;

    /* Find the next valid character */
    i = 0;
    while (*pb == ' ' || *pb == '\t' || *pb == '\r' || *pb == '\n'){
        pb++;
    }

    /* Read the label until end of line or until NURBS_MAX_LABEL_LENGTH characters */
    i = 0;
    while (*pb != '\n' && *pb != '\0' && p1 < &(label[DOMINO_NURBS_LABEL_LEN-1])){
        *p1 = *pb;
        pb++;
        p1++;
    }

    /* Trim the label */
    *p1 = ' ';
    for (; (*p1 == ' ' || *p1 == '\t' || *p1 == '\r') && p1 >= &(label[0]); ){

        *p1-- = '\0';
    }
}


/* Check the version */
static void check_version( FILE *fh, char* version )
{
    char buffer[NURBS_FILE_MAX_LINE_LEN+1];
    char* value;
    int keytype = 0;

    version[0] = '\0';

    /* Read lines until the version keyname is found or until the end of file.*/
    while( keytype != NURBS_END_OF_FILE ){
        value = get_keyname( buffer, fh, &keytype );
        if (check_keyname( buffer, "file format version" ) == 1){
            sscanf( value, "%15s", version );
            return;
        }
    }
}


/* Extracts the number of entities found in the file */
static void extract_number_of_entities
    ( FILE* fh
    , int* num_curves
    , int* num_surfaces
    , int* num_controlboxes 
    , int* num_ffdboxes
    )
{
    char buffer[NURBS_FILE_MAX_LINE_LEN+1];
    char* value;
    int keytype = 0;

    *num_curves = 0;
    *num_surfaces = 0;
    *num_controlboxes = 0;
    *num_ffdboxes = 0;

    /* Read lines and look for the keyname "entity type" */
    while( keytype != NURBS_END_OF_FILE ){
        value = get_keyname( buffer, fh, &keytype );

        if ( check_keyname( buffer, "entity type" ) == 1 ){
            if  (  check_keyname( value, "nurbs_curve" ) == 1 
                || check_keyname( value, "nurbs curve" ) == 1 )
            {
                *num_curves += 1;
            }
            else if  (  check_keyname( value, "nurbs_surface" ) == 1 
                     || check_keyname( value, "nurbs_surface" ) == 1 )
            {
                *num_surfaces += 1;
            }
            else if  (  check_keyname( value, "nurbs_control_box" ) == 1 
                     || check_keyname( value, "nurbs control box" ) == 1 )
            {
                *num_controlboxes += 1;
            }
        }
    }
}


/* Read one NURBS surface */
static void read_nurbs_curve( FILE* fh, NurbsCurve* curve )
{
    int i;
    float x, y, z, w, kn;
    char* value;
    char buffer[NURBS_FILE_MAX_LINE_LEN + 1];
    int keytype;
    int status;
    int cp_length = 0;
    int degree = 0;
    int init = 0;

    /* Read required information to create the entity */
    keytype = 0;
    while( keytype != NURBS_END_OF_FILE && keytype != NURBS_GROUP_SEPARATOR){
        if (cp_length > 0 && degree > 0 && init == 0){
            /* Create the nurbs */
            nurbs_curve_alloc( curve, cp_length, degree );
        }
        value = get_keyname( buffer, fh, &keytype );
        if ( check_keyname( buffer, "cp length" ) == 1 ){
            status = sscanf( value, "%i", &cp_length );
            if (status < 1){
                _handle_error_("Reading nurbs curve: "
                    "'cp length' requires one value!");
                return;
            }
        }
        else if ( check_keyname( buffer, "degree" ) == 1 ){
            status = sscanf( value, "%i", &degree );
            if (status < 1){
                _handle_error_("Reading nurbs curve: "
                    "'degree' requires one value!");
                return;
            }
        }
        else if ( check_keyname( buffer, "label" ) == 1 ){
            read_label_safe( curve->label, value );
        }
        else if ( check_keyname( buffer, "id" ) == 1 ){
            sscanf( value, "%i", &curve->id );
        }
        else if ( check_keyname( buffer, "control points" ) == 1 ){
            for (i = 0; i < cp_length;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL ){
                    _handle_error_("Reading nurbs curve: "
                        "Unexpected end of file!");
                    return; /* Ooops! */
                }
                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_("Reading nurbs curve: "
                        "Unexpected end of block!");
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _handle_error_( "Reading nurbs curve: "
                        "Unexpected keyname!" );
                }

                /* Read the control point coordinates */
                if (sscanf( buffer, "%f %f %f %f", &x, &y, &z, &w ) == 4){
                    curve->cp[i].x = (NurbsFloat)x;
                    curve->cp[i].y = (NurbsFloat)y;
                    curve->cp[i].z = (NurbsFloat)z;
                    curve->cp[i].w = (NurbsFloat)w;
                    i++;
                }
            }
        }
        else if ( check_keyname( buffer, "knots" ) == 1 ){
            for (i = 0; i < curve->knot_length;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL ){
                    _handle_error_
                        ("Reading nurbs curve: Unespected end of file!");
                    return; /* Ooops! */
                }
                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_( "Reading nurbs curve: "
                        "Unexpected end of block!" );
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _handle_error_( "Reading nurbs curve: "
                        "Unexpected keyname!" );
                }

                if (sscanf( buffer, "%f", &kn ) == 1){
                    curve->knot[i] = (NurbsFloat)kn;
                    i++;
                }
            }
        }
    }
}

/* Read one NURBS surface */
static void read_nurbs_surface( FILE* fh, NurbsSurface* surface )
{
    int i;
    float x, y, z, w, kn;
    char* value;
    char buffer[NURBS_FILE_MAX_LINE_LEN + 1];
    int keytype;
    int status;
    int cp_length_u = 0, cp_length_v = 0;
    int degree_u = 0, degree_v = 0;
    int init = 0;

    /* Read required information to create the entity */
    keytype = 0;
    while (keytype != NURBS_END_OF_FILE && keytype != NURBS_GROUP_SEPARATOR){
        if (cp_length_u > 0 && cp_length_v > 0 && degree_u > 0 && degree_v > 0 
            && init == 0){
            /* Create the NURBS */
            nurbs_surface_alloc
                ( surface, cp_length_u, cp_length_v, degree_u, degree_v );
            init = 1;
        }

        value = get_keyname( buffer, fh, &keytype );
        if (check_keyname( buffer, "cp length" ) == 1){
            status = sscanf( value, "%i %i", &cp_length_u, &cp_length_v );
            if (status < 2){
                _handle_error_( "Reading nurbs surface: "
                    "'cp length' requires two dimensions!" );
                return;
            }
        }
        else if (check_keyname( buffer, "highest indexes" ) == 1){
            status = sscanf( value, "%i %i", &cp_length_u, &cp_length_v );
            if (status < 2){
                _handle_error_( "Reading nurbs surface: "
                    "'highest indexes' requires two dimensions!" );
                return;
            }
            cp_length_u += 1;
            cp_length_v += 1;
        }
        else if (check_keyname( buffer, "degrees" ) == 1){
            status = sscanf( value, "%i %i", &degree_u, &degree_v );
            if (status < 2){
                _handle_error_( "Reading nurbs surface: "
                    "'degrees' requires two dimensions!" );
                return;
            }
        }
        else if (check_keyname( buffer, "order" ) == 1){
            status = sscanf( value, "%i %i", &degree_u, &degree_v );
            degree_u -= 1;
            degree_v -= 1;
            if (status < 2){
                _handle_error_( "Reading nurbs surface: "
                    "'degrees' requires two dimensions!" );
                return;
            }
        }
        else if (check_keyname( buffer, "label" ) == 1){
            read_label_safe( surface->label, value );
        }
        else if (check_keyname( buffer, "id" ) == 1){
            sscanf( value, "%i", &surface->id );
        }
        else if (check_keyname( buffer, "control points" ) == 1){
            for (i = 0; i < cp_length_u * cp_length_v;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL ){
                    _handle_error_("Reading nurbs surface: "
                        "Unexpected end of file!");
                    return; /* end of file! */
                }

                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_( "Reading nurbs surface: "
                        "Unexpected end of block!" );
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _warning_( "Reading nurbs surface: Unexpected keyname!" );
                }

                /* Read the control point coordinates */
                if (sscanf( buffer, "%f %f %f %f", &x, &y, &z, &w ) == 4){
                    surface->cp_stream[i].x = (NurbsFloat)x;
                    surface->cp_stream[i].y = (NurbsFloat)y;
                    surface->cp_stream[i].z = (NurbsFloat)z;
                    surface->cp_stream[i].w = (NurbsFloat)w;
                    i++;
                }
            }
        }
        else if ( check_keyname( buffer, "knots u" ) == 1 ){
            for (i = 0; i < surface->knot_length_u;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL ){
                    _handle_error_
                        ("Reading nurbs surface: Unespected end of file!");
                    return; /* end of file! */
                }
                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_( "Reading nurbs surface: "
                        "Unexpected end of block!" );
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _warning_( "Reading nurbs surface: Unexpected keyname!" );
                }

                if (sscanf( buffer, "%f", &kn ) == 1){
                    surface->knot_u[i] = (NurbsFloat)kn;
                    i++;
                }
            }
        }
        else if ( check_keyname( buffer, "knots v" ) == 1 ){
            for (i = 0; i < surface->knot_length_v;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL ){
                    _handle_error_
                        ("Reading nurbs surface: Unespected end of file!");
                    return; /* end of file! */
                }

                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_( "Reading nurbs surface: "
                        "Unexpected end of block!" );
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _warning_( "Reading nurbs surface: Unexpected keyname!" );
                }

                if (sscanf( buffer, "%f", &kn ) == 1){
                    surface->knot_v[i] = (NurbsFloat)kn;
                    i++;
                }
            }
        }
    }
}

/* Read one Control Box NURBS */
static void read_nurbs_controlbox( FILE* fh, NurbsControlBox* cb )
{
    int i;
    float x, y, z;
    char* value;
    char buffer[NURBS_FILE_MAX_LINE_LEN + 1] = { '\0' };
    int keytype;
    int status;
    int cp_length_u = 0, cp_length_v = 0, cp_length_w = 0;
    int order_u = 0, order_v = 0, order_w = 0;
    int init = 0;
    int basis_eq = 0;

    cb->basis_equation = 0; /* default basis are nurbs */

    /* Read required information to create the entity */
    keytype = 0;
    while (keytype != NURBS_END_OF_FILE && keytype != NURBS_GROUP_SEPARATOR){
        value = get_keyname( buffer, fh, &keytype );
        if (check_keyname( buffer, "cp length" ) == 1){
            status = sscanf
                ( value, "%i %i %i", &cp_length_u, &cp_length_v, &cp_length_w );

            if (status < 2){
                _handle_error_( "Reading nurbs control box: "
                    "'cp length' requires three dimensions!" );
                return;
            }
        }
        else if (check_keyname( buffer, "order" ) == 1){
            status = sscanf
                ( value, "%i %i %i", &order_u, &order_v, &order_w );

            if (status < 2){
                _handle_error_( "Reading nurbs control box: "
                    "'order' requires three dimensions!" );
                return;
            }
        }
        else if (check_keyname( buffer, "basis equation" ) == 1){
            if (check_keyname( value, "bezier" ) == 1){
                basis_eq = 1;
                cb->basis_equation = basis_eq;
            }
            else if (check_keyname( value, "nurbs" ) == 1){
                basis_eq = 0;
                cb->basis_equation = basis_eq;
            }
            else{
                _warning_( "Unrecognized basis equation" );
            }
        }
        else if (check_keyname( buffer, "label" ) == 1){
            read_label_safe( cb->label, value );
        }
        else if (check_keyname( buffer, "id" ) == 1){
            sscanf( value, "%i", &cb->id );
        }
        else if (check_keyname( buffer, "control points" ) == 1){
            /* Create the nurbs */
            if (cb->basis_equation == 0){ // uniform b-splines basis
                if (cp_length_u < order_u + 1){
                    _warning_( "Warning! Order %i requires at less %i control points!", order_u, order_u + 1 );
                    order_u = cp_length_u - 1;
                }
                if (order_v == 2 && cp_length_v < order_v + 1){
                    _warning_( "Warning! Order %i requires at less %i control points!", order_v, order_v + 1 );
                    order_v = cp_length_v - 1;
                }
                if (order_w == 2 && cp_length_w < order_w + 1){
                    _warning_( "Warning! Order %i requires at less %i control points!", order_w, order_w + 1 );
                    order_w = cp_length_w - 1;
                }
            }
            else{ // polynomial basis
                if ((order_u == 2 || order_u == 3) && cp_length_u < 4){
                    _warning_( "Warning! Order 2 or 3 requires at less 4 control points!" );
                    order_u = 1;
                }
                if ((order_v == 2 || order_v == 3) && cp_length_v < 4){
                    _warning_( "Warning! Order 2 or 3 requires at less 4 control points!" );
                    order_v = 1;
                }
                if ((order_w == 2 || order_w == 3) && cp_length_w < 4){
                    _warning_( "Warning! Order 2 or 3 requires at less 4 control points!" );
                    order_w = 1;
                }
            }

            if (init == 0){
                /* Allocate memory for the entity */
                nurbs_controlbox_alloc( cb, cp_length_u, cp_length_v, cp_length_w );
                cb->order_u = order_u;
                cb->order_v = order_v;
                cb->order_w = order_w;
                cb->basis_equation = basis_eq;
                init = 1;
            }

            for (i = 0; i < cp_length_u * cp_length_v * cp_length_w;){
                if (fgets( buffer, NURBS_FILE_MAX_LINE_LEN, fh ) == NULL){
                    _handle_error_( "Reading nurbs control box: "
                        "Unespected end of file!" );
                    return; /* end of file */
                }
                /* Check if there is no a block separator */
                if (check_keyname( buffer, BLOCK_SEPARATOR ) == 1){
                    _handle_error_( "Reading nurbs control box: "
                        "Unexpected end of block!" );
                    return; /* Ooops! */
                }

                /* Check that there is no keyname */
                value = strchr( buffer, KEYNAME_DESCRIPTOR );
                if (value != NULL){
                    _warning_( "Reading control box: Unexpected keyname!" );
                }

                /* Read control point coordinates */
                if (sscanf( buffer, "%f %f %f", &x, &y, &z ) == 3){
                    cb->cp_stream[i].x = (NurbsFloat)x;
                    cb->cp_stream[i].y = (NurbsFloat)y;
                    cb->cp_stream[i].z = (NurbsFloat)z;
                    i++;
                }
            }
        }
    }
}

/* Reads all NURBS entities from an ASCII file. */
static void nurbs_import_ascii_file
    ( FILE* fh
    , NurbsCurve** p_nurbs_curve
    , int* num_nurbs_curve
    , NurbsSurface** p_nurbs_surface
    , int* num_nurbs_surface
    , NurbsControlBox** p_nurbs_controlbox
    , int* num_nurbs_controlbox
    )
{
    int i, encoding, bom_length;
    char* value;
    char buffer[NURBS_FILE_MAX_LINE_LEN+1];
    int keytype;
    char version[16];
    NurbsCurve* curve_array = nullptr; 
    NurbsSurface* surface_array = nullptr;
    NurbsControlBox* cb_array = nullptr;

    int num_surfaces;
    int num_curves;
    int num_controlboxes;
    int num_ffdboxes;

    /* Checks the text encoding */
    encoding = check_file_encoding( fh, &bom_length );
    if (encoding != ENCODING_UTF8){
        _handle_error_("File does not use ANSI encoding! Currently, Unicode is not supported.");
        return;
    }

    /* Return to the start of the file */
    rewind( fh );

    /* Check the format version */
    check_version( fh, version );
    if (strcmp( version, "2.1" ) != 0       /* Older version */
        && strcmp( version, "2.2" ) != 0    /* Older version */
        && strcmp( version, "dn_2.2" ) != 0 /* Older version */
        && strcmp( version, CURRENT_FILE_VERSION ) != 0)
    {
        _handle_error_( "Unsupported file format!" );
        return;
    }

    /* Get the number of each entity in the file */
    rewind ( fh );
    extract_number_of_entities
        ( fh, &num_curves, &num_surfaces, &num_controlboxes, &num_ffdboxes );

    if (p_nurbs_curve != nullptr && num_curves > 0){
        _check_(curve_array = (NurbsCurve*)_malloc_
            ( sizeof(NurbsCurve) * num_curves));

        if (curve_array == nullptr){
            /* Out of mmory */
            return;
        }

        *p_nurbs_curve = curve_array;
        if (num_nurbs_curve != nullptr){
            *num_nurbs_curve = num_curves;
        }

        /* Initialize data structures */
        for (i = 0; i < num_curves; i++){
            nurbs_curve_init( &curve_array[i] );
            curve_array[i].id = i;
        }
    }

    if (p_nurbs_surface != nullptr && num_surfaces > 0){
        _check_( surface_array = (NurbsSurface*)_malloc_
            ( sizeof(NurbsSurface) * num_surfaces));

        if (surface_array == nullptr){
            /* Out of mmory */
            return;
        }

        *p_nurbs_surface = surface_array;
        if (num_nurbs_surface != nullptr){
            *num_nurbs_surface = num_surfaces;
        }

        /* Initialize data structures */
        for (i = 0; i < num_surfaces; i++){
            nurbs_surface_init( &surface_array[i] );
            surface_array[i].id = i;
        }
    }

    if (p_nurbs_controlbox != nullptr && num_controlboxes > 0){
        _check_( cb_array = (NurbsControlBox*)_malloc_
            ( sizeof(NurbsControlBox) * num_controlboxes));

        if (cb_array == nullptr){
            /* Out of mmory */
            return;
        }

        *p_nurbs_controlbox = cb_array;
        if (num_nurbs_controlbox != nullptr){
            *num_nurbs_controlbox = num_controlboxes;
        }

        /* Initialize data structures */
        for (i = 0; i < num_controlboxes; i++){
            nurbs_controlbox_init( &cb_array[i] );
            cb_array[i].id = i;
        }
    }

    /* Extract the entities. */
    keytype = 0;
    num_surfaces = 0;
    num_curves = 0;
    num_controlboxes = 0;
    num_ffdboxes = 0;
    rewind ( fh );
    while( keytype != NURBS_END_OF_FILE && keytype != NURBS_KEY_ERROR ){
        value = get_keyname( buffer, fh, &keytype );
        if (  check_keyname( value, "nurbs_curve" ) == 1
                || check_keyname( value, "nurbs curve" ) == 1 )
        {
            /* Read a NURBS curve */
            if (curve_array != nullptr){
                read_nurbs_curve( fh, &(curve_array[num_curves]) );
                num_curves += 1;
            }
        }
        else if (  check_keyname( value, "nurbs_surface" ) == 1
                || check_keyname( value, "nurbs surface" ) == 1 )
        {
            /* Read a NURBS surface */
            if (surface_array != nullptr){
                read_nurbs_surface( fh, &(surface_array[num_surfaces]) );
                num_surfaces += 1;
            }
        }
        else if (  check_keyname( value, "nurbs_control_box" ) == 1
                || check_keyname( value, "nurbs control box" ) == 1 )
        {
            /* Read a control box */
            if (cb_array != nullptr){
                read_nurbs_controlbox( fh, &(cb_array[num_controlboxes]) );
                num_controlboxes += 1;
            }
        }
    }
}

/* Read a NURBS entities from an ASCII file. */
void nurbs_import_ascii
    ( const char *filename
    , NurbsCurve** p_nurbs_curve
    , int* num_nurbs_curve
    , NurbsSurface** p_nurbs_surface
    , int* num_nurbs_surface
    , NurbsControlBox** p_nurbs_controlbox
    , int* num_nurbs_controlbox
    )
{
    FILE *fh;
    char* time_str = getlocaltime();

    _trace_( "\n--- %s", time_str );
    _trace_( "Importing NURBS file %s\n", filename );

    if (p_nurbs_curve != nullptr){
        *p_nurbs_curve = nullptr;
    }
    if (num_nurbs_curve != nullptr){
        *num_nurbs_curve = 0;
    }

    if (p_nurbs_surface != nullptr){
        *p_nurbs_surface = nullptr;
    }
    if (num_nurbs_surface != nullptr){
        *num_nurbs_surface = 0;
    }

    if (p_nurbs_controlbox != nullptr){
        *p_nurbs_controlbox = nullptr;
    }
    if (num_nurbs_controlbox != nullptr){
        *num_nurbs_controlbox = 0;
    }

    fh = fopen(filename, "rt" );
    if (fh == NULL){
        _handle_error_("Cannot open filename! %s", filename);
        return;
    }

    nurbs_import_ascii_file
        ( fh
        , p_nurbs_curve, num_nurbs_curve
        , p_nurbs_surface, num_nurbs_surface
        , p_nurbs_controlbox, num_nurbs_controlbox
        );

    fclose( fh );
}

/* Read all NURBS curves from an ASCII file. */
NurbsCurve* nurbs_curve_import_ascii
    ( const char *filename
    , int* num_curves
    )
{
    NurbsCurve* nurbs;

    nurbs_import_ascii
        ( filename
        , &nurbs, num_curves
        , nullptr, nullptr
        , nullptr, nullptr
        );

    _trace_( "   num curves: %i\n", *num_curves );

    return nurbs;
}


/* Reads all NURBS surfaces from an ASCII file. */
NurbsSurface* nurbs_surface_import_ascii
    ( const char *filename
    , int* num_nurbs
    )
{
    NurbsSurface* nurbs;
                
    nurbs_import_ascii
        ( filename
        , nullptr, nullptr
        , &nurbs, num_nurbs
        , nullptr, nullptr
        );

    _trace_( "   num surfaces: %i\n", *num_nurbs );

    return nurbs;
}


/* Reads all control boxes NURBS from an ASCII file. */
NurbsControlBox* nurbs_controlbox_import_ascii
    ( const char *filename
    , int* num_nurbs
    )
{
    NurbsControlBox* nurbs;

    nurbs_import_ascii
        ( filename
        , nullptr, nullptr
        , nullptr, nullptr
        , &nurbs, num_nurbs 
        );

    _trace_( "   num control boxes: %i\n", *num_nurbs );

    return nurbs;
}

/******************************************************************************/

/* Writes a NURBS curve to an ASCII file */
void nurbs_curve_fprintf( FILE* fh, const NurbsCurve *nurbs )
{
    int i;

    if (fh == NULL)
        return;
    if (nurbs == nullptr)
        return;

    /* Writting the output file  */
    fprintf( fh, "\nentity type\t: nurbs_curve " );
    if (nurbs->label != nullptr){
        fprintf( fh, "\nlabel\t: %s", nurbs->label );
    }
    else{
        fprintf( fh, "\nlabel\t: ");
    }
    fprintf( fh, "\nid\t\t: %i\t<--- Id of the NURBS", nurbs->id );
    fprintf( fh, "\ncp length\t: %i", nurbs->cp_length );
    fprintf( fh, "\t<--- Number of control points " );
    fprintf( fh, "\nDegree\t\t\t: %i", nurbs->degree );
    fprintf( fh, "\t<--- Degree of the NURBS" );

    fprintf( fh, "\n\nControl Points:" );
    for (i = 0; i < nurbs->cp_length; i++){
        fprintf 
        ( fh, "\n%.8f %.8f %.8f %.8f  \t<---- Pw(%i)"
        , nurbs->cp[i].x, nurbs->cp[i].y
        , nurbs->cp[i].z, nurbs->cp[i].w
        , i
        );
    }

    fprintf( fh, "\nKnots:" );
    for (i = 0; i < nurbs->knot_length; i++){
        fprintf( fh, "\n%f  \t<--- u(%i)", nurbs->knot[i], i );
    }

    fprintf( fh,"\n\n----------------------------------------\n" );
}

/* Writes a NURBS surface to an ASCII file */
static void nurbs_surface_fprintf( FILE* fh, const NurbsSurface *nurbs )
{
    int i, j;

    if (fh == NULL)
        return;
    if (nurbs == nullptr)
        return;

    /* Writting the output file  */
    fprintf( fh, "\nentity type\t: nurbs_surface " );
    if (nurbs->label != nullptr){
        fprintf( fh, "\nlabel\t: %s", nurbs->label );
    }
    else{
        fprintf( fh, "\nlabel\t: ");
    }
    fprintf( fh, "\nid\t\t: %i\t<--- Id of the NURBS", nurbs->id );
    fprintf( fh, "\ncp length\t: %i %i"
        , nurbs->cp_length_u, nurbs->cp_length_v );
    fprintf( fh, "\t<--- Number of control points in u- and v- directions " );
    fprintf( fh, "\ndegrees\t\t: %i %i", nurbs->degree_u, nurbs->degree_v );
    fprintf( fh, "\t<--- Degrees in u- and v-directions " );

    fprintf( fh, "\n\nControl Points:" );
    for (i = 0; i < nurbs->cp_length_u; i++){
        for (j = 0; j < nurbs->cp_length_v; j++){
            fprintf 
            ( fh, "\n%.8f %.8f %.8f %.8f  \t<---- Pw(%i,%i)"
            , nurbs->cp[i][j].x, nurbs->cp[i][j].y
            , nurbs->cp[i][j].z, nurbs->cp[i][j].w
            , i, j
            );
        }

        fprintf( fh, "\n" );
    }

    fprintf( fh, "\nKnots U:" );
    for (i = 0; i < nurbs->knot_length_u; i++){
        fprintf( fh, "\n%f  \t<--- u(%i)", nurbs->knot_u[i], i );
    }

    fprintf( fh, "\n\nKnots V:" );
    for (i = 0; i < nurbs->knot_length_v; i++){
        fprintf( fh, "\n%f  \t<--- v(%i)", nurbs->knot_v[i], i );
    }

    fprintf( fh,"\n\n----------------------------------------\n" );
}

/* Writes a NURBS control box to an ASCII file */
void nurbs_controlbox_fprintf( FILE* fd, const NurbsControlBox* nurbs )
{
    int i, j, k, index;

    if (fd == NULL)
        return;
    if (nurbs == nullptr)
        return;

    /* Writting the output file  */
    fprintf( fd, "\nentity type\t: nurbs_control_box " );
    if (nurbs->label != nullptr){
        fprintf( fd, "\nlabel\t: %s", nurbs->label );
    }
    else{
        fprintf( fd, "\nlabel\t: ");
    }
    fprintf( fd, "\nid\t\t: %i\t<--- Id of the control box", nurbs->id );
    fprintf( fd, "\ncp length\t: %i %i %i"
        , nurbs->cp_length_u, nurbs->cp_length_v, nurbs->cp_length_w );
    fprintf( fd, "\t<--- Number of control points in u, v, w directions " );
    fprintf( fd, "\norder\t: %i %i %i"
        , nurbs->order_u, nurbs->order_v, nurbs->order_w );
    fprintf( fd, "\t<--- Order of the interpolation in u, v, w directions " );
 
    fprintf( fd, "\nbasis equation: " );
    if (nurbs->basis_equation == 1){
        fprintf( fd, "bezier" );
    }
    else{
        fprintf( fd, "nurbs" );
    }
    fprintf( fd, "\t<--- Basis interporlation (nurbs, bezier) " );

    fprintf( fd, "\n\nControl Points:" );
    index = 0;
    for (i = 0; i < nurbs->cp_length_u; i++){
        for (j = 0; j < nurbs->cp_length_v; j++){
            for (k = 0; k < nurbs->cp_length_w; k++){
                fprintf 
                ( fd, "\n%.8f %.8f %.8f  \t<---- Pw(%i,%i,%i)"
                , nurbs->cp_stream[index].x
                , nurbs->cp_stream[index].y
                , nurbs->cp_stream[index].z
                , i, j, k
                );
                index++;
            }
            fprintf( fd, "\n" );
        }
    }

    fprintf( fd,"\n\n----------------------------------------\n" );
}


/******************************************************************************/

/* Writes all NURBS entities to an ASCII file */
void nurbs_export_ascii
    ( const char* filename
    , const NurbsCurve *curve_array
    , const int num_curves 
    , const NurbsSurface *surface_array
    , const int num_surfaces 
    , const NurbsControlBox *cb_array
    , const int num_cb 
    )
{
    int i;
    FILE* fd;
    char* time_str = getlocaltime();

    _trace_( "--- %s", time_str );
    _trace_( "Exporting NURBS file %s\n", filename );

    fd = fopen( filename, "wt" );
    if (fd == NULL){
        _handle_error_( "Failed to write filename! %s", filename );
        return;
    }
    
    fprintf( fd, "\nfile format version : %s", CURRENT_FILE_VERSION );

    fprintf( fd,"\n\n----------------------------------------\n" );

    for (i = 0; i < num_curves; i++){
        nurbs_curve_fprintf( fd, &(curve_array[i]) );
    }
    for (i = 0; i < num_surfaces; i++){
        nurbs_surface_fprintf( fd, &(surface_array[i]) );
    }
    for (i = 0; i < num_cb; i++){
        nurbs_controlbox_fprintf( fd, &(cb_array[i]) );
    }

    fclose( fd );
}


/* Writes NURBS curves to an ASCII file */
void nurbs_curve_export_ascii
    ( const char* filename
    , const NurbsCurve *curve_array
    , const int num_curves 
    )
{
    int i;
    FILE* fd;

    fd = fopen( filename, "wt" );
    if (fd == NULL){
        _handle_error_( "Failed to write filename! %s", filename );
        return;
    }
    
    fprintf( fd, "\nfile format version : %s", CURRENT_FILE_VERSION );

    fprintf( fd,"\n\n----------------------------------------\n" );

    for (i = 0; i < num_curves; i++){
        nurbs_curve_fprintf( fd, &(curve_array[i]) );
    }

    fclose( fd );
}


/* Writes NURBS surfaces to an ASCII file */
void nurbs_surface_export_ascii
    ( const char* filename
    , const NurbsSurface *surface_array
    , const int num_surfaces 
    )
{
    int i;
    FILE* fd;

    fd = fopen( filename, "wt" );
    if (fd == NULL){
        _handle_error_( "Failed to write filename! %s", filename );
        return;
    }
    
    fprintf( fd, "\nfile format version : %s", CURRENT_FILE_VERSION );

    fprintf( fd,"\n\n----------------------------------------\n" );

    for (i = 0; i < num_surfaces; i++){
        nurbs_surface_fprintf( fd, &(surface_array[i]) );
    }

    fclose( fd );
}


/* Writes control box NURBS to an ASCII file */
void nurbs_controlbox_export_ascii
    ( const char* filename
    , const NurbsControlBox *cb_array
    , const int num_cb 
    )
{
    int i;
    FILE* fd;

    fd = fopen( filename, "wt" );
    if (fd == NULL){
        _handle_error_( "Failed to write filename! %s", filename );
        return;
    }
    
    fprintf( fd, "\nfile format version : %s", CURRENT_FILE_VERSION );

    fprintf( fd,"\n\n----------------------------------------\n" );

    for (i = 0; i < num_cb; i++){
        nurbs_controlbox_fprintf( fd, &(cb_array[i]) );
    }

    fclose( fd );
}


/******************************************************************************/

