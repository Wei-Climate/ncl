#include <stdio.h>
/*
* The following are the required NCAR Graphics include files.
* They should be located in ${NCARG_ROOT}/include
*/
#include "wrapper.h"

extern void NGCALLF(dtrssph,DTRSSPH)(int *,int *,int *,int *,double *,int *,
                                     int *,int *,double *, double *,int *,
                                     int *,double *,int *,int *, double *,
                                     int *,int *);

extern void NGCALLF(dtrcwav,DTRCWAV)(int *,int *,int *,double *,double *,
                                     int *,double *,int *,double *,int *,
                                     int *,int *);

extern void NGCALLF(dfo2f,DFO2F)(double *,int *,int *,double *,int *,double *,
                                 int *,double *,int *,int *,int *);

extern void compute_jlatilon(int *,int,int *,int *,int *,int *,int *,int *,
                             int *,int *,int);

NhlErrorTypes g2gsh_W( void )
{
/*
 * Input array variables
 */
  void *Ta;
  double *tmp_Ta;
  int ndims_Ta, dsizes_Ta[NCL_MAX_DIMENSIONS];
  NclScalar missing_Ta, missing_dTa, missing_rTa;
  NclBasicDataTypes type_Ta, type_Tb;
  int has_missing_Ta, found_missing;
  int nlata, nlona, igrida[2];
/*
 * Output array variables
 */
  void *Tb;
  double *tmp_Tb;
  int *dsizes_Tb, dsizes_Tb2[NCL_MAX_DIMENSIONS], nlatb, nlonb, igridb[2];
/*
 * various
 */
  int *twave, intl, i, j, index_Ta, index_Tb;
  int total_size_leftmost, nlatanlona, nlatbnlonb;
  int total_size_Ta, total_size_Tb;
/*
 * Workspace variables
 */
  int lsave, lsvmin, lwork, ldwork, lwkmin, ker = 0;
  int klat, klon, k1, k2, lwa, lwb, ier = 0;
  double *work, *wsave, *dwork;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  Ta = (void*)NclGetArgValue(
           0,
           3,
           &ndims_Ta, 
           dsizes_Ta,
           &missing_Ta,
           &has_missing_Ta,
           &type_Ta,
           2);
/*
 * Get sizes for output array.
 */
  dsizes_Tb = (int*)NclGetArgValue(
                1,
                3,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                2);
/*
 * Get optional wave truncation value.
 */
  twave = (int*)NclGetArgValue(
            2,
            3, 
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_Ta < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Compute the total number of elements in our array.
 */
  compute_nlatanlona(dsizes_Ta,dsizes_Tb,ndims_Ta,2,
                     &nlata,&nlona,&nlatanlona,&nlatb,&nlonb,&nlatbnlonb,
                     &total_size_leftmost,&total_size_Ta,&total_size_Tb);
/*
 * Check output dimensions.
 */
  if( nlatb < 4 || nlonb < 4 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: The lat/lon dimensions of the output array must be at least 4");
    return(NhlFATAL);
  }
/*
 * Coerce missing values.
 */
  coerce_missing(type_Ta,has_missing_Ta,&missing_Ta,&missing_dTa,
                 &missing_rTa);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in Ta and Tb.
 */
  if(type_Ta != NCL_double) {
    type_Tb = NCL_float;
    Tb = (void*)calloc(total_size_Tb,sizeof(float));
    tmp_Tb = (double*)calloc(nlatbnlonb,sizeof(double));
    tmp_Ta = (double*)calloc(nlatanlona,sizeof(double));
    if(Tb == NULL || tmp_Tb == NULL || tmp_Ta == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_Tb = NCL_double;
    Tb = (void*)calloc(total_size_Tb,sizeof(double));
    if(Tb == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }

/* 
 * igrida describes the array going in, and igridb describes the array 
 * coming out.
 */
  igrida[0] = igridb[0] = -2;
  igrida[1] = igridb[1] =  0;

  intl = 0;

/*
 * Determine the workspace size.
 */
  klat  = nlata > nlatb ? nlata : nlatb;
  klon  = nlona > nlonb ? nlona : nlonb;
  k1    = klat < ((klon+2)/2) ? klat : ((klon+2)/2);
  k2    = (klat+1)/2;
  lwa   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lwb   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lsave = lwb + lwa + 2*(klon+15);
  lwork = klat*(4*k1+ klon+ 2*klat+ 4) + 3*((k1-2)*2*(2*klat-k1-1))/2;
  ldwork = klat*(klat+4);

  lsave = 5*lsave;     /* the above are only approximate */
  lwork = 5*lwork;

/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  dwork = (double *)calloc(ldwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL || dwork == NULL ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: workspace allocate failed\n" );
    return(NhlFATAL);
  }

/*
 * Loop through each nlata x nlona grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_Ta = index_Tb = 0;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_Ta != NCL_double) {
/*
 * Coerce nlata x nlona subsection of Ta (tmp_Ta) to double.
 */
      coerce_subset_input_double(Ta,tmp_Ta,index_Ta,type_Ta,nlatanlona,0,
                                 &missing_Ta,&missing_dTa);
    }
    else {
/*
 * Point tmp_Ta and tmp_Tb to appropriate locations in Ta and Tb.
 */
      tmp_Ta = &((double*)Ta)[index_Ta];
      tmp_Tb = &((double*)Tb)[index_Tb];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_Ta,nlatanlona,has_missing_Ta,
                                     missing_dTa.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(Tb,index_Tb,type_Ta,nlatbnlonb,
                                missing_dTa.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"g2gsh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'trssph' with the full argument list.
 */
      NGCALLF(dtrssph,DTRSSPH)(&intl,igrida,&nlona,&nlata,tmp_Ta,igridb,
                               &nlonb,&nlatb,tmp_Tb,wsave,&lsave,&lsvmin,
                               work,&lwork,&lwkmin,dwork,&ldwork,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"g2gsh: ier = %d\n", ier );
      }
      
      if (abs(*twave)) {
        NclFree(dwork);
        ldwork = 2*klat*(klat+1)+1;
        dwork  = (double *)calloc(ldwork,sizeof(double));
        if (dwork == NULL) {
          NhlPError(NhlFATAL,NhlEUNKNOWN,"g2gsh: workspace allocate failed\n" );
          return(NhlFATAL);
        }
/*
 * Truncate the data at a specified truncation.
 */
        NGCALLF(dtrcwav,DTRCWAV)(igridb,&nlatb,&nlonb,tmp_Tb,wsave,&lsave,
                                 work,&lwork,dwork,&ldwork,&ker,twave);
      }
/*
 * Copy output values from temporary array "tmp_Tb" to final array "Tb".
 */
      if(type_Tb == NCL_float) {
        for(j = 0; j < nlatbnlonb; j++) {
          ((float*)Tb)[index_Tb+j] = (float)(tmp_Tb[j]);
        }
      } 
    }
    index_Ta += nlatanlona;
    index_Tb += nlatbnlonb;
  }
/*
 * Free workspace arrays.
 */
  if (work != NULL) NclFree(work);
  if(dwork != NULL) NclFree(dwork);
  if(wsave != NULL) NclFree(wsave);
  if(type_Ta != NCL_double) {
    NclFree(tmp_Ta);
    NclFree(tmp_Tb);
  }
/*
 * Return output grid to NCL.
 */
  for( i = 0; i < ndims_Ta-2; i++ ) dsizes_Tb2[i] = dsizes_Ta[i];
  dsizes_Tb2[ndims_Ta-2] = nlatb;
  dsizes_Tb2[ndims_Ta-1] = nlonb;
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_Ta) {
    if(type_Tb == NCL_float) {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_rTa,type_Tb,0));
    }
    else {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_dTa,type_Tb,0));
    }
  }
  else {
    return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,NULL,type_Tb,0));
  }
}


