/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Test bench for importing datasets in CSV (comma-separated-variables) format 
*******************************************************************************/

#include <stdio.h>

#include "common/log.h"
#include "dataset/csv/csv_import.h"

int main( int argc, char *argv[] )
{
    size_t var_len;
    csvVarNames dataset = csv_inquiry
        ( "../test/naca0012/naca0012.gg.csv", &var_len );

    for (size_t i = 0; i < dataset.num_vars; i++){
        printf( "[%i] %s\n", i, dataset.var_names[i] );
    }

    csvDataSet* data = csv_import
        ( "../test/naca0012/naca0012.gg.csv", "dudy ,dudx, dudz, ," );

    int k = 0;
    _debug_( "\n" );
    for (size_t i = 0; i < data->var_len; i++){
        for (size_t j = 0; j < 5; j++){
            if (data->stream[k] > 2e11){
                _debug_( "none " );
            }
            else{
                _debug_( "%3.5f ", data->stream[k] );
            }
            k++;
        }
        _debug_( "\n" );
    }

    getchar();
}

