
#include <malloc.h>
#include <memory.h>
#include <math.h>

#include "domino_nurbs/domino_nurbs.h"
#include "common/log.h"

template< typename T >
struct Matrix
{
    int n_rows;
    int n_columns;

    T* stream;

    /* Deafult constructor */
    Matrix()
    {
        n_rows = 0;
        n_columns = 0;
        stream = nullptr;
    }

    /* Constructor */
    Matrix( const int n_rows, const int n_columns)
    {
        this->n_rows = n_rows;
        this->n_columns = n_columns;
        this->stream = (T*)malloc( n_rows * n_columns * sizeof(T) );
    }

    ~Matrix()
    {
        if (stream != nullptr){
            free( stream );
        }
    }

    T* operator[]( const int row ) const
    {
        if (stream == nullptr || row >= n_rows){
            throw( "matrix is empty!" );
            return nullptr;
        }

        return &(stream[row*n_columns]);
    }
};


template< typename T >
static bool check_zero( T& a )
{
    if (a > (T)0 || a < (T)0){
        return false;
    }

    return false;
}


template< typename T > 
static void swap( T& a, T& b ){
    T t = a;
    a = b;
    b = t;
}


template< typename T > 
static T absf( const T& a )
{
    return a < 0 ? -a : a;
}


/**
  * Find a non zero pivot that is closest to one
  */
template< typename T > 
static void swap_non_zero_row(const Matrix< T >& a, T* d, const int k)
{
    T min = -1;
    int k_row = -1;

    for (int i = k; i < a.n_rows; i++){
        T aik = a[i][k];
        T v1 = absf(absf(aik) - 1);
        if (check_zero(aik) == false && (v1 < min || min < 0)){
            k_row = i;
            min = v1;
        }
    }

    if (k_row < 0){
        if (check_zero(a[k][k]) == true){
            _handle_error_("Indeterm solution!!!");
        }
        return;
    }

   swap(d[k_row], d[k]);
   for (int j = 0; j < a.n_columns; j++){
       swap(a[k][j], a[k_row][j]);
   }
}

template< typename T > 
void matrix_trace( const Matrix< T >& m )
{
    int k = 0;
    for (int i = 0; i < m.n_rows; i++){
        trace("\n");
        for (int j = 0; j < m.n_columns; j++){
            trace("\t %.4f", m.stream[k++]);
        }
    }
}

/**
  * This is a light implementation of gauss triangulation 
  * for solving linear system of equations
  */
template< typename T > 
T* gauss_triangulation( const Matrix< T >& m, const T* ind )
{
    Matrix< T > a( m.n_rows, m.n_columns );
    T* d = (T*)malloc(sizeof(T) * m.n_rows);
    T* sol = (T*)malloc(sizeof(T) * m.n_rows);

    memcpy( d, ind, m.n_rows * sizeof(T) );
    memcpy(a.stream, m.stream, m.n_rows * m.n_columns * sizeof(T));

    //trace("\n");
    //matrix_trace(a);

    for (int k = 0; k < a.n_rows-1; k++){
        // find the non zero pivot closest to one
        swap_non_zero_row(a, d, k);

        for (int i = k+1; i < a.n_rows; i++){            
            //trace("\n");
            //matrix_trace(a);

            T aik = a[i][k];
            if (check_zero(aik) == false){
                // find zeros in the row, so all the column has zeros
                for (int j = 0; j < a.n_columns; j++){
                    a[i][j] = a[i][j] * a[k][k] - a[k][j] * aik;
                }
                d[i] = d[i] * a[k][k] - d[k] * aik;
            }

            //trace("\n");
            //matrix_trace(a);
        }

        // normalize with the pivot
        T akk = a[k][k];
        for (int ii = 0; ii < a.n_rows; ii++){
            for (int jj = 0; jj < a.n_columns; jj++){
                a[ii][jj] /= akk;
            }
            d[ii] /= akk;
        }
            //trace("\n");
            //matrix_trace(a);

    }

    for (int i = 0; i < m.n_rows; i++){
        sol[i] = 0;
    }

    for (int i = m.n_rows-1; i >= 0; i--){
        sol[i] = d[i];
        for (int j = 0; j < m.n_columns; j++){
            if (i != j){
                sol[i] -= sol[j] * a[i][j];
            }
        }
        sol[i] /= a[i][i];
    }

    if ( d!= nullptr ){
        free( d );
    }

    return sol;
}


void calculate_basis6_11()
{
    double ms[] = 
    {   1, -1,  1, -1,  1, -1   
    ,   1,  1,  1,  1,  1,  1
    ,   0,  1, -2,  3, -4,  5
    ,   0,  1,  2,  3,  4,  5
    ,   0,  0,  2, -6, 12,-20
    ,   0,  0,  2,  6, 12, 20     
    };

    Matrix< double > m(6,6);
    memcpy( m.stream, ms, 6*6*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_( "*4\n" );
    for (int k = 0; k < 6; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( "%.2f\t", b[i] * 4 );
        }
    }
}


