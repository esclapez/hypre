/*BHEADER**********************************************************************
 * (c) 1998   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 * $Revision$
 *********************************************************************EHEADER*/

#include "headers.h"
#include "amg.h"

/*****************************************************************************
 *
 * Routine for driving the setup phase of AMG
 *
 *****************************************************************************/

int
hypre_AMGSetup( void            *amg_vdata,
                hypre_CSRMatrix *A,
                hypre_Vector    *f,
                hypre_Vector    *u         )
{
   hypre_AMGData   *amg_data = amg_vdata;

   /* Data Structure variables */

   hypre_CSRMatrix **A_array;
   hypre_BCSRMatrix **B_array;
   hypre_Vector    **F_array;
   hypre_Vector    **U_array;
   hypre_CSRMatrix **P_array;
   hypre_BCSRMatrix **PB_array;

   int             **dof_func_array;
   int              *dof_func;
   int              *coarse_dof_func;
   int              *domain_i;
   int              *domain_j;

   int             **CF_marker_array;   
   double           *relax_weight;
   double           *unit_vector;
   double            strong_threshold;
   double            A_trunc_factor;
   double            P_trunc_factor;

   int      num_variables;
   int      max_levels; 
   int      A_max_elmts;
   int      P_max_elmts;
   int      amg_ioutdat;
   int      interp_type;
   int      num_functions;
   int      agg_levels;
   int      agg_coarsen_type;
   int      agg_interp_type;
   int      num_jacs;
   int      main_coarsen_type;
   int      main_interp_type;
   int use_block_flag;
 
   /* Local variables */
   int              *CF_marker;
   int              *new_CF_marker;
   hypre_CSRMatrix  *S;
   hypre_CSRMatrix  *S2;
   hypre_CSRMatrix  *S3;
   hypre_CSRMatrix  *P;
   hypre_CSRMatrix  *A_H;
   hypre_CSRMatrix  *A_tilde;
   hypre_BCSRMatrix *B;
   hypre_BCSRMatrix *PB;
   double *S2_data;
   int       num_nz;

   int       num_levels;
   int       level;
   int       coarse_size;
   int       first_coarse_size;
   int       fine_size;
   int       not_finished_coarsening = 1;
   int       Setup_err_flag;
   int       coarse_threshold = 9;
   int       i, j;
   int	     coarsen_type;
   int	    *grid_relax_type;
   int	     relax_type;
   int	     num_relax_steps;
   int	    *schwarz_option;
   int       mode, S_mode;
   int       num_domains;
   int      *i_domain_dof;
   int      *j_domain_dof;
   double   *domain_matrixinverse;

   int* fake_dof_func;

   char f_name[256];
   FILE* f_out;

   mode = hypre_AMGDataMode(amg_data);
   S_mode = 0;
   relax_weight = hypre_AMGDataRelaxWeight(amg_data);
   num_relax_steps = hypre_AMGDataNumRelaxSteps(amg_data);
   grid_relax_type = hypre_AMGDataGridRelaxType(amg_data);
   max_levels = hypre_AMGDataMaxLevels(amg_data);
   amg_ioutdat = hypre_AMGDataIOutDat(amg_data);
   main_interp_type = hypre_AMGDataInterpType(amg_data);
   num_functions = hypre_AMGDataNumFunctions(amg_data);
   relax_type = grid_relax_type[0];
   schwarz_option = hypre_AMGDataSchwarzOption(amg_data);
   A_trunc_factor = hypre_AMGDataATruncFactor(amg_data);
   P_trunc_factor = hypre_AMGDataPTruncFactor(amg_data);
   P_max_elmts = hypre_AMGDataPMaxElmts(amg_data);
   A_max_elmts = hypre_AMGDataAMaxElmts(amg_data);
   use_block_flag = hypre_AMGDataUseBlockFlag(amg_data);
 
   dof_func = hypre_AMGDataDofFunc(amg_data);

   A_array = hypre_CTAlloc(hypre_CSRMatrix*, max_levels);
   B_array = hypre_CTAlloc(hypre_BCSRMatrix*, max_levels);
   P_array = hypre_CTAlloc(hypre_CSRMatrix*, max_levels-1);
   PB_array = hypre_CTAlloc(hypre_BCSRMatrix*, max_levels-1);
   CF_marker_array = hypre_CTAlloc(int*, max_levels);
   dof_func_array = hypre_CTAlloc(int*, max_levels);
   coarse_dof_func = NULL;

   if (schwarz_option[0] > -1)
   {
      hypre_AMGDataNumDomains(amg_data) = hypre_CTAlloc(int, max_levels);
      hypre_AMGDataIDomainDof(amg_data) = hypre_CTAlloc(int*, max_levels);
      hypre_AMGDataJDomainDof(amg_data) = hypre_CTAlloc(int*, max_levels);
      hypre_AMGDataDomainMatrixInverse(amg_data) = 
				hypre_CTAlloc(double*, max_levels);
      for (i=0; i < max_levels; i++)
      {
         hypre_AMGDataIDomainDof(amg_data)[i] = NULL;
         hypre_AMGDataJDomainDof(amg_data)[i] = NULL;
         hypre_AMGDataDomainMatrixInverse(amg_data)[i] = NULL;
      }
   }

   if (num_functions > 0) dof_func_array[0] = dof_func;

   A_array[0] = A;

   use_block_flag = use_block_flag && (num_functions > 1);
   hypre_AMGDataUseBlockFlag(amg_data) = use_block_flag;
   if(use_block_flag) {
     A_tilde = hypre_CSRMatrixDeleteZeros(A, 0.0);
     if(A_tilde) {
       B = hypre_BCSRMatrixFromCSRMatrix(A_tilde, num_functions,
					 num_functions);
       hypre_CSRMatrixDestroy(A_tilde);
     }
     else {
       B = hypre_BCSRMatrixFromCSRMatrix(A, num_functions,
					 num_functions);
     }
     B_array[0] = B;
   }

   /*----------------------------------------------------------
    * Initialize hypre_AMGData
    *----------------------------------------------------------*/

   num_variables = hypre_CSRMatrixNumRows(A);
   unit_vector = hypre_CTAlloc(double, num_variables);

   for (i=0; i < num_variables; i++)
      unit_vector[i] = 1;

   hypre_AMGDataNumVariables(amg_data) = num_variables;

   not_finished_coarsening = 1;
   level = 0;
  
   strong_threshold = hypre_AMGDataStrongThreshold(amg_data);

   main_coarsen_type = hypre_AMGDataCoarsenType(amg_data);
   agg_levels = hypre_AMGDataAggLevels(amg_data);
   agg_coarsen_type = hypre_AMGDataAggCoarsenType(amg_data);
   agg_interp_type = hypre_AMGDataAggInterpType(amg_data);
   num_jacs = hypre_AMGDataNumJacs(amg_data);

   /*-----------------------------------------------------
    *  Enter Coarsening Loop
    *-----------------------------------------------------*/

   while (not_finished_coarsening)
   {
     /*****************************************************************/

      if (level < agg_levels)
      {
	 coarsen_type = agg_coarsen_type;
	 interp_type = agg_interp_type;
      }
      else
      {
	 coarsen_type = main_coarsen_type;
	 interp_type = main_interp_type;
      }

      if(use_block_flag) {
	A_tilde = hypre_BCSRMatrixCompress(B_array[level]);
	fine_size = hypre_CSRMatrixNumRows(A_tilde);
	fake_dof_func = hypre_CTAlloc(int, fine_size);
	hypre_AMGCreateS(A_tilde, strong_threshold, S_mode, fake_dof_func, &S);
	/* hypre_AMGCoarsen(A_tilde, strong_threshold, A_tilde,*/
	hypre_AMGCoarsen(A_tilde, strong_threshold, S,
			 &CF_marker, &coarse_size);
	hypre_TFree(fake_dof_func);
	hypre_CSRMatrixDestroy(A_tilde);

	CF_marker_array[level] = CF_marker;

	PB = hypre_BCSRMatrixBuildInterpD(B_array[level], CF_marker,
					  S, coarse_size);
	P_array[level] = hypre_BCSRMatrixToCSRMatrix(PB);
	if(P_trunc_factor > 0 || P_max_elmts > 0) {
	  hypre_AMGTruncation(P_array[level], P_trunc_factor, P_max_elmts);
	}

	hypre_BCSRMatrixDestroy(PB);
	PB_array[level] = hypre_BCSRMatrixFromCSRMatrix(P_array[level],
							num_functions,
							num_functions); 

	hypre_AMGBuildCoarseOperator(P_array[level], A_array[level],
				     P_array[level], &A_array[level + 1]);
	if(A_trunc_factor > 0 || A_max_elmts > 0) {
	  hypre_AMGOpTruncation(A_array[level + 1], 
				A_trunc_factor, A_max_elmts);
	}

	B_array[level + 1] = hypre_BCSRMatrixFromCSRMatrix(A_array[level + 1],
							   num_functions,
							   num_functions);

	level++;

	if (level+1 >= max_levels ||
	    coarse_size == fine_size || 
	    coarse_size <= coarse_threshold) {
	  not_finished_coarsening = 0;
	}

	continue;
      }
      /*****************************************************************/

      fine_size = hypre_CSRMatrixNumRows(A_array[level]);

      /*-------------------------------------------------------------
       * Select coarse-grid points on 'level' : returns CF_marker
       * for the level.  Returns strength matrix, S  
       *--------------------------------------------------------------*/

      if (relax_weight[level] == 0.0)
      {
	 hypre_CSRMatrixScaledNorm(A_array[level], &relax_weight[level]);
	 if (relax_weight[level] != 0.0)
            relax_weight[level] = (4.0/3.0)/relax_weight[level];
         else
           printf (" Warning ! Matrix norm is zero !!!");
      }

      if (schwarz_option[level] > -1)
      {
         /* if (level == 0) 
      	    hypre_AMGNodalSchwarzSmoother (A_array[level], dof_func_array[level],
				    	num_functions,
					schwarz_option[level],
					&i_domain_dof, &j_domain_dof,
					&domain_matrixinverse,
					&num_domains);
            else */
      	 hypre_AMGCreateDomainDof (A_array[level],
				&i_domain_dof, &j_domain_dof,
				&domain_matrixinverse,
				&num_domains); 
         hypre_AMGDataIDomainDof(amg_data)[level] = i_domain_dof;
         hypre_AMGDataJDomainDof(amg_data)[level] = j_domain_dof;
         hypre_AMGDataNumDomains(amg_data)[level] = num_domains;
         hypre_AMGDataDomainMatrixInverse(amg_data)[level] = 
			domain_matrixinverse;
      }

      if (coarsen_type == 1)
      {
	 hypre_AMGCreateS(A_array[level], strong_threshold, S_mode, 
	fake_dof_func, &S);
	 hypre_AMGCoarsenRuge(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size); 
         /*              A_array[level], &CF_marker, &coarse_size); */
      }
      else if (coarsen_type == 2)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
	 hypre_AMGCoarsenRugeLoL(A_array[level], strong_threshold,
				 S, &CF_marker, &coarse_size); 
			  /* A_array[level], &CF_marker, &coarse_size); */ 
      }
      /* begin HANS added */
      else if (coarsen_type == 4)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
         hypre_AMGCoarsenwLJP(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size); 
      }
      else if (coarsen_type == 5)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
	 hypre_AMGCoarsenRugeOnePass(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size); 
      }
      /* end HANS added */
      else if (coarsen_type == 8)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
         hypre_AMGCoarsen(A_array[level], strong_threshold,
                       S, &CF_marker, &first_coarse_size);  
	 hypre_AMGCreate2ndS(S, first_coarse_size, CF_marker, 2, &S2);
         hypre_AMGCoarsen(S2, strong_threshold,
                       S2, &new_CF_marker, &coarse_size); 
         hypre_CSRMatrixDestroy(S2);
         hypre_AMGCorrectCFMarker(CF_marker, fine_size, new_CF_marker); 
	 hypre_TFree(new_CF_marker);
	 /* if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S2);
	 S3 = hypre_CSRMatrixMultiply(S2,S2);
	 hypre_AMGCompressS(S3, 2);
	 S = hypre_CSRMatrixAdd(S2,S3);
         hypre_CSRMatrixDestroy(S2);
         hypre_CSRMatrixDestroy(S3);
         hypre_AMGCoarsen(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size);  */
      }
      else if (coarsen_type == 10)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
         hypre_AMGCoarsen(A_array[level], strong_threshold,
                       S, &CF_marker, &first_coarse_size); 
	 hypre_AMGCreate2ndS(S, first_coarse_size, CF_marker, 1, &S2);
         hypre_AMGCoarsen(S2, strong_threshold,
                       S2, &new_CF_marker, &coarse_size); 
         hypre_CSRMatrixDestroy(S2);
         hypre_AMGCorrectCFMarker(CF_marker, fine_size, new_CF_marker); 
	 hypre_TFree(new_CF_marker);
	 /*if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S2);
	 S3 = hypre_CSRMatrixMultiply(S2,S2);
	 hypre_AMGCompressS(S3, 1);
	 S = hypre_CSRMatrixAdd(S2,S3);
         hypre_CSRMatrixDestroy(S2);
         hypre_CSRMatrixDestroy(S3);
         hypre_AMGCoarsen(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size); */
			  /* A_array[level], &CF_marker, &coarse_size); */ 
      }
      else if (coarsen_type == 9)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
	 hypre_AMGCoarsenRugeOnePass(A_array[level], strong_threshold,
                       S, &CF_marker, &first_coarse_size); 
	 hypre_AMGCreate2ndS(S, first_coarse_size, CF_marker, 2, &S2);
	 hypre_AMGCoarsenRugeOnePass(S2, strong_threshold,
                       S2, &new_CF_marker, &coarse_size); 
         hypre_CSRMatrixDestroy(S2);
         hypre_AMGCorrectCFMarker(CF_marker, fine_size, new_CF_marker); 
	 hypre_TFree(new_CF_marker);
	 /* if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S2);
         S2_data = hypre_CSRMatrixData(S2);
	 num_nz = hypre_CSRMatrixI(S2)[hypre_CSRMatrixNumRows(S2)];
         for (i=0; i < num_nz; i++)
	   S2_data[i] = -S2_data[i];
	 S3 = hypre_CSRMatrixMultiply(S2,S2);
	 hypre_AMGCompressS(S3, 2);
	 S = hypre_CSRMatrixAdd(S2,S3);
         hypre_CSRMatrixDestroy(S2);
         hypre_CSRMatrixDestroy(S3);
	 hypre_AMGCoarsenRugeLoL(A_array[level], -strong_threshold,
				 S, &CF_marker, &coarse_size); */
			  /* A_array[level], &CF_marker, &coarse_size); */ 
      }
      else if (coarsen_type == 11)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
	 hypre_AMGCoarsenRugeOnePass(A_array[level], strong_threshold,
                       S, &CF_marker, &first_coarse_size); 
	 hypre_AMGCreate2ndS(S, first_coarse_size, CF_marker, 1, &S2);
	 hypre_AMGCoarsenRugeOnePass(S2, strong_threshold,
                       S2, &new_CF_marker, &coarse_size); 
         hypre_CSRMatrixDestroy(S2);
         hypre_AMGCorrectCFMarker(CF_marker, fine_size, new_CF_marker); 
	 hypre_TFree(new_CF_marker);
	 /* if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S2);
         S2_data = hypre_CSRMatrixData(S2);
	 num_nz = hypre_CSRMatrixI(S2)[hypre_CSRMatrixNumRows(S2)];
         for (i=0; i < num_nz; i++)
	   S2_data[i] = -S2_data[i];
	 S3 = hypre_CSRMatrixMultiply(S2,S2);
	 hypre_AMGCompressS(S3, 1);
	 S = hypre_CSRMatrixAdd(S2,S3);
         hypre_CSRMatrixDestroy(S2);
         hypre_CSRMatrixDestroy(S3);
	 hypre_AMGCoarsenRugeLoL(A_array[level], -strong_threshold,
				 S, &CF_marker, &coarse_size); */
			  /* A_array[level], &CF_marker, &coarse_size); */ 
      }
      else if (coarsen_type == 12)
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
         hypre_AMGCoarsenwLJP(A_array[level], strong_threshold,
                       S, &CF_marker, &first_coarse_size); 
	 hypre_AMGCreate2ndS(S, first_coarse_size, CF_marker, 1, &S2);
         hypre_AMGCoarsenwLJP(S2, strong_threshold,
                       S2, &new_CF_marker, &coarse_size); 
         hypre_CSRMatrixDestroy(S2);
         hypre_AMGCorrectCFMarker(CF_marker, fine_size, new_CF_marker); 
	 hypre_TFree(new_CF_marker);
      }
      else if (coarsen_type == 3)
      {
         hypre_AMGCoarsenCR(A_array[level], strong_threshold,
			relax_weight[level], relax_type, 
			num_relax_steps, &CF_marker, &coarse_size); 
      }
      else
      {
	 if (mode == 1 || mode == 2) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
         hypre_AMGCoarsen(A_array[level], strong_threshold,
                       S, &CF_marker, &coarse_size); 
                       /* A_array[level], &CF_marker, &coarse_size); */
      }
      /* if no coarse-grid, stop coarsening */
      if (coarse_size == 0)
         break;

      CF_marker_array[level] = CF_marker;
      
      /*-------------------------------------------------------------
       * Build prolongation matrix, P, and place in P_array[level] 
       *--------------------------------------------------------------*/

      if (interp_type == 2)
      {
          hypre_AMGBuildCRInterp(A_array[level], 
                                 CF_marker_array[level], 
                                 coarse_size,
                                 num_relax_steps,
                                 relax_type,
                                 relax_weight[level],
                                 &P);
      }
      else if (interp_type == 1)
      {
	  if (coarsen_type == 3)
             hypre_AMGBuildRBMInterp(A_array[level], 

                                  CF_marker_array[level], 
                                  A_array[level], 
                                  dof_func_array[level],
                                  num_functions,
                                  &coarse_dof_func,
                                  &P);
          else
             hypre_AMGBuildRBMInterp(A_array[level], 
                                  CF_marker_array[level], 
                                  S, 
                                  dof_func_array[level],
                                  num_functions,
                                  &coarse_dof_func,
                                  &P);
          /* this will need some cleanup, to make sure we do the right thing 
             when it is a scalar function */ 
      }
      else if (coarsen_type == 3)
      {
          hypre_AMGBuildInterp(A_array[level], CF_marker_array[level], 
					A_array[level], dof_func_array[level],
                                        &coarse_dof_func, &P);
      }
      else if (interp_type == 3)
      {
	 hypre_CreateDomain(CF_marker_array[level], A_array[level], coarse_size,
                             dof_func_array[level], &coarse_dof_func,
                             &domain_i, &domain_j);
         hypre_InexactPartitionOfUnityInterpolation(&P, 
				hypre_CSRMatrixI(A_array[level]), 
				hypre_CSRMatrixJ(A_array[level]), 
				hypre_CSRMatrixData(A_array[level]), 
				unit_vector, domain_i, domain_j,
				coarse_size, fine_size);
	 hypre_TFree(domain_i);
	 hypre_TFree(domain_j);
      }
      else if (interp_type == 5)
      {
          hypre_AMGBuildMultipass(A_array[level], 
                                 CF_marker_array[level], 
                                 S,
                             dof_func_array[level], &coarse_dof_func, &P);
      }
      else if (interp_type == 6)
      {
          hypre_AMGBuildMultipass(A_array[level], CF_marker_array[level], S,
                             dof_func_array[level], &coarse_dof_func, &P);
        for(i=0;i<num_jacs;i++)
	  hypre_AMGJacobiIterate(A_array[level], CF_marker_array[level], S,
				 dof_func_array[level], &coarse_dof_func, &P);
      }
      else 
      {
	 if (S) hypre_CSRMatrixDestroy(S);
	 S_mode = 0;
	 if (mode == 1) S_mode = 1;
	 hypre_AMGCreateS(A_array[level], strong_threshold, 
			S_mode, dof_func_array[level], &S);
	hypre_AMGBuildInterp(A_array[level], CF_marker_array[level], S,
                             dof_func_array[level], &coarse_dof_func, &P);
      }

      if (P_trunc_factor > 0 || P_max_elmts > 0)
         hypre_AMGTruncation(P,P_trunc_factor,P_max_elmts);

      /*printf("END computing level %d interpolation matrix; =======\n", level);
      */

      dof_func_array[level+1] = coarse_dof_func;
      P_array[level] = P; 
      
      if (amg_ioutdat == 5 && level == 0)
      {
         hypre_CSRMatrixPrint(S,"S_mat");
      }
      if (coarsen_type != 3 ) hypre_CSRMatrixDestroy(S);
 
      /*-------------------------------------------------------------
       * Build coarse-grid operator, A_array[level+1] by R*A*P
       *--------------------------------------------------------------*/

      hypre_AMGBuildCoarseOperator(P_array[level], A_array[level] , 
                                   P_array[level], &A_H);
      if (A_trunc_factor > 0 || A_max_elmts > 0)
         hypre_AMGOpTruncation(A_H,A_trunc_factor,A_max_elmts);

      ++level;
      A_array[level] = A_H;

      if (level+1 >= max_levels || 
          coarse_size == fine_size || 
          coarse_size <= coarse_threshold)
         not_finished_coarsening = 0;
   } 
   
   /*-----------------------------------------------------------------------
    * enter all the stuff created, A[level], P[level], CF_marker[level],
    * for levels 1 through coarsest, into amg_data data structure
    *-----------------------------------------------------------------------*/

   num_levels = level+1;
   hypre_AMGDataNumLevels(amg_data) = num_levels;
   hypre_AMGDataCFMarkerArray(amg_data) = CF_marker_array;
   hypre_AMGDataAArray(amg_data) = A_array;
   hypre_AMGDataPArray(amg_data) = P_array;
   hypre_AMGDataBArray(amg_data) = B_array;
   hypre_AMGDataPBArray(amg_data) = PB_array;

   hypre_AMGDataDofFuncArray(amg_data) = dof_func_array;
   hypre_AMGDataNumFunctions(amg_data) = num_functions;	
   /*-----------------------------------------------------------------------
    * Setup F and U arrays
    *-----------------------------------------------------------------------*/

   F_array = hypre_CTAlloc(hypre_Vector*, num_levels);
   U_array = hypre_CTAlloc(hypre_Vector*, num_levels);

   F_array[0] = f;
   U_array[0] = u;

   for (j = 1; j < num_levels; j++)
   {
     F_array[j] = hypre_SeqVectorCreate(hypre_CSRMatrixNumRows(A_array[j]));
     hypre_SeqVectorInitialize(F_array[j]);

     U_array[j] = hypre_SeqVectorCreate(hypre_CSRMatrixNumRows(A_array[j]));
     hypre_SeqVectorInitialize(U_array[j]);
   }

   hypre_AMGDataFArray(amg_data) = F_array;
   hypre_AMGDataUArray(amg_data) = U_array;

   /*-----------------------------------------------------------------------
    * Print some stuff
    *-----------------------------------------------------------------------*/

   if (amg_ioutdat == 1 || amg_ioutdat == 3)
	hypre_AMGSetupStats(amg_data);

   if (amg_ioutdat == -3)
   {  
      char     fnam[255];

      int j;

      for (j = 0; j < level+1; j++)
      {
         sprintf(fnam,"SP_A_%d.ysmp",j);
         hypre_CSRMatrixPrint(A_array[j],fnam);

      }                         

      for (j = 0; j < level; j++)
      { 
         sprintf(fnam,"SP_P_%d.ysmp",j);
         hypre_CSRMatrixPrint(P_array[j],fnam);
      }   
   } 

   hypre_TFree(unit_vector);
   Setup_err_flag = 0;
   return(Setup_err_flag);
}  