NhlErrorTypes f2gsh_W( void )
{
/*
 * Input array variables
 */
  void *Ta;
  double *tmp_Ta;
  int ndims_Ta, dsizes_Ta[NCL_MAX_DIMENSIONS];
  NclScalar missing_Ta, missing_dTa, missing_rTa;
  NclBasicDataTypes type_Ta, type_Tb;
  int has_missing_Ta, found_missing;
  int nlata, nlona, igrida[2];
/*
 * Output array variables
 */
  void *Tb;
  double *tmp_Tb;
  int *dsizes_Tb, dsizes_Tb2[NCL_MAX_DIMENSIONS], nlatb, nlonb, igridb[2];
/*
 * various
 */
  int *twave, intl, i, j, index_Ta, index_Tb;
  int total_size_leftmost, nlatanlona, nlatbnlonb;
  int total_size_Ta, total_size_Tb;
/*
 * Workspace variables
 */
  int lsave, lsvmin, lwork, ldwork, lwkmin, ker = 0;
  int klat, klon, k1, k2, lwa, lwb, ier = 0;
  double *work, *wsave, *dwork;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  Ta = (void*)NclGetArgValue(
           0,
           3,
           &ndims_Ta, 
           dsizes_Ta,
           &missing_Ta,
           &has_missing_Ta,
           &type_Ta,
           2);
/*
 * Get sizes for output array.
 */
  dsizes_Tb = (int*)NclGetArgValue(
                1,
                3,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                2);
/*
 * Get optional wave truncation value.
 */
  twave = (int*)NclGetArgValue(
            2,
            3, 
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_Ta < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Compute the total number of elements in our array.
 */
  compute_nlatanlona(dsizes_Ta,dsizes_Tb,ndims_Ta,2,
                     &nlata,&nlona,&nlatanlona,&nlatb,&nlonb,&nlatbnlonb,
                     &total_size_leftmost,&total_size_Ta,&total_size_Tb);
/*
 * Check output dimensions.
 */
  if( nlatb < 4 || nlonb < 4 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: The lat/lon dimensions of the output array must be at least 4");
    return(NhlFATAL);
  }
/*
 * Coerce missing values.
 */
  coerce_missing(type_Ta,has_missing_Ta,&missing_Ta,&missing_dTa,
                 &missing_rTa);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in Ta and Tb.
 */
  if(type_Ta != NCL_double) {
    type_Tb = NCL_float;
    Tb = (void*)calloc(total_size_Tb,sizeof(float));
    tmp_Tb = (double*)calloc(nlatbnlonb,sizeof(double));
    tmp_Ta = (double*)calloc(nlatanlona,sizeof(double));
    if(Tb == NULL || tmp_Tb == NULL || tmp_Ta == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_Tb = NCL_double;
    Tb = (void*)calloc(total_size_Tb,sizeof(double));
    if(Tb == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }
/*
 * igrida describes the array going in, and igridb describes the array 
 * coming out.
 */
  igrida[0] = -1;
  igridb[0] = -2;
  igrida[1] = igridb[1] =  0;

  intl = 0;

/*
 * Determine the workspace size.
 */
  klat  = nlata > nlatb ? nlata : nlatb;
  klon  = nlona > nlonb ? nlona : nlonb;
  k1    = klat < ((klon+2)/2) ? klat : ((klon+2)/2);
  k2    = (klat+1)/2;
  lwa   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lwb   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lsave = lwb + lwa + 2*(klon+15);
  lwork = klat*(4*k1+ klon+ 2*klat+ 4) + 3*((k1-2)*2*(2*klat-k1-1))/2;
  ldwork = klat*(klat+4);

  lsave = 5*lsave;     /* the above are only approximate */
  lwork = 5*lwork;

/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  dwork = (double *)calloc(ldwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL || dwork == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: workspace allocate failed\n" );
    return(NhlFATAL);
  }

/*
 * Loop through each nlata x nlona grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_Ta = index_Tb = 0;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_Ta != NCL_double) {
/*
 * Coerce nlata x nlona subsection of Ta (tmp_Ta) to double.
 */
      coerce_subset_input_double(Ta,tmp_Ta,index_Ta,type_Ta,nlatanlona,0,
                                 &missing_Ta,&missing_dTa);
    }
    else {
/*
 * Point tmp_Ta and tmp_Tb to appropriate locations in Ta and Tb.
 */
      tmp_Ta = &((double*)Ta)[index_Ta];
      tmp_Tb = &((double*)Tb)[index_Tb];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_Ta,nlatanlona,has_missing_Ta,
                                     missing_dTa.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(Tb,index_Tb,type_Ta,nlatbnlonb,
                                missing_dTa.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"f2gsh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'trssph' with the full argument list.
 */
      NGCALLF(dtrssph,DTRSSPH)(&intl,igrida,&nlona,&nlata,tmp_Ta,igridb,
                               &nlonb,&nlatb,tmp_Tb,wsave,&lsave,&lsvmin,
                               work,&lwork,&lwkmin,dwork,&ldwork,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"f2gsh: ier = %d\n", ier );
      }

      if (abs(*twave)) {
        NclFree(dwork);
        ldwork = 2*klat*(klat+1)+1;
        dwork  = (double *)calloc(ldwork,sizeof(double));
        if (dwork == NULL) {
          NhlPError(NhlFATAL,NhlEUNKNOWN,"f2gsh: workspace allocate failed\n" );
          return(NhlFATAL);
        }
/*
 * Truncate the data at a specified truncation.
 */
        NGCALLF(dtrcwav,DTRCWAV)(igridb,&nlatb,&nlonb,tmp_Tb,wsave,&lsave,
                                 work,&lwork,dwork,&ldwork,&ker,twave);
      }
/*
 * Copy output values from temporary array "tmp_Tb" to final array "Tb".
 */
      if(type_Tb == NCL_float) {
        for(j = 0; j < nlatbnlonb; j++) {
          ((float*)Tb)[index_Tb+j] = (float)(tmp_Tb[j]);
        }
      } 
    }
    index_Ta += nlatanlona;
    index_Tb += nlatbnlonb;
  }
/*
 * Free workspace arrays.
 */
  if (work != NULL) NclFree(work);
  if(dwork != NULL) NclFree(dwork);
  if(wsave != NULL) NclFree(wsave);
  if(type_Ta != NCL_double) {
    NclFree(tmp_Ta);
    NclFree(tmp_Tb);
  }
/*
 * Return output grid to NCL.
 */
  for( i = 0; i < ndims_Ta-2; i++ ) dsizes_Tb2[i] = dsizes_Ta[i];
  dsizes_Tb2[ndims_Ta-2] = nlatb;
  dsizes_Tb2[ndims_Ta-1] = nlonb;
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_Ta) {
    if(type_Tb == NCL_float) {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_rTa,type_Tb,0));
    }
    else {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_dTa,type_Tb,0));
    }
  }
  else {
    return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,NULL,type_Tb,0));
  }
}