void calculate_basis2_11()
{
    double ms[] = 
    {   1, -1
    ,   1,  1 
    };

    Matrix< double > m(2,2);
    memcpy( m.stream, ms, 2*2*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_("*2\n");
    for (int k = 0; k < 2; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 2; i++){
            _debug_( "%.2f\t", b[i] * 2 );
        }
    }
}


void calculate_basis3_11()
{
    double ms[] = 
    {   1,  -1, 1
    ,   1,  1,  1
    ,   0,  1, -2  
    };

    Matrix< double > m(3,3);
    memcpy( m.stream, ms, 3*3*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_( "*4\n" );
    for (int k = 0; k < 3; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 3; i++){
            _debug_( "%.2f\t", b[i] * 4 );
        }
    }
}

void calculate_basis4_11()
{
    double ms[] = 
    {   1, -1,  1, -1  
    ,   1,  1,  1,  1
    ,   0,  1, -2,  3
    ,   0,  1,  2,  3  
    };

    Matrix< double > m(4,4);
    memcpy( m.stream, ms, 4*4*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_( "*4\n" );
    for (int k = 0; k < 4; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( "%.2f\t", b[i] * 4 );
        }
    }
}

void calculate_basis5_11()
{
    double ms[] = 
    {   1, -1,  1, -1,  1   
    ,   1,  1,  1,  1,  1
    ,   0,  1, -2,  3, -4
    ,   0,  1,  2,  3,  4
    ,   0,  0,  2, -6, 12   
    };

    Matrix< double > m(5,5);
    memcpy( m.stream, ms, 5*5*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_( "*%i\n", 16 * 3 );
    for (int k = 0; k < 5; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 5; i++){
            _debug_( "%.2f\t", b[i] * 16 * 3 );
        }
    }
}

void calculate_basis2_01()
{
    double ms[] = 
    {   1,  0
    ,   1,  1
    };

    Matrix< double > m(2,2);
    memcpy( m.stream, ms, 2*2*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    _debug_( "\n" );
    for (int k = 0; k < 2; k++){
        d[k] = 1;
        b = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 2; i++){
            _debug_( " \t%.2f", b[i] );
        }
    }
}

void calculate_basis3_01()
{
    double ms[] = 
    {   1,  0,  0
    ,   1,  1,  1
    ,   0,  1,  2
    ,   0,  1,  0.5
    };

    Matrix< double > m(4,4);
    memcpy( m.stream, ms, 4*4*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b;

    d[0] = 1;
    b = gauss_triangulation( m, d );
    d[0] = 0;

    _debug_( "\n" );
    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b[i] );
    }

    d[1] = 1;
    b = gauss_triangulation( m, d );
    d[1] = 0;

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b[i] );
    }

    d[2] = 0;
    b = gauss_triangulation( m, d );
    d[2] = 0;

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b[i] );
    }

    d[2] = 0;
    b = gauss_triangulation( m, d );
    d[2] = 0;

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b[i] );
    }
}

