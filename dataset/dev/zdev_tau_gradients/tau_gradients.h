/***
Author: Mario J. Martin <dominonurbs$gmail.com>

Calculates the gradients in a TAU grid.
*******************************************************************************/

#include "adp_data/adp_arrays.h"
#include "adp_data/adpTauPrimGrid.h"

/* Calculates the gradients using Green Gauss (NOT WORKING!!!) */
adpVector3dStream* adpTauGrid_gradients_gg
( const adpTauPrimGrid* grid      /* Tau's primary grid */
, const adpDoubleStream* var       /* Stream of variables */
);

/* Another algorithm to calculate the gradients (NOT WORKING!!!) */
adpVector3dStream* adpTauGrid_gradients_jacobian
( const adpTauPrimGrid* grid      /* Tau's primary grid */
, const adpDoubleStream* var      /* Stream of variables */
);