NhlErrorTypes g2fsh_W( void )
{
/*
 * Input array variables
 */
  void *Ta;
  double *tmp_Ta;
  int ndims_Ta, dsizes_Ta[NCL_MAX_DIMENSIONS];
  NclScalar missing_Ta, missing_dTa, missing_rTa;
  NclBasicDataTypes type_Ta, type_Tb;
  int has_missing_Ta, found_missing;
  int nlata, nlona, igrida[2];
/*
 * Output array variables
 */
  void *Tb;
  double *tmp_Tb;
  int *dsizes_Tb, dsizes_Tb2[NCL_MAX_DIMENSIONS], nlatb, nlonb, igridb[2];
/*
 * various
 */
  int intl, i, j, index_Ta, index_Tb;
  int total_size_leftmost, nlatanlona, nlatbnlonb;
  int total_size_Ta, total_size_Tb;
/*
 * Workspace variables
 */
  int lsave, lsvmin, lwork, ldwork, lwkmin, ker = 0;
  int klat, klon, k1, k2, lwa, lwb, ier = 0;
  double *work, *wsave, *dwork;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  Ta = (void*)NclGetArgValue(
           0,
           2,
           &ndims_Ta, 
           dsizes_Ta,
           &missing_Ta,
           &has_missing_Ta,
           &type_Ta,
           2);
/*
 * Get sizes for output array.
 */
  dsizes_Tb = (int*)NclGetArgValue(
                1,
                2,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_Ta < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2fsh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Compute the total number of elements in our array.
 */
  compute_nlatanlona(dsizes_Ta,dsizes_Tb,ndims_Ta,2,
                     &nlata,&nlona,&nlatanlona,&nlatb,&nlonb,&nlatbnlonb,
                     &total_size_leftmost,&total_size_Ta,&total_size_Tb);
/*
 * Check output dimensions.
 */
  if( nlatb < 4 || nlonb < 4 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2fsh: The lat/lon dimensions of the output array must be at least 4");
    return(NhlFATAL);
  }
/*
 * Coerce missing values.
 */
  coerce_missing(type_Ta,has_missing_Ta,&missing_Ta,&missing_dTa,
                 &missing_rTa);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in Ta and Tb.
 */
  if(type_Ta != NCL_double) {
    type_Tb = NCL_float;
    Tb = (void*)calloc(total_size_Tb,sizeof(float));
    tmp_Tb = (double*)calloc(nlatbnlonb,sizeof(double));
    tmp_Ta = (double*)calloc(nlatanlona,sizeof(double));
    if(Tb == NULL || tmp_Tb == NULL || tmp_Ta == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"g2fsh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_Tb = NCL_double;
    Tb = (void*)calloc(total_size_Tb,sizeof(double));
    if(Tb == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"g2fsh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }

/* 
 * igrida describes the array going in, and igridb describes the array 
 * coming out.
 */
  igrida[0] = -2;
  igridb[0] = -1;
  igrida[1] = igridb[1] =  0;

  intl = 0;

/*
 * Determine the workspace size.
 */
  klat  = nlata > nlatb ? nlata : nlatb;
  klon  = nlona > nlonb ? nlona : nlonb;
  k1    = klat < ((klon+2)/2) ? klat : ((klon+2)/2);
  k2    = (klat+1)/2;
  lwa   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lwb   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lsave = lwb + lwa + 2*(klon+15);
  lwork = klat*(4*k1+ klon+ 2*klat+ 4) + 3*((k1-2)*2*(2*klat-k1-1))/2;
  ldwork = klat*(klat+4);

  lsave = 5*lsave;     /* the above are only approximate */
  lwork = 5*lwork;

/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  dwork = (double *)calloc(ldwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL || dwork == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"g2fsh: workspace allocate failed\n" );
    return(NhlFATAL);
  }
/*
 * Loop through each nlata x nlona grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_Ta = index_Tb = 0;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_Ta != NCL_double) {
/*
 * Coerce nlata x nlona subsection of Ta (tmp_Ta) to double.
 */
      coerce_subset_input_double(Ta,tmp_Ta,index_Ta,type_Ta,nlatanlona,0,
                                 &missing_Ta,&missing_dTa);
    }
    else {
/*
 * Point tmp_Ta and tmp_Tb to appropriate locations in Ta and Tb.
 */
      tmp_Ta = &((double*)Ta)[index_Ta]; 
      tmp_Tb = &((double*)Tb)[index_Tb];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_Ta,nlatanlona,has_missing_Ta,
                                     missing_dTa.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(Tb,index_Tb,type_Ta,nlatbnlonb,
                                missing_dTa.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"g2fsh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'trssph' with the full argument list.
 */
      NGCALLF(dtrssph,DTRSSPH)(&intl,igrida,&nlona,&nlata,tmp_Ta,igridb,
                               &nlonb,&nlatb,tmp_Tb,wsave,&lsave,&lsvmin,
                               work,&lwork,&lwkmin,dwork,&ldwork,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"g2fsh: ier = %d\n", ier );
      }
/*
 * Copy output values from temporary array "tmp_Tb" to final array "Tb".
 */
      if(type_Tb == NCL_float) {
        for(j = 0; j < nlatbnlonb; j++) {
          ((float*)Tb)[index_Tb+j] = (float)(tmp_Tb[j]);
        }
      } 
    }
    index_Ta += nlatanlona;
    index_Tb += nlatbnlonb;
  }
/*
 * Free workspace arrays.
 */
  if (work != NULL) NclFree(work);
  if(dwork != NULL) NclFree(dwork);
  if(wsave != NULL) NclFree(wsave);
  if(type_Ta != NCL_double) {
    NclFree(tmp_Ta);
    NclFree(tmp_Tb);
  }
/*
 * Return output grid to NCL.
 */
  for( i = 0; i < ndims_Ta-2; i++ ) dsizes_Tb2[i] = dsizes_Ta[i];
  dsizes_Tb2[ndims_Ta-2] = nlatb;
  dsizes_Tb2[ndims_Ta-1] = nlonb;
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_Ta) {
    if(type_Tb == NCL_float) {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_rTa,type_Tb,0));
    }
    else {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_dTa,type_Tb,0));
    }
  }
  else {
    return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,NULL,type_Tb,0));
  }
}