void calculate_basis4_01()
{
    double ms[] = 
    {   1,  0,  0,  0
    ,   1,  1,  1,  1
    ,   0,  1,  0,  0
    ,   0,  1,  2,  3   
    };

    Matrix< double > m(4,4);
    memcpy( m.stream, ms, 4*4*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b[6];

    _debug_( "\n" );
    for (int k = 0; k < 4; k++){
        d[k] = 1;
        b[k] = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( " \t%.2f", b[k][i] );
        }
    }

    double c[6][6] = {0};
    
    const int x0 = 0;
    const int x1 = 1;
    const int t0 = 2;
    const int t1 = 3;

    for (int i = 0; i < 4; i++){
        /* LEFT
         * T0 = (X1 - X0)  
         * T1 = (X2 - X0) / 2 */
        c[0][i] = 0;
        c[1][i] = b[x0][i] - b[t0][i] - b[t1][i]/2;
        c[2][i] = b[x1][i] + b[t0][i];
        c[3][i] = b[t1][i]/2;
    }
    _debug_( "\n\n left *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < 4; i++){
        /* CENTRAL
         * T0 = (X1 - X-1) / 2  
         * T1 = (X2 - X0) / 2 */
        c[0][i] = - b[t0][i]/2;
        c[1][i] = b[x0][i] - b[t1][i]/2;
        c[2][i] = b[x1][i] + b[t0][i]/2;
        c[3][i] = b[t1][i]/2;
    }
    _debug_( "\n\n central *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < 4; i++){
        /* RIGHT
         * T0 = (X1 - X-1) / 2  
         * T1 = (X1 - X0)*/
        c[0][i] = - b[t0][i]/2;
        c[1][i] = b[x0][i] - b[t1][i];
        c[2][i] = b[x1][i] + b[t0][i]/2 + b[t1][i];
        c[3][i] = 0;
    }
    _debug_( "\n\n right *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < 4; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }
}


void calculate_basis5_01()
{
    const int K = 5;
    double ms[5*5] = 
    {   1,  0,  0,  0,  0
    ,   1,  1,  1,  1,  1
    ,   0,  1,  0,  0,  0
    ,   0,  1,  2*0.5,  3*0.25,  4*0.125
    ,   0,  0,  2,  0,  0   
    };

    Matrix< double > m(5,5);
    memcpy( m.stream, ms, 5*5*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b[6];

    _debug_( "\n" );
    for (int k = 0; k < K; k++){
        d[k] = 1;
        b[k] = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.2f", b[k][i] );
        }
    }

    double c[6][6] = {0};
    
    const int x0 = 0;
    const int x1 = 1;
    const int t0 = 2;
    const int t1 = 3;
    const int c0 = 4;
    const int c1 = 5;

    for (int i = 0; i < K; i++){
        /* LEFT
         * T0 = (X1 - X0)  
         * T1 = (X2 - X0) / 2 
         * C0 = 0  
         * C1 = X2 + X0 - 2*X1
         */
        /* X-1*/ c[0][i] = 0;
        /* X0 */ c[1][i] = b[x0][i] - b[t0][i] - b[t1][i]/2;
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i];
        /* X2 */ c[3][i] = b[t1][i]/2;
    }
    _debug_( "\n\n left *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < K; i++){
        /* CENTRAL
         * T0 = (X1 - X_1) / 2  
         * T1 = (X2 - X0) / 2 
         * C0 = X1 + X_1 - 2*X0  
         * C1 = X2 + X0 - 2*X1
         */
        /* X-1*/ c[0][i] = - b[t0][i]/2;
        /* X0 */ c[1][i] = b[x0][i] - b[t1][i]/2;
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i]/2;
        /* X2 */ c[3][i] = b[t1][i]/2;
    }
    _debug_( "\n\n central *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < K; i++){
        /* RIGHT
         * T0 = (X1 - X-1) / 2  
         * T1 = (X1 - X0)
         * C0 = X1 + X_1 - 2*X0  
         * C1 = 0
         */
        /* X-1*/ c[0][i] = - b[t0][i]/2;
        /* X0 */ c[1][i] = b[x0][i] - b[t1][i];
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i]/2 + b[t1][i];
        /* X2 */ c[3][i] = 0;
    }
    _debug_( "\n\n right *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }
}


void calculate_basis6_01()
{
    const int K = 6;
    double ms[6*6] = 
    {   1,  0,  0,  0,  0,  0
    ,   1,  1,  1,  1,  1,  1
    ,   0,  1,  0,  0,  0,  0
    ,   0,  1,  2,  3,  4,  5
    ,   0,  0,  2,  0,  0,  0     
    ,   0,  0,  2,  6, 12, 20     
    };

    Matrix< double > m(6,6);
    memcpy( m.stream, ms, 6*6*sizeof(double) );

    double d[] = {0,0,0,0,0,0};
    double* b[6];

    _debug_( "\n" );
    for (int k = 0; k < K; k++){
        d[k] = 1;
        b[k] = gauss_triangulation( m, d );
        d[k] = 0;

        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.2f", b[k][i] );
        }
    }

    double c[6][6] = {0};
    
    const int x0 = 0;
    const int x1 = 1;
    const int t0 = 2;
    const int t1 = 3;
    const int c0 = 4;
    const int c1 = 5;

    for (int i = 0; i < K; i++){
        /* LEFT
         * T0 = (X1 - X0)  
         * T1 = (X2 - X0) / 2 
         * C0 = 0  
         * C1 = X2 + X0 - 2*X1
         */
        /* X-1*/ c[0][i] = 0;
        /* X0 */ c[1][i] = b[x0][i] - b[t0][i] - b[t1][i]/2 + b[c1][i];
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i] - 2*b[c1][i];
        /* X2 */ c[3][i] = b[t1][i]/2 + b[c1][i];
    }
    _debug_( "\n\n left *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < K; i++){
        /* CENTRAL
         * T0 = (X1 - X_1) / 2  
         * T1 = (X2 - X0) / 2 
         * C0 = X1 + X_1 - 2*X0  
         * C1 = X2 + X0 - 2*X1
         */
        /* X-1*/ c[0][i] = - b[t0][i]/2 + b[c0][i];
        /* X0 */ c[1][i] = b[x0][i] - b[t1][i]/2 - 2*b[c0][i] + b[c1][i];
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i]/2 - 2*b[c1][i] + b[c0][i];
        /* X2 */ c[3][i] = b[t1][i]/2 + b[c1][i] ;
    }
    _debug_( "\n\n central *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }

    for (int i = 0; i < K; i++){
        /* RIGHT
         * T0 = (X1 - X-1) / 2  
         * T1 = (X1 - X0)
         * C0 = X1 + X_1 - 2*X0  
         * C1 = 0
         */
        /* X-1*/ c[0][i] = - b[t0][i]/2 + b[c0][i];
        /* X0 */ c[1][i] = b[x0][i] - b[t1][i] - 2*b[c0][i];
        /* X1 */ c[2][i] = b[x1][i] + b[t0][i]/2 + b[t1][i] + b[c0][i];
        /* X2 */ c[3][i] = 0;
    }
    _debug_( "\n\n right *2" );
    for (int k = 0; k < 4; k++){
        _debug_( "\n" );
        for (int i = 0; i < K; i++){
            _debug_( " \t%.0f,", c[k][i] * 2 );
        }
    }
}


