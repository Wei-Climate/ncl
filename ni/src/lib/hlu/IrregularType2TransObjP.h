/*
 *      $Id: IrregularType2TransObjP.h,v 1.5 1994-06-03 19:23:51 dbrown Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1992			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		IrregularType2TransObjP.h
 *
 *	Author:		Ethan Alpert
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Fri Oct 16 13:04:03 MDT 1992
 *
 *	Description:	Private header file for irregular type 2 grid 
 *			transformations.
 */
#ifndef _NIrregularType2TransObjP_h
#define _NIrregularType2TransObjP_h

#include <ncarg/hlu/TransObjP.h>
#include <ncarg/hlu/IrregularType2TransObj.h>

#include <ncarg/hlu/CoordApprox.h>

typedef struct _NhlIrregularType2TransObjLayerPart {
	/* Publically setable resources */
	float 	*x_coord_points;
	float	*x_inter_points;
	float	x_min;
	float	x_max;
	int 	x_num_points;
	int	x_reverse;
	float	x_tension;
	int	x_samples;
	int	x_use_log;
	float 	*y_coord_points;
	float	*y_inter_points;
	float	y_min;
	float	y_max;
	int 	y_num_points;
	int	y_reverse;
	float	y_tension;
	int	y_samples;
	int	y_use_log;
	
	/* Private fields */
	NhlCoordDat thecoord;
	int xstatus;
	int ystatus;
	float	ur;
	float 	ul;
	float	ut;
	float	ub;
	float 	compc_x_min;
	float	compc_x_max;
	float	compc_y_min;
	float 	compc_y_max;
	int log_lin_value;
	NhlCompareDat *xmin_dat;
	NhlCompareDat *xmax_dat;
	NhlCompareDat *ymin_dat;
	NhlCompareDat *ymax_dat;
	NhlCompareDat *compc_xmin_dat;
	NhlCompareDat *compc_xmax_dat;
	NhlCompareDat *compc_ymin_dat;
	NhlCompareDat *compc_ymax_dat;

} NhlIrregularType2TransObjLayerPart;

typedef struct _NhlIrregularType2TransObjLayerRec {
	NhlObjLayerPart				base;
	NhlTransObjLayerPart			trobj;
	NhlIrregularType2TransObjLayerPart	ir2trans;
} NhlIrregularType2TransObjLayerRec;

typedef struct _NhlIrregularType2TransObjLayerClassPart {
	char *foo;
}NhlIrregularType2TransObjLayerClassPart;

typedef struct _NhlIrregularType2TransObjLayerClassRec {
	NhlObjLayerClassPart			base_class;
	NhlTransObjLayerClassPart		trobj_class;
	NhlIrregularType2TransObjLayerClassPart	ir2trans_class;
}NhlIrregularType2TransObjLayerClassRec;

typedef struct _NhlIrregularType2TransObjLayerClassRec
					*NhlIrregularType2TransObjLayerClass;
typedef struct _NhlIrregularType2TransObjLayerRec
						*NhlIrregularType2TransObjLayer;

extern NhlIrregularType2TransObjLayerClassRec
					NhlirregularType2TransObjLayerClassRec;

#endif /* _NIrregularType2TransObjP_h */