NhlErrorTypes f2fsh_W( void )
{
/*
 * Input array variables
 */
  void *Ta;
  double *tmp_Ta;
  int ndims_Ta, dsizes_Ta[NCL_MAX_DIMENSIONS];
  NclScalar missing_Ta, missing_dTa, missing_rTa;
  NclBasicDataTypes type_Ta, type_Tb;
  int has_missing_Ta, found_missing;
  int nlata, nlona, igrida[2];
/*
 * Output array variables
 */
  void *Tb;
  double *tmp_Tb;
  int *dsizes_Tb, dsizes_Tb2[NCL_MAX_DIMENSIONS], nlatb, nlonb, igridb[2];
/*
 * various
 */
  int intl, i, j, index_Ta, index_Tb;
  int total_size_leftmost, nlatanlona, nlatbnlonb;
  int total_size_Ta, total_size_Tb;
/*
 * Workspace variables
 */
  int lsave, lsvmin, lwork, ldwork, lwkmin, ker = 0;
  int klat, klon, k1, k2, lwa, lwb, ier = 0;
  double *work, *wsave, *dwork;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  Ta = (void*)NclGetArgValue(
           0,
           2,
           &ndims_Ta, 
           dsizes_Ta,
           &missing_Ta,
           &has_missing_Ta,
           &type_Ta,
           2);
/*
 * Get sizes for output array.
 */
  dsizes_Tb = (int*)NclGetArgValue(
                1,
                2,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_Ta < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fsh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Compute the total number of elements in our array.
 */
  compute_nlatanlona(dsizes_Ta,dsizes_Tb,ndims_Ta,2,
                     &nlata,&nlona,&nlatanlona,&nlatb,&nlonb,&nlatbnlonb,
                     &total_size_leftmost,&total_size_Ta,&total_size_Tb);
/*
 * Check output dimensions.
 */
  if( nlatb < 4 || nlonb < 4 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fsh: The lat/lon dimensions of the output array must be at least 4");
    return(NhlFATAL);
  }
/*
 * Coerce missing values.
 */
  coerce_missing(type_Ta,has_missing_Ta,&missing_Ta,&missing_dTa,
                 &missing_rTa);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in Ta and Tb.
 */
  if(type_Ta != NCL_double) {
    type_Tb = NCL_float;
    Tb = (void*)calloc(total_size_Tb,sizeof(float));
    tmp_Tb = (double*)calloc(nlatbnlonb,sizeof(double));
    tmp_Ta = (double*)calloc(nlatanlona,sizeof(double));
    if(Tb == NULL || tmp_Tb == NULL || tmp_Ta == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fsh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_Tb = NCL_double;
    Tb = (void*)calloc(total_size_Tb,sizeof(double));
    if(Tb == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fsh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }

/* 
 * igrida describes the array going in, and igridb describes the array 
 * coming out.
 */
  igrida[0] = igridb[0] = -1;
  igrida[1] = igridb[1] =  0;

  intl = 0;

/*
 * Determine the workspace size.
 */
  klat  = nlata > nlatb ? nlata : nlatb;
  klon  = nlona > nlonb ? nlona : nlonb;
  k1    = klat < ((klon+2)/2) ? klat : ((klon+2)/2);
  k2    = (klat+1)/2;
  lwa   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lwb   = 2*klat*k2 + 3*((k1-2)*(klat+klat-k1-1))/2;
  lsave = lwb + lwa + 2*(klon+15);
  lwork = klat*(4*k1+ klon+ 2*klat+ 4) + 3*((k1-2)*2*(2*klat-k1-1))/2;
  ldwork = klat*(klat+4);

  lsave = 5*lsave;     /* the above are only approximate */
  lwork = 5*lwork;

/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  dwork = (double *)calloc(ldwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL || dwork == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fsh: workspace allocate failed\n" );
    return(NhlFATAL);
  }
/*
 * Loop through each nlata x nlona grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_Ta = index_Tb = 0;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_Ta != NCL_double) {
/*
 * Coerce nlata x nlona subsection of Ta (tmp_Ta) to double.
 */
      coerce_subset_input_double(Ta,tmp_Ta,index_Ta,type_Ta,nlatanlona,0,
                                 &missing_Ta,&missing_dTa);
    }
    else {
/*
 * Point tmp_Ta and tmp_Tb to appropriate locations in Ta and Tb.
 */
      tmp_Ta = &((double*)Ta)[index_Ta];
      tmp_Tb = &((double*)Tb)[index_Tb];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_Ta,nlatanlona,has_missing_Ta,
                                     missing_dTa.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(Tb,index_Tb,type_Ta,nlatbnlonb,
                                missing_dTa.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"f2fsh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'trssph' with the full argument list.
 */
      NGCALLF(dtrssph,DTRSSPH)(&intl,igrida,&nlona,&nlata,tmp_Ta,igridb,
                               &nlonb,&nlatb,tmp_Tb,wsave,&lsave,&lsvmin,
                               work,&lwork,&lwkmin,dwork,&ldwork,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"f2fsh: ier = %d\n", ier );
      }
/*
 * Copy output values from temporary array "tmp_Tb" to final array "Tb".
 */
      if(type_Tb == NCL_float) {
        for(j = 0; j < nlatbnlonb; j++) {
          ((float*)Tb)[index_Tb+j] = (float)(tmp_Tb[j]);
        }
      } 
    }
    index_Ta += nlatanlona;
    index_Tb += nlatbnlonb;
  }
/*
 * Free workspace arrays.
 */
  if (work != NULL) NclFree(work);
  if(dwork != NULL) NclFree(dwork);
  if(wsave != NULL) NclFree(wsave);
  if(type_Ta != NCL_double) {
    NclFree(tmp_Ta);
    NclFree(tmp_Tb);
  }
/*
 * Return output grid to NCL.
 */
  for( i = 0; i < ndims_Ta-2; i++ ) dsizes_Tb2[i] = dsizes_Ta[i];
  dsizes_Tb2[ndims_Ta-2] = nlatb;
  dsizes_Tb2[ndims_Ta-1] = nlonb;
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_Ta) {
    if(type_Tb == NCL_float) {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_rTa,type_Tb,0));
    }
    else {
      return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,&missing_dTa,type_Tb,0));
    }
  }
  else {
    return(NclReturnValue(Tb,ndims_Ta,dsizes_Tb2,NULL,type_Tb,0));
  }
}


