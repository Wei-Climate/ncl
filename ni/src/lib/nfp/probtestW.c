#include <stdio.h>

/*
 * The following are the required NCAR Graphics include files.
 * They should be located in ${NCARG_ROOT}/include.
 */
#include "wrapper.h"

extern void NGCALLF(dttest,DTTEST)(double*,double*,double*,double*,double*,
                                   double*,int*,double*,double*,int*);

extern void NGCALLF(dftest,DFTEST)(double*,double*,double*,double*,double*,
                                   int*);

extern void NGCALLF(drtest,DRTEST)(double*,int*,double*);

extern void NGCALLF(deqvsiz,DEQVSIZ)(double*,int*,double*,double*,int*);

NhlErrorTypes ttest_W( void )
{
/*
 * Input array variables
 */
  void *ave1, *ave2, *var1, *var2, *s1, *s2;
  double *tmp_ave1, *tmp_ave2, *tmp_var1, *tmp_var2, *tmp_s1, *tmp_s2;
  NclScalar missing_ave1, missing_ave2, missing_rave1;
  NclScalar missing_dave1, missing_dave2;
  int has_missing_ave1, has_missing_ave2;
  logical *iflag, *tval_opt;
  int ndims_ave1, dsizes_ave1[NCL_MAX_DIMENSIONS];
  int ndims_ave2, dsizes_ave2[NCL_MAX_DIMENSIONS];
  int ndims_var1, dsizes_var1[NCL_MAX_DIMENSIONS];
  int ndims_var2, dsizes_var2[NCL_MAX_DIMENSIONS];
  int ndims_s1, dsizes_s1[NCL_MAX_DIMENSIONS];
  int ndims_s2, dsizes_s2[NCL_MAX_DIMENSIONS];
  NclBasicDataTypes type_ave1, type_ave2, type_var1, type_var2;
  NclBasicDataTypes type_s1, type_s2;
  int scalar_s1, scalar_s2, output_contains_msg;
/*
 * output variable 
 */
  void *prob;
  double *tmp_alpha, *tmp_tval;
  int ndims_prob, *dsizes_prob;
  NclBasicDataTypes type_prob;
/*
 * Declare various variables for random purposes.
 */
  int i, index_prob, index_s1, index_s2, size_ave1, size_prob, ier;
  int is_missing_ave1, is_missing_ave2, is_missing_var1, is_missing_var2;
  int is_missing_s1, is_missing_s2;
/*
 * Retrieve parameters
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */
  ave1 = (void*)NclGetArgValue(
          0,
          8,
          &ndims_ave1,
          dsizes_ave1,
          &missing_ave1,
          &has_missing_ave1,
          &type_ave1,
          2);

  var1 = (void*)NclGetArgValue(
          1,
          8,
          &ndims_var1,
          dsizes_var1,
          NULL,
          NULL,
          &type_var1,
          2);

  s1 = (void*)NclGetArgValue(
          2,
          8,
          &ndims_s1,
          dsizes_s1,
          NULL,
          NULL,
          &type_s1,
          2);

  ave2 = (void*)NclGetArgValue(
          3,
          8,
          &ndims_ave2,
          dsizes_ave2,
          &missing_ave2,
          &has_missing_ave2,
          &type_ave2,
          2);

  var2 = (void*)NclGetArgValue(
          4,
          8,
          &ndims_var2,
          dsizes_var2,
          NULL,
          NULL,
          &type_var2,
          2);

  s2 = (void*)NclGetArgValue(
          5,
          8,
          &ndims_s2,
          dsizes_s2,
          NULL,
          NULL,
          &type_s2,
          2);

  iflag = (logical*)NclGetArgValue(
          6,
          8,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

  tval_opt = (logical*)NclGetArgValue(
          7,
          8,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

/*
 * Check dimensions.  The ave* and var* variables must be the same size,
 * either all scalars, or all arrays the same size.  The s* variables can
 * either be scalars, or arrays the same size as ave* and var*.  
 */
  if (ndims_ave1 != ndims_ave2 || ndims_ave1 != ndims_var1 || 
      ndims_ave1 != ndims_var2) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: ave1, ave2, var1, and var2 must have the same number of dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_ave1; i++ ) {
    if (dsizes_ave1[i] != dsizes_ave2[i] || 
        dsizes_ave1[i] != dsizes_var1[i] ||
        dsizes_ave1[i] != dsizes_var2[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: ave1, ave2, var1, and var2 must have the same dimension sizes");
      return(NhlFATAL);
    }
  }

  if((ndims_s1 == 1 && dsizes_s1[0] != 1) || ndims_s1 != 1) {
    scalar_s1 = 0;
/*
 * s1 is not a scalar, so it must be an array of the same dimension
 * sizes as ave* and var*.
 */ 
    if(ndims_ave1 != ndims_s1) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: If s1 is not a scalar, then it must have the same number of dimensions as ave1, var1, ave2, and var2");
      return(NhlFATAL);
    }

    for( i = 0; i < ndims_ave1; i++ ) {
      if (dsizes_ave1[i] != dsizes_s1[i]) {
        NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: If s1 is not a scalar, then it must have the same dimension sizes as ave1, var1, ave2, and var2");
        return(NhlFATAL);
      }
    }
  }
  else {
    scalar_s1 = 1;
  }

  if((ndims_s2 == 1 && dsizes_s2[0] != 1) || ndims_s2 != 1) {
    scalar_s2 = 0;
/*
 * s2 is not a scalar, so it must be an array of the same dimension
 * sizes as ave* and var*.
 */ 
    if(ndims_ave1 != ndims_s2) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: If s2 is not a scalar, then it must have the same number of dimensions as ave1, var1, ave2, and var2");
      return(NhlFATAL);
    }

    for( i = 0; i < ndims_ave1; i++ ) {
      if (dsizes_ave1[i] != dsizes_s2[i]) {
        NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: If s2 is not a scalar, then it must have the same dimension sizes as ave1, var1, ave2, and var2");
        return(NhlFATAL);
      }
    }
  }
  else {
    scalar_s2 = 1;
  }
/*
 * Calculate size of output value. If tval_opt is False, then the output
 * will be the same size as ave1. If tval_opt is True, then the output 
 * will be 2 x dimsizes(ave1).
 */
  if(*tval_opt) {
    ndims_prob = ndims_ave1+1;
    size_prob = 2;
  }
  else {
    ndims_prob = ndims_ave1;
    size_prob = 1;
  }
  dsizes_prob = (int*)calloc(ndims_prob,sizeof(int));
  if( dsizes_prob == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for output variable");
    return(NhlFATAL);
  }
/*
 * Calculate size ave1, which will also be size of ave2, var1, and var2.
 * It will also be used to calculate size of prob.
 */
  size_ave1 = 1;
  for( i = 0; i < ndims_ave1; i++ ) {
    size_ave1 *= dsizes_ave1[i];
    dsizes_prob[ndims_prob-ndims_ave1+i] = dsizes_ave1[i];
  }
  if(*tval_opt) {
    dsizes_prob[0] = 2;
    size_prob = 2 * size_ave1;
  }
  else {
    size_prob = size_ave1;
  }
/*
 * Set up variables for coercing input to double (if necessary).
 */
  if(type_ave1 != NCL_double) {
    tmp_ave1 = (double*)calloc(1,sizeof(double));
    if( tmp_ave1 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for coercing ave1 to double precision");
      return(NhlFATAL);
    }
  }

  if(type_var1 != NCL_double) {
    tmp_var1 = (double*)calloc(1,sizeof(double));
    if( tmp_var1 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for coercing var1 to double precision");
      return(NhlFATAL);
    }
  }

  if(type_ave2 != NCL_double) {
    tmp_ave2 = (double*)calloc(1,sizeof(double));
    if( tmp_ave2 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for coercing ave2 to double precision");
      return(NhlFATAL);
    }
  }

  if(type_var2 != NCL_double) {
    tmp_var2 = (double*)calloc(1,sizeof(double));
    if( tmp_var2 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for coercing var2 to double precision");
      return(NhlFATAL);
    }
  }

  if(scalar_s1) {
    tmp_s1 = coerce_input_double(s1,type_s1,1,0,NULL,NULL);
  }
  else {
    if(type_s1 != NCL_double) tmp_s1 = (double*)calloc(1,sizeof(double));
  }

  if(scalar_s2) {
    tmp_s2 = coerce_input_double(s2,type_s2,1,0,NULL,NULL);
  }
  else {
    if(type_s2 != NCL_double) tmp_s2 = (double*)calloc(1,sizeof(double));
  }

/*
 * Allocate space for output array.
 */
  if(type_ave1 == NCL_double || type_var1 == NCL_double || 
     type_ave2 == NCL_double || type_var2 == NCL_double) {

    type_prob = NCL_double;
    prob      = (double*)calloc(size_prob,sizeof(double));
  }
  else {
    type_prob = NCL_float;
    prob      = (float*)calloc(size_prob,sizeof(float));
  }
  tmp_alpha = (double *)calloc(1,sizeof(double));
  tmp_tval  = (double *)calloc(1,sizeof(double));
  if(prob == NULL || tmp_alpha == NULL || tmp_tval == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"ttest: Unable to allocate memory for output array");
    return(NhlFATAL);
  }

