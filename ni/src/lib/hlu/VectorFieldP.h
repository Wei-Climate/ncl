/*
 *      $Id: VectorFieldP.h,v 1.2 1997-08-11 18:22:30 dbrown Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1993			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		VectorFieldP.h
 *
 *	Author:		David I. Brown
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Thu Sep 28 11:47:36 MDT 1995
 *
 *	Description:	Private declarations for VectorField object.
 */
#ifndef _NVectorFieldP_h
#define _NVectorFieldP_h

#include <ncarg/hlu/DataItemP.h>
#include <ncarg/hlu/VectorField.h>
#include <ncarg/hlu/VectorFieldFloatP.h>

#ifndef FLT_MAX
#define FLT_MAX			10.0e37
#endif

typedef struct _NhlVectorFieldLayerPart{

	NhlString		type_string;

	/* Public resources */

	NhlGenArray	d_arr;
	NhlGenArray	u_arr;
	NhlGenArray	v_arr;
	NhlGenArray	x_arr;
	NhlGenArray	y_arr;

	NhlBoolean	polar_data;
	NhlBoolean	subset_by_index;
	NhlBoolean	copy_arrays;
	NhlBoolean	exchange_dimensions;
	NhlBoolean	exchange_uv_data;

	NhlBoolean	single_missing;
	NhlGenArray	missing_u_value;
	NhlGenArray	missing_v_value;
	NhlGenArray	mag_min;
	NhlGenArray	mag_max;
	NhlGenArray	u_min;
	NhlGenArray	u_max;
	NhlGenArray	v_min;
	NhlGenArray	v_max;
	NhlGenArray	x_start;
	NhlGenArray	x_end;
	NhlGenArray	y_start;
	NhlGenArray	y_end;

	NhlGenArray	x_subset_start;
	NhlGenArray	x_subset_end;
	NhlGenArray	y_subset_start;
	NhlGenArray	y_subset_end;

	int		x_index_start;
	int		x_index_end;
	int		y_index_start;
	int		y_index_end;
	
	int		x_stride;
	int		y_stride;

	float		x_actual_start;
	float		x_actual_end;
	float		y_actual_start;
	float		y_actual_end;

	/* private fields */

	int		len_dims[2];
	NhlBoolean	use_d_arr;
	int		ix_start;
	int		ix_end;
	int		iy_start;
	int		iy_end;

	NhlVectorFieldFloatLayer	vffloat;
        NhlBoolean	up_to_date;

} NhlVectorFieldLayerPart;

typedef struct _NhlVectorFieldLayerRec{
	NhlBaseLayerPart		base;
	NhlDataItemLayerPart		dataitem;
	NhlVectorFieldLayerPart		vfield;
} NhlVectorFieldLayerRec;

typedef struct _NhlVectorFieldClassPart{
	int	foo;
} NhlVectorFieldClassPart;

typedef struct _NhlVectorFieldClassRec{
	NhlBaseClassPart		base_class;
	NhlDataItemClassPart	dataitem_class;
	NhlVectorFieldClassPart	vfield_class;
} NhlVectorFieldClassRec;

typedef struct _NhlVectorFieldClassRec *NhlVectorFieldClass;
typedef struct _NhlVectorFieldLayerRec *NhlVectorFieldLayer;

extern NhlVectorFieldClassRec NhlvectorFieldClassRec;

#endif  /* _NVectorFieldP_h */