NhlErrorTypes fo2fsh_W( void )
{
/*
 * Input array variables
 */
  void *goff;
  double *tmp_goff;
  int ndims_goff, jlat, jlat1, ilon;
  int dsizes_goff[NCL_MAX_DIMENSIONS];
  NclScalar missing_goff, missing_dgoff, missing_rgoff;
  NclBasicDataTypes type_goff, type_greg;
  int has_missing_goff, found_missing;
/*
 * Output array variables
 */
  void *greg;
  double *tmp_greg;
  int *dsizes_greg;
/*
 * Workspace variables
 */
  int lwork, lsave;
  double *work, *wsave;
/*
 * error code, various
 */
  int i, j, index_goff, index_greg, ioff, ier = 0;
  int total_size_leftmost, jlatilon, jlat1ilon;
  int total_size_goff, total_size_greg;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  goff = (void*)NclGetArgValue(
           0,
           1,
           &ndims_goff, 
           dsizes_goff,
           &missing_goff,
           &has_missing_goff,
           &type_goff,
           2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_goff < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"fo2fsh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Get sizes for output array.
 */
  compute_jlatilon(dsizes_goff,ndims_goff,&jlat,&ilon,&jlatilon,
                   &jlat1,&jlat1ilon,&total_size_leftmost,&total_size_goff,
                   &total_size_greg,1);
/*
 * Coerce missing values.
 */
  coerce_missing(type_goff,has_missing_goff,&missing_goff,&missing_dgoff,
                 &missing_rgoff);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in goff and greg.
 */
  if(type_goff != NCL_double) {
    type_greg = NCL_float;
    greg = (void*)calloc(total_size_greg,sizeof(float));
    tmp_greg = (double*)calloc(jlat1ilon,sizeof(double));
    tmp_goff = (double*)calloc(jlatilon,sizeof(double));
    if(greg == NULL || tmp_greg == NULL || tmp_goff == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"fo2fsh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_greg = NCL_double;
    greg = (void*)calloc(total_size_greg,sizeof(double));
    if(greg == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"fo2fsh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }

/*
 * Allocate space for dimension sizes and output array.
 */
  dsizes_greg = (int *)calloc(ndims_goff,sizeof(int));
  if( dsizes_greg == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"fo2fsh: Unable to allocate memory for output dimension array");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_goff-2; i++ ) dsizes_greg[i] = dsizes_goff[i];
  dsizes_greg[ndims_goff-2] = jlat1;
  dsizes_greg[ndims_goff-1] = ilon;
/*
 * Determine the workspace size.
 */
  if( ilon % 2 )  lwork = ilon*(5*jlat1+1);
  else            lwork = 2*ilon*(jlat1+1);
  lsave = 2*(2*jlat+ilon+16);

  lwork = (10*lwork)/9;
  lsave = (10*lsave)/9;

/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"fo2fsh: workspace allocate failed\n" );
    return(NhlFATAL);
  }
/*
 * Loop through each jlat x ilon grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_goff = index_greg = 0;
  ioff = 0;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_goff != NCL_double) {
/*
 * Coerce jlat x ilon subsection of goff (tmp_goff) to double, if necessary.
 */
      coerce_subset_input_double(goff,tmp_goff,index_goff,type_goff,jlatilon,
                                 0,&missing_goff,&missing_dgoff);
    }
    else {
/*
 * Point tmp_goff and tmp_greg to appropriate locations in goff and greg.
 */
      tmp_goff = &((double*)goff)[index_goff];
      tmp_greg = &((double*)greg)[index_greg];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_goff,jlatilon,has_missing_goff,
                                     missing_dgoff.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(greg,index_greg,type_goff,jlat1ilon,
                                missing_dgoff.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"fo2fsh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'fo2fsh' with the full argument list.
 */
      NGCALLF(dfo2f,DFO2F)(tmp_goff,&ilon,&jlat,tmp_greg,&jlat1,
                           work,&lwork,wsave,&lsave,&ioff,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"fo2fsh: ier = %d\n", ier );
      }
/*
 * Copy output values from temporary array "tmp_goff" to final array "goff".
 */
      if(type_greg == NCL_float) {
        for(j = 0; j < jlat1ilon; j++) {
          ((float*)greg)[index_greg+j] = (float)(tmp_greg[j]);
        }
      } 
    }
    index_goff += jlatilon;
    index_greg += jlat1ilon;
  }
