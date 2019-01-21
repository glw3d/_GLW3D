/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Loads CGNS file formats.

*******************************************************************************/

#ifndef NO_CGNS
#include <cgnslib.h>
#endif 

#include "common/log.h"
#include "common/check_malloc.h"
#include "common/string_ext.h"

#include "dataset/dataset_arrays.h"
#include "cgnsGrid.h"

void err_func( int is_error, char *errmsg )
{
    if (is_error == 0){
        /* Warning */
        _warning_( "CGNS Warning! %s", errmsg );
    }
    else if (is_error == 1){
        _handle_error_( "CGNS Error! %s", errmsg );
    }
    else if (is_error == -1){
        /* If the program is going to terminate */
    }
}


/* Equivalent to a constructor */
cgnsSection* cgnsSection_init( cgnsSection* p_section )
{
    cgnsSection* section = p_section;
    if (p_section == nullptr){
        _check_( section = (cgnsSection*)_malloc_( sizeof( cgnsSection ) ) );
    }
    if (section == nullptr){
        return nullptr;
    }

    section->label[0] = '\0';
    section->num_lines = 0;
    section->num_triangles = 0;
    section->num_quads = 0;
    section->num_tetrahedrons = 0;
    section->num_pyramids = 0;
    section->num_pentahedrons = 0;
    section->num_hexahedrons = 0;

    section->bar2_stream = nullptr;
    section->tri3_stream = nullptr;
    section->quad4_stream = nullptr;
    section->tetra4_stream = nullptr;
    section->pyra5_stream = nullptr;
    section->penta6_stream = nullptr;
    section->hexa8_stream = nullptr;
    section->next = nullptr;

    return section;
}

/* Equivalent to a constructor */
cgnsGrid* cgnsGrid_init( cgnsGrid* p_grid )
{
    cgnsGrid* grid = p_grid;
    if (p_grid == nullptr){
        _check_( grid = (cgnsGrid*)_malloc_( sizeof( cgnsGrid ) ) );
    }
    if (grid == nullptr){
        return nullptr;
    }

    grid->label[0] = '\0';
    grid->id = -1;
    grid->base = 0;
    grid->zone = 0;
    grid->type = owGridType_CGNS_UNKOWN;
    grid->dim = 0;

    grid->i_size = 0;
    grid->j_size = 0;
    grid->k_size = 0;

    grid->num_points = 0;
    grid->point_x.stream = nullptr;
    grid->point_x.length = 0;
    grid->point_y.stream = nullptr;
    grid->point_y.length = 0;
    grid->point_z.stream = nullptr;    
    grid->point_z.length = 0;

    grid->section = nullptr;
    grid->num_sections = 0;

    grid->next = nullptr;

    return grid;
}