void calculate_basisCC()
{
    double ms2[16] = 
    {   1,  0,  0,  0
    ,   1,  1,  1,  1
    ,   0,  1,  2,  3
    ,   0,  0,  2,  6  
    };

    Matrix< double > m(4,4);
    memcpy( m.stream, ms2, 16*sizeof(double) );

    double d2[] = {0,0,0,1};
    double* b2;

    b2 = gauss_triangulation( m, d2 );

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b2[i] );
    }

    double m_1[16] = 
    {   1,  0,  0,  0
    ,   1,  1,  1,  1
    ,   0,  1,  2,  3
    ,   0,  0,  2,  0  
    };

    memcpy( m.stream, m_1, 16*sizeof(double) );

    double d_1[] = {0,0,0,1};
    double* b_1;

    b_1 = gauss_triangulation( m, d_1 );

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b_1[i] );
    }

    double m0[16] = 
    {   1,  0,  0,  0
    ,   1,  1,  1,  1
    ,   0,  1,  2,  3
    ,   0,  0,  2,  6  
    };

    memcpy( m.stream, m0, 16*sizeof(double) );

    double d0[] = {1,0,-0.5,2*b_1[2]};
    double* b0;

    b0 = gauss_triangulation( m, d0 );

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b0[i] );
    }

    double m1[16] = 
    {   1,  0,  0,  0
    ,   1,  1,  1,  1
    ,   0,  1,  0,  0
    ,   0,  0,  2,  0  
    };

    memcpy( m.stream, m1, 16*sizeof(double) );

    double d1[] = {0,1,0.5,2*b0[2]+6*b0[3]};
    double* b1;

    b1 = gauss_triangulation( m, d1 );

    _debug_( "\n" );
    for (int i = 0; i < 4; i++){
        _debug_( " \t%.2f", b1[i] );
    }
}

void check_basis()
{
    const int order = 2;

    for (int i = -2; i < 129; i++){
        NurbsFloat b[7];
        NurbsFloat t = (NurbsFloat)i/128;
        int iknot = nurbs_controlbox_basis_function( b, t, 7, order, 0 );
        _debug_( "\n%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f %i"
            , t, b[0], b[1], b[2], b[3], b[4], b[5], b[6]
            , b[0]+b[1]+b[2]+b[3]+b[4]+b[5], iknot);
    }
}

static void bezier_basis3L
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,   0,   0, 	 0,   0,   0 },
        { 1,  -1.5, 0.5, 0,	  0,   0 },
        { 0,   2, 	-1,  0,   0,   0 },
        { 0,  -0.5, 0.5, 0,   0,   0 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];
}

static void bezier_basisC0
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,   0,   0, 	 0,   0,   0 },
        { 2,  -2,   0,  -3,	  5,   -2 },
        { 0,   2, 	0,  6,   -10,   4 },
        { 0,   0,   0,  -3,   5,   -2 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];

    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}

static void bezier_basis_duplicate_cp
    ( NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  0,   0,   0,  0,   0 },
        { 2,   -1,  -1,  -6,  10,  -4 },
        { 0,   1, 	1,   9, -15,   6 },
        { 0,   0,   0,  -3,   5,  -2 }
    };

    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];

    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}