/*
 * Free workspace arrays.
 */
  NclFree(work);
  NclFree(wsave);
  if(type_goff != NCL_double) {
    NclFree(tmp_goff);
    NclFree(tmp_greg);
  }
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_goff) {
    if(type_greg == NCL_float) {
      return(NclReturnValue(greg,ndims_goff,dsizes_greg,&missing_rgoff,
                            type_greg,0));
    }
    else {
      return(NclReturnValue(greg,ndims_goff,dsizes_greg,&missing_dgoff,
                            type_greg,0));
    }
  }
  else {
    return(NclReturnValue(greg,ndims_goff,dsizes_greg,NULL,type_greg,0));
  }
}


NhlErrorTypes f2fosh_W( void )
{
/*
 * Input array variables
 */
  void *greg;
  double *tmp_greg;
  int ndims_greg, jlat, jlat1, ilon;
  int dsizes_greg[NCL_MAX_DIMENSIONS];
  NclScalar missing_greg, missing_dgreg, missing_rgreg;
  NclBasicDataTypes type_greg, type_goff;
  int has_missing_greg, found_missing;
/*
 * Output array variables
 */
  void *goff;
  double *tmp_goff;
  int *dsizes_goff;
/*
 * Workspace variables
 */
  int lwork, lsave;
  double *work, *wsave;
/*
 * error code, various
 */
  int i, j, index_greg, index_goff, ioff, ier = 0;
  int total_size_leftmost, jlat1ilon, jlatilon;
  int total_size_greg, total_size_goff;
/*
 * Retrieve parameters
 *
 * Note any of the pointer parameters can be set to NULL, which
 * implies you don't care about its value.
 */
  greg = (void*)NclGetArgValue(
           0,
           1,
           &ndims_greg, 
           dsizes_greg,
           &missing_greg,
           &has_missing_greg,
           &type_greg,
           2);
/*
 * The grid coming in must be at least 2-dimensional.
 */
  if( ndims_greg < 2 ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fosh: The input array must be at least a 2-dimensional array");
    return(NhlFATAL);
  }
/*
 * Compute number of elements.
 */
  compute_jlatilon(dsizes_greg,ndims_greg,&jlat1,&ilon,&jlat1ilon,
                   &jlat,&jlatilon,&total_size_leftmost,&total_size_greg,
                   &total_size_goff,0);
/*
 * Coerce missing values.
 */
  coerce_missing(type_greg,has_missing_greg,&missing_greg,&missing_dgreg,
                 &missing_rgreg);
/*
 * Allocate space for temporary input and output. If the input is not double,
 * then the output will be float, otherwise the output will be double.
 * The temporary arrays are just big enough to hold a 2-dimensional subsection
 * of the input and output. We only need to allocate space for them if the
 * input is not already double, otherwise, we just have them point to the
 * appropriate locations in goff and greg.
 */
  if(type_greg != NCL_double) {
    type_goff = NCL_float;
    goff = (void*)calloc(total_size_goff,sizeof(float));
    tmp_greg = (double*)calloc(jlat1ilon,sizeof(double));
    tmp_goff = (double*)calloc(jlatilon,sizeof(double));
    if(goff == NULL || tmp_greg == NULL || tmp_goff == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fosh: Unable to allocate memory for in/output arrays");
      return(NhlFATAL);
    }
  } 
  else { 
    type_goff = NCL_double;
    goff = (void*)calloc(total_size_goff,sizeof(double));
    if(goff == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fosh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }
  }

/*
 * Allocate space for dimension sizes and output array.
 */
  dsizes_goff = (int *)calloc(ndims_greg,sizeof(int));
  if( dsizes_goff == NULL) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fosh: Unable to allocate memory for output dimension array");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_greg-2; i++ ) dsizes_goff[i] = dsizes_greg[i];
  dsizes_goff[ndims_greg-2] = jlat;
  dsizes_goff[ndims_greg-1] = ilon;
/*
 * Determine the workspace size.
 */
  if( ilon % 2 )  lwork = ilon*(5*jlat1+1);
  else            lwork = 2*ilon*(jlat1+1);
  lsave = 2*(2*jlat+ilon+16);
  lwork = (10*lwork)/9;
  lsave = (10*lsave)/9;
/*
 * Dynamically allocate the various work space.
 */
  work  = (double *)calloc(lwork,sizeof(double));
  wsave = (double *)calloc(lsave,sizeof(double));
  if (work == NULL || wsave == NULL ) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"f2fosh: workspace allocate failed\n" );
    return(NhlFATAL);
  }