#ifndef NO_CGNS
static void import_vertex_coordinates
( cgnsGrid* grid, const int file_index, const int ibase, const int izone
, const size_t num_points, const cgsize_t range_min[3], const cgsize_t range_max[3] )
{
    int ncoords = 0;
    DataType_t datatype;
    int datasize = 0;
    char GridCoordName[33] = { '\0' };
    char coordname[33] = { '\0' };
    size_t i;
    int ier;
    float* fbuf;
    double* dbuf;

    if (num_points == 0){
        return;
    }

    _check_( grid->point_x.stream = (owDouble*)_calloc_( num_points, sizeof( owDouble ) ) );
    _check_( grid->point_y.stream = (owDouble*)_calloc_( num_points, sizeof( owDouble ) ) );
    _check_( grid->point_z.stream = (owDouble*)_calloc_( num_points, sizeof( owDouble ) ) );

    if (grid->point_x.stream == nullptr || grid->point_y.stream == nullptr || grid->point_z.stream == nullptr){
        /* Out of memory */
        free( grid->point_x.stream );
        free( grid->point_y.stream );
        free( grid->point_z.stream );
        return;
    }

    grid->point_x.length = num_points;
    grid->point_y.length = num_points;
    grid->point_z.length = num_points;
    grid->num_points = num_points;
    _trace_( "   num_points: %i\n", num_points );

    /* Number of coordinates */
    ier = cg_ncoords( file_index, ibase, izone, &ncoords );
    grid->dim = ncoords;  /* dimensions of the grid */

    if (ncoords >= 1){
        ier = cg_coord_info( file_index, ibase, izone, 1, &datatype, coordname );
        if (datatype == RealSingle){
            datasize = sizeof( float );
        }
        else if (datatype == RealDouble){
            datasize = sizeof( double );
        }

        if (datasize == sizeof( owDouble )){
            ier = cg_coord_read( file_index, ibase, izone, coordname
                , datatype, range_min, range_max, grid->point_x.stream );
        }
        else{
            if (datatype == RealSingle){
                _check_( fbuf = (float*)_malloc_( sizeof( float ) * grid->num_points ) );
                if (fbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, fbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_x.stream[i] = (owDouble)fbuf[i];
                }
                free( fbuf );
            }
            else if (datatype == RealDouble){
                dbuf = (double*)malloc( sizeof( double ) * grid->num_points );
                if (dbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, dbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_x.stream[i] = (owDouble)dbuf[i];
                }
                free( dbuf );
            }
        }
    }
    if (ncoords >= 2){
        ier = cg_coord_info( file_index, ibase, izone, 2, &datatype, coordname );
        if (datatype == RealSingle){
            datasize = sizeof( float );
        }
        else if (datatype == RealDouble){
            datasize = sizeof( double );
        }

        if (datasize == sizeof( owDouble )){
            ier = cg_coord_read( file_index, ibase, izone, coordname
                , datatype, range_min, range_max, grid->point_y.stream );
        }
        else{
            if (datatype == RealSingle){
                _check_( fbuf = (float*)_malloc_( sizeof( float ) * grid->num_points ) );
                if (fbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, fbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_y.stream[i] = (owDouble)fbuf[i];
                }
                free( fbuf );
            }
            else if (datatype == RealDouble){
                dbuf = (double*)malloc( sizeof( double ) * grid->num_points );
                if (dbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, dbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_y.stream[i] = (owDouble)dbuf[i];
                }
                free( dbuf );
            }
        }
    }
    if (ncoords >= 3){
        ier = cg_coord_info( file_index, ibase, izone, 3, &datatype, coordname );
        if (datatype == RealSingle){
            datasize = sizeof( float );
        }
        else if (datatype == RealDouble){
            datasize = sizeof( double );
        }

        if (datasize == sizeof( owDouble )){
            ier = cg_coord_read( file_index, ibase, izone, coordname
                , datatype, range_min, range_max, grid->point_z.stream );
        }
        else{
            if (datatype == RealSingle){
                _check_( fbuf = (float*)_malloc_( sizeof( float ) * grid->num_points ) );
                if (fbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, fbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_z.stream[i] = (owDouble)fbuf[i];
                }
                free( fbuf );
            }
            else if (datatype == RealDouble){
                dbuf = (double*)malloc( sizeof( double ) * grid->num_points );
                if (dbuf == nullptr){
                    /* Out of memory */
                    return;
                }
                ier = cg_coord_read
                    ( file_index, ibase, izone, coordname
                    , datatype, range_min, range_max, dbuf );

                for (i = 0; i < grid->num_points; i++){
                    grid->point_z.stream[i] = (owDouble)dbuf[i];
                }
                free( dbuf );
            }
        }
    }
}