static void bezier_basis3C
    ( NurbsFloat* basis_1       /* (out) Basis coefficient for i-1 */
    , NurbsFloat* basis0        /* (out) Basis coefficient for i+0 */
    , NurbsFloat* basis1        /* (out) Basis coefficient for i+1 */
    , NurbsFloat* basis2        /* (out) Basis coefficient for i+2 */
    , const NurbsFloat t        /* Normalized parameter to the interval {0,1} */
    )
{
    NurbsFloat t2, t3, t4, t5;
    const NurbsFloat coeff[4][6] = 
    {
        { 0,  -1,   1,   3,  -5,   2 },
        { 2,   0,  -2,  -9,  15,  -6 },
        { 0,   1, 	1,   9, -15,   6 },
        { 0,   0,   0,  -3,   5,  -2 }
    };
    
    t2 = t*t;
    t3 = t2*t;
    t4 = t3*t;
    t5 = t4*t;

    *basis_1 = coeff[0][0] + t*coeff[0][1] + t2*coeff[0][2] + t3*coeff[0][3] 
        + t4*coeff[0][4] + t5*coeff[0][5];

    *basis0 = coeff[1][0] + t*coeff[1][1] + t2*coeff[1][2] + t3*coeff[1][3] 
        + t4*coeff[1][4] + t5*coeff[1][5];

    *basis1 = coeff[2][0] + t*coeff[2][1] + t2*coeff[2][2] + t3*coeff[2][3] 
        + t4*coeff[2][4] + t5*coeff[2][5];

    *basis2 = coeff[3][0] + t*coeff[3][1] + t2*coeff[3][2] + t3*coeff[3][3] 
        + t4*coeff[3][4] + t5*coeff[3][5];

    *basis_1 /= 2;
    *basis0 /= 2;
    *basis1 /= 2;
    *basis2 /= 2;
}


void compare_border_basis()
{
    for (int i = 0; i < 129; i++){
        NurbsFloat b0, b1, b2, b3;
        NurbsFloat t = (NurbsFloat)i/128;
        //bezier_basis_duplicate_cp( &b0, &b1, &b2, t );
        //bezier_basis3L( &b0, &b1, &b2, t );
        //bezier_basisC0( &b0, &b1, &b2, t );
        bezier_basis3C( &b0, &b1, &b2, &b3, t );

        _debug_( "\n%.3f %.4f %.4f %.4f", t, b0, b1, b2 );
    }
}

void check_basis_derivate()
{
    const int order = 3;
    const NurbsFloat epsilon = 1e-6;

    for (int i = -2; i < 130; i++){
        NurbsFloat t = (NurbsFloat)i/128;
        NurbsFloat t0 = t-epsilon;
        NurbsFloat t1 = t+epsilon;

        NurbsFloat d[6];
        NurbsFloat b[6];
        NurbsFloat b0[6];
        NurbsFloat b1[6];
        int iknot = nurbs_controlbox_d_basis_function( d, b, t, 6, order, 1 );
        nurbs_controlbox_basis_function( b, t, 6, order, 1 );
        nurbs_controlbox_basis_function( b0, t0, 6, order, 1 );
        nurbs_controlbox_basis_function( b1, t1, 6, order, 1 );

        _debug_( "\n%.2f {%.4f %.4f} {%.4f %.4f} {%.4f %.4f} {%.4f %.4f} {%.4f %.4f} {%.4f %.4f} %i"
            , t
            , (b1[0]-b0[0])/(2*epsilon), d[0]
            , (b1[1]-b0[1])/(2*epsilon), d[1]
            , (b1[2]-b0[2])/(2*epsilon), d[2]
            , (b1[3]-b0[3])/(2*epsilon), d[3]
            , (b1[4]-b0[4])/(2*epsilon), d[4]
            , (b1[5]-b0[5])/(2*epsilon), d[5]
            , iknot
            );
    }
}


void check_derivates()
{
    NurbsControlBox* cb = nurbs_controlbox_import_ascii( "../test/controlbox_1.nurbs", nullptr );
    NurbsFloat epsilon = 1e-6;

    cb->order_u = 2;
    cb->order_v = 2;
    cb->order_w = 2;

    for (int iu = -1; iu < 9; iu++){
        for (int iv = -1; iv < 9; iv++){
            for (int iw = -1; iw < 9; iw++){
                NurbsFloat tu = (NurbsFloat)iu/8;
                NurbsFloat tv = (NurbsFloat)iv/8;
                NurbsFloat tw = (NurbsFloat)iw/8;
                NurbsFloat tu0 = tu-epsilon;
                NurbsFloat tu1 = tu+epsilon;
                NurbsFloat tv0 = tv-epsilon;
                NurbsFloat tv1 = tv+epsilon;
                NurbsFloat tw0 = tw-epsilon;
                NurbsFloat tw1 = tw+epsilon;
                NurbsVector3 p, pu0, pu1, pv0, pv1, pw0, pw1;
                NurbsVector3 du, dv, dw;

                nurbs_controlbox_get_derivatives( cb, &du, &dv, &dw, &p, tu, tv, tw );

                pu0 = nurbs_controlbox_get_point( cb, tu0, tv, tw );
                pu1 = nurbs_controlbox_get_point( cb, tu1, tv, tw );

                pv0 = nurbs_controlbox_get_point( cb, tu, tv0, tw );
                pv1 = nurbs_controlbox_get_point( cb, tu, tv1, tw );

                pw0 = nurbs_controlbox_get_point( cb, tu, tv, tw0 );
                pw1 = nurbs_controlbox_get_point( cb, tu, tv, tw1 );

                _debug_( "\n" );
                _debug_( "\nu:%.3f {%.4f %.4f} {%.4f %.4f} {%.4f %.4f}"
                    , tu
                    , (pu1.x - pu0.x)/(2*epsilon), du.x
                    , (pu1.y - pu0.y)/(2*epsilon), du.y
                    , (pu1.z - pu0.z)/(2*epsilon), du.z
                    );

                _debug_( "\nv:%.3f {%.4f %.4f} {%.4f %.4f} {%.4f %.4f}"
                    , tv
                    , (pv1.x - pv0.x)/(2*epsilon), dv.x
                    , (pv1.y - pv0.y)/(2*epsilon), dv.y
                    , (pv1.z - pv0.z)/(2*epsilon), dv.z
                    );

                _debug_( "\nw:%.3f {%.4f %.4f} {%.4f %.4f} {%.4f %.4f}"
                    , tw
                    , (pw1.x - pw0.x)/(2*epsilon), dw.x
                    , (pw1.y - pw0.y)/(2*epsilon), dw.y
                    , (pw1.z - pw0.z)/(2*epsilon), dw.z
                    );
            }
        }
    }
}