/*
 * Loop through each jlat x ilon grid.  If it contains any missing values,
 * then return all missing values for that grid.  Otherwise, keep going 
 * and do the regridding.
 */ 
  index_greg = index_goff = 0;
  ioff = 1;
  for(i = 1; i <= total_size_leftmost; i++) {
    if(type_greg != NCL_double) {
/*
 * Coerce jlat1 x ilon subsection of greg (tmp_greg) to double.
 */
      coerce_subset_input_double(greg,tmp_greg,index_greg,type_greg,jlat1ilon,
                                 0,&missing_greg,&missing_dgreg);
    }
    else {
/*
 * Point tmp_goff and tmp_greg to appropriate locations in goff and greg.
 */
      tmp_goff = &((double*)goff)[index_goff];
      tmp_greg = &((double*)greg)[index_greg];
    }
/*
 * Check for missing values.
 */
    found_missing = contains_missing(tmp_greg,jlat1ilon,has_missing_greg,
                                     missing_dgreg.doubleval);
    if(found_missing) {
/*
 * Set all elements of this 2D grid to a missing value.
 */
      set_subset_output_missing(goff,index_goff,type_greg,jlatilon,
                                missing_dgreg.doubleval);
      NhlPError(NhlWARNING,NhlEUNKNOWN,"f2fosh: A 2-dimensional input array contains missing values. No interpolation performed on this 2d array.");
    }
    else {
/*
 * Call the f77 version of 'f2fosh' with the full argument list.
 */
      NGCALLF(dfo2f,DFO2F)(tmp_goff,&ilon,&jlat,tmp_greg,&jlat1,
                           work,&lwork,wsave,&lsave,&ioff,&ier);
      if (ier) {
        NhlPError(NhlWARNING,NhlEUNKNOWN,"f2fosh: ier = %d\n", ier );
      }
/*
 * Copy output values from temporary array "tmp_greg" to final array "greg".
 */
      if(type_goff == NCL_float) {
        for(j = 0; j < jlatilon; j++) {
          ((float*)goff)[index_goff+j] = (float)(tmp_goff[j]);
        }
      } 
    }
    index_greg += jlat1ilon;
    index_goff += jlatilon;
  }
