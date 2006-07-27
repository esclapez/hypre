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
 *
 *****************************************************************************/

#include "headers.h"
#include "smg.h"

#define OLDRAP 1
#define NEWRAP 0

/*--------------------------------------------------------------------------
 * hypre_SMGCreateRAPOp
 *
 *   Wrapper for 2 and 3d CreateRAPOp routines which set up new coarse
 *   grid structures.
 *--------------------------------------------------------------------------*/
 
hypre_StructMatrix *
hypre_SMGCreateRAPOp( hypre_StructMatrix *R,
                      hypre_StructMatrix *A,
                      hypre_StructMatrix *PT,
                      hypre_StructGrid   *coarse_grid )
{
   hypre_StructMatrix    *RAP;
   hypre_StructStencil   *stencil;

#if NEWRAP
   int                    cdir;
   int                    P_stored_as_transpose = 1;
#endif

   stencil = hypre_StructMatrixStencil(A);

#if OLDRAP
   switch (hypre_StructStencilDim(stencil)) 
   {
      case 2:
      RAP = hypre_SMG2CreateRAPOp(R ,A, PT, coarse_grid);
      break;
    
      case 3:
      RAP = hypre_SMG3CreateRAPOp(R ,A, PT, coarse_grid);
      break;
   } 
#endif

#if NEWRAP
   switch (hypre_StructStencilDim(stencil)) 
   {
      case 2:
      cdir = 1;
      RAP = hypre_SemiCreateRAPOp(R ,A, PT, coarse_grid, cdir,
                                     P_stored_as_transpose);
      break;
    
      case 3:
      cdir = 2;
      RAP = hypre_SemiCreateRAPOp(R ,A, PT, coarse_grid, cdir,
                                     P_stored_as_transpose);
      break;
   } 
#endif

   return RAP;
}

/*--------------------------------------------------------------------------
 * hypre_SMGSetupRAPOp
 *
 * Wrapper for 2 and 3d, symmetric and non-symmetric routines to calculate
 * entries in RAP. Incomplete error handling at the moment. 
 *--------------------------------------------------------------------------*/
 
int
hypre_SMGSetupRAPOp( hypre_StructMatrix *R,
                     hypre_StructMatrix *A,
                     hypre_StructMatrix *PT,
                     hypre_StructMatrix *Ac,
                     hypre_Index         cindex,
                     hypre_Index         cstride )
{
   int ierr = 0;
 
#if NEWRAP
   int                    cdir;
   int                    P_stored_as_transpose = 1;
#endif

   hypre_StructStencil   *stencil;

   stencil = hypre_StructMatrixStencil(A);

#if OLDRAP
   switch (hypre_StructStencilDim(stencil)) 
   {

      case 2:

      /*--------------------------------------------------------------------
       *    Set lower triangular (+ diagonal) coefficients
       *--------------------------------------------------------------------*/
      ierr = hypre_SMG2BuildRAPSym(A, PT, R, Ac, cindex, cstride);

      /*--------------------------------------------------------------------
       *    For non-symmetric A, set upper triangular coefficients as well
       *--------------------------------------------------------------------*/
      if(!hypre_StructMatrixSymmetric(A))
      {
         ierr += hypre_SMG2BuildRAPNoSym(A, PT, R, Ac, cindex, cstride);
         /*-----------------------------------------------------------------
          *    Collapse stencil for periodic probems on coarsest grid.
          *-----------------------------------------------------------------*/
         ierr = hypre_SMG2RAPPeriodicNoSym(Ac, cindex, cstride);
      }
      else
      {
         /*-----------------------------------------------------------------
          *    Collapse stencil for periodic problems on coarsest grid.
          *-----------------------------------------------------------------*/
         ierr = hypre_SMG2RAPPeriodicSym(Ac, cindex, cstride);
      }

      break;

      case 3:

      /*--------------------------------------------------------------------
       *    Set lower triangular (+ diagonal) coefficients
       *--------------------------------------------------------------------*/
      ierr = hypre_SMG3BuildRAPSym(A, PT, R, Ac, cindex, cstride);

      /*--------------------------------------------------------------------
       *    For non-symmetric A, set upper triangular coefficients as well
       *--------------------------------------------------------------------*/
      if(!hypre_StructMatrixSymmetric(A))
      {
         ierr += hypre_SMG3BuildRAPNoSym(A, PT, R, Ac, cindex, cstride);
         /*-----------------------------------------------------------------
          *    Collapse stencil for periodic probems on coarsest grid.
          *-----------------------------------------------------------------*/
         ierr = hypre_SMG3RAPPeriodicNoSym(Ac, cindex, cstride);
      }
      else
      {
         /*-----------------------------------------------------------------
          *    Collapse stencil for periodic problems on coarsest grid.
          *-----------------------------------------------------------------*/
         ierr = hypre_SMG3RAPPeriodicSym(Ac, cindex, cstride);
      }

      break;

   }
#endif

#if NEWRAP
   switch (hypre_StructStencilDim(stencil)) 
   {

      case 2:
      cdir = 1;
      ierr = hypre_SemiBuildRAP(A, PT, R, cdir, cindex, cstride,
                                       P_stored_as_transpose, Ac);
      break;

      case 3:
      cdir = 2;
      ierr = hypre_SemiBuildRAP(A, PT, R, cdir, cindex, cstride,
                                       P_stored_as_transpose, Ac);
      break;

   }
#endif
   hypre_StructMatrixAssemble(Ac);

   return ierr;
}