void check_interpolation()
{
    NurbsControlBox cb;

    nurbs_controlbox_alloc( &cb, 10, 1, 1 );

    cb.order_u = 1;
    cb.order_v = 0;
    cb.order_w = 0;
    //cb.basis_equation = 0, 1;
    cb.basis_equation = 1;

    cb.cp[0][0][0].x = 0;
    cb.cp[0][0][0].y = 0;
    cb.cp[0][0][0].z = 0;

    cb.cp[1][0][0].x = 1;
    cb.cp[1][0][0].y = 1;
    cb.cp[1][0][0].z = 0;

    cb.cp[2][0][0].x = 2;
    cb.cp[2][0][0].y = 1;
    cb.cp[2][0][0].z = 0;

    cb.cp[3][0][0].x = 3;
    cb.cp[3][0][0].y = 0;
    cb.cp[3][0][0].z = 0;

    cb.cp[4][0][0].x = 3;
    cb.cp[4][0][0].y = 0;
    cb.cp[4][0][0].z = 0;

    cb.cp[5][0][0].x = 3;
    cb.cp[5][0][0].y = 1;
    cb.cp[5][0][0].z = 0;

    cb.cp[6][0][0].x = 4;
    cb.cp[6][0][0].y = 1;
    cb.cp[6][0][0].z = 0;

    cb.cp[7][0][0].x = 5;
    cb.cp[7][0][0].y = 1;
    cb.cp[7][0][0].z = 0;

    cb.cp[8][0][0].x = 6;
    cb.cp[8][0][0].y = 0;
    cb.cp[8][0][0].z = 0;

    cb.cp[9][0][0].x = 7;
    cb.cp[9][0][0].y = 0;
    cb.cp[9][0][0].z = 0;

    for (NurbsFloat t = 0; t <= 1.11; t +=1./256){
        NurbsVector3 p;
        //cb.order_u = 0;
        //cb.order_u = 1;
        //cb.order_u = 2;
        cb.order_u = 2;
        p = nurbs_controlbox_get_point( &cb, t, 0, 1 );
        _debug_( "\n%.4f %.4f ", p.x, p.y );
    }
}