/* Imports the elements connectivities of an structured grid
* Elements with additional vertex like TRI_6 or QUAD_9 are not supported */
static void import_cgns_unstructured_sections
( cgnsGrid* grid, const int file_index, const int ibase, const int izone )
{
    int i;
    int ier;
    int num_sections = 0;
    int elm_data_size;
    int num_elem;
    cgsize_t elm_begin, elm_end;
    ElementType_t elm_type;
    int isec, nBdry, parentFlag;
    char secname[33] = { '\0' };
    cgsize_t* ibuf = nullptr;
    cgsize_t elm;
    int unsuported = 0;
    /* Number of volumetric elements. */
    int ntetra4 = 0, npyra5 = 0, npenta6 = 0, nhexa8 = 0;
    cgnsSection* section = nullptr;

    /* Surface elements */
    int ntri3 = 0, nquad4 = 0;

    /* Number of edges */
    int nbar2 = 0;

    /* Determine the number of sections for this zone. Note that
    * surface elements can be stored in a volume zone, but they
    * are NOT taken into account in the number obtained from
    * cg_zone_read. */
    ier = cg_nsections( file_index, ibase, izone, &num_sections );
    if (ier != CG_OK){
        _handle_error_( "Failed to read grid sections!" );
        return;
    }

    if (num_sections <= 0){
        _handle_error_( "There are no sections defined in the file!" );
        return;
    }

    _check_( grid->section = (cgnsSection*)_malloc_
        ( sizeof( cgnsSection ) * num_sections ) );

    if (grid->section == nullptr){
        /* Out of memory */
        return;
    }

    for (isec = 0; isec < num_sections; isec++){
        section = &(grid->section[isec]);
        cgnsSection_init( section );
        if (isec < num_sections - 1){
            section->next = &(grid->section[isec + 1]);
        }
    }
    grid->num_sections = num_sections;
    _trace_( "   num sections: %i\n", num_sections );

    for (isec = 1, section = &(grid->section[0])
        ; isec <= num_sections; isec++, section++)
    {
        ibuf = nullptr;
        nbar2 = 0; ntri3 = 0; nquad4 = 0;
        ntetra4 = 0; npyra5 = 0; npenta6 = 0; nhexa8 = 0;

        cg_section_read
            ( file_index, ibase, izone, isec, secname, &elm_type
            , &elm_begin, &elm_end, &nBdry, &parentFlag );
        num_elem = elm_end - elm_begin + 1;

        strcpy_safe( section->label, secname, 1024 );
        _trace_( "   Section: %s\n", section->label );

        ier = cg_ElementDataSize
            ( file_index, ibase, izone, isec, &elm_data_size );

        if (elm_data_size > 0){
            _check_( ibuf
                = (cgsize_t*)_malloc_( sizeof( cgsize_t )
                * elm_data_size ) );
        }
        if (ibuf == nullptr){
            /* Out of memory or empty data */
            continue;
        }

        cg_elements_read( file_index, ibase, izone, isec, ibuf, NULL );

        /* Count the elements of each type */
        if (elm_type == BAR_2){
            nbar2 = num_elem;
        }
        else if (elm_type == TRI_3){
            ntri3 = num_elem;
        }
        else if (elm_type == QUAD_4){
            nquad4 = num_elem;
        }
        else if (elm_type == TETRA_4){
            ntetra4 = num_elem;
        }
        else if (elm_type == PYRA_5){
            npyra5 = num_elem;
        }
        else if (elm_type == PENTA_6){
            npenta6 = num_elem;
        }
        else if (elm_type == HEXA_8){
            nhexa8 = num_elem;
        }
        else if (elm_type == MIXED){
            nbar2 = 0; ntri3 = 0; nquad4 = 0;
            ntetra4 = 0; npyra5 = 0; npenta6 = 0; nhexa8 = 0;
            for (i = 0; i < elm_data_size;){
                /* The first value indicates the type */
                elm = ibuf[i];
                if (elm == BAR_2){
                    nbar2 += 1;
                    i += 3;
                }
                else if (elm == TRI_3){
                    ntri3 += 1;
                    i += 4;
                }
                else if (elm == QUAD_4){
                    nquad4 += 1;
                    i += 5;
                }
                else if (elm == TETRA_4){
                    ntetra4 += 1;
                    i += 5;
                }
                else if (elm == PYRA_5){
                    npyra5 += 1;
                    i += 6;
                }
                else if (elm == PENTA_6){
                    npenta6 += 1;
                    i += 7;
                }
                else if (elm == HEXA_8){
                    nhexa8 += 1;
                    i += 9;
                }
                else{
                    if (unsuported == 0){
                        _handle_error_( "Unsuported element type!" );
                        unsuported = 1;
                    }
                }
            }
        }
        else{
            if (unsuported == 0){
                _handle_error_( "Unsuported element type!" );
                unsuported = 1;
            }
        }

        /* Create the streams */
        if (nbar2 > 0){
            _check_( section->bar2_stream
                = (int*)_malloc_( sizeof( int ) * nbar2 * 2 ) );
            if (section->bar2_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_lines = nbar2;
        }
        if (ntri3 > 0){
            _check_( section->tri3_stream
                = (int*)_malloc_( sizeof( int ) * ntri3 * 3 ) );
            if (section->tri3_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_triangles = ntri3;
        }
        if (nquad4 > 0){
            _check_( section->quad4_stream
                = (int*)_malloc_( sizeof( int ) * nquad4 * 4 ) );
            if (section->quad4_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_quads = nquad4;
        }
        if (ntetra4 > 0){
            _check_( section->tetra4_stream
                = (int*)_malloc_( sizeof( int ) * ntetra4 * 4 ) );
            if (section->tetra4_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_tetrahedrons = ntetra4;
        }
        if (npyra5 > 0){
            _check_( section->pyra5_stream
                = (int*)_malloc_( sizeof( int ) * npyra5 * 5 ) );
            if (section->pyra5_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_pyramids = npyra5;
        }
        if (npenta6 > 0){
            _check_( section->penta6_stream
                = (int*)_malloc_( sizeof( int ) * npenta6 * 6 ) );
            if (section->penta6_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_pentahedrons = npenta6;
        }
        if (nhexa8 > 0){
            _check_( section->hexa8_stream
                = (int*)_malloc_( sizeof( int ) * nhexa8 * 8 ) );
            if (section->hexa8_stream == nullptr){
                /* Out of memory */
                return;
            }
            section->num_hexahedrons = nhexa8;
        }

        nbar2 = 0; ntri3 = 0; nquad4 = 0;
        ntetra4 = 0; npyra5 = 0; npenta6 = 0; nhexa8 = 0;
        if (elm_type == MIXED)
        {
            for (i = 0; i < elm_data_size;){
                /* The first value indicates the type */
                elm = ibuf[i];
                i++;
                if (elm == BAR_2){
                    section->bar2_stream[nbar2] = (int)ibuf[i] - 1;
                    nbar2++; i++;
                    section->bar2_stream[nbar2] = (int)ibuf[i] - 1;
                    nbar2++; i++;
                }
                else if (elm == TRI_3){
                    section->tri3_stream[ntri3] = (int)ibuf[i] - 1;
                    ntri3++; i++;
                    section->tri3_stream[ntri3] = (int)ibuf[i] - 1;
                    ntri3++; i++;
                    section->tri3_stream[ntri3] = (int)ibuf[i] - 1;
                    ntri3++; i++;
                }
                else if (elm == QUAD_4){
                    section->quad4_stream[nquad4] = (int)ibuf[i] - 1;
                    nquad4++; i++;
                    section->quad4_stream[nquad4] = (int)ibuf[i] - 1;
                    nquad4++; i++;
                    section->quad4_stream[nquad4] = (int)ibuf[i] - 1;
                    nquad4++; i++;
                    section->quad4_stream[nquad4] = (int)ibuf[i] - 1;
                    nquad4++; i++;
                }
                else if (elm == TETRA_4){
                    section->tetra4_stream[ntetra4] = (int)ibuf[i] - 1;
                    ntetra4++; i++;
                    section->tetra4_stream[ntetra4] = (int)ibuf[i] - 1;
                    ntetra4++; i++;
                    section->tetra4_stream[ntetra4] = (int)ibuf[i] - 1;
                    ntetra4++; i++;
                    section->tetra4_stream[ntetra4] = (int)ibuf[i] - 1;
                    ntetra4++; i++;
                }
                else if (elm == PYRA_5){
                    section->pyra5_stream[npyra5] = (int)ibuf[i] - 1;
                    npyra5++; i++;
                    section->pyra5_stream[npyra5] = (int)ibuf[i] - 1;
                    npyra5++; i++;
                    section->pyra5_stream[npyra5] = (int)ibuf[i] - 1;
                    npyra5++; i++;
                    section->pyra5_stream[npyra5] = (int)ibuf[i] - 1;
                    npyra5++; i++;
                    section->pyra5_stream[npyra5] = (int)ibuf[i] - 1;
                    npyra5++; i++;
                }
                else if (elm == PENTA_6){
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                    section->penta6_stream[npenta6] = (int)ibuf[i] - 1;
                    npenta6++; i++;
                }
                else if (elm == HEXA_8){
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                    section->hexa8_stream[nhexa8] = (int)ibuf[i] - 1;
                    nhexa8++; i++;
                }
            }
        }
        else
        {
            cg_elements_read( file_index, ibase, izone, isec, ibuf, NULL );

            if (elm_type == BAR_2){
                for (i = 0; i < elm_data_size; i++){
                    section->bar2_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == TRI_3){
                for (i = 0; i < elm_data_size; i++){
                    section->tri3_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == QUAD_4){
                for (i = 0; i < elm_data_size; i++){
                    section->quad4_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == TETRA_4){
                for (i = 0; i < elm_data_size; i++){
                    section->tetra4_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == PYRA_5){
                for (i = 0; i < elm_data_size; i++){
                    section->pyra5_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == PENTA_6){
                for (i = 0; i < elm_data_size; i++){
                    section->penta6_stream[i] = (int)ibuf[i] - 1;
                }
            }
            else if (elm_type == HEXA_8){
                for (i = 0; i < elm_data_size; i++){
                    section->hexa8_stream[i] = (int)ibuf[i] - 1;
                }
            }
            /* (elm_type == MIXED) Loaded above */
        }

        _trace_( "   bar2:%i tri3:%i quad4:%i tetra4:%i pyra5:%i penta6:%i hexa8:%i\n"
            , section->num_lines, section->num_triangles, section->num_quads
            , section->num_tetrahedrons, section->num_pyramids
            , section->num_pentahedrons, section->num_hexahedrons 
            );

        free( ibuf );
    }
}

/* Imports an unstructured CGNS grid */
static void import_cgns_unstructured_grid
( cgnsGrid* grid, const int file_index, const int ibase, const int izone )
{
    int ier, ncoords;
    char zonename[33] = { '\0' };
    /* zero if elements not sorted */
    cgsize_t size[9] = { 0 };
    cgsize_t range_min[3] = { 0 };
    cgsize_t range_max[3] = { 0 };

    ier = cg_zone_read( file_index, ibase, izone, zonename, size );
    if (ier != CG_OK){
        return;
    }

    strcpy_safe( grid->label, zonename, 1024 );

    if (size[0] <= 0 && size[1] <= 0 && size[2] <= 0){
        _handle_error_( "Warning! Empty grid" );
        return;
    }

    /* vertex_size = size[0]; */
    /* Number of volume elements, surface elements should not count */
    /* cell_size = size[1]; */
    /* boundary_vertex_size = size[2]; */

    range_min[0] = 1;
    range_max[0] = size[0];

    ier = cg_ncoords( file_index, ibase, izone, &ncoords );
    grid->dim = ncoords;

    /* Import the vertices */
    import_vertex_coordinates( grid, file_index, ibase, izone, size[0], range_min, range_max );

    /* Import the elements connectivities */
    import_cgns_unstructured_sections( grid, file_index, ibase, izone );

    /* Get boundaries */
    //import_cgns_unstructured_boundaries( grid, file_index, ibase, izone );
}

/* Imports an structured CGNS grid zone */
static void import_cgns_structured_grid
( cgnsGrid* grid, const int file_index, const int ibase, const int izone )
{
    int ier;
    char coordname[33] = { '\0' };
    char zonename[33] = { '\0' };
    cgsize_t size[9] = { 0 };
    cgsize_t range_min[3];
    cgsize_t range_max[3];
    int num_points;

    ier = cg_zone_read( file_index, ibase, izone, zonename, size );
    if (ier != CG_OK){
        return;
    }

    strcpy_safe( grid->label, zonename, 1024 );

    if (size[0] <= 0 && size[1] <= 0 && size[2] <= 0){
        _handle_error_( "Warning! Empty grid" );
        return;
    }
    range_min[0] = 1;
    range_min[1] = 1;
    range_min[2] = 1;

    range_max[0] = size[0];
    range_max[1] = size[1];
    range_max[2] = size[2];

    grid->i_size = size[0];
    if (grid->i_size <= 0){
        grid->i_size = 1;
    }
    grid->j_size = size[1];
    if (grid->j_size <= 0){
        grid->j_size = 1;
    }
    grid->k_size = size[2];
    if (grid->k_size <= 0){
        grid->k_size = 1;
    }
    num_points = grid->i_size * grid->j_size * grid->k_size;

    /* Import the vertices */
    import_vertex_coordinates( grid, file_index, ibase, izone, num_points, range_min, range_max );
}

/* Imports a surface grid in CGNS file format */
cgnsGrid* cgnsGrid_import( const char* filename, int* num_grids )
{
    int ibase, izone, igrid;
    int ier;
    int file_index = -1;
    cgnsGrid* grid_array = nullptr;
    cgnsGrid* grid = nullptr;
    float version = 0;
    int precision = 0;
    int file_type = 0;
    int nbases = 0;         /* Number of bases */
    int nzones = 0;         /* Number of zones */
    int grid_length = 0;    /* Number of grids */
    ZoneType_t zonetype;
    char* time_str = getlocaltime();

    _trace_( "--- %s", time_str );
    _trace_( "Loading CGNS grid %s\n", filename );

    *num_grids = 0;
    ier = cg_open( filename, CG_MODE_READ, &file_index );
    if (ier != CG_OK){
        _handle_error_( "Cannot open filename %s", filename );
        return nullptr;
    }

    /* Function that shows the error message */
    cg_configure( CG_CONFIG_ERROR, err_func );

    /* Get the file format version */
    cg_version( file_index, &version );

    /* The value will be one of 32 bit, 64 bit, or 0 if not known */
    ier = cg_precision( file_index, &precision );

    ier = cg_get_file_type( file_index, &file_type );
    if (file_type == CG_FILE_ADF){
        /* OK */
    }
    else if (file_type == CG_FILE_HDF5){
        /* Not supported */
        _handle_error_( "HDF5 file format is not yet supported (only ADF is supported)" );
        return nullptr;
    }
    else{
        /* do not know */
        _handle_error_( "Unknown CGNS file format" );
        return nullptr;
    }

    ier = cg_nbases( file_index, &nbases );

    /* Count the total number of grids */
    for (ibase = 1; ibase <= nbases; ibase++){
        nzones = 0;
        ier = cg_nzones( file_index, ibase, &nzones );
        grid_length += nzones;
    }

    if (grid_length <= 0){
        _handle_error_( "File is empty: %s" );
        return nullptr;
    }

    /* Create the array of grids */
    _check_( grid_array = (cgnsGrid*)_malloc_( sizeof( cgnsGrid ) * grid_length ) );
    for (igrid = 0; igrid < grid_length; igrid++){
        cgnsGrid_init( &(grid_array[igrid]) );
    }
    for (igrid = 0; igrid < grid_length - 1; igrid++){
        grid_array[igrid].next = &(grid_array[igrid + 1]);
    }

    grid = grid_array;
    for (ibase = 1; ibase <= nbases; ibase++){
        nzones = 0;
        ier = cg_nzones( file_index, ibase, &nzones );
        for (izone = 1; izone <= nzones; izone++){
            grid->base = ibase;
            grid->zone = izone;

            ier = cg_zone_type( file_index, ibase, izone, &zonetype );

            if (zonetype == Structured){
                grid->type = owGridType_CGNS_STRUCT;
                import_cgns_structured_grid( grid, file_index, ibase, izone );
            }
            else if (zonetype == Unstructured){
                grid->type = owGridType_CGNS_UNSTRUCT;
                import_cgns_unstructured_grid( grid, file_index, ibase, izone );
            }
            else{
                _handle_error_( "User defined grids are not yet supported" );
            }

            /* Next grid */
            grid++;
        }
    }

    cg_close( file_index );

    *num_grids = grid_length;

    return grid_array;
}

#else

/* Imports a surface grid in CGNS file format */
cgnsGrid* cgnsGrid_import( const char* filename, int* num_grids )
{
    _handle_error_("Library is compiled without CGNS support!");
    return nullptr;
}

#endif /* NO CGNS */