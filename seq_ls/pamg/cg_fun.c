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
 * csr matrix-vector implementation of PCG interface routines.
 *
 *****************************************************************************/

#include "headers.h"

/*--------------------------------------------------------------------------
 * hypre_CGCAlloc
 *--------------------------------------------------------------------------*/

char *
hypre_CGCAlloc( int count,
                 int elt_size )
{
   return( hypre_CAlloc( count, elt_size ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGFree
 *--------------------------------------------------------------------------*/

int
hypre_CGFree( char *ptr )
{
   int ierr = 0;

   hypre_Free( ptr );

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_CGCreateVector
 *--------------------------------------------------------------------------*/

void *
hypre_CGCreateVector( void *vvector )
{
   hypre_Vector *vector = vvector;
   hypre_Vector *new_vector;

   new_vector = hypre_SeqVectorCreate( hypre_VectorSize(vector));
   hypre_SeqVectorInitialize(new_vector);

   return ( (void *) new_vector );
}

/*--------------------------------------------------------------------------
 * hypre_CGCreateVectorArray
 *--------------------------------------------------------------------------*/

void *
hypre_CGCreateVectorArray(int n, void *vvector )
{
   hypre_Vector *vector = vvector;
   hypre_Vector **new_vector;
   int i;

   new_vector = hypre_CTAlloc(hypre_Vector*,n);
   for (i=0; i < n; i++)
   {
      new_vector[i] = hypre_SeqVectorCreate( hypre_VectorSize(vector));
      hypre_SeqVectorInitialize(new_vector[i]);
   }

   return ( (void *) new_vector );
}

/*--------------------------------------------------------------------------
 * hypre_CGDestroyVector
 *--------------------------------------------------------------------------*/

int
hypre_CGDestroyVector( void *vvector )
{
   hypre_Vector *vector = vvector;

   return( hypre_SeqVectorDestroy( vector ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGMatvecCreate
 *--------------------------------------------------------------------------*/

void *
hypre_CGMatvecCreate( void   *A,
                       void   *x )
{
   void *matvec_data;

   matvec_data = NULL;

   return ( matvec_data );
}

/*--------------------------------------------------------------------------
 * hypre_CGMatvec
 *--------------------------------------------------------------------------*/

int
hypre_CGMatvec( void   *matvec_data,
                 double  alpha,
                 void   *A,
                 void   *x,
                 double  beta,
                 void   *y           )
{
   return ( hypre_CSRMatrixMatvec ( alpha,
                              (hypre_CSRMatrix *) A,
                              (hypre_Vector *) x,
                               beta,
                              (hypre_Vector *) y ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGMatvecT
 *--------------------------------------------------------------------------*/

int
hypre_CGMatvecT(void   *matvec_data,
                 double  alpha,
                 void   *A,
                 void   *x,
                 double  beta,
                 void   *y           )
{
   return ( hypre_CSRMatrixMatvecT( alpha,
                              (hypre_CSRMatrix *) A,
                              (hypre_Vector *) x,
                               beta,
                              (hypre_Vector *) y ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGMatvecDestroy
 *--------------------------------------------------------------------------*/

int
hypre_CGMatvecDestroy( void *matvec_data )
{
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CGInnerProd
 *--------------------------------------------------------------------------*/

double
hypre_CGInnerProd( void *x, 
                    void *y )
{
   return ( hypre_SeqVectorInnerProd( (hypre_Vector *) x,
                                (hypre_Vector *) y ) );
}


/*--------------------------------------------------------------------------
 * hypre_CGCopyVector
 *--------------------------------------------------------------------------*/

int
hypre_CGCopyVector( void *x, 
                     void *y )
{
   return ( hypre_SeqVectorCopy( (hypre_Vector *) x,
                                 (hypre_Vector *) y ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGClearVector
 *--------------------------------------------------------------------------*/

int
hypre_CGClearVector( void *x )
{
   return ( hypre_SeqVectorSetConstantValues( (hypre_Vector *) x, 0.0 ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGScaleVector
 *--------------------------------------------------------------------------*/

int
hypre_CGScaleVector( double  alpha,
                      void   *x     )
{
   return ( hypre_SeqVectorScale( alpha, (hypre_Vector *) x ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGAxpy
 *--------------------------------------------------------------------------*/

int
hypre_CGAxpy( double alpha,
               void   *x,
               void   *y )
{
   return ( hypre_SeqVectorAxpy( alpha, (hypre_Vector *) x,
                              (hypre_Vector *) y ) );
}

/*--------------------------------------------------------------------------
 * hypre_CGCommInfo
 *--------------------------------------------------------------------------*/

int
hypre_CGCommInfo( void   *A, int *my_id, int *num_procs)
{
   *num_procs = 1;
   *my_id = 0;
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CGIdentitySetup
 *--------------------------------------------------------------------------*/

int
hypre_CGIdentitySetup( void *vdata,
                        void *A,
                        void *b,
                        void *x     )

{
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CGIdentity
 *--------------------------------------------------------------------------*/

int
hypre_CGIdentity( void *vdata,
                   void *A,
                   void *b,
                   void *x     )

{
   return( hypre_CGCopyVector( b, x ) );
}