void check_inversion_xy()
{
    NurbsVector3 p[128];
    NurbsVector3 uvw[128];

    double _2pi = atan(1.)*8;

    for (int i = 0; i < 128; i++){
        p[i].x = sin( (_2pi * i) / 128 );
        p[i].y = cos( (_2pi * i) / 128 );
        p[i].z = 0;
    }

    NurbsControlBox cb;
    nurbs_controlbox_alloc( &cb, 4, 4, 1 );

    cb.order_u = 3;
    cb.order_v = 3;
    cb.order_w = 0;

    cb.cp[0][0][0].x = -2;
    cb.cp[0][0][0].y = -2;
    cb.cp[0][0][0].z = 0;

    cb.cp[1][0][0].x = -1;
    cb.cp[1][0][0].y = -2;
    cb.cp[1][0][0].z = 0;

    cb.cp[2][0][0].x = 1;
    cb.cp[2][0][0].y = -2;
    cb.cp[2][0][0].z = 0;

    cb.cp[3][0][0].x = 2;
    cb.cp[3][0][0].y = -2;
    cb.cp[3][0][0].z = 0;


    cb.cp[0][1][0].x = -2;
    cb.cp[0][1][0].y = -1;
    cb.cp[0][1][0].z = 0;

    cb.cp[1][1][0].x = -1;
    cb.cp[1][1][0].y = -1;
    cb.cp[1][1][0].z = 0;

    cb.cp[2][1][0].x = 1;
    cb.cp[2][1][0].y = -1;
    cb.cp[2][1][0].z = 0;

    cb.cp[3][1][0].x = 2;
    cb.cp[3][1][0].y = -1;
    cb.cp[3][1][0].z = 0;


    cb.cp[0][2][0].x = -2;
    cb.cp[0][2][0].y = 1;
    cb.cp[0][2][0].z = 0;

    cb.cp[1][2][0].x = -1;
    cb.cp[1][2][0].y = 1;
    cb.cp[1][2][0].z = 0;

    cb.cp[2][2][0].x = 1;
    cb.cp[2][2][0].y = 1;
    cb.cp[2][2][0].z = 0;

    cb.cp[3][2][0].x = 2;
    cb.cp[3][2][0].y = 1;
    cb.cp[3][2][0].z = 0;


    cb.cp[0][3][0].x = -2;
    cb.cp[0][3][0].y = 2;
    cb.cp[0][3][0].z = 0;

    cb.cp[1][3][0].x = -1;
    cb.cp[1][3][0].y = 2;
    cb.cp[1][3][0].z = 0;

    cb.cp[2][3][0].x = 1;
    cb.cp[2][3][0].y = 2;
    cb.cp[2][3][0].z = 0;

    cb.cp[3][3][0].x = 2;
    cb.cp[3][3][0].y = 2;
    cb.cp[3][3][0].z = 0;

    for (int i = 0; i < 128; i++){
        NurbsFloat err;
        uvw[i] = nurbs_controlbox_inversion( &cb, p[i].x, p[i].y, p[i].z, 1e-6, &err );
        _debug_( "\n%i %g", i, err );
    }

    for (int i = 0; i < 128; i++){
        NurbsVector3 q = nurbs_controlbox_get_point( &cb, uvw[i].x, uvw[i].y, uvw[i].z );
        _debug_( "\n%f %f    %f %f", p[i].x, q.x, p[i].y, q.y );
    }
}

void check_inversion_full()
{
    NurbsControlBox* cb = new(NurbsControlBox);
    nurbs_controlbox_alloc( cb, 3, 3, 3 );
    for (int iu = 0; iu < cb->cp_length_u; iu++){
        for (int iv = 0; iv < cb->cp_length_v; iv++){
            for (int iw = 0; iw < cb->cp_length_w; iw++){
                cb->cp[iu][iv][iw].x = iu;
                cb->cp[iu][iv][iw].y = iv*iw;
                cb->cp[iu][iv][iw].z = sqrt((double)iw);
            }
        }
    }

    cb->order_u = 2;
    cb->order_v = 2;
    cb->order_w = 1;


    int num_cb;
    cb = nurbs_controlbox_import_ascii("domino/dpw-w1_testcase/controlbox.nurbs", &num_cb);
    NurbsFloat epsilon = 1e-6;
    NurbsVector3 t, p0, p1;

    for (NurbsFloat tu = 0; tu <= 1; tu += 0.1){
        for (NurbsFloat tv = 0; tv <= 1; tv += 0.1){
            for (NurbsFloat tw = 0; tw <= 1; tw += 0.1){
                NurbsFloat err;

                p0 = nurbs_controlbox_get_point( cb, tu, tv, tw );

                t = nurbs_controlbox_inversion( cb, p0.x, p0.y, p0.z, 1e-6, &err );
                p1 = nurbs_controlbox_get_point( cb, t.x, t.y, t.z );
                err = (p1.x-p0.x)*(p1.x-p0.x) 
                    + (p1.y-p0.y)*(p1.y-p0.y);

                if (err > 0.01){
                    for (double uu = 0; uu <= 1; uu += 0.001){
                        p1 = nurbs_controlbox_get_point( cb, uu, tv, tw );
                        _debug_( "\n%f %f %f %f %f", uu
                            , (p1.x - p0.x) 
                            , (p1.y - p0.y) 
                            , (p1.z - p0.z)
                            , (p1.x - p0.x) * (p1.x - p0.x)
                            + (p1.y - p0.y) * (p1.y - p0.y)
                            + (p1.z - p0.z) * (p1.z - p0.z)
                            );
                        p1 = nurbs_controlbox_get_point( cb, tu, uu, tw );
                        _debug_( "%f %f %f %f"
                            , (p1.x - p0.x) 
                            , (p1.y - p0.y) 
                            , (p1.z - p0.z)
                            , (p1.x - p0.x) * (p1.x - p0.x)
                            + (p1.y - p0.y) * (p1.y - p0.y)
                            + (p1.z - p0.z) * (p1.z - p0.z)
                            );
                        p1 = nurbs_controlbox_get_point( cb, tu, tv, uu );
                        _debug_( "%f %f %f %f"
                            , (p1.x - p0.x) 
                            , (p1.y - p0.y) 
                            , (p1.z - p0.z)
                            , (p1.x - p0.x) * (p1.x - p0.x)
                            + (p1.y - p0.y) * (p1.y - p0.y)
                            + (p1.z - p0.z) * (p1.z - p0.z)
                            );
                    }
                    printf("\nFAIL!");
                }
            }
        }
    }
}


