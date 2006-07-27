/*BHEADER**********************************************************************
 * Copyright (c) 2006   The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by the HYPRE team <hypre-users@llnl.gov>, UCRL-CODE-222953.
 * All rights reserved.
 *
 * This file is part of HYPRE (see http://www.llnl.gov/CASC/hypre/).
 * Please see the COPYRIGHT_and_LICENSE file for the copyright notice, 
 * disclaimer and the GNU Lesser General Public License.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * $Revision$
 ***********************************************************************EHEADER*/


/******************************************************************************
 *
 * HYPRE_CGNR interface
 *
 *****************************************************************************/
#include "krylov.h"

/*--------------------------------------------------------------------------
 * HYPRE_CGNRCreate does not exist.  Call the appropriate function which
 * also specifies the vector type, e.g. HYPRE_ParCSRCGNRCreate
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * HYPRE_CGNRDestroy
 *--------------------------------------------------------------------------*/

int 
HYPRE_CGNRDestroy( HYPRE_Solver solver )
{
   return( hypre_CGNRDestroy( (void *) solver ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetup
 *--------------------------------------------------------------------------*/

int 
HYPRE_CGNRSetup( HYPRE_Solver solver,
                        HYPRE_Matrix A,
                        HYPRE_Vector b,
                        HYPRE_Vector x      )
{
   return( hypre_CGNRSetup( solver, A, b, x ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSolve
 *--------------------------------------------------------------------------*/

int 
HYPRE_CGNRSolve( HYPRE_Solver solver,
                        HYPRE_Matrix A,
                        HYPRE_Vector b,
                        HYPRE_Vector x      )
{
   return( hypre_CGNRSolve( solver, A, b, x ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetTol
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetTol( HYPRE_Solver solver,
                         double             tol    )
{
   return( hypre_CGNRSetTol( (void *) solver, tol ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetMinIter
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetMinIter( HYPRE_Solver solver,
                             int                min_iter )
{
   return( hypre_CGNRSetMinIter( (void *) solver, min_iter ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetMaxIter
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetMaxIter( HYPRE_Solver solver,
                             int                max_iter )
{
   return( hypre_CGNRSetMaxIter( (void *) solver, max_iter ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetStopCrit
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetStopCrit( HYPRE_Solver solver,
                             int                stop_crit )
{
   return( hypre_CGNRSetStopCrit( (void *) solver, stop_crit ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetPrecond
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetPrecond( HYPRE_Solver         solver,
                            HYPRE_PtrToSolverFcn precond,
                            HYPRE_PtrToSolverFcn precondT,
                            HYPRE_PtrToSolverFcn precond_setup,
                            HYPRE_Solver         precond_solver )
{
   return( hypre_CGNRSetPrecond( (void *) solver,
                                 precond, precondT, precond_setup,
                                 (void *) precond_solver ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRGetPrecond
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRGetPrecond( HYPRE_Solver   solver,
                            HYPRE_Solver  *precond_data_ptr )
{
   return( hypre_CGNRGetPrecond( (void *)         solver,
                                 (HYPRE_Solver *) precond_data_ptr ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRSetLogging
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRSetLogging( HYPRE_Solver solver,
                             int logging)
{
   return( hypre_CGNRSetLogging( (void *) solver, logging ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRGetNumIterations
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRGetNumIterations( HYPRE_Solver  solver,
                                   int                *num_iterations )
{
   return( hypre_CGNRGetNumIterations( (void *) solver, num_iterations ) );
}

/*--------------------------------------------------------------------------
 * HYPRE_CGNRGetFinalRelativeResidualNorm
 *--------------------------------------------------------------------------*/

int
HYPRE_CGNRGetFinalRelativeResidualNorm( HYPRE_Solver  solver,
                                               double             *norm   )
{
   return( hypre_CGNRGetFinalRelativeResidualNorm( (void *) solver, norm ) );
}