/*
 * Check for missing values.
 */
  coerce_missing(type_ave1,has_missing_ave1,&missing_ave1,&missing_dave1,
                 &missing_rave1);
  coerce_missing(type_ave2,has_missing_ave2,&missing_ave2,&missing_dave2,
                 NULL);

  if(!has_missing_ave1) {
    missing_dave1.doubleval = (double)((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    missing_rave1.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
  }

/*
 * Call the Fortran version of this routine.
 */
  index_s1 = index_s2 = index_prob = 0;

  output_contains_msg = 0;   /* Flag to keep track of whether output contains
                                any missing values.*/
  for( i = 0; i < size_ave1; i++ ) {
    ier = 0;
    if(type_ave1 != NCL_double) {
/*
 * Coerce subsection of ave1 (tmp_ave1) to double.
 */
      coerce_subset_input_double(ave1,tmp_ave1,i,type_ave1,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_ave1 to appropriate location in ave1.
 */
      tmp_ave1 = &((double*)ave1)[i];
    }
    if(type_var1 != NCL_double) {
/*
 * Coerce subsection of var1 (tmp_var1) to double.
 */
      coerce_subset_input_double(var1,tmp_var1,i,type_var1,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_var1 to appropriate location in var1.
 */
      tmp_var1 = &((double*)var1)[i];
    }

    if(type_ave2 != NCL_double) {
/*
 * Coerce subsection of ave2 (tmp_ave2) to double.
 */
      coerce_subset_input_double(ave2,tmp_ave2,i,type_ave2,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_ave2 to appropriate location in ave2.
 */
      tmp_ave2 = &((double*)ave2)[i];
    }
    if(type_var2 != NCL_double) {
/*
 * Coerce subsection of var2 (tmp_var2) to double.
 */
      coerce_subset_input_double(var2,tmp_var2,i,type_var2,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_var2 to appropriate location in var2.
 */
      tmp_var2 = &((double*)var2)[i];
    }
    if(!scalar_s1) {
      if(type_s1 != NCL_double) {
/*
 * Coerce subsection of s1 (tmp_s1) to double.
 */
        coerce_subset_input_double(s1,tmp_s1,index_s1,type_s1,1,0,NULL,NULL);
      }
      else {
/*
 * Point tmp_s1 to appropriate location in s1.
 */
        tmp_s1 = &((double*)s1)[index_s1];
      }
    }

    if(!scalar_s2) {
      if(type_s2 != NCL_double) {
/*
 * Coerce subsection of s2 (tmp_s2) to double.
 */
        coerce_subset_input_double(s2,tmp_s2,index_s2,type_s2,1,0,NULL,NULL);
      }
      else {
/*
 * Point tmp_s2 to appropriate location in s2.
 */
        tmp_s2 = &((double*)s2)[index_s2];
      }
    }

/*
 * Test for presence of missing values.
 */
    is_missing_ave1 = contains_missing(tmp_ave1,1,has_missing_ave1,
                                       missing_dave1.doubleval);
    is_missing_ave2 = contains_missing(tmp_ave2,1,has_missing_ave2,
                                       missing_dave2.doubleval);
    if(is_missing_ave1 || is_missing_ave2) {
      ier = 1;
      output_contains_msg = 1;
    }
    if(*tmp_s1 < 2. || *tmp_s2 < 2.) {
      NhlPError(NhlWARNING,NhlEUNKNOWN,"ttest: s1 and/or s2 is less than 2; setting output to missing");
      ier = 1;
      output_contains_msg = 1;
    }
    if(*tmp_var1 <= 0. || *tmp_var2 <= 0.) {
      NhlPError(NhlWARNING,NhlEUNKNOWN,"ttest: var1 and/or var2 is less than or equal to 0; setting output to missing");
      ier = 1;
      output_contains_msg = 1;
    }
    if(!ier) {
      NGCALLF(dttest,DTTEST)(tmp_ave1,tmp_var1,tmp_s1,tmp_ave2,tmp_var2,
                             tmp_s2,iflag,tmp_alpha,tmp_tval,&ier);

      if(type_prob == NCL_float) {
        ((float*)prob)[index_prob]                 = (float)*tmp_alpha;
        if(*tval_opt) ((float*)prob)[index_prob+1] = (float)*tmp_tval;
      }
      else {
        ((double*)prob)[index_prob]                 = *tmp_alpha;
        if(*tval_opt) ((double*)prob)[index_prob+1] = *tmp_tval;
      }
    }
    else {
      if(type_prob == NCL_float) {
        ((float*)prob)[index_prob]                 = missing_rave1.floatval;
        if(*tval_opt) ((float*)prob)[index_prob+1] = missing_rave1.floatval;
      }
      else {
        ((double*)prob)[index_prob]                 = missing_dave1.doubleval;
        if(*tval_opt) ((double*)prob)[index_prob+1] = missing_dave1.doubleval;
      }
    }
/*
 * Implement indices.
 */
    if(!scalar_s1) index_s1++;
    if(!scalar_s2) index_s2++;
    if(*tval_opt) {
      index_prob+=2;
    }
    else {
      index_prob++;
    }
        
  }
/*
 * free memory.
 */
  if(type_ave1 != NCL_double) NclFree(tmp_ave1);
  if(type_var1 != NCL_double) NclFree(tmp_var1);
  if(type_s1 != NCL_double)   NclFree(tmp_s1);
  if(type_ave2 != NCL_double) NclFree(tmp_ave2);
  if(type_var2 != NCL_double) NclFree(tmp_var2);
  if(type_s2 != NCL_double)   NclFree(tmp_s2);
  NclFree(tmp_alpha);
  NclFree(tmp_tval);

/*
 * Return.
 */
  if(output_contains_msg) {
    if(type_prob == NCL_double) {
      return(NclReturnValue(prob,ndims_prob,dsizes_prob,&missing_dave1,
                            type_prob,0));
    }
    else {
      return(NclReturnValue(prob,ndims_prob,dsizes_prob,&missing_rave1,
                            type_prob,0));
    }
  }
  else {
    return(NclReturnValue(prob,ndims_prob,dsizes_prob,NULL,type_prob,0));
  }
}

NhlErrorTypes ftest_W( void )
{
/*
 * Input array variables
 */
  void *var1, *var2, *s1, *s2;
  double *tmp_var1, *tmp_var2, *tmp_s1, *tmp_s2;
  NclScalar missing_var1, missing_dvar1, missing_rvar1;
  NclScalar missing_var2, missing_dvar2;
  int has_missing_var1, has_missing_var2;
  int *opt;
  int ndims_var1, dsizes_var1[NCL_MAX_DIMENSIONS];
  int ndims_var2, dsizes_var2[NCL_MAX_DIMENSIONS];
  int ndims_s1, dsizes_s1[NCL_MAX_DIMENSIONS];
  int ndims_s2, dsizes_s2[NCL_MAX_DIMENSIONS];
  NclBasicDataTypes type_var1, type_var2;
  NclBasicDataTypes type_s1, type_s2;
  int scalar_s1, scalar_s2, output_contains_msg;
/*
 * output variable 
 */
  void *prob;
  double *tmp_prob;
  NclBasicDataTypes type_prob;
/*
 * Declare various variables for random purposes.
 */
  int i, index_s1, index_s2, size_prob, is_missing_var1, is_missing_var2, ier;
/*
 * Retrieve parameters
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */

  var1 = (void*)NclGetArgValue(
          0,
          5,
          &ndims_var1,
          dsizes_var1,
          &missing_var1,
          &has_missing_var1,
          &type_var1,
          2);

  s1 = (void*)NclGetArgValue(
          1,
          5,
          &ndims_s1,
          dsizes_s1,
          NULL,
          NULL,
          &type_s1,
          2);

  var2 = (void*)NclGetArgValue(
          2,
          5,
          &ndims_var2,
          dsizes_var2,
          &missing_var2,
          &has_missing_var2,
          &type_var2,
          2);

  s2 = (void*)NclGetArgValue(
          3,
          5,
          &ndims_s2,
          dsizes_s2,
          NULL,
          NULL,
          &type_s2,
          2);

  opt = (int*)NclGetArgValue(
          4,
          5,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

/*
 * Check dimensions.  The var* variables must be the same size,
 * either all scalars, or all arrays the same size.  The s* variables can
 * either be scalars, or arrays the same size as var*.  
 */
  if (ndims_var1 != ndims_var2) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: var1 and var2 must have the same number of dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_var1; i++ ) {
    if (dsizes_var1[i] != dsizes_var2[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: var1 and var2 must have the same dimension sizes");
      return(NhlFATAL);
    }
  }

  if((ndims_s1 == 1 && dsizes_s1[0] != 1) || ndims_s1 != 1) {
    scalar_s1 = 0;
/*
 * s1 is not a scalar, so it must be an array of the same dimension
 * sizes as var*.
 */ 
    if(ndims_var1 != ndims_s1) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: If s1 is not a scalar, then it must have the same number of dimensions as var1 and var2");
      return(NhlFATAL);
    }

    for( i = 0; i < ndims_var1; i++ ) {
      if (dsizes_var1[i] != dsizes_s1[i]) {
        NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: If s1 is not a scalar, then it must have the same dimension sizes as var1 and var2");
        return(NhlFATAL);
      }
    }
  }
  else {
    scalar_s1 = 1;
  }

  if((ndims_s2 == 1 && dsizes_s2[0] != 1) || ndims_s2 != 1) {
    scalar_s2 = 0;
/*
 * s2 is not a scalar, so it must be an array of the same dimension
 * sizes as var*.
 */ 
    if(ndims_var1 != ndims_s2) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: If s2 is not a scalar, then it must have the same number of dimensions as var1 and var2");
      return(NhlFATAL);
    }

    for( i = 0; i < ndims_var1; i++ ) {
      if (dsizes_var1[i] != dsizes_s2[i]) {
        NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: If s2 is not a scalar, then it must have the same dimension sizes as var1 and var2");
        return(NhlFATAL);
      }
    }
  }
  else {
    scalar_s2 = 1;
  }

/*
 * Calculate the size of the output.
 */
  size_prob = 1;
  for( i = 0; i < ndims_var1; i++ ) {
    size_prob *= dsizes_var1[i];
  }
/*
 * Set up variables for coercing input to double (if necessary).
 */
  if(type_var1 != NCL_double) {
    tmp_var1 = (double*)calloc(1,sizeof(double));
    if( tmp_var1 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: Unable to allocate memory for coercing var1 to double precision");
      return(NhlFATAL);
    }
  }

  if(type_var2 != NCL_double) {
    tmp_var2 = (double*)calloc(1,sizeof(double));
    if( tmp_var2 == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: Unable to allocate memory for coercing var2 to double precision");
      return(NhlFATAL);
    }
  }

  if(scalar_s1) {
    tmp_s1 = coerce_input_double(s1,type_s1,1,0,NULL,NULL);
  }
  else {
    if(type_s1 != NCL_double) tmp_s1 = (double*)calloc(1,sizeof(double));
  }

  if(scalar_s2) {
    tmp_s2 = coerce_input_double(s2,type_s2,1,0,NULL,NULL);
  }
  else {
    if(type_s2 != NCL_double) tmp_s2 = (double*)calloc(1,sizeof(double));
  }

/*
 * Allocate space for output array.
 */
  if(type_var1 == NCL_double || type_var2 == NCL_double) {
    type_prob = NCL_double;
    prob      = (double*)calloc(size_prob,sizeof(double));
  }
  else {
    type_prob = NCL_float;
    prob      = (float*)calloc(size_prob,sizeof(float));
  }
  tmp_prob = (double *)calloc(1,sizeof(double));
  if(prob == NULL || tmp_prob == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"ftest: Unable to allocate memory for output array");
    return(NhlFATAL);
  }

/*
 * Check for missing values.
 */
  coerce_missing(type_var1,has_missing_var1,&missing_var1,&missing_dvar1,
                 &missing_rvar1);
  coerce_missing(type_var2,has_missing_var2,&missing_var2,&missing_dvar2,
                 NULL);
  if(!has_missing_var1) {
    missing_dvar1.doubleval = (double)((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    missing_rvar1.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
  }

/*
 * Call the Fortran version of this routine.
 */
  index_s1 = index_s2 = 0;

  output_contains_msg = 0;   /* Flag to keep track of whether output contains
                                any missing values.*/
  for( i = 0; i < size_prob; i++ ) {
    if(type_var1 != NCL_double) {
/*
 * Coerce subsection of var1 (tmp_var1) to double.
 */
      coerce_subset_input_double(var1,tmp_var1,i,type_var1,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_var1 to appropriate location in var1.
 */
      tmp_var1 = &((double*)var1)[i];
    }

    if(type_var2 != NCL_double) {
/*
 * Coerce subsection of var2 (tmp_var2) to double.
 */
      coerce_subset_input_double(var2,tmp_var2,i,type_var2,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_var2 to appropriate location in var2.
 */
      tmp_var2 = &((double*)var2)[i];
    }

    if(!scalar_s1) {
      if(type_s1 != NCL_double) {
/*
 * Coerce subsection of s1 (tmp_s1) to double.
 */
        coerce_subset_input_double(s1,tmp_s1,index_s1,type_s1,1,0,NULL,NULL);
      }
      else {
/*
 * Point tmp_s1 to appropriate location in s1.
 */
        tmp_s1 = &((double*)s1)[index_s1];
      }
    }

    if(!scalar_s2) {
      if(type_s2 != NCL_double) {
/*
 * Coerce subsection of s2 (tmp_s2) to double.
 */
        coerce_subset_input_double(s2,tmp_s2,index_s2,type_s2,1,0,NULL,NULL);
      }
      else {
/*
 * Point tmp_s2 to appropriate location in s2.
 */
        tmp_s2 = &((double*)s2)[index_s2];
      }
    }

    ier = 0;

    is_missing_var1 = contains_missing(tmp_var1,1,has_missing_var1,
                                       missing_dvar1.doubleval);
    is_missing_var2 = contains_missing(tmp_var2,1,has_missing_var2,
                                       missing_dvar2.doubleval);
    if(is_missing_var1 || is_missing_var2) {
      ier = 1;
      output_contains_msg = 1;
    }
    if(*tmp_s1 < 2. || *tmp_s2 < 2.) {
      NhlPError(NhlWARNING,NhlEUNKNOWN,"ftest: s1 and/or s2 is less than 2; setting output to missing");
      ier = 1;
      output_contains_msg = 1;
    }
    if(*tmp_var1 <= 0. || *tmp_var2 <= 0.) {
      NhlPError(NhlWARNING,NhlEUNKNOWN,"ftest: var1 and/or var2 is less than or equal to 0; setting output to missing");
      ier = 1;
      output_contains_msg = 1;
    }
    if(!ier) {
      NGCALLF(dftest,DFTEST)(tmp_var1,tmp_var2,tmp_s1,tmp_s2,tmp_prob,&ier);

      if(type_prob == NCL_float) ((float*)prob)[i]  = (float)*tmp_prob;
      else                       ((double*)prob)[i] = *tmp_prob;
    }
    else {
      if(type_prob == NCL_float) ((float*)prob)[i]  = missing_rvar1.floatval;
      else                       ((double*)prob)[i] = missing_dvar1.doubleval;
    }
/*
 * Implement indices.
 */
    if(!scalar_s1) index_s1++;
    if(!scalar_s2) index_s2++;
  }
/*
 * free memory.
 */
  if(type_var1 != NCL_double) NclFree(tmp_var1);
  if(type_s1 != NCL_double)   NclFree(tmp_s1);
  if(type_var2 != NCL_double) NclFree(tmp_var2);
  if(type_s2 != NCL_double)   NclFree(tmp_s2);
  NclFree(tmp_prob);

/*
 * Return.
 */
  if(output_contains_msg) {
    if(type_prob == NCL_double) {
      return(NclReturnValue(prob,ndims_var1,dsizes_var1,&missing_dvar1,
                            type_prob,0));
    }
    else {
      return(NclReturnValue(prob,ndims_var1,dsizes_var1,&missing_rvar1,
                            type_prob,0));
    }
  }
  else {
    return(NclReturnValue(prob,ndims_var1,dsizes_var1,NULL,type_prob,0));
  }
}

NhlErrorTypes rtest_W( void )
{
/*
 * Input array variables
 */
  void *r;
  int *n, tmp_n, *opt;
  double *tmp_r;
  NclScalar missing_r, missing_dr, missing_rr;
  int has_missing_r;
  int ndims_r, dsizes_r[NCL_MAX_DIMENSIONS];
  int ndims_n, dsizes_n[NCL_MAX_DIMENSIONS];
  NclBasicDataTypes type_r;
/*
 * output variable 
 */
  void *prob;
  double *tmp_prob;
  NclBasicDataTypes type_prob;
/*
 * Declare various variables for random purposes.
 */
  int i, size_prob, ier, scalar_n;
/*
 * Retrieve parameters
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */

  r = (void*)NclGetArgValue(
          0,
          3,
          &ndims_r,
          dsizes_r,
          &missing_r,
          &has_missing_r,
          &type_r,
          2);

  n = (int*)NclGetArgValue(
          1,
          3,
          &ndims_n,
          dsizes_n,
          NULL,
          NULL,
          NULL,
          2);

  opt = (int*)NclGetArgValue(
          2,
          3,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

/*
 * Check dimension sizes.
 */
  if((ndims_n == 1 && dsizes_n[0] != 1) || ndims_n != 1) {
    scalar_n = 0;
/*
 * If n is not a scalar, it must be an array of the same dimension
 * sizes as r.
 */ 
    if(ndims_r != ndims_n) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"rtest: If Nr is not a scalar, then it must have the same number of dimensions as r");
      return(NhlFATAL);
    }

    for( i = 0; i < ndims_r; i++ ) {
      if (dsizes_n[i] != dsizes_r[i]) {
        NhlPError(NhlFATAL,NhlEUNKNOWN,"rtest: If Nr is not a scalar, then it must have the same dimension sizes as r");
        return(NhlFATAL);
      }
    }
  }
  else {
    scalar_n = 1;
  }

/*
 * Calculate the size of the output.
 */
  size_prob = 1;
  for( i = 0; i < ndims_r; i++ ) {
    size_prob *= dsizes_r[i];
  }
/*
 * Set up variables for coercing input to double (if necessary).
 */
  if(type_r != NCL_double) {
    tmp_r = (double*)calloc(1,sizeof(double));
    if( tmp_r == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"rtest: Unable to allocate memory for coercing r to double precision");
      return(NhlFATAL);
    }
  }

/*
 * Allocate space for output array.
 */
  if(type_r == NCL_double) {
    type_prob = NCL_double;
    prob      = (double*)calloc(size_prob,sizeof(double));
  }
  else {
    type_prob = NCL_float;
    prob      = (float*)calloc(size_prob,sizeof(float));
  }
  tmp_prob = (double *)calloc(1,sizeof(double));
  if(prob == NULL || tmp_prob == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"rtest: Unable to allocate memory for output array");
    return(NhlFATAL);
  }

/*
 * Check for missing values.
 */
  coerce_missing(type_r,has_missing_r,&missing_r,&missing_dr,&missing_rr);

/*
 * Call the Fortran version of this routine.
 */
  if(scalar_n) tmp_n = *n;

  for( i = 0; i < size_prob; i++ ) {
    if(type_r != NCL_double) {
/*
 * Coerce subsection of r (tmp_r) to double.
 */
      coerce_subset_input_double(r,tmp_r,i,type_r,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_r to appropriate location in r.
 */
      tmp_r = &((double*)r)[i];
    }
    if(!scalar_n) tmp_n = n[i];

    NGCALLF(drtest,DRTEST)(tmp_r,&tmp_n,tmp_prob);

    if(type_prob == NCL_float) ((float*)prob)[i]  = (float)*tmp_prob;
    else                       ((double*)prob)[i] = *tmp_prob;
  }
/*
 * free memory.
 */
  if(type_r != NCL_double) NclFree(tmp_r);
  NclFree(tmp_prob);

/*
 * Return.
 */
  return(NclReturnValue(prob,ndims_r,dsizes_r,NULL,type_prob,0));
}

NhlErrorTypes equiv_sample_size_W( void )
{
/*
 * Input array variables
 */
  void *x, *siglvl;
  double *tmp_x, *tmp_siglvl;
  int *opt;
  NclScalar missing_x, missing_dx;
  int has_missing_x;
  int ndims_x, dsizes_x[NCL_MAX_DIMENSIONS];
  NclBasicDataTypes type_x, type_siglvl;
/*
 * output variable 
 */
  int *neqv, *dsizes_neqv, ndims_neqv;
/*
 * Declare various variables for random purposes.
 */
  int nx, i, size_neqv, ier, index_x;
/*
 * Retrieve parameters
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */

  x = (void*)NclGetArgValue(
          0,
          3,
          &ndims_x,
          dsizes_x,
          &missing_x,
          &has_missing_x,
          &type_x,
          2);

  siglvl = (int*)NclGetArgValue(
          1,
          3,
          NULL,
          NULL,
          NULL,
          NULL,
          &type_siglvl,
          2);

  opt = (int*)NclGetArgValue(
          2,
          3,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

  nx = dsizes_x[ndims_x-1];
/*
 * Calculate the size of the output.
 */
  ndims_neqv = max(1,ndims_x-1);

  dsizes_neqv = (int*)calloc(ndims_neqv,sizeof(int));
  if(dsizes_neqv == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"equiv_sample_size: Unable to allocate memory for output variable");
    return(NhlFATAL);
  }

  size_neqv = dsizes_neqv[0] = 1;
  for( i = 0; i < ndims_x-1; i++ ) {
    dsizes_neqv[i] = dsizes_x[i];
    size_neqv *= dsizes_x[i];
  }

/*
 * Set up variables for coercing input to double (if necessary).
 */
  if(type_x != NCL_double) {
    tmp_x = (double*)calloc(nx,sizeof(double));
    if( tmp_x == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"equiv_sample_size: Unable to allocate memory for coercing x to double precision");
      return(NhlFATAL);
    }
  }

  tmp_siglvl = coerce_input_double(siglvl,type_siglvl,1,0,NULL,NULL);
  if(tmp_siglvl == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"equiv_sample_size: Unable to allocate memory for coercing siglvl to double precision");
    return(NhlFATAL);
  }
/*
 * Allocate space for output array.
 */
  neqv = (int*)calloc(size_neqv,sizeof(int));
  if(neqv == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"equiv_sample_size: Unable to allocate memory for output array");
    return(NhlFATAL);
  }

/*
 * Check for missing values.
 */
  coerce_missing(type_x,has_missing_x,&missing_x,&missing_dx,NULL);

/*
 * Call the Fortran version of this routine.
 */
  index_x = 0;
  for( i = 0; i < size_neqv; i++ ) {
    if(type_x != NCL_double) {
/*
 * Coerce subsection of x (tmp_x) to double.
 */
      coerce_subset_input_double(x,tmp_x,index_x,type_x,nx,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_x to appropriate location in x.
 */
      tmp_x = &((double*)x)[index_x];
    }
    NGCALLF(deqvsiz,DEQVSIZ)(tmp_x,&nx,&missing_dx.doubleval,tmp_siglvl,
                             &neqv[i]);
    index_x += nx;
  }
/*
 * free memory.
 */
  if(type_x      != NCL_double) NclFree(tmp_x);
  if(type_siglvl != NCL_double) NclFree(tmp_siglvl);

/*
 * Return.
 */
  return(NclReturnValue(neqv,ndims_neqv,dsizes_neqv,NULL,NCL_int,0));
}