/*
 * Free workspace arrays.
 */
  NclFree(work);
  NclFree(wsave);
  if(type_greg != NCL_double) {
    NclFree(tmp_goff);
    NclFree(tmp_greg);
  }
/*
 * Check if we need to return the missing value attribute.
 */
  if(has_missing_greg) {
    if(type_goff == NCL_float) {
      return(NclReturnValue(goff,ndims_greg,dsizes_goff,&missing_rgreg,
                            type_goff,0));
    }
    else {
      return(NclReturnValue(goff,ndims_greg,dsizes_goff,&missing_dgreg,
                            type_goff,0));
    }
  }
  else {
    return(NclReturnValue(goff,ndims_greg,dsizes_goff,NULL,type_goff,0));
  }
}

void compute_jlatilon(int *dsizes,int ndims,int *jlat,int *ilon,
                      int *jlatilon,int *jlat1,int *jlat1ilon,int *nt,
                      int *total_in, int *total_out, int iopt)
{
  int i;

  *jlat = dsizes[ndims-2];
  *ilon = dsizes[ndims-1];
  *jlatilon = *jlat * *ilon;
  *nt = 1;
  for(i = 0; i < ndims-2; i++) *nt *= dsizes[i];
  if(iopt) {
    *jlat1 = *jlat + 1;
  }
  else {
    *jlat1 = *jlat - 1;
  }
  *jlat1ilon = *jlat1 * *ilon;

  *total_in  = *jlatilon  * *nt;
  *total_out = *jlat1ilon * *nt;
}