void check_nurbs_basis()
{
    NurbsControlBox cb;
    NurbsSurface nurbs2;
    NurbsSurface nurbs3;

    nurbs_controlbox_alloc( &cb, 4, 1, 1 );

    cb.order_u = 3;

    nurbs_surface_alloc( &nurbs2, 7, 1, 2, 0 );

    nurbs2.knot_u[0] = 0;
    nurbs2.knot_u[1] = 0;
    nurbs2.knot_u[2] = 0;
    nurbs2.knot_u[3] = 1;
    nurbs2.knot_u[4] = 2;
    nurbs2.knot_u[5] = 3;
    nurbs2.knot_u[6] = 4;
    nurbs2.knot_u[7] = 5;
    nurbs2.knot_u[8] = 5;
    nurbs2.knot_u[9] = 5;


    nurbs2.knot_v[0] = 0;
    nurbs2.knot_v[1] = 1;

    nurbs_surface_alloc( &nurbs3, 8, 1, 3, 0 );

    nurbs3.knot_u[0] = 0;
    nurbs3.knot_u[1] = 0;
    nurbs3.knot_u[2] = 0;
    nurbs3.knot_u[3] = 0;
    nurbs3.knot_u[4] = 1;
    nurbs3.knot_u[5] = 2;
    nurbs3.knot_u[6] = 3;
    nurbs3.knot_u[7] = 4;
    nurbs3.knot_u[8] = 5;
    nurbs3.knot_u[9] = 5;
    nurbs3.knot_u[10] = 5;
    nurbs3.knot_u[11] = 5;

    nurbs3.knot_v[0] = 0;
    nurbs3.knot_v[1] = 1;

    NurbsFloat cp_basis[12];
    NurbsFloat cp_d_basis[12];
    //NurbsFloat cp_d2_basis[5];

    NurbsFloat nurbs_basis[12];
    NurbsFloat nurbs_d_basis[12];
    //NurbsFloat nurbs_d2_basis[8];

    for (NurbsFloat t = 0; t < 5.0001; t += 0.025f){
        nurbs_basis_derivate_function( nurbs_d_basis, nurbs_basis
            , t, nurbs3.degree_u, nurbs3.knot_u, nurbs3.knot_length_u );
        nurbs_controlbox_d_basis_function( cp_d_basis, cp_basis, t/5, cb.cp_length_u, cb.order_u, 1 );

        NurbsFloat t0 =  t*t*t;
        NurbsFloat t1 =  -3*t*t*t+9*t*t-5*t+1;
        NurbsFloat t2 =  3*t*t*t-12*t*t+14*t-4;
        NurbsFloat t3 =   (1-t)*(1-t)*(1-t)/6;
        //trace("\n[%f] {%f %f} {%f %f} {%f %f} {%f %f} {%f %f}"
        //    ,t
        //    , nurbs_basis[0], t0
        //    , nurbs_basis[1], t0
        //    , nurbs_basis[2], t0
        //    , nurbs_basis[3], t0
        //    , nurbs_basis[4], t0
        //    );

        //trace("\n%f %f %f %f %f", t, t0, t1, t2, t3);
        //trace("\n%f %f %f %f %f %f %f %f %f %f %f", t/4
        //    , nurbs_basis[0], nurbs_basis[1], nurbs_basis[2], nurbs_basis[3], nurbs_basis[4]
        //, nurbs_basis[5], nurbs_basis[6], nurbs_basis[7], nurbs_basis[8]);
        _debug_( "\n%f %f %f %f %f %f %f %f %f", t / 5
            , cp_basis[0], cp_basis[1], cp_basis[2], cp_basis[3], cp_basis[4]
            , cp_basis[5], cp_basis[6]);

/*        trace("\n[%f] {%f %f} {%f %f} {%f %f} {%f %f} {%f %f}"
            ,t
            , nurbs_d_basis[0], cp_d_basis[0]
            , nurbs_d_basis[1], cp_d_basis[1]
            , nurbs_d_basis[2], cp_d_basis[2]
            , nurbs_d_basis[3], cp_d_basis[3]
            , nurbs_d_basis[4], cp_d_basis[4]
            );  */  
    }
}


void main()
{
    calculate_basis2_11();
    calculate_basis3_11();
    calculate_basis4_11();
    calculate_basis5_11();
    calculate_basis6_11();
    calculate_basis2_01();
    calculate_basis3_01();
    calculate_basis4_01();
    calculate_basis5_01();
    calculate_basis6_01();
    calculate_basisCC();
    check_basis();
    check_basis_derivate();
    check_derivates();
    check_interpolation();
    check_inversion_xy();
    check_inversion_full();
    check_nurbs_basis();
    compare_border_basis();
}

