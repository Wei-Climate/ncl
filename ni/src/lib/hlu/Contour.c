/*
 *      $Id: Contour.c,v 1.15 1994-06-03 19:23:37 dbrown Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1992			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		Contour.c
 *
 *	Author:		David Brown
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Tue Nov 16 15:18:58 MST 1993
 *
 *	Description:	Creates and manages a Contour plot object
 */

#include <stdio.h>
#include <math.h>
#include <ncarg/hlu/hluutil.h>
#include <ncarg/hlu/ContourP.h>
#include <ncarg/hlu/Workstation.h>
#include <ncarg/hlu/IrregularTransObj.h>
#include <ncarg/hlu/IrregularType2TransObj.h>

/*
 * Function:	ResourceUnset
 *
 * Description:	This function can be used to determine if a resource has
 *		been set at initialize time either in the Create call or
 *		from a resource data base. In order to use it the Boolean
 *		'..resource_set' variable MUST directly proceed the name
 *		of the resource variable it refers to in the LayerPart
 *		struct. Also a .nores Resource for the resource_set variable
 *		must directly preceed the Resource of interest in the 
 *		Resource initialization list in this module.
 *
 * In Args:	
 *		NrmName		name,
 *		NrmClass	class,
 *		NhlPointer	base,
 *		unsigned int	offset
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */

/*ARGSUSED*/
static NhlErrorTypes
ResourceUnset
#if	__STDC__
(
	NrmName		name,
	NrmClass	class,
	NhlPointer	base,
	unsigned int	offset
)
#else
(name,class,base,offset)
	NrmName		name;
	NrmClass	class;
	NhlPointer	base;
	unsigned int	offset;
#endif
{
	char *cl = (char *) base;
	NhlBoolean *set = (NhlBoolean *)(cl + offset - sizeof(NhlBoolean));

	*set = False;

	return NhlNOERROR;
}

#define	Oset(field)	NhlOffset(NhlContourDataDepLayerRec,cndata.field)
static NhlResource data_resources[] = {

	{NhlNcnExplicitLabels,NhlCcnExplicitLabels,NhlT1DStringGenArray,
		 sizeof(NhlPointer),
		 Oset(labels),NhlTImmediate,
		 _NhlUSET((NhlPointer)NULL),0,(NhlFreeFunc)NhlFreeGenArray}
};
#undef Oset

#define Oset(field)     NhlOffset(NhlContourLayerRec,contour.field)
static NhlResource resources[] = {
	
	{NhlNcnScalarFieldData,NhlCcnScalarFieldData,_NhlTDataList,
		 sizeof(NhlGenArray),
		 Oset(scalar_field_data),NhlTImmediate,_NhlUSET(NULL),0,NULL},
	{ NhlNcnOutOfRangeValF,NhlCcnOutOfRangeValF,NhlTFloat,sizeof(float),
		  Oset(out_of_range_val),NhlTString,_NhlUSET("1.0E12"),0,NULL},
	{ NhlNcnSpecialValF,NhlCcnSpecialValF,NhlTFloat,sizeof(float),
		  Oset(special_val),NhlTString,_NhlUSET("-9999.0"),0,NULL},
	{ NhlNcnLevelCount,NhlCcnLevelCount,NhlTInteger,sizeof(int),
		  Oset(level_count),NhlTImmediate,
		  _NhlUSET((NhlPointer) 16),0,NULL},
	{ NhlNcnLevelSelectionMode,NhlCcnLevelSelectionMode,
		  NhlTInteger,sizeof(int),Oset(level_selection_mode),
		  NhlTImmediate,_NhlUSET((NhlPointer) Nhl_cnAUTOMATIC),0,NULL},
	{ NhlNcnMaxLevelCount,NhlCcnMaxLevelCount,NhlTInteger,sizeof(int),
		  Oset(max_level_count),NhlTImmediate,
		  _NhlUSET((NhlPointer) 16),0,NULL},
	{ NhlNcnLabelMasking,NhlCcnLabelMasking,NhlTBoolean,sizeof(NhlBoolean),
		  Oset(label_masking),
		  NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{ NhlNcnLevelSpacingF,NhlCcnLevelSpacingF,NhlTFloat,sizeof(float),
		  Oset(level_spacing),NhlTString,_NhlUSET("5.0"),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(min_level_set),NhlTImmediate,
		 _NhlUSET((NhlPointer)True),0,NULL},
	{ NhlNcnMinLevelValF,NhlCcnMinLevelValF,NhlTFloat,sizeof(float),
		  Oset(min_level_val),
		  NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(max_level_set),NhlTImmediate,
		 _NhlUSET((NhlPointer)True),0,NULL},
	{ NhlNcnMaxLevelValF,NhlCcnMaxLevelValF,NhlTFloat,sizeof(float),
		  Oset(max_level_val),
		  NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(llabel_interval_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{ NhlNcnLineLabelInterval,NhlCcnLineLabelInterval,
		  NhlTFloat,sizeof(float),
		  Oset(llabel_interval),
		  NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{NhlNcnDelayLabels, NhlCcnDelayLabels, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(delay_labels),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnDelayLines, NhlCcnDelayLines, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(delay_lines),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},

	{NhlNcnMonoLevelFlag, NhlCcnMonoLevelFlag, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_level_flag),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{NhlNcnMonoLevelFlag, NhlCcnMonoLevelFlag, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_level_flag),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{NhlNcnMonoFillColor, NhlCcnMonoFillColor, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_fill_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{NhlNcnMonoFillPattern, NhlCcnMonoFillPattern, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_fill_pattern),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{NhlNcnMonoFillScale, NhlCcnMonoFillScale, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_fill_scale),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnMonoLineColor, NhlCcnMonoLineColor, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_line_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnMonoLineDashPattern, NhlCcnMonoLineDashPattern, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_line_dash_pattern),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},

	{NhlNcnMonoLineThickness, NhlCcnMonoLineThickness, NhlTBoolean,
		 sizeof(NhlBoolean),Oset(mono_line_thickness),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},

/* Array resources */

	{NhlNcnLevels, NhlCcnLevels, NhlT1DFloatGenArray,
		 sizeof(NhlPointer),Oset(levels),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLevelFlags, NhlCcnLevelFlags, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(level_flags),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnFillColors, NhlCcnFillColors, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(fill_colors),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnFillPatterns, NhlCcnFillPatterns, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(fill_patterns),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnFillScales, NhlCcnFillScales, NhlTGenArray,
		 sizeof(NhlPointer),Oset(fill_scales),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLineColors, NhlCcnLineColors, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(line_colors),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLineDashPatterns, NhlCcnLineDashPatterns, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(line_dash_patterns),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLineThicknesses, NhlCcnLineThicknesses, NhlT1DFloatGenArray,
		 sizeof(NhlPointer),Oset(line_thicknesses),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLineLabelStrings, NhlCcnLineLabelStrings, NhlT1DStringGenArray,
		 sizeof(NhlPointer),Oset(llabel_strings),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNcnLineLabelColors, NhlCcnLineLabelColors, NhlT1DIntGenArray,
		 sizeof(NhlPointer),Oset(llabel_colors),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},

	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(line_dash_seglen_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        { NhlNcnLineDashSegLenF, NhlCcnLineDashSegLenF,NhlTFloat,sizeof(float),
		  Oset(line_dash_seglen),
		  NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{ NhlNcnLineLabelSpacing,NhlCcnLineLabelSpacing,
		  NhlTInteger,sizeof(int),Oset(llabel_spacing),
		  NhlTImmediate,_NhlUSET((NhlPointer)Nhl_cnRANDOMIZED),0,NULL},

	{NhlNcnLowUseHighLabelRes,NhlCcnLowUseHighLabelRes,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(low_use_high_attrs),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnHighUseLineLabelRes,NhlCcnHighUseLineLabelRes,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(high_use_line_attrs),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnConstFUseInfoLabelRes,NhlCcnConstFUseInfoLabelRes,
		 NhlTBoolean,sizeof(NhlBoolean),Oset(constf_use_info_attrs),
		 NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},

	{NhlNcnLineLabelsOn,NhlCcnLineLabelsOn,NhlTBoolean,sizeof(NhlBoolean),
		 Oset(line_lbls.on),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(line_lbls.height_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        {NhlNcnLineLabelTextHeightF,NhlCcnLineLabelTextHeightF,
		  NhlTFloat,sizeof(float),Oset(line_lbls.height),
		  NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{NhlNcnLineLabelFont,NhlCcnLineLabelFont,NhlTFont, 
		 sizeof(int),Oset(line_lbls.font),
		 NhlTImmediate,_NhlUSET((NhlPointer) 1),0,NULL},
	{NhlNcnMonoLineLabelColor,NhlCcnMonoLineLabelColor,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(line_lbls.mono_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnLineLabelFontAspectF,NhlCcnLineLabelFontAspectF,NhlTFloat, 
		 sizeof(float),Oset(line_lbls.aspect),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnLineLabelFontThicknessF,NhlCcnLineLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(line_lbls.thickness),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnLineLabelFontQuality,NhlCcnLineLabelFontQuality,NhlTFQuality, 
		 sizeof(NhlFontQuality),Oset(line_lbls.quality),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlHIGH),0,NULL},
	{NhlNcnLineLabelConstantSpacingF,NhlCcnLineLabelConstantSpacingF,
		 NhlTFloat,sizeof(float),Oset(line_lbls.cspacing),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnLineLabelAngleF,NhlCcnLineLabelAngleF,
		 NhlTFloat,sizeof(float),Oset(line_lbls.angle),
		 NhlTString,_NhlUSET("-1.0"),0,NULL},
	{NhlNcnLineLabelFuncCode,NhlCcnLineLabelFuncCode,NhlTCharacter, 
		 sizeof(char),Oset(line_lbls.fcode[0]),
		 NhlTString, _NhlUSET(":"),0,NULL},
	{NhlNcnLineLabelBackgroundColor,NhlCcnLineLabelBackgroundColor,
		 NhlTInteger,sizeof(int),Oset(line_lbls.back_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlBACKGROUND),0,NULL},
	{NhlNcnLineLabelPerim,NhlCcnLineLabelPerim,NhlTInteger,sizeof(int),
		 Oset(line_lbls.perim_on),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnLineLabelPerimSpaceF,NhlCcnLineLabelPerimSpaceF,
		 NhlTFloat,sizeof(float),Oset(line_lbls.perim_space),
		 NhlTString,_NhlUSET("0.33"),0,NULL},
	{NhlNcnLineLabelPerimColor,NhlCcnLineLabelPerimColor,NhlTInteger,
		 sizeof(int),Oset(line_lbls.perim_lcolor),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlFOREGROUND),0,NULL},
	{NhlNcnLineLabelPerimThicknessF,NhlCcnLineLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(line_lbls.perim_lthick),
		 NhlTString, _NhlUSET("1.0"),0,NULL},


	{NhlNcnHighLabelsOn,NhlCcnHighLabelsOn,NhlTBoolean,sizeof(NhlBoolean),
		 Oset(high_lbls.on),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{NhlNcnHighLabelString,NhlCcnHighLabelString,
		 NhlTStringPtr,sizeof(NhlPointer),
		 Oset(high_lbls.text),NhlTImmediate,_NhlUSET(NULL),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(high_lbls.height_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        {NhlNcnHighLabelTextHeightF,NhlCcnHighLabelTextHeightF,
		 NhlTFloat,sizeof(float),Oset(high_lbls.height),
		 NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{NhlNcnHighLabelFont,NhlCcnHighLabelFont,NhlTFont, 
		 sizeof(int),Oset(high_lbls.font),
		 NhlTImmediate,_NhlUSET((NhlPointer) 1),0,NULL},
	{NhlNcnHighLabelFontColor,NhlCcnHighLabelFontColor,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(high_lbls.mono_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnHighLabelFontAspectF,NhlCcnHighLabelFontAspectF,NhlTFloat, 
		 sizeof(float),Oset(high_lbls.aspect),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnHighLabelFontThicknessF,NhlCcnHighLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(high_lbls.thickness),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnHighLabelFontQuality,NhlCcnHighLabelFontQuality,NhlTFQuality, 
		 sizeof(NhlFontQuality),Oset(high_lbls.quality),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlHIGH),0,NULL},
	{NhlNcnHighLabelConstantSpacingF,NhlCcnHighLabelConstantSpacingF,
		 NhlTFloat,sizeof(float),Oset(high_lbls.cspacing),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnHighLabelFuncCode,NhlCcnHighLabelFuncCode,NhlTCharacter, 
		 sizeof(char),Oset(high_lbls.fcode[0]),
		 NhlTString, _NhlUSET(":"),0,NULL},
	{ NhlNcnHighLabelBackgroundColor,NhlCcnHighLabelBackgroundColor,
		  NhlTInteger,sizeof(int),Oset(high_lbls.back_color),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlBACKGROUND),0,NULL},
	{ NhlNcnHighLabelPerim,NhlCcnHighLabelPerim,NhlTInteger,sizeof(int),
		Oset(high_lbls.perim_on),
		NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{ NhlNcnHighLabelPerimSpaceF,NhlCcnHighLabelPerimSpaceF,
		  NhlTFloat,sizeof(float),Oset(high_lbls.perim_space),
		  NhlTString,_NhlUSET("0.33"),0,NULL},
	{ NhlNcnHighLabelPerimColor,NhlCcnHighLabelPerimColor,NhlTInteger,
		  sizeof(int),Oset(high_lbls.perim_lcolor),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlFOREGROUND),0,NULL},
	{NhlNcnHighLabelPerimThicknessF,NhlCcnHighLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(high_lbls.perim_lthick),
		 NhlTString, _NhlUSET("1.0"),0,NULL},

	{NhlNcnLowLabelsOn,NhlCcnLowLabelsOn,NhlTBoolean,sizeof(NhlBoolean),
		 Oset(low_lbls.on),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{NhlNcnLowLabelString,NhlCcnLowLabelString,
		 NhlTStringPtr,sizeof(NhlPointer),
		 Oset(low_lbls.text),NhlTImmediate,_NhlUSET(NULL),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(low_lbls.height_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        {NhlNcnLowLabelTextHeightF,NhlCcnLowLabelTextHeightF,
		 NhlTFloat,sizeof(float),Oset(low_lbls.height),
		 NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
	{NhlNcnLowLabelFont,NhlCcnLowLabelFont,NhlTFont, 
		 sizeof(int),Oset(low_lbls.font),
		 NhlTImmediate,_NhlUSET((NhlPointer) 1),0,NULL},
	{NhlNcnLowLabelFontColor,NhlCcnLowLabelFontColor,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(low_lbls.mono_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnLowLabelFontAspectF,NhlCcnLowLabelFontAspectF,NhlTFloat, 
		 sizeof(float),Oset(low_lbls.aspect),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnLowLabelFontThicknessF,NhlCcnLowLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(low_lbls.thickness),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnLowLabelFontQuality,NhlCcnLowLabelFontQuality,NhlTFQuality, 
		 sizeof(NhlFontQuality),Oset(low_lbls.quality),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlHIGH),0,NULL},
	{NhlNcnLowLabelConstantSpacingF,NhlCcnLowLabelConstantSpacingF,
		 NhlTFloat,sizeof(float),Oset(low_lbls.cspacing),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnLowLabelFuncCode,NhlCcnLowLabelFuncCode,NhlTCharacter, 
		 sizeof(char),Oset(low_lbls.fcode[0]),
		 NhlTString, _NhlUSET(":"),0,NULL},
	{ NhlNcnLowLabelBackgroundColor,NhlCcnLowLabelBackgroundColor,
		  NhlTInteger,sizeof(int),Oset(low_lbls.back_color),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlBACKGROUND),0,NULL},
	{ NhlNcnLowLabelPerim,NhlCcnLowLabelPerim,NhlTInteger,sizeof(int),
		Oset(low_lbls.perim_on),
		NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{ NhlNcnLowLabelPerimSpaceF,NhlCcnLowLabelPerimSpaceF,
		  NhlTFloat,sizeof(float),Oset(low_lbls.perim_space),
		  NhlTString,_NhlUSET("0.33"),0,NULL},
	{ NhlNcnLowLabelPerimColor,NhlCcnLowLabelPerimColor,NhlTInteger,
		  sizeof(int),Oset(low_lbls.perim_lcolor),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlFOREGROUND),0,NULL},
	{NhlNcnLowLabelPerimThicknessF,NhlCcnLowLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(low_lbls.perim_lthick),
		 NhlTString, _NhlUSET("1.0"),0,NULL},

	{NhlNcnInfoLabelOn,NhlCcnInfoLabelOn,NhlTBoolean,sizeof(NhlBoolean),
		 Oset(info_lbl.on),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{NhlNcnInfoLabelString,NhlCcnInfoLabelString,
		 NhlTStringPtr,sizeof(NhlPointer),
		 Oset(info_string),NhlTImmediate,_NhlUSET(NULL),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(info_lbl.height_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        {NhlNcnInfoLabelTextHeightF,NhlCcnInfoLabelTextHeightF,
		 NhlTFloat,sizeof(float),Oset(info_lbl.height),
		 NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
        {NhlNcnInfoLabelTextDirection,NhlCcnInfoLabelTextDirection,
		 NhlTTextDirection,sizeof(NhlTextDirection),
		 Oset(info_lbl.direction),
		 NhlTImmediate,_NhlUSET((NhlPointer)NhlACROSS),0,NULL},
	{NhlNcnInfoLabelFont,NhlCcnInfoLabelFont,NhlTFont, 
		 sizeof(int),Oset(info_lbl.font),
		 NhlTImmediate,_NhlUSET((NhlPointer) 1),0,NULL},
	{NhlNcnInfoLabelFontColor,NhlCcnInfoLabelFontColor,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(info_lbl.mono_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnInfoLabelFontAspectF,NhlCcnInfoLabelFontAspectF,NhlTFloat, 
		 sizeof(float),Oset(info_lbl.aspect),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnInfoLabelFontThicknessF,NhlCcnInfoLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(info_lbl.thickness),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnInfoLabelFontQuality,NhlCcnInfoLabelFontQuality,NhlTFQuality, 
		 sizeof(NhlFontQuality),Oset(info_lbl.quality),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlHIGH),0,NULL},
	{NhlNcnInfoLabelConstantSpacingF,NhlCcnInfoLabelConstantSpacingF,
		 NhlTFloat,sizeof(float),Oset(info_lbl.cspacing),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnInfoLabelAngleF,NhlCcnInfoLabelAngleF,
		 NhlTFloat,sizeof(float),Oset(info_lbl.angle),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnInfoLabelFuncCode,NhlCcnInfoLabelFuncCode,NhlTCharacter, 
		 sizeof(char),Oset(info_lbl.fcode[0]),
		 NhlTString, _NhlUSET(":"),0,NULL},
	{ NhlNcnInfoLabelBackgroundColor,NhlCcnInfoLabelBackgroundColor,
		  NhlTInteger,sizeof(int),Oset(info_lbl.back_color),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlBACKGROUND),0,NULL},
	{ NhlNcnInfoLabelPerim,NhlCcnInfoLabelPerim,NhlTInteger,sizeof(int),
		Oset(info_lbl.perim_on),
		NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{ NhlNcnInfoLabelPerimSpaceF,NhlCcnInfoLabelPerimSpaceF,
		  NhlTFloat,sizeof(float),Oset(info_lbl.perim_space),
		  NhlTString,_NhlUSET("0.33"),0,NULL},
	{ NhlNcnInfoLabelPerimColor,NhlCcnInfoLabelPerimColor,NhlTInteger,
		  sizeof(int),Oset(info_lbl.perim_lcolor),
		  NhlTImmediate,_NhlUSET((NhlPointer) NhlFOREGROUND),0,NULL},
	{NhlNcnInfoLabelPerimThicknessF,NhlCcnInfoLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(info_lbl.perim_lthick),
		 NhlTString, _NhlUSET("1.0"),0,NULL},

	{NhlNcnInfoLabelZone,NhlCcnInfoLabelZone,NhlTInteger,
		 sizeof(int),Oset(info_lbl_rec.zone),NhlTImmediate,
		 _NhlUSET((NhlPointer) 5),0,NULL},
	{NhlNcnInfoLabelSide,NhlCcnInfoLabelSide,NhlTPosition,
		 sizeof(NhlPosition),Oset(info_lbl_rec.side),NhlTImmediate,
		 _NhlUSET((NhlPointer)NhlBOTTOM),0,NULL},
	{NhlNcnInfoLabelJust,NhlCcnInfoLabelJust,
		 NhlTJustification,sizeof(NhlJustification),
		 Oset(info_lbl_rec.just),NhlTImmediate,
		 _NhlUSET((NhlPointer)NhlTOPRIGHT),0,NULL},
	{NhlNcnInfoLabelParallelPosF,NhlCcnInfoLabelParallelPosF,NhlTFloat,
		 sizeof(float),Oset(info_lbl_rec.para_pos),NhlTString,
		 _NhlUSET("1.0"),0,NULL},
	{NhlNcnInfoLabelOrthogonalPosF,NhlCcnInfoLabelOrthogonalPosF,NhlTFloat,
		 sizeof(float),Oset(info_lbl_rec.ortho_pos),NhlTString,
		 _NhlUSET("0.02"),0,NULL},

	{NhlNcnConstFLabelOn,NhlCcnConstFLabelOn,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(constf_lbl.on),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
	{NhlNcnConstFLabelString,NhlCcnConstFLabelString,
		 NhlTStringPtr,sizeof(NhlPointer),
		 Oset(constf_string),NhlTImmediate,_NhlUSET(NULL),0,NULL},
	{"no.res","No.res",NhlTBoolean,sizeof(NhlBoolean),
		 Oset(constf_lbl.height_set),
		 NhlTImmediate,_NhlUSET((NhlPointer)True),0,NULL},
        {NhlNcnConstFLabelTextHeightF,NhlCcnConstFLabelTextHeightF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.height),
		 NhlTProcedure,_NhlUSET((NhlPointer)ResourceUnset),0,NULL},
        {NhlNcnConstFLabelTextDirection,NhlCcnConstFLabelTextDirection,
		 NhlTTextDirection,sizeof(NhlTextDirection),
		 Oset(constf_lbl.direction),
		 NhlTImmediate,_NhlUSET((NhlPointer)NhlACROSS),0,NULL},
	{NhlNcnConstFLabelFont,NhlCcnConstFLabelFont,NhlTFont, 
		 sizeof(int),Oset(constf_lbl.font),
		 NhlTImmediate,_NhlUSET((NhlPointer) 1),0,NULL},
	{NhlNcnConstFLabelFontColor,NhlCcnConstFLabelFontColor,NhlTBoolean,
		 sizeof(NhlBoolean),Oset(constf_lbl.mono_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnConstFLabelFontAspectF,NhlCcnConstFLabelFontAspectF,NhlTFloat, 
		 sizeof(float),Oset(constf_lbl.aspect),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnConstFLabelFontThicknessF,NhlCcnConstFLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.thickness),
		 NhlTString, _NhlUSET("1.0"),0,NULL},
	{NhlNcnConstFLabelFontQuality,NhlCcnConstFLabelFontQuality,
		 NhlTFQuality, 
		 sizeof(NhlFontQuality),Oset(constf_lbl.quality),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlHIGH),0,NULL},
	{NhlNcnConstFLabelConstantSpacingF,NhlCcnConstFLabelConstantSpacingF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.cspacing),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnConstFLabelAngleF,NhlCcnConstFLabelAngleF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.angle),
		 NhlTString,_NhlUSET("0.0"),0,NULL},
	{NhlNcnConstFLabelFuncCode,NhlCcnConstFLabelFuncCode,NhlTCharacter, 
		 sizeof(char),Oset(constf_lbl.fcode[0]),
		 NhlTString, _NhlUSET(":"),0,NULL},
	{NhlNcnConstFLabelBackgroundColor,NhlCcnConstFLabelBackgroundColor,
		 NhlTInteger,sizeof(int),Oset(constf_lbl.back_color),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlBACKGROUND),0,NULL},
	{NhlNcnConstFLabelPerim,NhlCcnConstFLabelPerim,NhlTInteger,
		 sizeof(int),Oset(constf_lbl.perim_on),
		 NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL},
	{NhlNcnConstFLabelPerimSpaceF,NhlCcnConstFLabelPerimSpaceF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.perim_space),
		 NhlTString,_NhlUSET("0.33"),0,NULL},
	{NhlNcnConstFLabelPerimColor,NhlCcnConstFLabelPerimColor,NhlTInteger,
		 sizeof(int),Oset(constf_lbl.perim_lcolor),
		 NhlTImmediate,_NhlUSET((NhlPointer) NhlFOREGROUND),0,NULL},
	{NhlNcnConstFLabelPerimThicknessF,NhlCcnConstFLabelFontThicknessF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl.perim_lthick),
		 NhlTString, _NhlUSET("1.0"),0,NULL},

	{NhlNcnConstFLabelZone,NhlCcnConstFLabelZone,NhlTInteger,
		 sizeof(int),Oset(constf_lbl_rec.zone),NhlTImmediate,
		 _NhlUSET((NhlPointer) 0),0,NULL},
	{NhlNcnConstFLabelSide,NhlCcnConstFLabelSide,NhlTPosition,
		 sizeof(NhlPosition),Oset(constf_lbl_rec.side),NhlTImmediate,
		 _NhlUSET((NhlPointer)NhlBOTTOM),0,NULL},
	{NhlNcnConstFLabelJust,NhlCcnConstFLabelJust,
		 NhlTJustification,sizeof(NhlJustification),
		 Oset(constf_lbl_rec.just),NhlTImmediate,
		 _NhlUSET((NhlPointer)NhlCENTERCENTER),0,NULL},
	{NhlNcnConstFLabelParallelPosF,NhlCcnConstFLabelParallelPosF,NhlTFloat,
		 sizeof(float),Oset(constf_lbl_rec.para_pos),NhlTString,
		 _NhlUSET("0.0"),0,NULL},
	{NhlNcnConstFLabelOrthogonalPosF,NhlCcnConstFLabelOrthogonalPosF,
		 NhlTFloat,sizeof(float),Oset(constf_lbl_rec.ortho_pos),
		 NhlTString,_NhlUSET("0.0"),0,NULL},

	{ NhlNtrXMinF,NhlCtrXMinF,NhlTFloat,sizeof(float),
		Oset(x_min),NhlTString,_NhlUSET("0.0"),0,NULL},
	{ NhlNtrXMaxF,NhlCtrXMaxF,NhlTFloat,sizeof(float),
		Oset(x_max),NhlTString,_NhlUSET("1.0"),0,NULL},
	{ NhlNtrXLog,NhlCtrXLog,NhlTBoolean,sizeof(NhlBoolean),
		Oset(x_log),NhlTImmediate,_NhlUSET((NhlPointer)False),0,NULL},
	{ NhlNtrXReverse,NhlCtrXReverse,NhlTBoolean,sizeof(NhlBoolean),
		Oset(x_reverse),
		  NhlTImmediate,_NhlUSET((NhlPointer)False),0,NULL},
	{ NhlNtrYMinF,NhlCtrYMinF,NhlTFloat,sizeof(float),
		Oset(y_min),NhlTString,_NhlUSET("0.0"),0,NULL},
	{ NhlNtrYMaxF,NhlCtrYMaxF,NhlTFloat,sizeof(float),
		Oset(y_max),NhlTString,_NhlUSET("1.0"),0,NULL},
	{ NhlNtrYLog,NhlCtrYLog,NhlTBoolean,sizeof(NhlBoolean),
		Oset(y_log),NhlTImmediate,_NhlUSET((NhlPointer)False),0,NULL},
	{ NhlNtrYReverse,NhlCtrYReverse,NhlTBoolean,sizeof(NhlBoolean),
		Oset(y_reverse),
		NhlTImmediate,_NhlUSET((NhlPointer)False),0,NULL},
	{ NhlNovDisplayLabelBar,NhlCovDisplayLabelBar,NhlTInteger,sizeof(int),
		  Oset(display_labelbar),
		  NhlTImmediate,_NhlUSET((NhlPointer) Nhl_ovNoCreate),0,NULL},
	{ NhlNovDisplayLegend,NhlCovDisplayLegend,NhlTInteger,sizeof(int),
		  Oset(display_legend),
		  NhlTImmediate,_NhlUSET((NhlPointer) Nhl_ovNoCreate),0,NULL},
	{NhlNlgLabelStrings, NhlClgLabelStrings, NhlT1DStringGenArray,
		 sizeof(NhlPointer),Oset(legend_labels),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNlgTitleString, NhlClgTitleString, NhlTString,
		 sizeof(NhlPointer),Oset(legend_title),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFree},
	{NhlNlbLabelStrings, NhlClbLabelStrings, NhlT1DStringGenArray,
		 sizeof(NhlPointer),Oset(labelbar_labels),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFreeGenArray},
	{NhlNlbTitleString, NhlClbTitleString, NhlTString,
		 sizeof(NhlPointer),Oset(labelbar_title),
		 NhlTImmediate,_NhlUSET((NhlPointer) NULL),0,
		 (NhlFreeFunc)NhlFree},
	{ NhlNovUpdateReq,NhlCovUpdateReq,NhlTInteger,sizeof(int),
		  Oset(update_req),
		  NhlTImmediate,_NhlUSET((NhlPointer) False),0,NULL},
	{ NhlNcnDataChanged,NhlCcnDataChanged,NhlTBoolean,sizeof(NhlBoolean),
		  Oset(data_changed),
		  NhlTImmediate,_NhlUSET((NhlPointer) True),0,NULL}
};
#undef Oset

typedef enum _cnCoord { cnXCOORD, cnYCOORD} cnCoord;
typedef enum _cnValueType { 
	cnCONSTFVAL, 
	cnCONINTERVAL,
	cnCONMINVAL,
	cnCONMAXVAL,
	cnDATAMINVAL,
	cnDATAMAXVAL,
	cnSCALEFACTOR } cnValueType;
	

/* base methods */

static NhlErrorTypes ContourClassInitialize(
#ifdef NhlNeedProto
	void
#endif
);

static NhlErrorTypes ContourClassPartInitialize(
#ifdef NhlNeedProto
	NhlLayerClass	lc
#endif
);

static NhlErrorTypes ContourInitialize(
#ifdef NhlNeedProto
        NhlLayerClass,  /* class */
        NhlLayer,       /* req */
        NhlLayer,       /* new */
        _NhlArgList,    /* args */
        int             /* num_args */
#endif
);

static NhlErrorTypes ContourSetValues(
#ifdef NhlNeedProto
        NhlLayer,       /* old */
        NhlLayer,       /* reference */
        NhlLayer,       /* new */
        _NhlArgList,    /* args */
        int             /* num_args*/
#endif
);


static NhlErrorTypes    ContourGetValues(
#ifdef NhlNeedProto
	NhlLayer,       /* l */
	_NhlArgList,    /* args */
	int             /* num_args */
#endif
);

static NhlErrorTypes ContourDestroy(
#ifdef NhlNeedProto
        NhlLayer        /* inst */
#endif
);

static NhlErrorTypes ContourDraw(
#ifdef NhlNeedProto
        NhlLayer	/* layer */
#endif
);

static NhlErrorTypes ContourPostDraw(
#ifdef NhlNeedProto
        NhlLayer	/* layer */
#endif
);


static NhlErrorTypes ContourUpdateData(
#ifdef NhlNeedProto
	NhlDataCommLayer	new,
	NhlDataCommLayer	old
#endif
);


static NhlErrorTypes ContourDataClassInitialize(
#ifdef NhlNeedProto
	void
#endif
);

static NhlErrorTypes ContourDataInitialize(
#ifdef NhlNeedProto
        NhlLayerClass	class,
        NhlLayer	req,
        NhlLayer	new,
        _NhlArgList	args,
        int             num_args
#endif
);

/* internal static functions */

static NhlErrorTypes AddDataBoundToAreamap(
#ifdef NhlNeedProto
	NhlContourLayer	cl,
	NhlString	entry_name
#endif
);

static NhlErrorTypes QualifyCoordBounds(
#ifdef NhlNeedProto
        NhlContourLayerPart	*cnp,
	char			*entry_name
#endif
);

static NhlErrorTypes SetUpLLTransObj(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init
#endif
);

static NhlErrorTypes SetIrrCoordBounds(
#ifdef NhlNeedProto
	NhlContourLayerPart	*cnp,
	cnCoord			ctype,
	int			count,
	NhlString		entry_name
#endif
);

static NhlErrorTypes SetUpIrrTransObj(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init
#endif
);


static NhlErrorTypes ManageLegend(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
#endif
);

static NhlErrorTypes ManageLabelBar(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
#endif
);

static NhlErrorTypes ManageInfoLabel(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
#endif
);

static NhlErrorTypes ManageConstFLabel(
#ifdef NhlNeedProto
	NhlContourLayer	cnew,
	NhlContourLayer	cold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
#endif
);

static NhlErrorTypes ManageAnnotation
#ifdef NhlNeedProto
(
	NhlContourLayer		cnnew,
	NhlContourLayerPart	*ocnp,
	NhlBoolean		init,
	_cnAnnoType		atype
#endif
);

static NhlErrorTypes SetTextPosition
#ifdef NhlNeedProto
(
	NhlContourLayer		cnnew,
	NhlContourLayerPart	*ocnp,
	_cnAnnoType		atype,
	NhlBoolean		*pos_changed,
	NhlString		entry_name
#endif
);

static NhlErrorTypes ReplaceSubstitutionChars
#ifdef NhlNeedProto
(
	NhlContourLayerPart	*cnp,
	NhlContourLayerPart	*ocnp,
	NhlBoolean		init,
	_cnAnnoType		atype,
	NhlBoolean		*text_changed,
	NhlString		entry_name
#endif
);

static void Substitute(
#ifdef NhlNeedProto
	char		*buf,
	int		replace_count,
	char		*subst
#endif
);


static char *ContourFormat(
#ifdef NhlNeedProto
	NhlContourLayerPart	*cnp,
	cnValueType		vtype
#endif
);

static NhlErrorTypes    SetupLevels(
#ifdef NhlNeedProto
	NhlLayer	new,
	NhlLayer	old,
        NhlBoolean	init,					    
	float		**levels,
	NhlBoolean	*modified				    
#endif
);

static NhlErrorTypes    SetupLevelsManual(
#ifdef NhlNeedProto
	NhlContourLayer	cnew, 
	NhlContourLayer	cold,
	float		**levels,
	char		*entry_name
#endif
);

static NhlErrorTypes    SetupLevelsEqual(
#ifdef NhlNeedProto
	NhlContourLayer	cnew, 
	NhlContourLayer	cold,
	float		**levels,
	char		*entry_name
#endif
);

static NhlErrorTypes    SetupLevelsAutomatic(
#ifdef NhlNeedProto
	NhlContourLayer	cnew, 
	NhlContourLayer	cold,
	float		**levels,
	char		*entry_name
#endif
);

static NhlErrorTypes    SetupLevelsExplicit(
#ifdef NhlNeedProto
	NhlContourLayer	cnew, 
	NhlContourLayer	cold,
	float		**levels,
	char		*entry_name
#endif
);

static NhlErrorTypes ChooseSpacingLin(
#ifdef NhlNeedProto
	float		*tstart,
	float		*tend,
	float		*spacing,
	int		convert_precision,
	int		max_ticks,
	NhlString	entry_name
#endif
);

static NhlErrorTypes    ManageData(
#ifdef NhlNeedProto
	NhlContourLayer	cnnew, 
	NhlContourLayer	cnold,
	NhlBoolean	init,
	_NhlArgList	args,
	int		num_args
#endif
);

static NhlErrorTypes    ManageViewDepResources(
#ifdef NhlNeedProto
	NhlLayer	new,
	NhlLayer	old,
        NhlBoolean	init,					    
	_NhlArgList	args,
	int		num_args
#endif
);

static NhlErrorTypes    AdjustText(
#ifdef NhlNeedProto
	NhlcnLabelAttrs *lbl_attrp,
	NhlContourLayer	new, 
	NhlContourLayer	old,
	NhlBoolean	init
#endif
);

static NhlErrorTypes    ManageDynamicArrays(
#ifdef NhlNeedProto
	NhlLayer	new,
	NhlLayer	old,
        NhlBoolean	init,					    
	_NhlArgList	args,
	int		num_args
#endif
);

static NhlErrorTypes    ManageGenArray(
#ifdef NhlNeedProto
	NhlGenArray	*ga,
	int		count,
	NhlGenArray	copy_ga,
	NrmQuark	type,
	NhlPointer	init_val,
	int		*old_count,
	int		*init_count,
	NhlBoolean	*need_check,
	NhlBoolean	*changed,				       
	NhlString	resource_name,
	NhlString	entry_name
#endif
);

static NhlErrorTypes	CheckColorArray(
#ifdef NhlNeedProto
	NhlContourLayer	cl,
	NhlGenArray	ga,
	int		count,
	int		init_count,
	int		last_count,
	int		**gks_colors,
	NhlString	resource_name,
	NhlString	entry_name
#endif
);

static NhlGenArray GenArraySubsetCopy(
#ifdef NhlNeedProto
        NhlGenArray     ga,
        int             length
#endif
);

static NhlErrorTypes GetData(
#ifdef NhlNeedProto
	NhlContourLayer	cl,
	float		**scalar_field,
	int		*first_dim,
	int		*second_dim
#endif
);

static NhlErrorTypes UpdateLineAndLabelParams(
#ifdef NhlNeedProto
	NhlContourLayer cl,
	NhlBoolean	*do_lines,
	NhlBoolean	*do_fill
#endif
);

static NhlErrorTypes UpdateFillInfo(
#ifdef NhlNeedProto
	NhlContourLayer cl,
	NhlBoolean	*do_fill
#endif
);

extern int (_NHLCALLF(cpdrpl,CPDRPL))(
#ifdef NhlNeedProto
	float *xcs, 
	float *ycs,
	int *ncs,
	int *iai,
	int *iag,
	int *nai
#endif
);

extern int (_NHLCALLF(nhlfll,NHLFLL))(
#ifdef NhlNeedProto
	float *xcs, 
	float *ycs, 
	int *ncs, 
	int *iai, 
	int *iag, 
	int *nai
#endif
);

extern void   (_NHLCALLF(cpchcl,CPCHCL))(
#ifdef NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(cpchhl,CPCHHL))(
#ifdef NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(cpchll,CPCHLL))(
#ifdef NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(cpmpxy,CPMPXY))(
#ifdef NhlNeedProto
	int	*imap,
	float	*xinp,
	float	*yinp,
	float	*xotp,
	float	*yotp
#endif
);


NhlContourDataDepLayerClassRec NhlcontourDataDepLayerClassRec = {
	/* base_class */
        {
/* class_name			*/	"ContourDataDep",
/* nrm_class			*/	NrmNULLQUARK,
/* layer_size			*/	sizeof(NhlContourDataDepLayerRec),
/* class_inited			*/	False,
/* superclass			*/	(NhlLayerClass)
						&NhldataSpecLayerClassRec,

/* layer_resources		*/	data_resources,
/* num_resources		*/	NhlNumber(data_resources),
/* all_resources		*/	NULL,

/* class_part_initialize	*/	NULL,
/* class_initialize		*/	ContourDataClassInitialize,
/* layer_initialize		*/	ContourDataInitialize,
/* layer_set_values		*/	NULL,
/* layer_set_values_hook	*/	NULL,
/* layer_get_values		*/	NULL,
/* layer_reparent		*/	NULL,
/* layer_destroy		*/	NULL
	},
	/* dataspec_class */
	{
/* foo				*/	0
	},
	/* contour datadep_class */
	{
/* foo				*/	0
	}
};

NhlContourLayerClassRec NhlcontourLayerClassRec = {
	/* base_class */
        {
/* class_name			*/      "Contour",
/* nrm_class			*/      NrmNULLQUARK,
/* layer_size			*/      sizeof(NhlContourLayerRec),
/* class_inited			*/      False,
/* superclass			*/  (NhlLayerClass)&NhldataCommLayerClassRec,

/* layer_resources		*/	resources,
/* num_resources		*/	NhlNumber(resources),
/* all_resources		*/	NULL,

/* class_part_initialize	*/	ContourClassPartInitialize,
/* class_initialize		*/	ContourClassInitialize,
/* layer_initialize		*/	ContourInitialize,
/* layer_set_values		*/	ContourSetValues,
/* layer_set_values_hook	*/	NULL,
/* layer_get_values		*/	ContourGetValues,
/* layer_reparent		*/	NULL,
/* layer_destroy		*/	ContourDestroy,

/* child_resources		*/	NULL,

/* layer_draw			*/      ContourDraw,

/* layer_pre_draw		*/      NULL,
/* layer_draw_segonly		*/	NULL,
/* layer_post_draw		*/      ContourPostDraw,
/* layer_clear			*/      NULL

        },
	/* view_class */
	{
/* segment_wkid			*/	0,
/* get_bb			*/	NULL
	},
	/* trans_class */
	{
/* overlay_capability 		*/	_tfOverlayBaseOrMember,
/* data_to_ndc			*/	NULL,
/* ndc_to_data			*/	NULL,
/* data_polyline		*/	NULL,
/* ndc_polyline			*/	NULL,
	},
	/* datacomm_class */
	{
/* data_offsets			*/	NULL,
/* update_data			*/	ContourUpdateData
	},
	{
/* foo				*/	NULL
	}
};
	

NhlLayerClass NhlcontourDataDepLayerClass =
		(NhlLayerClass) &NhlcontourDataDepLayerClassRec;
NhlLayerClass NhlcontourLayerClass = 
		(NhlLayerClass) &NhlcontourLayerClassRec;



static NrmQuark	Qfloat = NrmNULLQUARK;
static NrmQuark Qint = NrmNULLQUARK;
static NrmQuark Qstring = NrmNULLQUARK;
static NrmQuark	Qlevels = NrmNULLQUARK; 
static NrmQuark	Qlevel_flags = NrmNULLQUARK; 
static NrmQuark	Qfill_colors = NrmNULLQUARK;
static NrmQuark	Qfill_patterns = NrmNULLQUARK;
static NrmQuark	Qfill_scales = NrmNULLQUARK;
static NrmQuark	Qline_colors = NrmNULLQUARK; 
static NrmQuark	Qline_dash_patterns = NrmNULLQUARK; 
static NrmQuark	Qline_thicknesses = NrmNULLQUARK; 
static NrmQuark	Qllabel_strings = NrmNULLQUARK;
static NrmQuark	Qllabel_colors = NrmNULLQUARK; 

static NhlLayer		Trans_Obj;
static NhlBoolean	Do_Lines;
static NhlBoolean	Do_Labels;
static NhlWorkspace	*Fws,*Iws,*Aws;
static NhlBoolean	Use_Area_Ws;
static float		*Data;
static int		M,N;


#define NhlDASHBUFSIZE	128
static NhlString	*Dash_Table;
static int		Dash_Table_Len;
static int		*Dash_Patterns;
static int		Mono_Dash_Pattern;
static float		Dash_Seglen;
static float		View_Width;
static int		*Line_Colors;
static NhlBoolean	Mono_Line_Color;
static float		*Line_Thicknesses;
static NhlBoolean	Mono_Line_Thickness;
static NhlBoolean	Constant_Labels;

static NhlcnLabelAttrs *LLabel_AttrsP;
static NhlcnLabelAttrs *High_Label_AttrsP;
static NhlcnLabelAttrs *Low_Label_AttrsP;

static 	NhlBoolean	Color_Fill, Pattern_Fill;
static NhlBoolean	Mono_Fill_Color, Mono_Fill_Pattern, Mono_Fill_Scale;
static int		*Fill_Colors;
static int		*Fill_Patterns;
static float		*Fill_Scales;
static int		Fill_Count;
static NhlLayer		Wkptr;
static NhlLayer		Clayer;

/*
 * Function:	nhlfcontourclass
 *
 * Description:	fortran ref to contour class
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	global Fortran
 * Returns:	NhlLayerClass
 * Side Effect:	
 */
NhlLayerClass
_NHLCALLF(nhlfcontourclass,NHLFCONTOURCLASS)
#if	__STDC__
(
	void
)
#else
()
#endif
{
	return NhlcontourLayerClass;
}

/*
 * Function:	ContourDataClassInitialize
 *
 * Description:	init quark for latter use.
 *
 * In Args:	
 *		void
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
static NhlErrorTypes
ContourDataClassInitialize
#if	__STDC__
(
	void
)
#else
()
#endif
{
	Qint = NrmStringToQuark(NhlTInteger);
	Qstring = NrmStringToQuark(NhlTString);

	return NhlNOERROR;
}

/*
 * Function:	ContourDataInitialize
 *
 * Description:	Initializes the ContourData Dependent class instance.
 *
 * In Args:	
 *		NhlLayerClass	class,
 *		NhlLayer		req,
 *		NhlLayer		new,
 *		_NhlArgList	args,
 *		int		num_args
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
/*ARGSUSED*/
static NhlErrorTypes
ContourDataInitialize
#if     __STDC__
(
	NhlLayerClass	class,
	NhlLayer		req,
	NhlLayer		new,
	_NhlArgList	args,
	int		num_args
)
#else
(class,req,new,args,num_args)
        NhlLayerClass      class;
        NhlLayer           req;
        NhlLayer           new;
        _NhlArgList     args;
        int             num_args;
#endif
{
	NhlErrorTypes	ret = NhlNOERROR;

	return ret;
}


/*
 * Function:	ContourUpdateData
 *
 * Description:	This function is called whenever the data pointed to by the
 *		data resources change.  This function needs to check if
 *		this specific data resource changed - it may have been another
 *		data resource in a sub/super class.
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
static NhlErrorTypes
ContourUpdateData
#if	__STDC__
(
	NhlDataCommLayer	new,
	NhlDataCommLayer	old
)
#else
(new,old)
	NhlDataCommLayer	new;
	NhlDataCommLayer	old;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;

/*
 * For now simply call SetValues setting the data changed resource true
 */
	NhlVASetValues(new->base.id,NhlNcnDataChanged,True,
		       NULL);

	return ret;
}

/*
 * Function:	ContourClassInitialize
 *
 * Description:
 *
 * In Args:	NONE
 *
 * Out Args:	NONE
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:	NONE
 */
static NhlErrorTypes
ContourClassInitialize
#if __STDC__
(
	void
)
#else
()
#endif
{
	Qint = NrmStringToQuark(NhlTInteger);
	Qstring = NrmStringToQuark(NhlTString);
	Qfloat = NrmStringToQuark(NhlTFloat);
	Qlevels = NrmStringToQuark(NhlNcnLevels);
	Qlevel_flags = NrmStringToQuark(NhlNcnLevelFlags);
	Qfill_colors = NrmStringToQuark(NhlNcnFillColors);
	Qfill_patterns = NrmStringToQuark(NhlNcnFillPatterns);
	Qfill_scales = NrmStringToQuark(NhlNcnFillScales);
	Qline_colors = NrmStringToQuark(NhlNcnLineColors);
	Qline_dash_patterns = NrmStringToQuark(NhlNcnLineDashPatterns);
	Qline_thicknesses = NrmStringToQuark(NhlNcnLineThicknesses);
	Qllabel_strings = NrmStringToQuark(NhlNcnLineLabelStrings);
	Qllabel_colors = NrmStringToQuark(NhlNcnLineLabelColors);

	return NhlNOERROR;
}

/*
 * Function:	ContourClassPartInitialize
 *
 * Description:	This function initializes fields in the 
 *		NhlContourLayerClassPart that cannot be initialized statically.
 *		Calls _NhlRegisterChildClass for the overlay manager object.
 *
 * In Args:	
 *		NhlLayerClass	lc	NhlLayer Class to init
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */

static NhlErrorTypes
ContourClassPartInitialize
#if	__STDC__
(
	NhlLayerClass	lc	/* Layer Class to init	*/
)
#else
(lc)
	NhlLayerClass	lc;	/* Layer Class to init	*/
#endif
{
	NhlErrorTypes	ret = NhlNOERROR, subret = NhlNOERROR;
	char		*e_text;
	char		*entry_name = "ContourClassPartInitialize";

/*
 * Register children objects
 */
	subret = _NhlRegisterChildClass(lc,NhloverlayLayerClass,
					False,False,
					NhlNovDisplayLegend,
					NhlNlgItemCount,
					NhlNlgTitleString,
					NhlNlgItemIndexes,
					NhlNlgLabelStrings,
					NhlNlgMonoItemColor,
					NhlNlgItemColors,
					NhlNlgMonoItemThickness,
					NhlNlgItemThicknesses,
					NhlNlgItemStrings,
					NhlNlgMonoItemStringColor,
					NhlNlgItemStringColors,
					NhlNlgMonoItemTextHeight,
					NhlNlgItemTextHeights,
					NhlNovDisplayLabelBar,
					NhlNlbBoxCount,
					NhlNlbTitleString,
					NhlNlbLabelStrings,
					NhlNlbMonoFillColor,
					NhlNlbFillColors,
					NhlNlbMonoFillPattern,
					NhlNlbFillPatterns,
					NhlNlbMonoFillScale,
					NhlNlbFillScales,
					NULL);

	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error registering %s";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
			  "NhloverlayLayerClass");
		return(NhlFATAL);
	}

	subret = _NhlRegisterDataRes((NhlDataCommLayerClass)lc,
				     NhlNcnScalarFieldData,
				     NhlcontourDataDepLayerClass,
				     NhlscalarFieldFloatLayerClass,
				     NULL);

	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error registering data resource %s";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
			  "NhlcoordArrTableFloatLayerClass");
		return(NhlFATAL);
	}

	return ret;
}


/*
 * Function:	ContourInitialize
 *
 * Description: 
 *
 * In Args: 	class	objects layer_class
 *		req	instance record of requested values
 *		new	instance record of new object
 *		args	list of resources and values for reference
 *		num_args 	number of elements in args.
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	state change in GKS due to mapping transformations.
 */
/*ARGSUSED*/
static NhlErrorTypes
ContourInitialize
#if     __STDC__
(
	NhlLayerClass	class,
	NhlLayer		req,
	NhlLayer		new,
	_NhlArgList	args,
	int		num_args
)
#else
(class,req,new,args,num_args)
        NhlLayerClass      class;
        NhlLayer           req;
        NhlLayer           new;
        _NhlArgList     args;
        int             num_args;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*entry_name = "ContourInitialize";
	char			*e_text;
	NhlContourLayer		cnew = (NhlContourLayer) new;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	NhlSArg			sargs[64];
	int			nargs = 0;

/* Initialize Contour float and integer workspaces */

	if ((cnp->iws_id =_NhlNewWorkspace(NhlTConpackInt,NhlwsNONE,
					   4000*sizeof(int))) < 0) {
		e_text = "%s: integer workspace allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}
	if ((cnp->fws_id = _NhlNewWorkspace(NhlTConpackFloat,NhlwsNONE,
					    4000*sizeof(float))) < 0) {
		e_text = "%s: float workspace allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}
	cnp->label_aws_id = -1;
	cnp->fill_aws_id = -1;
	cnp->ezmap_aws_id = -1;
	cnp->info_anno_id = -1;
	cnp->constf_anno_id = -1;
	cnp->info_lbl_rec.id = -1;
	cnp->constf_lbl_rec.id = -1;

/* Initialize unset resources */
	
	if (! cnp->min_level_set) cnp->min_level_val = BIGNUMBER;
	if (! cnp->max_level_set) cnp->max_level_val = LITTLENUMBER;
	if (! cnp->llabel_interval_set) cnp->llabel_interval = 2;
	if (! cnp->line_dash_seglen_set) 
		cnp->line_dash_seglen = 0.15;

	if (! cnp->line_lbls.height_set) 
		cnp->line_lbls.height = 0.011;
	if (! cnp->high_lbls.height_set) 
		cnp->high_lbls.height = 0.013;
	if (! cnp->low_lbls.height_set) 
		cnp->low_lbls.height = 0.013;
	if (! cnp->info_lbl.height_set) 
		cnp->info_lbl.height = 0.013;
	if (! cnp->constf_lbl.height_set) 
		cnp->constf_lbl.height = 0.013;

/* Initialize private members */

	cnp->line_lbls.fcode[1] = '\0';
	cnp->info_lbl.fcode[1] = '\0';
	cnp->high_lbls.fcode[1] = '\0';
	cnp->low_lbls.fcode[1] = '\0';
	cnp->constf_lbl.fcode[1] = '\0';
	cnp->line_lbls.text = NULL;
	cnp->info_lbl.text = NULL;
	cnp->high_lbls.text = NULL;
	cnp->low_lbls.text = NULL;
	cnp->constf_lbl.text = NULL;
	cnp->label_amap = NULL;
	cnp->new_draw_req = True;
	cnp->trans_dat = NULL;
	cnp->update_req = False;
	cnp->overlay_object = NULL;
	cnp->data_changed = True;
	cnp->ll_text_heights = NULL;
	cnp->ll_strings = NULL;
	cnp->use_irr_trans = False;
	cnp->const_field = False;

	subret = QualifyCoordBounds(cnp,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING) return(ret);

	if (! cnp->x_reverse) {
		cnp->xc1 = cnp->x_min;
		cnp->xcm = cnp->x_max;
	}
	else {
		cnp->xc1 = cnp->x_max;
		cnp->xcm = cnp->x_min;
	}
	if (! cnp->y_reverse) {
		cnp->yc1 = cnp->y_min;
		cnp->ycn = cnp->y_max;
	}
	else {
		cnp->yc1 = cnp->y_max;
		cnp->ycn = cnp->y_min;
	}

	subret = ManageData(cnew,(NhlContourLayer) req,True,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error setting view dependent resources";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}


/* Manage constant field label */

	subret = ManageConstFLabel(cnew,
				   (NhlContourLayer)req,True,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing constant field label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Set view dependent resources */

	subret = ManageViewDepResources(new,req,True,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error setting view dependent resources";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Set up the dynamic arrays */

	subret = ManageDynamicArrays(new,req,True,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error initializing dynamic arrays";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Set up the contour object transformation  */

	if (cnp->use_irr_trans) {
		subret = SetUpIrrTransObj(cnew,(NhlContourLayer) req,True);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: error setting up transformation";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return(ret);
		}
	}
	else {
		subret = SetUpLLTransObj(cnew,(NhlContourLayer) req,True);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: error setting up transformation";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return(ret);
		}
	}

/* Manage Legend object */

	/* adds 18 arguments */
	subret = ManageLegend(cnew,(NhlContourLayer) req,True,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing Legend";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage LabelBar object */

	subret = ManageLabelBar(cnew,(NhlContourLayer) req,True,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing LabelBar";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage info label */

	subret = ManageInfoLabel(cnew,(NhlContourLayer)req,True,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing information label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage the overlay */

	subret = _NhlManageOverlay(&cnp->overlay_object,new,req,
			       True,sargs,nargs,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING) 
		return ret;

	cnp->data_changed = False;
	cnp->line_dash_seglen_set = False;
	cnp->llabel_interval_set = False;
	cnp->line_lbls.height_set = False;
	cnp->high_lbls.height_set = False;
	cnp->low_lbls.height_set = False;
	cnp->info_lbl.height_set = False;
	cnp->constf_lbl.height_set = False;

	if (cnp->data_init) {
		cnp->min_level_set = False;
		cnp->max_level_set = False;
	}

	return ret;
}

/*
 * Function:	ContourSetValues
 *
 * Description: 
 *
 * In Args:	old	copy of old instance record
 *		reference	requested instance record
 *		new	new instance record	
 *		args 	list of resources and values for reference
 *		num_args	number of elements in args.
 *
 * Out Args:	NONE
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:
 */
/*ARGSUSED*/
static NhlErrorTypes ContourSetValues
#if  __STDC__
(
	NhlLayer	old,
	NhlLayer	reference,
	NhlLayer	new,
	_NhlArgList	args,
	int		num_args
)
#else
(old,reference,new,args,num_args)
	NhlLayer	old;
	NhlLayer	reference;
	NhlLayer	new;
	_NhlArgList	args;
	int		num_args;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*entry_name = "ContourSetValues";
	char			*e_text;
	NhlContourLayer		cnew = (NhlContourLayer) new;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	NhlContourLayer		cold = (NhlContourLayer) old;
	/* Note that both ManageLegend and ManageLabelBar add to sargs */
	NhlSArg			sargs[128];
	int			nargs = 0;

	if (cnew->view.use_segments != cold->view.use_segments) {
		cnew->view.use_segments = cold->view.use_segments;
		ret = MIN(ret,NhlWARNING);
		e_text = "%s: attempt to set create-only resource overridden";
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
	}

	if (_NhlArgIsSet(args,num_args,NhlNcnMinLevelValF))
		cnp->min_level_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnMaxLevelValF))
		cnp->max_level_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnLineLabelInterval))
		cnp->llabel_interval_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnLineDashSegLenF))
		cnp->line_dash_seglen_set = True;

	if (_NhlArgIsSet(args,num_args,NhlNcnLineLabelTextHeightF))
		cnp->line_lbls.height_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnHighLabelTextHeightF))
		cnp->high_lbls.height_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnLowLabelTextHeightF))
		cnp->low_lbls.height_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnInfoLabelTextHeightF))
		cnp->info_lbl.height_set = True;
	if (_NhlArgIsSet(args,num_args,NhlNcnConstFLabelTextHeightF))
		cnp->constf_lbl.height_set = True;

	subret = QualifyCoordBounds(cnp,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING) return(ret);

/* Manage the data */

	subret = ManageData(cnew,cold,False,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error setting view dependent resources";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}


/* Set view dependent resources */

	subret = ManageViewDepResources(new,old,False,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error setting view dependent resources";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage constant field label */

	subret = ManageConstFLabel(cnew,cold,False,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing contour zero field label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage the dynamic arrays */

	subret = ManageDynamicArrays(new,old,False,args,num_args);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error initializing dynamic arrays";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}


/* Set up the contour object transformation  */

	if (cnp->use_irr_trans) {
		subret = SetUpIrrTransObj(cnew,(NhlContourLayer) old,False);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: error setting up transformation";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return(ret);
		}
	}
	else {
		subret = SetUpLLTransObj(cnew,(NhlContourLayer) old,False);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: error setting up transformation";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return(ret);
		}
	}

/* Manage Legend object */

	/* 18 arguments possible */
	subret = ManageLegend(cnew,cold,False,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing Legend";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage LabelBar object */

	subret = ManageLabelBar(cnew,cold,False,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing LabelBar";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}


/* Manage info label */

	subret = ManageInfoLabel(cnew,cold,False,sargs,&nargs);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error managing contour information label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Manage the overlay */

	/* 1 arg */
	if (cnp->update_req) {
		cnp->new_draw_req = True;
		NhlSetSArg(&sargs[nargs++],NhlNovUpdateReq,True);
	}
		
	subret = _NhlManageOverlay(&cnp->overlay_object,new,old,
				   False,sargs,nargs,entry_name);
	ret = MIN(ret,subret);

	cnp->update_req = False;
	cnp->data_changed = False;
	cnp->llabel_interval_set = False;
	cnp->line_dash_seglen_set = False;
	cnp->high_lbls.height_set = False;
	cnp->low_lbls.height_set = False;
	cnp->line_lbls.height_set = False;
	cnp->info_lbl.height_set = False;
	cnp->constf_lbl.height_set = False;

	if (cnp->data_init) {
		cnp->min_level_set = False;
		cnp->max_level_set = False;
	}

	return ret;
}

/*
 * Function:    ContourGetValues
 *
 * Description: Retrieves the current setting of one or more Legend resources.
 *      This routine only retrieves resources that require special methods
 *      that the generic GetValues method cannot handle. For now this means
 *      all the GenArray resources. Note that space is allocated; the user
 *      is responsible for freeing this space.
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects:
 *      Memory is allocated when any of the following resources are retrieved:
 *              NhlNcnLevels
 *              NhlNcnLevelFlags
 *              NhlNcnFillColors
 *              NhlNcnFillPatterns
 *              NhlNcnFillScales
 *              NhlNcnLineColors
 *              NhlNcnLineDashPatterns
 *              NhlNcnLineThicknesses
 *		NhlNcnLineLabelStrings
 *		NhlNcnLineLabelColors
 *      The caller is responsible for freeing this memory.
 */

static NhlErrorTypes    ContourGetValues
#if __STDC__
(NhlLayer l, _NhlArgList args, int num_args)
#else
(l,args,num_args)
        NhlLayer        l;
        _NhlArgList     args;
        int     	num_args;
#endif
{
        NhlContourLayer cl = (NhlContourLayer)l;
        NhlContourLayerPart *cnp = &(cl->contour);
        NhlGenArray ga;
        char *e_text;
        int i, count = 0;
        char *type = "";

        for( i = 0; i< num_args; i++ ) {

                ga = NULL;
                if(args[i].quark == Qlevels) {
                        ga = cnp->levels;
                        count = cnp->level_count;
                        type = NhlNcnLevels;
                }
                else if (args[i].quark == Qlevel_flags) {
                        ga = cnp->level_flags;
                        count = cnp->mono_level_flag ? 1 : cnp->level_count;
                        type = NhlNcnLevelFlags;
                }
                else if (args[i].quark == Qfill_colors) {
                        ga = cnp->fill_colors;
                        count = cnp->mono_fill_color ? 1 : cnp->level_count;
                        type = NhlNcnFillColors;
                }
                else if (args[i].quark == Qfill_patterns) {
                        ga = cnp->fill_patterns;
                        count = cnp->mono_fill_pattern ? 1 : cnp->level_count;
                        type = NhlNcnFillPatterns;
                }
                else if (args[i].quark == Qfill_scales) {
                        ga = cnp->fill_scales;
                        count = cnp->mono_fill_scale ? 1 : cnp->level_count;
                        type = NhlNcnFillScales;
                }
                else if (args[i].quark == Qline_colors) {
                        ga = cnp->line_colors;
                        count = cnp->mono_line_color ? 1 : cnp->level_count;
                        type = NhlNcnLineColors;
                }
                else if (args[i].quark == Qline_dash_patterns) {
                        ga = cnp->line_dash_patterns;
                        count = cnp->mono_line_dash_pattern ? 
				1 : cnp->level_count;
                        type = NhlNcnLineDashPatterns;
                }
                else if (args[i].quark == Qline_thicknesses) {
                        ga = cnp->line_thicknesses;
                        count = cnp->mono_line_thickness ? 
				1 : cnp->level_count;
                        type = NhlNcnLineThicknesses;
                }
                else if (args[i].quark == Qllabel_strings) {
                        ga = cnp->llabel_strings;
                        count = cnp->level_count;
                        type = NhlNcnLineLabelStrings;
                }
                else if (args[i].quark == Qllabel_colors) {
                        ga = cnp->llabel_colors;
                        count = cnp->line_lbls.mono_color ? 
				1 : cnp->level_count;
                        type = NhlNcnLineLabelColors;
                }
                if (ga != NULL) {
                        if ((ga = GenArraySubsetCopy(ga, count)) == NULL) {
                                e_text = "%s: error copying %s GenArray";
                                NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,
                                          "ContourGetValues",type);
                                return NhlFATAL;
                        }
                        *((NhlGenArray *)(args[i].value.ptrval)) = ga;
                }
        }

        return(NhlNOERROR);

}


/*
 * Function:	QualifyCoordBounds
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:	NONE
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:
 */
/*ARGSUSED*/
static NhlErrorTypes QualifyCoordBounds
#if  __STDC__
(
        NhlContourLayerPart	*cnp,
	char			*entry_name
)
#else
(cnp,entry_name)
        NhlContourLayerPart	*cnp;
	char			*entry_name;
#endif
{
	NhlErrorTypes	ret = NhlNOERROR;
	char		*e_text;
	float		ftmp;

	if (cnp->x_min == cnp->x_max) {
		e_text = "%s: Zero X coordinate span: defaulting";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
		cnp->x_min = 0.0; 
		cnp->x_max = 1.0;
	}
	else if (cnp->x_min > cnp->x_max) {
		e_text = "%s: Min X coordinate exceeds max: exchanging";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
		ftmp = cnp->x_min;
		cnp->x_min = cnp->x_max;
		cnp->x_max = ftmp;
	}
	if (cnp->x_log && cnp->x_min <= 0.0) {
		e_text = 
		    "%s: Log style invalid for X coordinates: setting %s off";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name,NhlNtrXLog);
		cnp->x_log = False;
	}

	if (cnp->y_min == cnp->y_max) {
		e_text = "%s: Zero Y coordinate span: defaulting";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
		cnp->y_min = 0.0; 
		cnp->y_max = 1.0;
	}
	else if (cnp->y_min > cnp->y_max) {
		e_text = "%s: Min Y coordinate exceeds max: exchanging";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
		ftmp = cnp->y_min;
		cnp->y_min = cnp->y_max;
		cnp->y_max = ftmp;
	}
	if (cnp->y_log && cnp->y_min <= 0.0) {
		e_text = 
		    "%s: Log style invalid for Y coordinates: setting %s off";
		ret = MIN(ret,NhlWARNING);
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name,NhlNtrYLog);
		cnp->y_log = False;
	}
	return ret;
}

/*
 * Function:  GenArraySubsetCopy
 *
 * Description: Since the internal GenArrays maintained by the Contour object
 *      may be bigger than the size currently in use, this function allows
 *      a copy of only a portion of the array to be created. This is for
 *      use by the GetValues routine when returning GenArray resources to
 *      the user level. The array is assumed to be valid. The only pointer
 *      type arrays that the routine can handle are NhlString arrays.
 *      Note: this might be another candidate for inclusion as a global
 *      routine.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects:
 */

static NhlGenArray GenArraySubsetCopy
#if __STDC__
        (NhlGenArray    ga,
        int             length)
#else
(ga,length)
        NhlGenArray     ga;
        int             length;
#endif
{
        NhlGenArray gto;

        if (length > ga->num_elements)
                return NULL;

        if ((gto = _NhlCopyGenArray(ga,False)) == NULL) {
                return NULL;
        }
        if ((gto->data = (NhlPointer) NhlMalloc(length * ga->size)) == NULL) {
                return NULL;
        }
        if (ga->typeQ != Qstring) {
                memcpy((void *)gto->data, (Const void *) ga->data,
                       length * ga->size);
        }
        else {
                NhlString *cfrom = (NhlString *) ga->data;
                NhlString *cto = (NhlString *) gto->data;
                int i;
                for (i=0; i<length; i++) {
                        if ((*cto = (char *)
                             NhlMalloc(strlen(*cfrom)+1)) == NULL) {
                                return NULL;
                        }
                        strcpy(*cto++,*cfrom++);
                }
        }
        gto->num_elements = length;
        return gto;
}


/*
 * Function:	ContourDestroy
 *
 * Description:
 *
 * In Args:	inst		instance record pointer
 *
 * Out Args:	NONE
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:	NONE
 */
static NhlErrorTypes ContourDestroy
#if __STDC__
(NhlLayer inst)
#else
(inst)
NhlLayer inst;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	NhlContourLayerPart	*cnp = &(((NhlContourLayer) inst)->contour);
	NhlTransformLayerPart	*cntp = &(((NhlTransformLayer) inst)->trans);
	int			ovbase_id;

	ovbase_id = cntp->overlay_object->base.parent->base.id;
	if (cntp->overlay_status == _tfCurrentOverlayMember ||
	    cntp->overlay_status == _tfCurrentOverlayBase) {
		if (cnp->info_anno_id != -1) {
			subret = NhlUnregisterAnnotation(ovbase_id,
							 cnp->info_anno_id);
			if ((ret = MIN(subret,ret)) < NhlWARNING)
				return NhlFATAL;
		}
		if (cnp->constf_anno_id != -1) {
			subret = NhlUnregisterAnnotation(ovbase_id,
							 cnp->constf_anno_id);
			if ((ret = MIN(subret,ret)) < NhlWARNING)
				return NhlFATAL;
		}
	}
	if (cntp->overlay_status == _tfCurrentOverlayMember) {
		subret = NhlRemoveFromOverlay(ovbase_id,inst->base.id,False);
		if ((ret = MIN(subret,ret)) < NhlWARNING)
			return NhlFATAL;
	}

	if (cnp->overlay_object != NULL) {
		(void) _NhlDestroyChild(cnp->overlay_object->base.id,inst);
		cnp->overlay_object = NULL;
	}
	if (cntp->trans_obj != NULL) {
		(void) NhlDestroy(cntp->trans_obj->base.id);
		cntp->trans_obj = NULL;
	}
	if (cnp->info_anno_id != -1) {
		(void) NhlDestroy(cnp->info_anno_id);
	}
	if (cnp->constf_anno_id != -1) {
		(void) NhlDestroy(cnp->constf_anno_id);
	}
	if (cnp->info_lbl_rec.id != -1) {
 		(void) NhlDestroy(cnp->info_lbl_rec.id);
	}
	if (cnp->constf_lbl_rec.id != -1) {
 		(void) NhlDestroy(cnp->constf_lbl_rec.id);
	}

	NhlFreeGenArray(cnp->levels);
	NhlFreeGenArray(cnp->level_flags);
	NhlFreeGenArray(cnp->fill_colors);
	NhlFreeGenArray(cnp->fill_patterns);
	NhlFreeGenArray(cnp->fill_scales);
	NhlFreeGenArray(cnp->line_colors);
	NhlFreeGenArray(cnp->line_dash_patterns);
	NhlFreeGenArray(cnp->line_thicknesses);
	NhlFreeGenArray(cnp->llabel_strings);
	NhlFreeGenArray(cnp->llabel_colors);
	NhlFreeGenArray(cnp->legend_labels);
	if (cnp->ll_text_heights != NULL)
		NhlFreeGenArray(cnp->ll_text_heights);
	if (cnp->ll_strings != NULL) {
		NhlFree(cnp->ll_strings->data);
		NhlFreeGenArray(cnp->ll_strings);
	}
	NhlFree(cnp->gks_fill_colors);
	NhlFree(cnp->gks_line_colors);
	NhlFree(cnp->gks_llabel_colors);
	if (cnp->label_amap != NULL) NhlFree(cnp->label_amap);

	_NhlFreeWorkspace(cnp->fws_id);
	_NhlFreeWorkspace(cnp->iws_id);
	if (cnp->label_aws_id >= 0)
		_NhlFreeWorkspace(cnp->label_aws_id);
	if (cnp->fill_aws_id >= 0)
		_NhlFreeWorkspace(cnp->fill_aws_id);

	return(ret);
}

/*
 * Function:	ContourDraw
 *
 * Description:	
 *
 * In Args:	layer	Contour instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes ContourDraw
#if  __STDC__
(NhlLayer layer)
#else
(layer)
        NhlLayer layer;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	char			*entry_name = "ContourDraw";
	NhlContourLayer		cl = (NhlContourLayer) layer;
	NhlContourLayerPart	*cnp = &(cl->contour);
	NhlTransformLayerPart	*tfp = &(cl->trans);
	float			out_of_range_val;
	NhlBoolean		do_fill;

	if (cnp->const_field || ! cnp->data_init) return NhlNOERROR;

	if (cl->view.use_segments && ! cnp->new_draw_req) {
                subret = _NhlActivateWorkstation(cl->base.wkptr);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
                subret = _NhlDrawSegment(cnp->trans_dat,
				_NhlWorkstationId(cl->base.wkptr));
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
                subret = _NhlDeactivateWorkstation(cl->base.wkptr);
		return MIN(subret,ret);
		
	}
	cnp->new_draw_req = False;
	
	subret = GetData(cl,&Data,&N,&M);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	subret = _NhlActivateWorkstation(cl->base.wkptr);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	if (cl->view.use_segments) {
		if (cnp->trans_dat != NULL)
			_NhlDeleteViewSegment(layer, cnp->trans_dat);
		if ((cnp->trans_dat = _NhlNewViewSegment(layer)) == NULL) {
			e_text = "%s: error opening segment";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text, entry_name);
			return(ret);
		}
		_NhlStartSegment(cnp->trans_dat);
	}

	
/*
 * If the plot is an overlay member, use the overlay manager's trans object.
 */
	if (tfp->overlay_status == _tfCurrentOverlayMember && 
	    tfp->overlay_trans_obj != NULL) {
		Trans_Obj = tfp->overlay_trans_obj;
	}
	else {
		Trans_Obj = tfp->trans_obj;
		subret = _NhlSetTrans(tfp->trans_obj, layer);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: error setting transformation";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text, entry_name);
			return(ret);
 		}
	}

	NhlVAGetValues(Trans_Obj->base.id, 
		     NhlNtrOutOfRangeF, &out_of_range_val,
		     NULL);
        c_cpsetr("ORV",out_of_range_val);

	if (cnp->sfp->missing_value_set)
		c_cpsetr("SPV",cnp->sfp->missing_value);
	else
		c_cpsetr("SPV",0.0);

	c_cpsetr("XC1",cnp->xc1);
	c_cpsetr("XCM",cnp->xcm);
	c_cpsetr("YC1",cnp->yc1);
	c_cpsetr("YCN",cnp->ycn);
	c_cpseti("WSO", 3);
        c_cpseti("SET",0);
        c_cpseti("MAP",NhlcnMAPVAL);
	c_cpseti("PIC",1);

	gset_fill_colr_ind((Gint)_NhlGetGksCi(cl->base.wkptr,0));

	subret = UpdateLineAndLabelParams(cl, &Do_Lines,&Do_Labels);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	subret = UpdateFillInfo(cl, &do_fill);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

/* Retrieve workspace pointers */

	if ((Fws = _NhlUseWorkspace(cnp->fws_id)) == NULL) {
		e_text = "%s: error reserving float workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	if ((Iws = _NhlUseWorkspace(cnp->iws_id)) == NULL) {
		e_text = "%s: error reserving integer workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

/* Draw the contours */

	subret = _NhlCprect(Data,cnp->sfp->fast_dim,cnp->sfp->fast_len,
			    cnp->sfp->slow_len,Fws,Iws,entry_name);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;


		{ float flx,frx,fby,fuy,wlx,wrx,wby,wuy; int ll;
		  c_getset(&flx,&frx,&fby,&fuy,&wlx,&wrx,&wby,&wuy,&ll);
		  printf("getset - %f,%f,%f,%f,%f,%f,%f,%f\n",
			 flx,frx,fby,fuy,wlx,wrx,wby,wuy); 
	        }
	Use_Area_Ws = False;
	if (do_fill || (Do_Labels && cnp->label_masking)) {

		if (cnp->label_aws_id < 0) {
			cnp->label_aws_id = 
				_NhlNewWorkspace(NhlTLabelAreaMap,
						 NhlwsDISK,25000*sizeof(int));
			if (cnp->label_aws_id < 0) 
				return MIN(ret,cnp->label_aws_id);
		}
		if ((Aws = _NhlUseWorkspace(cnp->label_aws_id)) == NULL) {
			e_text = 
			      "%s: error reserving label area map workspace";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return(ret);
		}
		Use_Area_Ws = True;

		subret = _NhlArinam(Aws,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

		subret = AddDataBoundToAreamap(cl,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

		if (Do_Lines) {
			subret = _NhlCpclam(Data,Fws,Iws,Aws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
			_NhlDumpAreaMap(Aws,entry_name);
		}

		if (Do_Labels && cnp->label_masking) {
			subret = _NhlCplbam(Data,Fws,Iws,Aws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
		
	}

	if (do_fill) {
		
		subret = _NhlArscam(Aws,(_NHLCALLF(nhlfll,NHLFLL)),
				    entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}
	
	
	if (Do_Lines && ! cnp->delay_lines) {
		if (Do_Labels && cnp->label_masking) {
			subret = _NhlCpcldm(Data,Fws,Iws,Aws,
					    (_NHLCALLF(cpdrpl,CPDRPL)),
					    entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
		else {
			subret = _NhlCpcldr(Data,Fws,Iws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
	}
	
	if (Do_Labels && ! cnp->delay_labels) {	
		gset_fill_int_style(GSTYLE_SOLID);

		subret = _NhlCplbdr(Data,Fws,Iws,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}

	if (cl->view.use_segments) {
		_NhlEndSegment();
	}
        subret = _NhlDeactivateWorkstation(cl->base.wkptr);
	ret = MIN(subret,ret);

	return MIN(subret,ret);
}


/*
 * Function:	ContourPostDraw
 *
 * Description:	
 *
 * In Args:	layer	Contour instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes ContourPostDraw
#if  __STDC__
(NhlLayer layer)
#else
(layer)
        NhlLayer layer;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*entry_name = "ContourPostDraw";
	NhlContourLayer		cl = (NhlContourLayer) layer;
	NhlContourLayerPart	*cnp = &(cl->contour);
	NhlTransformLayerPart	*tfp = &(cl->trans);

	if (! cnp->data_init) return NhlNOERROR;

	if (cnp->display_constf) {
		if (tfp->overlay_status == _tfNotInOverlay) {
			subret = NhlDraw(cnp->constf_lbl_rec.id);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
		return ret;
	}

	subret = _NhlActivateWorkstation(cl->base.wkptr);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	
	if (Do_Lines && cnp->delay_lines) {
		if (Do_Labels && cnp->label_masking) {
			subret = _NhlCpcldm(Data,Fws,Iws,Aws,
					    (_NHLCALLF(cpdrpl,CPDRPL)),
					    entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
		else {
			subret = _NhlCpcldr(Data,Fws,Iws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
	}
	
	if (Do_Labels && cnp->delay_labels) {	
		gset_fill_int_style(GSTYLE_SOLID);

		subret = _NhlCplbdr(Data,Fws,Iws,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}

        subret = _NhlDeactivateWorkstation(cl->base.wkptr);
	ret = MIN(subret,ret);


	if (Use_Area_Ws) {
		subret = _NhlIdleWorkspace(Aws);
		ret = MIN(subret,ret);
	}
	subret = _NhlIdleWorkspace(Fws);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	subret = _NhlIdleWorkspace(Iws);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	if (tfp->overlay_status == _tfNotInOverlay) {
		if (cnp->info_lbl.on) {
			subret = NhlDraw(cnp->info_lbl_rec.id);
			if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
		}
	}

	return MIN(subret,ret);
}


/*
 * Function:	AddDataBoundToAreamap
 *
 * Description:	
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: 
 *		 
 */	

static NhlErrorTypes AddDataBoundToAreamap
#if  __STDC__
(
	NhlContourLayer	cl,
	NhlString	entry_name
)
#else
(cl,entry_name)
	NhlContourLayer	cl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	NhlContourLayerPart	*cnp = (NhlContourLayerPart *) &cl->contour;
	int			i;
	int			status;
	NhlBoolean		ezmap = False;
	int			xrev,yrev;
	float			xa[5],ya[5];

#define _cnMAPBOUNDINC	20

	if (! strcmp(Trans_Obj->base.layer_class->base_class.class_name,
		   "MapTransObj")) {
		ezmap = True;
	}

	gset_linewidth(4.0);
	gset_line_colr_ind(30);

	if (! ezmap) {
		float wlx,wby,wrx,wuy;

		_NhlDataToWin(Trans_Obj,(NhlLayer) cl,&cnp->xlb,&cnp->ylb,
			      1,&wlx,&wby,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}

		_NhlDataToWin(Trans_Obj,(NhlLayer) cl,&cnp->xub,&cnp->yub,
			      1,&wrx,&wuy,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}
		xa[0] = xa[1] = xa[4] = wlx;
		xa[2] = xa[3] = wrx;
		ya[0] = ya[3] = ya[4] = wuy;
		ya[1] = ya[2] = wby;
		xrev=wlx>wrx;
		yrev=wby>wuy;
		if (! (xrev || yrev) || (xrev && yrev)) 
			_NhlAredam(Aws,xa,ya,5,3,0,-1,entry_name);
		else
			_NhlAredam(Aws,xa,ya,5,3,-1,0,entry_name);
	}
	else {
		NhlBoolean	started = False;
		float		xinc,yinc; 
		int		j;

		xa[0] = xa[1] = xa[4] = cnp->xlb;
		xa[2] = xa[3] = cnp->xub;
		ya[0] = ya[3] = ya[4] = cnp->ylb;
		ya[1] = ya[2] = cnp->yub;

		for (i=0;  i < 4; i++) {
			xinc = (xa[i+1] - xa[i]) / _cnMAPBOUNDINC;
			yinc = (ya[i+1] - ya[i]) / _cnMAPBOUNDINC;
			if (! started) {
				_NhlMapita(Aws,ya[i],xa[i],
					   0,3,-1,0,entry_name);
				c_mapit(ya[i],xa[i],0);
				started = True;
			}
			for (j = 0; j < _cnMAPBOUNDINC + 1; j++) {
				_NhlMapita(Aws,ya[i]+j*yinc,xa[i]+j*xinc,
					   1,3,-1,0,entry_name);
				c_mapit(ya[i]+j*yinc,xa[i]+j*xinc,1);
			}
		}
		_NhlMapiqa(Aws,3,-1,0,entry_name);
		c_mapiq();
	}
	return NhlNOERROR;
}

/*
 * Function:	GetData
 *
 * Description:	
 *
 * In Args:	cl	ContourLayer instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: 
 *		 
 */	

static NhlErrorTypes GetData
#if  __STDC__
(
	NhlContourLayer	cl,
	float		**scalar_field,
	int		*first_dim,
	int		*second_dim
)
#else
(cl,scalar_field,first_dim,second_dim)
	NhlContourLayer	cl;
	float		**scalar_field;
	int		*first_dim;
	int		*second_dim;
#endif
{
	char			*e_text;
	char			*entry_name = "ContourDraw";
	NhlContourLayerPart	*cnp = &(cl->contour);
	NhlScalarFieldFloatLayer	sfl;
	NhlScalarFieldFloatLayerPart	*sfp;
	_NhlDataNodePtr			*dlist = NULL;
	NhlBoolean			new;

	if (_NhlGetDataInfo(cnp->scalar_field_data,&dlist) != 1) {
		e_text = "%s: internal error retrieving data info";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}

	sfl = (NhlScalarFieldFloatLayer) _NhlGetDataSet(dlist[0],&new);
	if (sfl == NULL) {
		e_text = "%s: internal error retrieving data set";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}
	sfp = (NhlScalarFieldFloatLayerPart *) &sfl->sfieldfloat;

	*scalar_field = &((float *) sfp->d_arr->data)[sfp->begin];

	return NhlNOERROR;
}
/*
 * Function:	UpdateLineAndLabelParams
 *
 * Description:	
 *
 * In Args:	layer	Contour instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: Updates various internal parameters in Conpack,Plotchar,
 *		 etc.
 *		 
 */	

static NhlErrorTypes UpdateLineAndLabelParams
#if  __STDC__
(
	NhlContourLayer	cl,
	NhlBoolean	*do_lines,
	NhlBoolean	*do_labels
)
#else
(cl,do_lines,do_labels)
        NhlContourLayer	cl;
	NhlBoolean	*do_lines;
	NhlBoolean	*do_labels;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourLayerPart	*cnp = &(cl->contour);
	float			*clvp;
	int			*clup;
	int			i;
	float			height;

	Mono_Dash_Pattern = cnp->mono_line_dash_pattern;
	Dash_Patterns = (int *) cnp->line_dash_patterns->data;
	Dash_Seglen = cnp->line_dash_seglen;
	View_Width = cl->view.width;
	Line_Colors = (int *) cnp->gks_line_colors;
	Mono_Line_Color = cnp->mono_line_color;
	Line_Thicknesses = (float *) cnp->line_thicknesses->data;
	Mono_Line_Thickness = cnp->mono_line_thickness;
	Constant_Labels = False;

	LLabel_AttrsP = &cnp->line_lbls;
	High_Label_AttrsP = &cnp->high_lbls;
	Low_Label_AttrsP = &cnp->low_lbls;

	LLabel_AttrsP->text = (NhlString *) cnp->llabel_strings->data;
	LLabel_AttrsP->colors = (int *) cnp->gks_llabel_colors;
	LLabel_AttrsP->gks_bcolor = 
		_NhlGetGksCi(cl->base.wkptr,LLabel_AttrsP->back_color);
	LLabel_AttrsP->gks_plcolor = 
		_NhlGetGksCi(cl->base.wkptr,LLabel_AttrsP->perim_lcolor);

	High_Label_AttrsP->colors = (int *)
		_NhlGetGksCi(cl->base.wkptr,High_Label_AttrsP->mono_color);
	High_Label_AttrsP->gks_bcolor = 
		_NhlGetGksCi(cl->base.wkptr,High_Label_AttrsP->back_color);
	High_Label_AttrsP->gks_plcolor = 
		_NhlGetGksCi(cl->base.wkptr,High_Label_AttrsP->perim_lcolor);

	Low_Label_AttrsP->colors = (int *)
		_NhlGetGksCi(cl->base.wkptr,Low_Label_AttrsP->mono_color);
	Low_Label_AttrsP->gks_bcolor = 
		_NhlGetGksCi(cl->base.wkptr,Low_Label_AttrsP->back_color);
	Low_Label_AttrsP->gks_plcolor = 
		_NhlGetGksCi(cl->base.wkptr,Low_Label_AttrsP->perim_lcolor);

	*do_lines = True;
	*do_labels = False;

	gset_line_colr_ind((Gint)_NhlGetGksCi(cl->base.wkptr,0));
	gset_text_colr_ind((Gint)_NhlGetGksCi(cl->base.wkptr,0));
	gset_linewidth(1.0);

	c_cpseti("CLS",0);		/* Conpack not to select levels */
	c_cpseti("NCL",cnp->level_count); 
	clvp = (float *) cnp->levels->data;
	clup = (int *) cnp->level_flags->data;
	c_cpseti("DPU",1); /* dash pattern use flag */

	if (cnp->mono_line_color && Line_Colors[0] == NhlTRANSPARENT)
		*do_lines = False;

	for (i=0; i<cnp->level_count; i++) {

		c_cpseti("PAI",i+1);
		c_cpsetr("CLV",clvp[i]);
		c_cpseti("CLU",clup[i]);
		c_cpseti("AIA",100+i+1);
		c_cpseti("AIB",100+i);
		c_cpsetc("LLT",LLabel_AttrsP->text[i]);
	}
	c_cpsetr("CIU",cnp->level_spacing);
 
/* Set up for labels */

/* Conpack not to render the Informational label */
	c_cpsetc("ILT"," ");

/* Line labels */
	if (! cnp->line_lbls.on) {
		c_cpseti("LLP",0); 
	}
	else if (cnp->llabel_spacing == Nhl_cnCONSTANT) {
		*do_labels = True;
		c_cpseti("LLP",1);
		c_cpsetr("DPS",cnp->line_lbls.real_height / cl->view.width);
		Constant_Labels = True;
	}
	else if (cnp->llabel_spacing == Nhl_cnRANDOMIZED) {
		*do_labels = True;
		c_cpseti("LLP",2);
		if (cnp->line_lbls.angle < 0.0) 
			c_cpseti("LLO",1); /* angle to contour direction */
		else {
			c_cpseti("LLO",0); /* fixed angle  */
			c_cpsetr("LLA",cnp->line_lbls.angle);
		}
	}
	else {
		*do_labels = True;
		c_cpseti("LLP",3);
		if (cnp->line_lbls.angle < 0.0) 
			c_cpseti("LLO",1); /* angle to contour direction */
		else {
			c_cpseti("LLO",0); /* fixed angle  */
			c_cpsetr("LLA",cnp->line_lbls.angle);
		}
	}

	if (*do_labels) {
		height = cnp->line_lbls.real_height / cl->view.width;
		c_cpsetr("LLS",height);
		c_cpsetr("LLW",height * cnp->line_lbls.perim_space);
		if (cnp->line_lbls.back_color == NhlTRANSPARENT) {
			if (! cnp->line_lbls.perim_on) 
				c_cpseti("LLB",0); 
			else
				c_cpseti("LLB",1);
		}
		else {
			c_cpseti("LBC",cnp->line_lbls.back_color);
			if (! cnp->line_lbls.perim_on)
				c_cpseti("LLB",2);
			else
				c_cpseti("LLB",3);
		}
	}
/*
 * In order to allow user control of the high and low attributes 
 * individually set the appropriate part of the flag on if either 
 * the high or the low is on. Further distinguishing between high and low
 * occurs in the low level routine cpchhl_
 */
	if (! cnp->high_lbls.on)
		c_cpsetc("HIT"," ");
	else {
		static char cval[21];
		c_cpgetc("HIT",cval,20);
		cnp->high_lbls.text = (NhlString *) cval;
	}
	if (! cnp->low_lbls.on)
		c_cpsetc("LOT"," ");
	else {
		static char cval[21];
		c_cpgetc("LOT",cval,20);
		cnp->low_lbls.text = (NhlString *) cval;
	}
/*
 * Due to the way Conpack works it is not possible to have different text
 * sizes, white space, background or perim on/off settings for the high
 * and low labels. The high labels take precedence, so set up accordingly.
 */

	if (cnp->high_lbls.on || cnp->low_lbls.on) {
		*do_labels = True;
		height = cnp->high_lbls.real_height / cl->view.width;
		c_cpsetr("HLS",height);
		c_cpsetr("HLW",cnp->high_lbls.perim_space  * height);

		if (cnp->high_lbls.back_color == NhlTRANSPARENT) {
			if (! cnp->high_lbls.perim_on) 
				c_cpseti("HLB",0); 
			else
				c_cpseti("HLB",1);
		}
		else {
			if (! cnp->high_lbls.perim_on)
				c_cpseti("HLB",2);
			else
				c_cpseti("HLB",3);
		}
	}

	c_pcsetc("FC",":");
	return ret;

}

/*
 * Function:	UpdateFillInfo
 *
 * Description:	
 *
 * In Args:	layer	Contour instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: sets the do_fill Boolean flag depending on whether 
 *		 fill is to be done.
 *		 
 */	

static NhlErrorTypes UpdateFillInfo
#if  __STDC__
(
	NhlContourLayer	cl,
	NhlBoolean	*do_fill
)
#else
(cl,do_fill)
        NhlContourLayer	cl;
	NhlBoolean	*do_fill;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourLayerPart	*cnp = &(cl->contour);

	Fill_Colors = cnp->gks_fill_colors;
	Fill_Patterns = (int *) cnp->fill_patterns->data;
	Fill_Scales = (float *) cnp->fill_scales->data;
	Fill_Count = cnp->fill_count;
	Mono_Fill_Color = cnp->mono_fill_color;
	Mono_Fill_Pattern = cnp->mono_fill_pattern;
	Mono_Fill_Scale = cnp->mono_fill_scale;
	Wkptr = cl->base.wkptr;
	Clayer = (NhlLayer) cl;

	Color_Fill = (cnp->mono_fill_color && 
		      Fill_Colors[0] == NhlTRANSPARENT) ? False : True;
	Pattern_Fill = (cnp->mono_fill_pattern && 
			Fill_Patterns[0] == NhlHOLLOWFILL) ? False : True;

	if (Color_Fill &&  Pattern_Fill) {
		*do_fill = True;
		return ret;
	}

	*do_fill = False;
	return ret;
}

/*
 * Function:	SetUpLLTransObj
 *
 * Description: Sets up a LogLinear transformation object.
 *
 * In Args:	xnew	new instance record
 *		xold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes SetUpLLTransObj
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init
)
#else 
(cnnew,cnold,init)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
#endif
{
 	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	char			buffer[_NhlMAXRESNAMLEN];
	int			tmpid;
        NhlSArg			sargs[16];
        int			nargs = 0;


	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

	if (init)
		tfp->trans_obj = NULL;
	else if (ocnp->use_irr_trans && tfp->trans_obj != NULL) {
		subret = NhlDestroy(tfp->trans_obj->base.id);
		if ((ret = MIN(ret,subret) < NhlWARNING)) {
			e_text = "%s: Error destroying irregular trans object";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		tfp->trans_obj = NULL;
	}

	if (cnp->data_init) {
		NhlBoolean	rev;
		float		tmin,tmax;

		cnp->xc1 = cnp->sfp->x_start;
		cnp->xcm = cnp->sfp->x_end;
		rev = cnp->sfp->x_start > cnp->sfp->x_end;
		if (! rev) {
			tmin = MAX(cnp->sfp->x_start,cnp->x_min); 
			tmax = MIN(cnp->sfp->x_end,cnp->x_max);
		}
		else {
			tmin = MAX(cnp->sfp->x_end,cnp->x_min); 
			tmax = MIN(cnp->sfp->x_start,cnp->x_max);
		}
		cnp->xlb = tmin;
		cnp->xub = tmax;

		cnp->yc1 = cnp->sfp->y_start;
		cnp->ycn = cnp->sfp->y_end;
		rev = cnp->sfp->y_start > cnp->sfp->y_end;
		if (! rev) {
			tmin = MAX(cnp->sfp->y_start,cnp->y_min); 
			tmax = MIN(cnp->sfp->y_end,cnp->y_max);
		}
		else {
			tmin = MAX(cnp->sfp->y_end,cnp->y_min); 
			tmax = MIN(cnp->sfp->y_start,cnp->y_max);
		}
		cnp->ylb = tmin;
		cnp->yub = tmax;
	}
	if (tfp->trans_obj == NULL) {

		cnp->new_draw_req = True;
		NhlSetSArg(&sargs[nargs++],NhlNtrXLog,cnp->x_log);
		NhlSetSArg(&sargs[nargs++],NhlNtrYLog,cnp->y_log);

		NhlSetSArg(&sargs[nargs++],NhlNtrXMinF,cnp->x_min);
		NhlSetSArg(&sargs[nargs++],NhlNtrXMaxF,cnp->x_max);
		NhlSetSArg(&sargs[nargs++],NhlNtrYMinF,cnp->y_min);
		NhlSetSArg(&sargs[nargs++],NhlNtrYMaxF,cnp->y_max);

		NhlSetSArg(&sargs[nargs++],NhlNtrXReverse,cnp->x_reverse);
		NhlSetSArg(&sargs[nargs++],NhlNtrYReverse,cnp->y_reverse);
		NhlSetSArg(&sargs[nargs++],NhlNtrOutOfRangeF,
			   cnp->out_of_range_val);

		sprintf(buffer,"%s",cnnew->base.name);
		strcat(buffer,".Trans");

		subret = NhlALCreate(&tmpid,buffer,
				     NhllogLinTransObjLayerClass,
				     cnnew->base.id,sargs,nargs);

		ret = MIN(subret,ret);

		tfp->trans_obj = _NhlGetLayer(tmpid);

		if(tfp->trans_obj == NULL){
			e_text = "%s: Error creating transformation object";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}

		return ret;
	}
		
	if (cnp->x_min != ocnp->x_min)
		NhlSetSArg(&sargs[nargs++],NhlNtrXMinF,cnp->x_min);
	if (cnp->x_max != ocnp->x_max)
		NhlSetSArg(&sargs[nargs++],NhlNtrXMaxF,cnp->x_max);
	if (cnp->y_min != ocnp->y_min)
		NhlSetSArg(&sargs[nargs++],NhlNtrYMinF,cnp->y_min);
	if (cnp->y_max != ocnp->y_max)
		NhlSetSArg(&sargs[nargs++],NhlNtrYMaxF,cnp->y_max);

	if (cnp->x_reverse != ocnp->x_reverse)
		NhlSetSArg(&sargs[nargs++],NhlNtrXReverse,cnp->x_reverse);
	if (cnp->y_reverse != ocnp->y_reverse)
		NhlSetSArg(&sargs[nargs++],NhlNtrYReverse,cnp->y_reverse);

	if (cnp->x_log != ocnp->x_log)
		NhlSetSArg(&sargs[nargs++],NhlNtrXLog,cnp->x_log);
	if (cnp->y_log != ocnp->y_log)
		NhlSetSArg(&sargs[nargs++],NhlNtrYLog,cnp->y_log);

	if (cnp->out_of_range_val != ocnp->out_of_range_val)
		NhlSetSArg(&sargs[nargs++],NhlNtrOutOfRangeF,
			   cnp->out_of_range_val);

	subret = NhlALSetValues(tfp->trans_obj->base.id,sargs,nargs);
	if (nargs > 0)
		cnp->new_draw_req = True;
	return MIN(ret,subret);

}


/*
 * Function:	SetIrrCoordBounds
 *
 * Description: Sets the max and min coord bounds for an Irregular trans
 *
 * In Args:	cnp	pointer to Contour layer part
 *		ctype	which coordinate
 *		count	length of irregular coord array
 *		entry_name the high level entry point
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:   A number of fields in the ContourLayerPart are set
 */

static NhlErrorTypes SetIrrCoordBounds
#if  __STDC__
(
	NhlContourLayerPart	*cnp,
	cnCoord			ctype,
	int			count,
	NhlString		entry_name
)
#else 
(cnp,ctype,count,entry_name)
	NhlContourLayerPart	*cnp;
	cnCoord			ctype;
	int			count;
	NhlString		entry_name;
#endif
{
	NhlErrorTypes	ret = NhlNOERROR;
	char		*e_text;
	NhlBoolean	rev;
	float		tmin,tmax;

	if (ctype == cnXCOORD) {

		cnp->xc1 = 0;
		cnp->xcm = count - 1;
		rev = cnp->sfp->x_start > cnp->sfp->x_end;
		if (! rev) {
			tmin = MAX(cnp->sfp->x_start,cnp->x_min); 
			tmax = MIN(cnp->sfp->x_end,cnp->x_max);
		}
		else {
			tmin = MAX(cnp->sfp->x_end,cnp->x_min); 
			tmax = MIN(cnp->sfp->x_start,cnp->x_max);
		}
		cnp->xlb = tmin;
		cnp->xub = tmax;
		if (tmin > cnp->x_min) {
			e_text = 
			 "%s: %s cannot be less than %s min value: resetting";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
				  entry_name,NhlNtrXMinF,NhlNsfXArray);
			ret = MIN(ret,NhlWARNING);
			cnp->x_min = tmin;
		}
		if (tmax < cnp->x_max) {
			e_text = 
		      "%s: %s cannot be greater than %s max value: resetting";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
				  entry_name,NhlNtrXMaxF,NhlNsfXArray);
			ret = MIN(ret,NhlWARNING);
			cnp->x_max = tmax;
		}
	}
	else if (ctype == cnYCOORD) {
		
		cnp->yc1 = 0;
		cnp->ycn = count - 1;

		rev = cnp->sfp->y_start > cnp->sfp->y_end;
		if (! rev) {
			tmin = MAX(cnp->sfp->y_start,cnp->y_min); 
			tmax = MIN(cnp->sfp->y_end,cnp->y_max);
		}
		else {
			tmin = MAX(cnp->sfp->y_end,cnp->y_min); 
			tmax = MIN(cnp->sfp->y_start,cnp->y_max);
		}
		cnp->ylb = tmin;
		cnp->yub = tmax;
		if (tmin > cnp->y_min) {
			e_text = 
			 "%s: %s cannot be less than %s min value: resetting";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
				  entry_name,NhlNtrYMinF,NhlNsfYArray);
			ret = MIN(ret,NhlWARNING);
			cnp->y_min = tmin;
		}
		if (tmax < cnp->y_max) {
			e_text = 
		      "%s: %s cannot be greater than %s max value: resetting";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
				  entry_name,NhlNtrYMaxF,NhlNsfYArray);
			ret = MIN(ret,NhlWARNING);
			cnp->y_max = tmax;
		}
	}
	return ret;
}
/*
 * Function:	SetUpIrrTransObj
 *
 * Description: Sets up an Irregular transformation object.
 *
 * In Args:	xnew	new instance record
 *		xold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes SetUpIrrTransObj
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init
)
#else 
(cnnew,cnold,init)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
#endif
{
 	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	char			buffer[_NhlMAXRESNAMLEN];
	int			tmpid;
        NhlSArg			sargs[16];
        int			nargs = 0;
	float			float_buf[3];
	NhlBoolean		x_irr, y_irr;
	float			*fp;
	int			count;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

	if (! ocnp->use_irr_trans && tfp->trans_obj != NULL) {
		subret = NhlDestroy(tfp->trans_obj->base.id);
		if ((ret = MIN(ret,subret) < NhlWARNING)) {
			e_text = "%s: Error destroying irregular trans object";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		tfp->trans_obj = NULL;
	}

	x_irr = cnp->sfp->x_arr == NULL ? False : True;
	y_irr = cnp->sfp->y_arr == NULL ? False : True;
	if (! x_irr && ! y_irr) {
		e_text = "%s: Internal inconsistency setting irregular trans";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}

	if (init || tfp->trans_obj == NULL ||
	    cnp->sfp->x_arr != ocnp->sfp->x_arr ||
	    cnp->sfp->x_start != ocnp->sfp->x_start ||
	    cnp->sfp->x_end != ocnp->sfp->x_end ||
	    cnp->x_min != ocnp->x_min ||
	    cnp->x_max != ocnp->x_max) {

		if (x_irr) {
			fp = (float *) cnp->sfp->x_arr->data;
			count = cnp->sfp->x_arr->len_dimensions[0];
		}
		else {
			fp = float_buf;
			fp[0] = cnp->sfp->x_start;
			fp[2] = cnp->sfp->x_end;
			if (! cnp->x_log) { 
				fp[1] = (fp[0] + fp[2])/2.0;
			}
			else {
				fp[1] = pow(10.0,
					    (log10(fp[0])+log10(fp[2])) / 2.0);
				NhlSetSArg(&sargs[nargs++],NhlNtrXUseLog,True);
			}
			count = 3;
		}
		subret = SetIrrCoordBounds(cnp,cnXCOORD,count,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING)
			return ret;

		NhlSetSArg(&sargs[nargs++],NhlNtrXCoordPoints,fp);
		NhlSetSArg(&sargs[nargs++],NhlNtrXNumPoints,count);
		NhlSetSArg(&sargs[nargs++],NhlNtrXMinF,cnp->x_min);
		NhlSetSArg(&sargs[nargs++],NhlNtrXMaxF,cnp->x_max);
	}

	if (init || tfp->trans_obj == NULL ||
	    cnp->sfp->y_arr != ocnp->sfp->y_arr ||
	    cnp->sfp->y_start != ocnp->sfp->y_start ||
	    cnp->sfp->y_end != ocnp->sfp->y_end ||
	    cnp->y_min != ocnp->y_min ||
	    cnp->y_max != ocnp->y_max) {

		if (y_irr) {
			fp = (float *) cnp->sfp->y_arr->data;
			count = cnp->sfp->y_arr->len_dimensions[0];
		}
		else {
			fp = float_buf;
			fp[0] = cnp->sfp->y_start;
			fp[2] = cnp->sfp->y_end;
			if (! cnp->y_log) { 
				fp[1] = (fp[0] + fp[2])/2.0;
			}
			else {
				fp[1] = pow(10.0,
					    (log10(fp[0])+log10(fp[2])) / 2.0);
				NhlSetSArg(&sargs[nargs++],NhlNtrYUseLog,True);
			}
			count = 3;
		}
		subret = SetIrrCoordBounds(cnp,cnYCOORD,count,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING)
			return ret;

		NhlSetSArg(&sargs[nargs++],NhlNtrYCoordPoints,fp);
		NhlSetSArg(&sargs[nargs++],NhlNtrYNumPoints,count);
		NhlSetSArg(&sargs[nargs++],NhlNtrYMinF,cnp->y_min);
		NhlSetSArg(&sargs[nargs++],NhlNtrYMaxF,cnp->y_max);
	}

	if (init || tfp->trans_obj == NULL) {

		cnp->new_draw_req = True;

		NhlSetSArg(&sargs[nargs++],NhlNtrXReverse,cnp->x_reverse);
		NhlSetSArg(&sargs[nargs++],NhlNtrYReverse,cnp->y_reverse);
		NhlSetSArg(&sargs[nargs++],NhlNtrOutOfRangeF,
			   cnp->out_of_range_val);

		sprintf(buffer,"%s",cnnew->base.name);
		strcat(buffer,".Trans");

		subret = NhlALCreate(&tmpid,buffer,
				     NhlirregularType2TransObjLayerClass,
				     cnnew->base.id,sargs,nargs);

		ret = MIN(subret,ret);

		tfp->trans_obj = _NhlGetLayer(tmpid);

		if(tfp->trans_obj == NULL){
			e_text = "%s: Error creating transformation object";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}

		return ret;
	}

	if (cnp->x_reverse != ocnp->x_reverse)
		NhlSetSArg(&sargs[nargs++],NhlNtrXReverse,cnp->x_reverse);
	if (cnp->y_reverse != ocnp->y_reverse)
		NhlSetSArg(&sargs[nargs++],NhlNtrYReverse,cnp->y_reverse);

	if (cnp->out_of_range_val != ocnp->out_of_range_val)
		NhlSetSArg(&sargs[nargs++],NhlNtrOutOfRangeF,
			   cnp->out_of_range_val);

	subret = NhlALSetValues(tfp->trans_obj->base.id,sargs,nargs);

	if (nargs > 0)
		cnp->new_draw_req = True;
	return MIN(ret,subret);

}

/*
 * Function:	ManageLegend
 *
 * Description: If the Contour object has an overlay object attached, and
 *		the Legend is activated, manages the Legend resources 
 *		relevant to the Contour object.
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ManageLegend
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
)
#else 
(cnnew, cnold, init, sargs, nargs)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
	NhlSArg		*sargs;
	int		*nargs;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	char 			*title_add = " Legend";

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

 	if (! tfp->overlay_plot_base ||
	    cnp->display_legend == Nhl_ovNoCreate) 
		return NhlNOERROR;

	if (init || 
	    cnp->display_legend != ocnp->display_legend ||
	    cnp->const_field != ocnp->const_field) {
		if (cnp->const_field) {
			e_text = "%s: constant field: Legend not drawn";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNovDisplayLegend,Nhl_ovNever);
		}
		else
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNovDisplayLegend,cnp->display_legend);
	}

	if (init) {
		int count = 1;

		if (cnp->legend_title == NULL) {
			if ((cnp->legend_title = 
			    NhlMalloc(strlen(cnnew->base.name) +
				      strlen(title_add) + 1)) == NULL) {
				e_text = "%s: dynamic memory allocation error";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				return NhlFATAL;
			}
			strcpy(cnp->legend_title,cnnew->base.name);
			strcat(cnp->legend_title,title_add);
		}
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgTitleString,cnp->legend_title);

		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemCount,cnp->level_count);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemIndexes,cnp->line_dash_patterns);
		if (cnp->llabel_spacing == Nhl_cnNOLABELS) {
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgDrawLineLabels,False);
		}
		else {
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgDrawLineLabels,True);
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgLabelStrings,cnp->llabel_strings);
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgMonoItemStringColor,
				   cnp->line_lbls.mono_color);
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringColors,cnp->llabel_colors);
		}
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgMonoItemColor,cnp->mono_line_color);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemColors,cnp->line_colors);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgMonoItemThickness,cnp->mono_line_thickness);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemThicknesses,cnp->line_thicknesses);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemStrings,cnp->ll_strings);
  
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgMonoItemTextHeight,True);
	        if ((cnp->ll_text_heights = 
		     NhlCreateGenArray((NhlPointer)
				       &cnp->line_lbls.height,
				       NhlTFloat,sizeof(float),1,&count))
		    == NULL) {
			e_text = "%s: error creating %s GenArray";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
				  NhlNlgItemTextHeights);
			return NhlFATAL;
		}
		cnp->ll_text_heights->my_data = False;
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemTextHeights,cnp->ll_text_heights);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgLineDashSegLenF,cnp->line_dash_seglen);

#if 0 /* no support for text attributes in legend item strings yet */

		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemStringFont,cnp->line_lbls.font);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemStringFontAspectF,cnp->line_lbls.aspect);
		NhlSetSArg(&sargs[(*nargs)++],
		      NhlNlgItemStringFontThicknessF,cnp->line_lbls.thickness);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemStringFontQuality,cnp->line_lbls.quality);
		NhlSetSArg(&sargs[(*nargs)++],
		   NhlNlgItemStringConstantSpacingF,cnp->line_lbls.cspacing);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlgItemStringFuncCode,cnp->line_lbls.fcode);

#endif		
	}
	else {
		if (cnp->level_count != ocnp->level_count)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemCount,cnp->level_count);
		if (cnp->line_dash_patterns != ocnp->line_dash_patterns)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemIndexes,cnp->line_dash_patterns);
		if (cnp->llabel_spacing != ocnp->llabel_spacing) {
			if (cnp->llabel_spacing == Nhl_cnNOLABELS) {
				NhlSetSArg(&sargs[(*nargs)++],
					   NhlNlgDrawLineLabels,False);
			}
			else
				NhlSetSArg(&sargs[(*nargs)++],
					   NhlNlgDrawLineLabels,True);
		}
		if (cnp->llabel_strings != ocnp->llabel_strings)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgLabelStrings,cnp->llabel_strings);
		if (cnp->mono_line_color != ocnp->mono_line_color)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgMonoItemColor,cnp->mono_line_color);
		if (cnp->line_colors != ocnp->line_colors)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemColors,cnp->line_colors);
		if (cnp->mono_line_thickness != ocnp->mono_line_thickness)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgMonoItemThickness,
				   cnp->mono_line_thickness);
		if (cnp->line_thicknesses != ocnp->line_thicknesses)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemThicknesses,
				   cnp->line_thicknesses);
		if (cnp->line_lbls.mono_color != ocnp->line_lbls.mono_color)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgMonoItemStringColor,
				   cnp->line_lbls.mono_color);
		if (cnp->llabel_colors != ocnp->llabel_colors)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringColors,
				   cnp->llabel_colors);
		if (cnp->llabel_strings != ocnp->llabel_strings ||
		    cnp->level_flags != ocnp->level_flags ||
		    cnp->level_count != ocnp->level_count ||
		    cnp->llabel_interval_set) 
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStrings,cnp->ll_strings);
		if (cnp->line_lbls.height != ocnp->line_lbls.height)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemTextHeights,cnp->ll_text_heights);
		if (cnp->line_dash_seglen != ocnp->line_dash_seglen)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgLineDashSegLenF,
				   cnp->line_dash_seglen);

#if 0 /* no support for text attributes in legend strings yet */

		if (cnp->line_lbls.font != ocnp->line_lbls.font)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringFont,cnp->line_lbls.font);
		if (cnp->line_lbls.aspect != ocnp->line_lbls.aspect)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringFontAspectF,
				   cnp->line_lbls.aspect);
		if (cnp->line_lbls.thickness != ocnp->line_lbls.thickness)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringFontThicknessF,
				   cnp->line_lbls.thickness);
		if (cnp->line_lbls.quality != ocnp->line_lbls.quality)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringFontQuality,
				   cnp->line_lbls.quality);
		if (cnp->line_lbls.cspacing != ocnp->line_lbls.cspacing)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringConstantSpacingF,
				   cnp->line_lbls.cspacing);
		if (cnp->line_lbls.fcode != ocnp->line_lbls.fcode)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlgItemStringFuncCode,
				   cnp->line_lbls.fcode);
#endif
	}

	return ret;
}


/*
 * Function:	ManageLabelBar
 *
 * Description: If the Contour object has an overlay object attached, and
 *		the LabelBar is activated, manages the LabelBar resources 
 *		relevant to the Contour object.
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ManageLabelBar
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
)
#else 
(cnnew,cnold, init, sargs, nargs)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
	NhlSArg		*sargs;
	int		*nargs;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	char			*title_add = " LabelBar";

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

 	if (! tfp->overlay_plot_base ||
	    cnp->display_labelbar == Nhl_ovNoCreate) 
		return NhlNOERROR;

	if (init || 
	    cnp->display_labelbar != ocnp->display_labelbar ||
	    cnp->const_field != ocnp->const_field) {
		if (cnp->const_field) {
			e_text = "%s: constant field: LabelBar not drawn";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNovDisplayLabelBar,Nhl_ovNever);
		}
		else
			NhlSetSArg(&sargs[(*nargs)++],NhlNovDisplayLabelBar,
				   cnp->display_labelbar);
	}

	if (init) {

		if (cnp->labelbar_title == NULL) {
			if ((cnp->labelbar_title = 
			    NhlMalloc(strlen(cnnew->base.name) +
				      strlen(title_add) + 1)) == NULL) {
				e_text = "%s: dynamic memory allocation error";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				return NhlFATAL;
			}
			strcpy(cnp->labelbar_title,cnnew->base.name);
			strcat(cnp->labelbar_title,title_add);
		}
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbTitleString,cnp->labelbar_title);

		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbBoxCount,cnp->fill_count);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbLabelAlignment,NhlLB_INTERIOREDGES);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbLabelStrings,cnp->llabel_strings);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbMonoFillColor,cnp->mono_fill_color);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbFillColors,cnp->fill_colors);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbMonoFillPattern,cnp->mono_fill_pattern);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbFillPatterns,cnp->fill_patterns);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbMonoFillScale,cnp->mono_fill_scale);
		NhlSetSArg(&sargs[(*nargs)++],
			   NhlNlbFillScales,cnp->fill_scales);
	}
	else {
		if (cnp->level_count != ocnp->level_count)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbBoxCount,cnp->fill_count);
		if (cnp->llabel_strings != ocnp->llabel_strings)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbLabelStrings,cnp->llabel_strings);
		if (cnp->mono_fill_color != ocnp->mono_fill_color)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbMonoFillColor,cnp->mono_fill_color);
		if (cnp->fill_colors != ocnp->fill_colors)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbFillColors,cnp->fill_colors);
		if (cnp->mono_fill_pattern != ocnp->mono_fill_pattern)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbMonoFillPattern,
				   cnp->mono_fill_pattern);
		if (cnp->fill_patterns != ocnp->fill_patterns)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbFillPatterns,cnp->fill_patterns);
		if (cnp->mono_fill_scale != ocnp->mono_fill_scale)
			NhlSetSArg(&sargs[(*nargs)++],
				   NhlNlbMonoFillScale,cnp->mono_fill_scale);
	}

	return ret;
}

/*
 * Function:	ManageInfoLabel
 *
 * Description: If the information label label is 
 *		activated text items to store the strings are created.
 *		If there is an Overlay an annotation wrapper object is 
 *		created so that the overlay object can manage the
 *		annotations.
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ManageInfoLabel
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
)
#else 
(cnnew,cnold, init, sargs, nargs)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
	NhlSArg		*sargs;
	int		*nargs;
#endif
{
	char			*e_text;
	char			*entry_name;
	NhlErrorTypes		ret,subret;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	NhlcnLabelAttrs		*ilp = &cnp->info_lbl;
	NhlcnLabelAttrs		*oilp = &ocnp->info_lbl;
	NhlString		lstring;
	NhlBoolean		text_changed,pos_changed = False;
	NhlSArg			targs[16];
	int			targc = 0;
	char			buffer[_NhlMAXRESNAMLEN];
	int			tmpid;
	

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

	if (! ilp->on && (init || ! oilp->on))
		return NhlNOERROR;

	if (cnp->info_string == NULL
	    || cnp->info_string != ocnp->info_string) {
		int strsize = cnp->info_string == NULL ? 
			strlen(NhlcnDEF_INFO_LABEL) + 1 : 
				strlen(cnp->info_string) + 1;

		if ((lstring = NhlMalloc(strsize)) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		if (cnp->info_string == NULL)
			strcpy(lstring,NhlcnDEF_INFO_LABEL);
		else
			strcpy(lstring,cnp->info_string);
		cnp->info_string = lstring;
		if (ocnp->info_string != NULL) 
			NhlFree(ocnp->info_string);
	}

	subret = ReplaceSubstitutionChars(cnp,ocnp,init,_cnINFO,
					  &text_changed,entry_name);
	if ((ret = MIN(ret,subret) < NhlWARNING)) return ret;

	if (tfp->overlay_status == _tfNotInOverlay) {
		subret = SetTextPosition(cnnew,ocnp,_cnINFO,
					 &pos_changed,entry_name);
		if ((ret = MIN(ret,subret) < NhlWARNING)) return ret;
	}

	if (init || cnp->info_lbl_rec.id < 0) {
		if (pos_changed) {
			NhlSetSArg(&targs[(targc)++],NhlNtxPosXF,ilp->x_pos);
			NhlSetSArg(&targs[(targc)++],NhlNtxPosYF,ilp->y_pos);
			NhlSetSArg(&targs[(targc)++],NhlNtxJust,ilp->just);
		}
		NhlSetSArg(&targs[(targc)++],NhlNtxString,
			   (NhlString)ilp->text);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontHeightF,ilp->height);
		NhlSetSArg(&targs[(targc)++],NhlNtxDirection,ilp->direction);
		NhlSetSArg(&targs[(targc)++],NhlNtxAngleF,ilp->angle);
		NhlSetSArg(&targs[(targc)++],NhlNtxFont,ilp->font);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontColor,ilp->mono_color);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontAspectF,ilp->aspect);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxFontThicknessF,ilp->thickness);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxConstantSpacingF,ilp->cspacing);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontQuality,ilp->quality);
		NhlSetSArg(&targs[(targc)++],NhlNtxFuncCode,ilp->fcode[0]);

		NhlSetSArg(&targs[(targc)++],NhlNtxPerimOn,ilp->perim_on);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimColor,ilp->perim_lcolor);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimThicknessF,ilp->perim_lthick);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimSpaceF,ilp->perim_space);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxBackgroundFillColor,ilp->back_color);

		sprintf(buffer,"%s",cnnew->base.name);
		strcat(buffer,".Text");
		subret = NhlALCreate(&tmpid,buffer,NhltextItemLayerClass,
				     cnnew->base.id,targs,targc);
		
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error creating information label";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		cnp->info_lbl_rec.id = tmpid;
		
		if (tfp->overlay_status != _tfNotInOverlay) {
			subret = ManageAnnotation(cnnew,ocnp,init,_cnINFO);
			ret = MIN(ret,subret);
		}
		return ret;
	}

	if (pos_changed) {
		NhlSetSArg(&targs[(targc)++],NhlNtxPosXF,ilp->x_pos);
		NhlSetSArg(&targs[(targc)++],NhlNtxPosYF,ilp->y_pos);
		NhlSetSArg(&targs[(targc)++],NhlNtxJust,ilp->just);
	}
	if (text_changed)
		NhlSetSArg(&targs[(targc)++],NhlNtxString,ilp->text[0]);
	if (ilp->height != oilp->height)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontHeightF,ilp->height);
	if (ilp->direction != oilp->direction)
		NhlSetSArg(&targs[(targc)++],NhlNtxDirection,ilp->direction);
	if (ilp->angle != oilp->angle)
		NhlSetSArg(&targs[(targc)++],NhlNtxAngleF,ilp->angle);
	if (ilp->font != oilp->font)
		NhlSetSArg(&targs[(targc)++],NhlNtxFont,ilp->font);
	if (ilp->mono_color != oilp->mono_color)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontColor,ilp->mono_color);
	if (ilp->aspect != oilp->aspect)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontAspectF,ilp->aspect);
	if (ilp->thickness != oilp->thickness)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxFontThicknessF,ilp->thickness);
	if (ilp->cspacing != oilp->cspacing)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxConstantSpacingF,ilp->cspacing);
	if (ilp->quality != oilp->quality)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontQuality,ilp->quality);
	if (ilp->fcode[0] != oilp->fcode[0])
		NhlSetSArg(&targs[(targc)++],NhlNtxFuncCode,ilp->fcode[0]);
	
	if (ilp->perim_on != oilp->perim_on)
		NhlSetSArg(&targs[(targc)++],NhlNtxPerimOn,ilp->perim_on);
	if (ilp->perim_lcolor != oilp->perim_lcolor)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimColor,ilp->perim_lcolor);
	if (ilp->perim_lthick != oilp->perim_lthick)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimThicknessF,ilp->perim_lthick);
	if (ilp->perim_space != oilp->perim_space)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimSpaceF,ilp->perim_space);
	if (ilp->back_color != oilp->back_color)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxBackgroundFillColor,ilp->back_color);
	
	subret = NhlALSetValues(cnp->info_lbl_rec.id,targs,targc);

	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		e_text = "%s: error setting values for information label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}

	if (tfp->overlay_status != _tfNotInOverlay) {
		subret = ManageAnnotation(cnnew,ocnp,init,_cnINFO);
		ret = MIN(ret,subret);
	}

	return ret;
}


/*
 * Function:	ManageConstFLabel
 *
 * Description: If a constant field is detected a constant field label
 *		is created, or turned on.
 *		If there is an Overlay an annotation wrapper object is 
 *		created so that the overlay object can manage the
 *		annotations.
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ManageConstFLabel
#if  __STDC__
(
	NhlContourLayer	cnnew,
	NhlContourLayer	cnold,
	NhlBoolean	init,
	NhlSArg		*sargs,
	int		*nargs
)
#else 
(cnnew,cnold, init, sargs, nargs)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
	NhlSArg		*sargs;
	int		*nargs;
#endif
{
	char			*e_text;
	char			*entry_name;
	NhlErrorTypes		ret,subret;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlContourLayerPart	*ocnp = &(cnold->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	NhlcnLabelAttrs		*cflp = &cnp->constf_lbl;
	NhlcnLabelAttrs		*ocflp = &ocnp->constf_lbl;
	NhlString		lstring;
	NhlBoolean		text_changed,pos_changed = False;
	NhlSArg			targs[16];
	int			targc = 0;
	char			buffer[_NhlMAXRESNAMLEN];
	int			tmpid;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

/*
 * The constant field label resource must be turned on AND a constant
 * field condition must exist for the constant field annotation to 
 * created and/or displayed.
 */
	cnp->display_constf = cflp->on && cnp->const_field;

	if (! cnp->display_constf && (init || ! ocnp->display_constf))
		return NhlNOERROR;

	if (cnp->constf_string == NULL
	    || cnp->constf_string != ocnp->constf_string) {
		int strsize = cnp->constf_string == NULL ? 
			strlen(NhlcnDEF_CONSTF_LABEL) + 1 : 
				strlen(cnp->constf_string) + 1;

		if ((lstring = NhlMalloc(strsize)) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		if (cnp->constf_string == NULL)
			strcpy(lstring,NhlcnDEF_CONSTF_LABEL);
		else
			strcpy(lstring,cnp->constf_string);
		cnp->constf_string = lstring;
		if (ocnp->constf_string != NULL) 
			NhlFree(ocnp->constf_string);
	}

	subret = ReplaceSubstitutionChars(cnp,ocnp,init,_cnCONSTF,
					  &text_changed,entry_name);
	if ((ret = MIN(ret,subret) < NhlWARNING)) return ret;

	if (tfp->overlay_status == _tfNotInOverlay) {
		subret = SetTextPosition(cnnew,ocnp,_cnCONSTF,
					 &pos_changed,entry_name);
		if ((ret = MIN(ret,subret) < NhlWARNING)) return ret;
	}

	if (init || cnp->constf_lbl_rec.id < 0) {
		if (pos_changed) {
			NhlSetSArg(&targs[(targc)++],NhlNtxPosXF,cflp->x_pos);
			NhlSetSArg(&targs[(targc)++],NhlNtxPosYF,cflp->y_pos);
			NhlSetSArg(&targs[(targc)++],NhlNtxJust,cflp->just);
		}
		NhlSetSArg(&targs[(targc)++],NhlNtxString,
			   (NhlString)cflp->text);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontHeightF,cflp->height);
		NhlSetSArg(&targs[(targc)++],NhlNtxDirection,cflp->direction);
		NhlSetSArg(&targs[(targc)++],NhlNtxAngleF,cflp->angle);
		NhlSetSArg(&targs[(targc)++],NhlNtxFont,cflp->font);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontColor,cflp->mono_color);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontAspectF,cflp->aspect);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxFontThicknessF,cflp->thickness);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxConstantSpacingF,cflp->cspacing);
		NhlSetSArg(&targs[(targc)++],NhlNtxFontQuality,cflp->quality);
		NhlSetSArg(&targs[(targc)++],NhlNtxFuncCode,cflp->fcode[0]);

		NhlSetSArg(&targs[(targc)++],NhlNtxPerimOn,cflp->perim_on);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimColor,cflp->perim_lcolor);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimThicknessF,cflp->perim_lthick);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimSpaceF,cflp->perim_space);
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxBackgroundFillColor,cflp->back_color);

		sprintf(buffer,"%s",cnnew->base.name);
		strcat(buffer,".Text");
		subret = NhlALCreate(&tmpid,buffer,NhltextItemLayerClass,
				     cnnew->base.id,targs,targc);
		
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error creating information label";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		cnp->constf_lbl_rec.id = tmpid;
		
		if (tfp->overlay_status != _tfNotInOverlay) {
			subret = ManageAnnotation(cnnew,ocnp,init,_cnCONSTF);
			ret = MIN(ret,subret);
		}
		return ret;
	}

	if (pos_changed) {
		NhlSetSArg(&targs[(targc)++],NhlNtxPosXF,cflp->x_pos);
		NhlSetSArg(&targs[(targc)++],NhlNtxPosYF,cflp->y_pos);
		NhlSetSArg(&targs[(targc)++],NhlNtxJust,cflp->just);
	}
	if (text_changed)
		NhlSetSArg(&targs[(targc)++],NhlNtxString,cflp->text[0]);
	if (cflp->height != ocflp->height)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontHeightF,cflp->height);
	if (cflp->direction != ocflp->direction)
		NhlSetSArg(&targs[(targc)++],NhlNtxDirection,cflp->direction);
	if (cflp->angle != ocflp->angle)
		NhlSetSArg(&targs[(targc)++],NhlNtxAngleF,cflp->angle);
	if (cflp->font != ocflp->font)
		NhlSetSArg(&targs[(targc)++],NhlNtxFont,cflp->font);
	if (cflp->mono_color != ocflp->mono_color)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontColor,cflp->mono_color);
	if (cflp->aspect != ocflp->aspect)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontAspectF,cflp->aspect);
	if (cflp->thickness != ocflp->thickness)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxFontThicknessF,cflp->thickness);
	if (cflp->cspacing != ocflp->cspacing)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxConstantSpacingF,cflp->cspacing);
	if (cflp->quality != ocflp->quality)
		NhlSetSArg(&targs[(targc)++],NhlNtxFontQuality,cflp->quality);
	if (cflp->fcode[0] != ocflp->fcode[0])
		NhlSetSArg(&targs[(targc)++],NhlNtxFuncCode,cflp->fcode[0]);
	
	if (cflp->perim_on != ocflp->perim_on)
		NhlSetSArg(&targs[(targc)++],NhlNtxPerimOn,cflp->perim_on);
	if (cflp->perim_lcolor != ocflp->perim_lcolor)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimColor,cflp->perim_lcolor);
	if (cflp->perim_lthick != ocflp->perim_lthick)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimThicknessF,cflp->perim_lthick);
	if (cflp->perim_space != ocflp->perim_space)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxPerimSpaceF,cflp->perim_space);
	if (cflp->back_color != ocflp->back_color)
		NhlSetSArg(&targs[(targc)++],
			   NhlNtxBackgroundFillColor,cflp->back_color);
	
	subret = NhlALSetValues(cnp->constf_lbl_rec.id,targs,targc);

	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		e_text = "%s: error setting values for information label";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}

	if (tfp->overlay_status != _tfNotInOverlay) {
		subret = ManageAnnotation(cnnew,ocnp,init,_cnCONSTF);
		ret = MIN(ret,subret);
	}

	return ret;
}

/*
 * Function:	ManageAnnotation
 *
 * Description: 
 *		
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ManageAnnotation
#if  __STDC__
(
	NhlContourLayer		cnnew,
	NhlContourLayerPart	*ocnp,
	NhlBoolean		init,
	_cnAnnoType		atype
)
#else 
(cnnew,ocnp,init,atype)
	NhlContourLayer		cnnew;
	NhlContourLayerPart	*ocnp;
	NhlBoolean		init;
	_cnAnnoType		atype;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlTransformLayerPart	*tfp = &(cnnew->trans);
	NhlAnnotationRec	*rec, *orec;
	int			*idp;
	NhlSArg			sargs[16];
	int			nargs = 0;
	char			buffer[_NhlMAXRESNAMLEN];
	int			tmpid;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

	if (atype == _cnINFO) {
		rec = &cnp->info_lbl_rec;
		orec = &ocnp->info_lbl_rec;
		idp = &cnp->info_anno_id;
		rec->on = cnp->info_lbl.on && ! cnp->const_field;
	}
	else {
		rec = &cnp->constf_lbl_rec;
		orec = &ocnp->constf_lbl_rec;
		idp = &cnp->constf_anno_id;
		rec->on = cnp->constf_lbl.on && cnp->const_field;
	}

	if (*idp < 0) {
		NhlSetSArg(&sargs[(nargs)++],NhlNanOn,rec->on);
		NhlSetSArg(&sargs[(nargs)++],NhlNanPlotId,rec->id);
		NhlSetSArg(&sargs[(nargs)++],NhlNanZone,rec->zone);
		NhlSetSArg(&sargs[(nargs)++],NhlNanSide,rec->side);
		NhlSetSArg(&sargs[(nargs)++],NhlNanJust,rec->just);
		NhlSetSArg(&sargs[(nargs)++],
			   NhlNanParallelPosF,rec->para_pos);
		NhlSetSArg(&sargs[(nargs)++],
			   NhlNanOrthogonalPosF,rec->ortho_pos);
		sprintf(buffer,"%s",cnnew->base.name);
		strcat(buffer,".Annotation");
		subret = NhlALCreate(&tmpid,buffer,NhlannotationLayerClass,
				     cnnew->base.id,sargs,nargs);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error creating annotation layer";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		*idp = tmpid;
/*
 * If the Contour plot is an overlay plot base register the annotation with
 * its own base, ensuring that it will always follow the overlay.
 */
		if (tfp->overlay_plot_base)
			subret = NhlRegisterAnnotation(cnnew->base.id,
						       *idp);
		else 
			subret = NhlRegisterAnnotation(
				    tfp->overlay_object->base.parent->base.id,
				    *idp);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error registering annotation layer";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		return ret;
	}
	if (rec->on != orec->on) 
		NhlSetSArg(&sargs[(nargs)++],NhlNanOn,rec->on);
	if (rec->id != orec->id) 
		NhlSetSArg(&sargs[(nargs)++],NhlNanPlotId,rec->id);
	if (rec->zone != orec->zone) 
		NhlSetSArg(&sargs[(nargs)++],NhlNanZone,rec->zone);
	if (rec->side != orec->side) 
		NhlSetSArg(&sargs[(nargs)++],NhlNanSide,rec->side);
	if (rec->just != orec->just) 
		NhlSetSArg(&sargs[(nargs)++],NhlNanJust,rec->just);
	if (rec->para_pos != orec->para_pos) 
		NhlSetSArg(&sargs[(nargs)++],
			   NhlNanParallelPosF,rec->para_pos);
	if (rec->ortho_pos != orec->ortho_pos) 
		NhlSetSArg(&sargs[(nargs)++],
			   NhlNanOrthogonalPosF,rec->ortho_pos);
	
	subret = NhlALSetValues(*idp,sargs,nargs);

	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		e_text = "%s: error setting annotation object values";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}
	return ret;
}

/*
 * Function:	ReplaceSubstitutionChars
 *
 * Description: Replaces the substitution characters in the info and
 *		zero field labels with the correct numerical values.
 *
 * In Args:	cnnew	new instance record
 *		cnold	old instance record if not initializing
 *		init	true if initialization
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes ReplaceSubstitutionChars
#if  __STDC__
(
	NhlContourLayerPart	*cnp,
	NhlContourLayerPart	*ocnp,
	NhlBoolean		init,
	_cnAnnoType		atype,
	NhlBoolean		*text_changed,
	NhlString		entry_name
)
#else 
(cnp,ocnp,init,atype,text_changed,entry_name)
	NhlContourLayerPart	*cnp;
	NhlContourLayerPart	*ocnp;
	NhlBoolean		init;
	_cnAnnoType		atype;
	NhlBoolean		*text_changed;
	NhlString		entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	NhlBoolean		done = False;
	char			buffer[256];
	char			*matchp,*subst;

	*text_changed = False;

	switch (atype) {
	case _cnINFO:
		if (! init && (cnp->info_string == ocnp->info_string))
			return NhlNOERROR;
		strcpy(buffer,cnp->info_string);
		while (! done) {
			if ((matchp = strstr(buffer,"$CIU$")) != NULL) {
				subst = ContourFormat(cnp,cnCONINTERVAL);
				Substitute(matchp,5,subst);
			}
			else if ((matchp = strstr(buffer,"$CMN$")) != NULL) {
				subst = ContourFormat(cnp,cnCONMINVAL);
				Substitute(matchp,5,subst);
			}
			else if ((matchp = strstr(buffer,"$CMX$")) != NULL) {
				subst = ContourFormat(cnp,cnCONMAXVAL);
				Substitute(matchp,5,subst);
			}
			else if ((matchp = strstr(buffer,"$SFU$")) != NULL) {
				subst = ContourFormat(cnp,cnSCALEFACTOR);
				Substitute(matchp,5,subst);
			}
			else if ((matchp = strstr(buffer,"$ZMN$")) != NULL) {
				subst = ContourFormat(cnp,cnDATAMINVAL);
				Substitute(matchp,5,subst);
			}
			else if ((matchp = strstr(buffer,"$ZMX$")) != NULL) {
				subst = ContourFormat(cnp,cnDATAMAXVAL);
				Substitute(matchp,5,subst);
			}
			else {
				done = True;
			}
		}
		if (cnp->info_lbl.text != NULL)
			NhlFree(cnp->info_lbl.text);
		if ((cnp->info_lbl.text = 
		     NhlMalloc(strlen(buffer)+1)) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		strcpy((NhlString)cnp->info_lbl.text,buffer);
		break;
	case _cnCONSTF:
		if (! init && (cnp->constf_string == ocnp->constf_string))
			return NhlNOERROR;
		strcpy(buffer,cnp->constf_string);
		while (! done) {
			if ((matchp = strstr(buffer,"$ZDV$")) != NULL) {
				subst = ContourFormat(cnp,cnCONSTFVAL);
				Substitute(matchp,5,subst);
			}
			else {
				done = True;
			}
		}
		if (cnp->constf_lbl.text != NULL)
			NhlFree(cnp->constf_lbl.text);
		if ((cnp->constf_lbl.text = 
		     NhlMalloc(strlen(buffer)+1)) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		strcpy((NhlString)cnp->constf_lbl.text,buffer);
		break;
	default:
		e_text = "%s: internal enumeration error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text, entry_name);
		return(ret);
	}

	*text_changed = True;

	return ret;
}



/*
 * Function:	Substitute
 *
 * Description: substitutes a string for a Conpack substitution sequence.
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static void Substitute
#if  __STDC__
(
	char		*buf,
	int		replace_count,
	char		*subst
)
#else 
(buf,replace_count,subst)
	char		*buf;
	int		replace_count;
	char		*subst;
#endif
{
	int subst_count,add,buflen;
	char *from, *to;

	buflen = strlen(buf);
	subst_count = strlen(subst);
	if (subst_count - replace_count < 0) {
		for (from = buf+replace_count,to = buf+subst_count; 
		     from != NULL; )
			*to++ = *from++;
	}
	else if ((add = subst_count - replace_count) > 0) {
		for (from = buf + buflen,to = buf + buflen + add; 
		     from >= buf + replace_count;)
			*to-- = *from--;
	}
	strncpy(buf,subst,subst_count);
}
/*
 * Function:	ContourFormat
 *
 * Description: formats a numeric internal contour value type into a string.
 *		the string is stored in static memory; previous values 
 *		overwritten at each invocation.
 *
 * In Args:	cnp	Contour layer part
 *		type	the type of value requested
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static char *ContourFormat
#if  __STDC__
(
	NhlContourLayerPart	*cnp,
	cnValueType		vtype
)
#else 
(cnp,vtype)
	NhlContourLayerPart	*cnp;
	cnValueType		vtype;

#endif
{
	static char		buffer[64];

	switch (vtype) {

	case cnCONSTFVAL:
		sprintf(buffer,"%6.2f",cnp->zmax);
		return buffer;
	case cnCONINTERVAL:
		sprintf(buffer,"%6.2f",cnp->level_spacing);
		return buffer;
	case cnCONMINVAL:
		sprintf(buffer,"%6.2f",cnp->min_level_val);
		return buffer;
	case cnCONMAXVAL:
		sprintf(buffer,"%6.2f",cnp->max_level_val);
		return buffer;
	case cnDATAMINVAL:
		sprintf(buffer,"%6.2f",cnp->zmin);
		return buffer;
	case cnDATAMAXVAL:
		sprintf(buffer,"%6.2f",cnp->zmax);
		return buffer;
	case cnSCALEFACTOR:
		buffer[0] = '\0';
		return buffer;
	}
}
/*
 * Function:	ConstrainJustification
 *
 * Description: Constrain justification depending on the annotation side;
 *
 * In Args:	NhlAnnoRec	anno_rec - the annotation record
 *
 * Out Args:
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:	NONE
 */
static NhlJustification
ConstrainJustification
#if __STDC__
(
	NhlAnnotationRec	*anno_rec
)
#else
(anno_rec)
	NhlAnnotationRec	*anno_rec;
#endif
{
	switch (anno_rec->side) {
	case NhlBOTTOM:
		switch (anno_rec->just) {
		case NhlTOPLEFT:
		case NhlCENTERLEFT:
		case NhlBOTTOMLEFT:
			return NhlTOPLEFT;
		case NhlTOPCENTER:
		case NhlCENTERCENTER:
		case NhlBOTTOMCENTER:
			return NhlTOPCENTER;
		case NhlTOPRIGHT:
		case NhlCENTERRIGHT:
		case NhlBOTTOMRIGHT:
			return NhlTOPRIGHT;
		}
	case NhlTOP:
		switch (anno_rec->just) {
		case NhlTOPLEFT:
		case NhlCENTERLEFT:
		case NhlBOTTOMLEFT:
			return NhlBOTTOMLEFT;
		case NhlTOPCENTER:
		case NhlCENTERCENTER:
		case NhlBOTTOMCENTER:
			return NhlBOTTOMCENTER;
		case NhlTOPRIGHT:
		case NhlCENTERRIGHT:
		case NhlBOTTOMRIGHT:
			return NhlBOTTOMRIGHT;
		}
	case NhlLEFT:
		switch (anno_rec->just) {
		case NhlTOPLEFT:
		case NhlTOPCENTER:
		case NhlTOPRIGHT:
			return NhlTOPRIGHT;
		case NhlCENTERLEFT:
		case NhlCENTERCENTER:
		case NhlCENTERRIGHT:
			return NhlCENTERRIGHT;
		case NhlBOTTOMLEFT:
		case NhlBOTTOMCENTER:
		case NhlBOTTOMRIGHT:
			return NhlBOTTOMRIGHT;
		}
	case NhlRIGHT:
		switch (anno_rec->just) {
		case NhlTOPLEFT:
		case NhlTOPCENTER:
		case NhlTOPRIGHT:
			return NhlTOPLEFT;
		case NhlCENTERLEFT:
		case NhlCENTERCENTER:
		case NhlCENTERRIGHT:
			return NhlCENTERLEFT;
		case NhlBOTTOMLEFT:
		case NhlBOTTOMCENTER:
		case NhlBOTTOMRIGHT:
			return NhlBOTTOMLEFT;
		}
	case NhlBOTH:
	case NhlCENTER:
	default:
		break;
	}
	return (NhlJustification) NhlFATAL;

}

/*
 * Function:	SetTextPosition
 *
 * Description: Sets the text positional attribute fields in label strings
 *		when they are not members of an overlay, and therefore do
 *		not have an associated annotation object.
 *
 * In Args:	
 *		
 *		
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static NhlErrorTypes SetTextPosition
#if  __STDC__
(
	NhlContourLayer		cnnew,
	NhlContourLayerPart	*ocnp,
	_cnAnnoType		atype,
	NhlBoolean		*pos_changed,
	NhlString		entry_name
)
#else 
(cnnew,ocnp,atype,text_changed,entry_name)
	NhlContourLayer		cnnew;
	NhlContourLayerPart	*ocnp;
	_cnAnnoType		atype;
	NhlBoolean		*pos_changed;
	NhlString		entry_name;
#endif
{
	char			*e_text;
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourLayerPart	*cnp = &(cnnew->contour);
	NhlAnnotationRec	*anno_rec;
	NhlcnLabelAttrs		*lap;
	NhlcnLabelAttrs		*olap;
	char			*res_prefix;
	float			width_vp, height_vp;

	if (atype == _cnINFO) {
		anno_rec = &cnp->info_lbl_rec;
		lap = &cnp->info_lbl;
		olap = &ocnp->info_lbl;
		res_prefix = "NhlNcnInfoLabel";
	}
	else {
		anno_rec = &cnp->constf_lbl_rec;
		lap = &cnp->constf_lbl;
		olap = &ocnp->constf_lbl;
		res_prefix = "NhlNcnConstFLabel";
	}

	switch (anno_rec->side) {
	case NhlBOTTOM:
		lap->x_pos = cnnew->view.x + 
			anno_rec->para_pos * cnnew->view.width;
		lap->y_pos = cnnew->view.y - cnnew->view.height - 
			anno_rec->ortho_pos * cnnew->view.height;
		break;
	case NhlTOP:
		lap->x_pos = cnnew->view.x + 
			anno_rec->para_pos * cnnew->view.width;
		lap->y_pos = cnnew->view.y + 
			anno_rec->ortho_pos * cnnew->view.height;
		break;
	case NhlLEFT:
		lap->x_pos = cnnew->view.x - 
			anno_rec->ortho_pos * cnnew->view.width;
		lap->y_pos = cnnew->view.y - cnnew->view.height +
			anno_rec->para_pos * cnnew->view.height;
		break;
	case NhlRIGHT:
		lap->x_pos = cnnew->view.x + cnnew->view.width + 
			anno_rec->ortho_pos * cnnew->view.width;
		lap->y_pos = cnnew->view.y - cnnew->view.height +
			anno_rec->para_pos * cnnew->view.height;
		break;
	default:
		e_text = "%s: internal enumeration error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text, entry_name);
		return(ret);
	}
	if (anno_rec->just < NhlTOPLEFT || anno_rec->just > NhlTOPRIGHT) {
		e_text = "%s: internal enumeration error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text, entry_name);
		return(ret);
	}
	if (anno_rec->zone > 0)
		lap->just = ConstrainJustification(anno_rec);

/*
 * Adjust the annotation position based on the justification value
 */
	switch (lap->just) {
	case NhlTOPLEFT:
		break;
	case NhlTOPCENTER:
		lap->x_pos = lap->x_pos - width_vp / 2.0;
		break;
	case NhlTOPRIGHT:
		lap->x_pos = lap->x_pos - width_vp;
		break;
	case NhlCENTERLEFT:
		lap->y_pos = lap->y_pos + height_vp / 2.0;
		break;
	case NhlCENTERCENTER:
		lap->x_pos = lap->x_pos - width_vp / 2.0;
		lap->y_pos = lap->y_pos + height_vp / 2.0;
		break;
	case NhlCENTERRIGHT:
		lap->x_pos = lap->x_pos - width_vp;
		lap->y_pos = lap->y_pos + height_vp / 2.0;
		break;
	case NhlBOTTOMLEFT:
		lap->y_pos = lap->y_pos + height_vp;
		break;
	case NhlBOTTOMCENTER:
		lap->x_pos = lap->x_pos - width_vp / 2.0;
		lap->y_pos = lap->y_pos + height_vp;
		break;
	case NhlBOTTOMRIGHT:
		lap->y_pos = lap->y_pos + height_vp;
		lap->x_pos = lap->x_pos - width_vp;
		break;
	}

	return ret;
}

/*
 * Function:  ManageData
 *
 * Description: Handles updating of data dependent resources
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    ManageData
#if __STDC__
(
	NhlContourLayer	cnnew, 
	NhlContourLayer	cnold,
	NhlBoolean	init,
	_NhlArgList	args,
	int		num_args
)
#else
(cnnew,cnold,init,args,num_args)
	NhlContourLayer	cnnew;
	NhlContourLayer	cnold;
	NhlBoolean	init;
	_NhlArgList	args;
	int		num_args;
#endif

{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*entry_name;
	char			*e_text;
	NhlContourLayerPart	*cnp = &cnnew->contour;
	NhlContourLayerPart	*ocnp = &cnold->contour;
	NhlScalarFieldFloatLayer	sfl;
	_NhlDataNodePtr			*dlist = NULL;
	NhlBoolean			new;
	int				ndata = 0;

	if (! cnp->data_changed && 
	    (cnp->scalar_field_data == ocnp->scalar_field_data)) 
		return NhlNOERROR;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

	if (cnp->scalar_field_data != NULL)
		ndata = _NhlGetDataInfo(cnp->scalar_field_data,&dlist);
	if (ndata <= 0) {
		cnp->zmin = 0.0;
		cnp->zmax = 1.0;
		cnp->data_init = False;
		cnp->sfp = NULL;
		return NhlNOERROR;
	}
	else if (ndata != 1) {
		cnp->data_init = False;
		e_text = "%s: internal error retrieving data info";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}

 	sfl = (NhlScalarFieldFloatLayer) _NhlGetDataSet(dlist[0],&new);
	if (sfl == NULL) {
		cnp->data_init = False;
		e_text = "%s: internal error retrieving data set";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return NhlFATAL;
	}
	cnp->sfp = (NhlScalarFieldFloatLayerPart *) &sfl->sfieldfloat;

	cnp->zmin = cnp->sfp->data_min;
	cnp->zmax = cnp->sfp->data_max;
	cnp->const_field = _NhlCmpFAny(cnp->zmax,cnp->zmin,8) <= 0.0 ?
		True : False;
	if (cnp->const_field) {
		e_text = 
		     "%s: scalar field is constant; no Contour plot possible";
		NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
		ret = MIN(NhlWARNING,ret);
	}

	cnp->data_init = True;
	cnp->data_changed = True;
	cnp->use_irr_trans = (cnp->sfp->x_arr == NULL &&
			      cnp->sfp->y_arr == NULL) ? False : True;

	return ret;
}

/*
 * Function:  ManageViewDepResources
 *
 * Description: Modifies resources that may need to change when the view
 *	is modified.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    ManageViewDepResources
#if __STDC__
	(NhlLayer	new, 
	NhlLayer	old,
	NhlBoolean	init,
	_NhlArgList	args,
	int		num_args)
#else
(new,old,init,args,num_args)
	NhlLayer	new;
	NhlLayer	old;
	NhlBoolean	init;
	_NhlArgList	args;
	int		num_args;
#endif

{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*entry_name;
	NhlContourLayer		cnew = (NhlContourLayer) new;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	NhlContourLayer		cold = (NhlContourLayer) old;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

/* Adjust line dash segment length */

	if (! cnp->line_dash_seglen_set) {
		if (init) {
			cnp->line_dash_seglen *= 
				cnew->view.width / Nhl_cnSTD_VIEW_WIDTH;
		}
		else if (cnew->view.width != cold->view.width) {
			cnp->line_dash_seglen *= 
				cnew->view.width / cold->view.width;
		}
	}

	subret = AdjustText(&cnp->line_lbls,cnew,cold,init);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	if (cnp->high_use_line_attrs && cnp->line_lbls.on) {
		memcpy(&cnp->high_lbls,
		       &cnp->line_lbls,sizeof(NhlcnLabelAttrs));
	}
	else {
		subret = AdjustText(&cnp->high_lbls,cnew,cold,init);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}
		
	if (cnp->low_use_high_attrs && cnp->high_lbls.on) {
		memcpy(&cnp->low_lbls,
		       &cnp->high_lbls,sizeof(NhlcnLabelAttrs));
	}
	else {
		subret = AdjustText(&cnp->low_lbls,cnew,cold,init);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}

	subret = AdjustText(&cnp->info_lbl,cnew,cold,init);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	if (cnp->constf_use_info_attrs && cnp->info_lbl.on) {
		memcpy(&cnp->constf_lbl,
		       &cnp->info_lbl,sizeof(NhlcnLabelAttrs));
	}
	else {
		subret = AdjustText(&cnp->constf_lbl,cnew,cold,init);
		if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;
	}

	return ret;
}

/*
 * Function:  AdjustText
 *
 * Description: Adjusts the text height and aspect ratio
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    AdjustText
#if __STDC__
(
	NhlcnLabelAttrs *lbl_attrp,
	NhlContourLayer	new, 
	NhlContourLayer	old,
	NhlBoolean	init
)
#else
(lbl_attrp,new,old,init)
	NhlcnLabelAttrs *lbl_attrp;
	NhlLayer	new;
	NhlLayer	old;
	NhlBoolean	init;
#endif

{
	NhlErrorTypes		ret = NhlNOERROR;
	char 			*e_text;
	char			*entry_name;
	NhlContourLayer		cnew = (NhlContourLayer) new;
	NhlContourLayer		cold = (NhlContourLayer) old;

	entry_name = (init) ? "ContourInitialize" : "ContourSetValues";

/* 
 * Adjust text height. Then determine principal width and height
 * and the "real " text height based on aspect ratio. 21.0 is the default 
 * principle height. This code handles aspect ratio like the TextItem.
 */

	if (! lbl_attrp->height_set) {
		if (init) {
			lbl_attrp->height *= 
				cnew->view.width / Nhl_cnSTD_VIEW_WIDTH;
		}
		else if (cnew->view.width != cold->view.width) {
			lbl_attrp->height *= 
				cnew->view.width / cold->view.width;
		}
	}

        if (lbl_attrp->aspect <= 0.0 ) {
		e_text = "%s: Invalid value for text aspect ratio %d";
                NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
			  entry_name,lbl_attrp->aspect);
                ret = NhlWARNING;
                lbl_attrp->aspect = 1.3125;
        }
        if (lbl_attrp->aspect <= 1.0) {
                lbl_attrp->pheight = 21.0 * lbl_attrp->aspect;
                lbl_attrp->pwidth = 21.0;
        } else {
                lbl_attrp->pwidth = 21.0 * 1.0/lbl_attrp->aspect;
                lbl_attrp->pheight = 21.0;
        }
        lbl_attrp->real_height = 
		1.0 / lbl_attrp->aspect * lbl_attrp->height;

	return ret;
}
/*
 * Function:  ManageDynamicArrays
 *
 * Description: Creates and manages internal copies of each of the 
 *	Contour GenArrays. Populates the copies with the values specified 
 *	via ContourCreate or ContourSetValues calls. Assigns default 
 *	values and sizes to any array resource not set by the caller.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    ManageDynamicArrays
#if __STDC__
	(NhlLayer		new, 
	NhlLayer		old,
	NhlBoolean	init,
	_NhlArgList	args,
	int		num_args)
#else
(new,old,init,args,num_args)
	NhlLayer		new;
	NhlLayer		old;
	NhlBoolean	init;
	_NhlArgList	args;
	int		num_args;
#endif

{
	NhlContourLayer	cnew = (NhlContourLayer) new;
	NhlContourLayerPart *cnp = &(cnew->contour);
	NhlContourLayer	cold = (NhlContourLayer) old;
	NhlContourLayerPart *ocnp = &(cold->contour);
	NhlErrorTypes ret = NhlNOERROR, subret = NhlNOERROR;
	int i,count;
	NhlGenArray ga;
	char *entry_name;
	char *e_text;
	int *ip;
	float *fp;
	float fval;
	int	init_count;
	NhlBoolean need_check,changed;
	int old_count;
	float *levels = NULL;
	NhlBoolean levels_modified = False, flags_modified = False;

	entry_name =  init ? "ContourInitialize" : "ContourSetValues";

/* 
 * If constant field don't bother setting up the arrays: they will not
 * be used
 */
	if (cnp->const_field) return NhlNOERROR;

/* Determine the contour level state */

	subret = SetupLevels(new,old,init,&levels,&levels_modified);
	if ((ret = MIN(ret,subret)) < NhlWARNING) {
		e_text = "%s: error getting contour level information";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	count = cnp->level_count;

/*=======================================================================*/

/* 
 * The levels array 
 */
	ga = init ? NULL : ocnp->levels;
	subret = ManageGenArray(&ga,count,cnp->levels,Qfloat,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLevels,entry_name);

	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->levels = changed ? NULL : cnp->levels;
	cnp->levels = ga;
	if (levels_modified) {
		if (levels == NULL) {
			e_text = "%s: internal error getting levels";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		NhlFree(cnp->levels->data);
		cnp->levels->data = (NhlPointer) levels;
	}
		

/*=======================================================================*/

/*
 * Level flags
 */

	flags_modified = 
		(ga = init ? NULL : ocnp->level_flags) != cnp->level_flags;
	subret = ManageGenArray(&ga,count,cnp->level_flags,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLevelFlags,entry_name);

	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->level_flags = changed ? NULL : cnp->level_flags;
	cnp->level_flags = ga;

	ip = (int *) cnp->level_flags->data;
	if (! flags_modified && 
	    (levels_modified || cnp->llabel_interval_set)) {
		flags_modified = True;
		if (cnp->llabel_interval <= 0) {
			for (i = 0; i < count; i++) 
				ip[i] = Nhl_cnLINEONLY;
		}
		else {
			for (i = 0; i < count; i++)
				ip[i] = (i+1) % cnp->llabel_interval == 0 ?
					Nhl_cnLINEANDLABEL : Nhl_cnLINEONLY;
		}
	}
	else if (need_check) {
		flags_modified = True;
		if (cnp->llabel_interval <= 0) {
			for (i = init_count; i < count; i++) 
				ip[i] = Nhl_cnLINEONLY;
		}
		else {
			for (i = init_count; i < count; i++)
				ip[i] = (i+1) % cnp->llabel_interval == 0 ?
					Nhl_cnLINEANDLABEL : Nhl_cnLINEONLY;
		}
		for (i=0; i<init_count; i++) {
			if (ip[i] < Nhl_cnNOLINE || 
			    ip[i] > Nhl_cnLINEANDLABEL) {
				e_text =
	      "%s: %s index %d contains an invalid level flag, %d: defaulting";
				NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
					  entry_name,NhlNcnLevelFlags,i,ip[i]);
				ret = MIN(ret, NhlWARNING);
				ip[i] = Nhl_cnLINEONLY;
			}
		}
	}

			
/*=======================================================================*/
	
/*
 * Fill colors
 */

	if (init) {
		ga = NULL;
		count = cnp->fill_count;
	}
	else {
		ga = ocnp->fill_colors;
		count = cnp->mono_fill_color ? 1 : cnp->fill_count;
	}
	subret = ManageGenArray(&ga,count,cnp->fill_colors,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnFillColors,entry_name);

	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;
	ocnp->fill_colors = changed ? NULL : cnp->fill_colors;
	cnp->fill_colors = ga;

	if (need_check) {
		subret = CheckColorArray(cnew,ga,count,init_count,old_count,
					 &cnp->gks_fill_colors,
					 NhlNcnFillColors, entry_name);
		if ((ret = MIN(ret,subret)) < NhlWARNING)
			return ret;
	}
/*=======================================================================*/
	
/*
 * Fill patterns
 */

	if (init) {
		ga = NULL;
		count = cnp->fill_count;
	}
	else {
		ga = ocnp->fill_patterns;
		count = cnp->mono_fill_pattern ? 1 : cnp->fill_count;
	}
	if (ga != cnp->fill_patterns) cnp->new_draw_req = True;
	subret = ManageGenArray(&ga,count,cnp->fill_patterns,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnFillPatterns,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;
	ocnp->fill_patterns = changed ? NULL : cnp->fill_patterns;
	cnp->fill_patterns = ga;

	if (need_check) {
		ip = (int *) ga->data;
		for (i=init_count; i < count; i++) {
			ip[i] = i + 1;
		}
		for (i=0; i<init_count; i++) {
			if (ip[i] < NhlHOLLOWFILL) {
				e_text =
	      "%s: %s index %d holds an invalid pattern value, %d: defaulting";
				NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
					  entry_name,
					  NhlNcnFillPatterns,i,ip[i]);
				ret = MIN(ret, NhlWARNING);
				ip[i] = NhlHOLLOWFILL;
			}
		}
	}
	
/*=======================================================================*/
	
/*
 * Fill scales
 */

	if (init) {
		ga = NULL;
		count = cnp->level_count;
	}
	else {
		ga = ocnp->fill_scales;
		count = cnp->mono_fill_scale ? 1 : cnp->level_count;
	}
	fval = 1.0;
	subret = ManageGenArray(&ga,count,cnp->fill_scales,Qfloat,&fval,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnFillScales,entry_name);
	
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->fill_scales = changed ? NULL : cnp->fill_scales;
	cnp->fill_scales = ga;
	
	if (need_check) {
		fp = (float *) ga->data;
		for (i=0; i<count; i++) {
			if (fp[i] <= 0.0) {
				e_text =
	            "%s: %s index %d holds an invalid fill scale: defaulting";
				NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
					  entry_name,NhlNcnFillScales,i);
				ret = MIN(ret, NhlWARNING);
				fp[i] = 1.0;
			}
		}
	}

/*=======================================================================*/
	
/*
 * Line colors
 */

	if (init) {
		ga = NULL;
		count = cnp->level_count;
	}
	else {
		ga = ocnp->line_colors;
		count = cnp->mono_line_color ? 1 : cnp->level_count;
	}
	subret = ManageGenArray(&ga,count,cnp->line_colors,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLineColors,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;
	ocnp->line_colors = changed ? NULL : cnp->line_colors;
	cnp->line_colors = ga;

		
	if (need_check) {
		subret = CheckColorArray(cnew,ga,count,init_count,old_count,
					 &cnp->gks_line_colors,
					 NhlNcnLineColors,entry_name);
		if ((ret = MIN(ret,subret)) < NhlWARNING)
			return ret;
	}
/*=======================================================================*/

/*
 * Line dash patterns
 * Since Conpack needs to know the actual strings, it is necessary to 
 * get a copy of the dash table.
 */
		
	if (init) {
		subret = NhlVAGetValues(cnew->base.wkptr->base.id,
				      NhlNwkDashTable, &ga, NULL);
		if ((ret = MIN(ret,subret)) < NhlWARNING) {
			e_text = "%s: NhlFATAL error retrieving dash table";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			return ret;
		}
		cnp->dash_table = ga;
		Dash_Table_Len = ga->num_elements;
		Dash_Table = (NhlString *) ga->data;

		ga = NULL;
		count = cnp->level_count;
	}
	else {
		ga = ocnp->line_dash_patterns;
		count = cnp->mono_line_dash_pattern ? 1 : cnp->level_count;
	}
	subret = ManageGenArray(&ga,count,cnp->line_dash_patterns,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLineDashPatterns,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->line_dash_patterns = changed ? NULL : cnp->line_dash_patterns;
	cnp->line_dash_patterns = ga;

	if (need_check) {
		ip = (int *) ga->data;
		for (i=init_count; i < count; i++) {
			ip[i] = i;
		}
		for (i=0; i<init_count; i++) {
			if (ip[i] < 1) {
				e_text =
			 "%s: %s index %d has invalid value: %d: defaulting";
				NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
					  entry_name,
					  NhlNcnLineDashPatterns,i,ip[i]);
				ret = MIN(ret, NhlWARNING);
				ip[i] = Nhl_cnDEF_DASH_PATTERN;
			}
		}
	}

/*=======================================================================*/
	
/*
 * Line thicknesses
 */

	if (init) {
		ga = NULL;
		count = cnp->level_count;
	}
	else {
		ga = ocnp->line_thicknesses;
		count = cnp->mono_line_thickness ? 1 : cnp->level_count;
	}
	fval = 1.0;
	subret = ManageGenArray(&ga,count,cnp->line_thicknesses,Qfloat,&fval,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLineThicknesses,entry_name);
	
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->line_thicknesses = changed ? NULL : cnp->line_thicknesses;
	cnp->line_thicknesses = ga;
	
	if (need_check) {
		fp = (float *) ga->data;
		for (i=0; i<count; i++) {
			if (fp[i] <= 0.0) {
				e_text =
	        "%s: %s index %d holds an invalid line thickness: defaulting";
				NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
					  entry_name,NhlNcnLineThicknesses,i);
				ret = MIN(ret, NhlWARNING);
				fp[i] = 1.0;
			}
		}
	}

/*=======================================================================*/
	
/*
 * Line Label strings
 * Need both the user accessible array that holds all the Line Label strings,
 * and a private array containing pointers to the strings that get drawn,
 * for the benefit of Legend. Since this GenArray only allocates the 
 * pointer array, not the strings, it will need to be freed specially.
 */
	count = cnp->level_count;
	ga = init ? NULL : ocnp->llabel_strings;
	subret = ManageGenArray(&ga,count,cnp->llabel_strings,
				Qstring,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLineLabelStrings,entry_name);

	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

	ocnp->llabel_strings = changed ? NULL : cnp->llabel_strings;
	cnp->llabel_strings = ga;

	if (levels_modified || need_check) {
		NhlString *sp = (NhlString *) ga->data;
		NhlBoolean modified = False;

		fp = (float *) cnp->levels->data;
		init_count = levels_modified && 
			cnp->llabel_strings == ocnp->llabel_strings ?
				0 : init_count;

		for (i=init_count; i<count; i++) {
			if (sp[i] != NULL) NhlFree(sp[i]);
			if ((sp[i] = (char *) 
			     NhlMalloc(7 * sizeof(char))) == NULL) {
				e_text = "%s: dynamic memory allocation error";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				return NhlFATAL;
			}
			sprintf(sp[i],"%6.2f",fp[i]);
			sp[i][6] = '\0';
			modified = True;
		}
		if (modified) ocnp->llabel_strings = NULL;
	}
	if (flags_modified && init) {
		NhlString *sp;

		if ((sp = (NhlString *) 
		     NhlMalloc(count * sizeof(NhlString))) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		if ((cnp->ll_strings = 
		     NhlCreateGenArray((NhlPointer)sp,NhlTString,
				       sizeof(NhlString),1,&count)) == NULL) {
			e_text = "%s: error creating GenArray";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
		cnp->ll_strings->my_data = False;
	}
	if (flags_modified || need_check) {
		NhlString *sp = cnp->ll_strings->data;
		NhlString *llsp = cnp->llabel_strings->data;

		if (count > cnp->ll_strings->num_elements) {
			if ((sp = (NhlString *) 
			     NhlRealloc(sp, 
					count * sizeof(NhlString))) == NULL) {
				e_text = "%s: dynamic memory allocation error";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				return NhlFATAL;
			}
			cnp->ll_strings->data = (void *) sp;
			cnp->ll_strings->num_elements = count;
		}
		ip = (int *) cnp->level_flags->data;
		for (i = 0; i < count; i++) {
			if (ip[i] < Nhl_cnLABELONLY) {
				sp[i] = NULL;
			}
			else {
				sp[i] = llsp[i];
			}
		}
	}
				

/*=======================================================================*/
	
/*
 * Line Label colors
 */

	if (init) {
		ga = NULL;
		count = cnp->level_count;
	}
	else {
		ga = ocnp->llabel_colors;
		count = cnp->line_lbls.mono_color ? 1 : cnp->level_count;
	}
	subret = ManageGenArray(&ga,count,cnp->llabel_colors,Qint,NULL,
				&old_count,&init_count,&need_check,&changed,
				NhlNcnLineLabelColors,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;
	ocnp->llabel_colors = changed ? NULL : cnp->llabel_colors;
	cnp->llabel_colors = ga;


	if (need_check) {
		subret = CheckColorArray(cnew,ga,count,init_count,old_count,
					 &cnp->gks_llabel_colors,
					 NhlNcnLineLabelColors,entry_name);
		if ((ret = MIN(ret,subret)) < NhlWARNING)
			return ret;
	}

	if (cnp->high_use_line_attrs && cnp->line_lbls.on) {
		cnp->high_lbls.mono_color = 
			((int *)cnp->llabel_colors->data)[0];
	}
	if (cnp->low_use_high_attrs && cnp->high_lbls.on) {
		cnp->low_lbls.mono_color = cnp->low_lbls.mono_color;
	}
	

/*=======================================================================*/

/*
 * Test for changes that require a new draw (when in segment mode)
 */
	if (flags_modified ||
	    cnp->fill_colors != ocnp->fill_colors ||
	    cnp->mono_fill_color != ocnp->mono_fill_color ||
	    cnp->fill_patterns != ocnp->fill_patterns ||
	    cnp->mono_fill_pattern != ocnp->mono_fill_pattern ||
	    cnp->fill_scales != ocnp->fill_scales ||
	    cnp->mono_fill_scale != ocnp->mono_fill_scale ||
	    cnp->line_colors != ocnp->line_colors ||
	    cnp->mono_line_color != ocnp->mono_line_color ||
	    cnp->line_dash_patterns != ocnp->line_dash_patterns ||
	    cnp->mono_line_dash_pattern != ocnp->mono_line_dash_pattern ||
	    cnp->line_thicknesses != ocnp->line_thicknesses ||
	    cnp->mono_line_thickness != ocnp->mono_line_thickness ||
	    cnp->llabel_strings != ocnp->llabel_strings ||
	    cnp->llabel_colors != ocnp->llabel_colors ||
	    cnp->line_lbls.mono_color != ocnp->line_lbls.mono_color) {
		cnp->new_draw_req = True;
	}

	return ret;
}

/*
 * Function:    ManageGenArray
 *
 * Description:	Handles details of managing a GenArray
 *
 * In Args:	count		number of elements to create in the GenArray
 *		copy_ga 	GenArray to copy values from - if
 *				NULL it is ignored.
 *		type		type of GenArray to create - int,float,string
 *		*init_val	if non-null an initialization value to use -
 *				strings have the array index appended
 *		resource_name	name of the GenArray resource 		
 *		entry_name	name of the high level caller of the routine 
 *
 * Out Args:	*ga		If non-NULL on input, contains a previously
 *				allocated GenArray, whose data will be 
 *				replaced if necessary.
 *				Out: An allocated GenArray with allocated data
 *		*old_count	the previous count in the old gen array
 *		*init_count	number of values initialized - if init_val is
 *				non-NULL, will contain count; if init_val is
 *				NULL will contain MIN(count,number of 
 *				elements in copy_ga); if copy_ga is also NULL
 *				will contain 0.
 *		*need_check     True if a GenArray copy occurs or the number
 *				of elements increases and no initialization
 *				value is supplied. False otherwise.
 *		*changed	True if the data has been modified in any way.
 *
 *
 * Return Values:
 *
 * Side Effects: The internal copy of each GenArray is modified to reflect
 *	changes requested via ContourSetValues
 */

/*ARGSUSED*/
static NhlErrorTypes    ManageGenArray
#if __STDC__
	(NhlGenArray	*ga,
	 int		count,
	 NhlGenArray	copy_ga,
	 NrmQuark	type,
	 NhlPointer	init_val,
	 int		*old_count,
	 int		*init_count,
	 NhlBoolean	*need_check,
	 NhlBoolean	*changed,
	 NhlString	resource_name,
	 NhlString	entry_name)
#else
(ga,count,copy_ga,type,init_val,old_count,init_count,
 need_check,changed,resource_name,entry_name)
	NhlGenArray	*ga;
	int		count;
	NhlGenArray	copy_ga;
	NrmQuark	type;
	NhlPointer	init_val;
	int		*old_count;
	int		*init_count;
	NhlBoolean	*need_check;
	NhlBoolean	*changed;
	NhlString	resource_name;
	NhlString	entry_name;
#endif
{
	char		*str_type;
	NhlErrorTypes	ret = NhlNOERROR;
	int		i, size;
	NhlPointer	datap;
	char		*e_text;

	*init_count = 0;
	*need_check = False;
	*changed = False;
	*old_count = 0;

	if (type == Qint) {
		str_type = NhlTInteger;
		size = sizeof(int);
	}
	else if (type == Qfloat) {
		str_type = NhlTFloat;
		size = sizeof(float);
	}
	else if (type == Qstring) {
		str_type = NhlTString;
		size = sizeof(NhlString);
	}
	else {
		e_text = "%s: internal error; unsupported type for %s";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
			  resource_name);
		return NhlFATAL;
	}

	if (*ga != NULL) {
		datap = (*ga)->data;
		*old_count = (*ga)->num_elements;

		if (count > (*ga)->num_elements) {
			if ((datap = (NhlPointer)
			     NhlRealloc(datap, count * size)) == NULL) {
				e_text = "%s: error reallocating %s data";
				NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,
					  entry_name,resource_name);
				return NhlFATAL;
			}
			(*ga)->data = datap;
			(*ga)->num_elements = count;
			*changed = True;
		}
		else if (*ga == copy_ga) {
			*init_count = (*ga)->num_elements;
			return ret;
		}
	}
	else {
		if ((datap = (NhlPointer) NhlMalloc(count * size)) == NULL) {
			e_text = "%s: error creating %s array";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
				  resource_name);
			return NhlFATAL;
		}

		if ((*ga = NhlCreateGenArray((NhlPointer)datap,str_type,
					     size,1,&count)) == NULL) {
			e_text = "%s: error creating %s GenArray";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,
				  resource_name);
			return NhlFATAL;
		}
		(*ga)->my_data = True;
		*changed = True;
	}

/* 
 * If there is a GenArray to copy, copy it; then initialize all remaining
 * uninitialized elements if an initialization value has been passed in.
 */

	if (copy_ga != NULL && copy_ga != *ga) {

		*need_check = True;
		ret = _NhlValidatedGenArrayCopy(ga,copy_ga,Nhl_cnMAX_LEVELS,
						True,False,resource_name, 
						entry_name);
		if (ret < NhlWARNING) {
			e_text = "%s: error copying %s GenArray";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name,
				  resource_name);
			return ret;
		}
		*init_count = copy_ga->num_elements;
		*changed = True;
	}

	if (*init_count < count) {

		if (init_val == NULL) {
			if (type == Qstring) {
				NhlString *sp = (NhlString *) datap;
				for (i = *init_count; i< count; i++) {
					if (i < *old_count) NhlFree(sp[i]);
					sp[i] = NULL;
				}
			}
			*need_check = True;
			return ret;
		}
		else if (type == Qint)
			for (i = *init_count; i< count; i++)
				((int *)datap)[i] = *((int *)init_val);
		else if (type == Qfloat)
			for (i = *init_count; i< count; i++)
				((float *)datap)[i] = *((float *)init_val);
		else if (type == Qstring) {
			char *sp;
			char *init_str = (char *) init_val;
			char numstr[10];
			for (i = *init_count; i< count; i++) {
				sprintf(numstr,"%d",i);
				if ((sp = (char *) 
				     NhlMalloc(sizeof(init_str)+
					       sizeof(numstr)+1)) == NULL) {
					e_text = "%s: error creating %s array";
					NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,
						  entry_name,resource_name);
					return NhlFATAL;
				}
				((char **)datap)[i] = sp;
				strcpy(sp,init_str);
				strcat(sp,numstr);
			}
		}
		*init_count = count;
		*changed = True;
	}

	return ret;
}


/*
 * Function:    CheckColorArray
 *
 * Description:	Checks color array values for validity, initializing any
 *		currently uninitialized values and keeps the GKS index
 *		arrays up-to-date.
 *
 * In Args:	count		number of elements to create in the GenArray
 *		resource_name	name of the GenArray resource 		
 *		entry_name	name of the high level caller of the routine 
 *
 * Out Args:	*ga		If non-NULL on input, contains a previously
 *				allocated GenArray, whose data will be 
 *				replaced if necessary.
 *				Out: An allocated GenArray with allocated data
 *		*init_count	number of values initialized - if init_val is
 *				non-NULL, will contain count; if init_val is
 *				NULL will contain MIN(count,number of 
 *				elements in copy_ga); if copy_ga is also NULL
 *				will contain 0.
 *
 * Return Values:
 *
 * Side Effects: The internal copy of each GenArray is modified to reflect
 *	changes requested via ContourSetValues
 */

/*ARGSUSED*/

static NhlErrorTypes	CheckColorArray
#if __STDC__
	(NhlContourLayer	cl,
	NhlGenArray	ga,
	int		count,
	int		init_count,
	int		last_count,
	int		**gks_colors,
	NhlString	resource_name,
	NhlString	entry_name)
#else
(cl,ga,count,init_count,last_count,gks_colors,resource_name,entry_name)
	NhlContourLayer	cl;
	NhlGenArray	ga;
	int		count;
	int		init_count;
	int		last_count;
	int		**gks_colors;
	NhlString	resource_name;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes ret = NhlNOERROR;
	char *e_text;
	int *ip;
	int i, len, spacing;
	

	ip = (int *) ga->data;
	NhlVAGetValues(cl->base.wkptr->base.id,
		     NhlNwkColorMapLen, &len, NULL);
	
	spacing = MAX(len / count, 1); 
	for (i=init_count; i < count; i++) 
		ip[i] = 1 + i * spacing;

	if (last_count == 0) {
		if ((*gks_colors = 
		     (int *) NhlMalloc(count * sizeof(int))) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
	}
	else if (count > last_count) {
		if ((*gks_colors = 
		     (int *) NhlRealloc(*gks_colors,
					count * sizeof(int))) == NULL) {
			e_text = "%s: dynamic memory allocation error";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
	}

	for (i=0; i<count; i++) {
		(*gks_colors)[i] =
			_NhlGetGksCi(cl->base.wkptr,ip[i]);
	}
	return ret;
}



/*
 * Function:  SetupLevels
 *
 * Description: Depending on the setting of the LevelCount resource,
 *		decides whether to allow Conpack to determine the 
 *		number of Contour levels. If so, makes the appropriate
 *		Contour calls.
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    SetupLevels
#if __STDC__
	(NhlLayer		new, 
	 NhlLayer		old,
	 NhlBoolean	init,
	 float		**levels,
	 NhlBoolean	*modified)
#else
(new,old,init,levels,modified)
	NhlLayer		new;
	NhlLayer		old;
	NhlBoolean	init;
	float		**levels;
	NhlBoolean	*modified;

#endif

{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	char			*entry_name;
	NhlContourLayer		cnew = (NhlContourLayer) new;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	NhlContourLayer		cold = (NhlContourLayer) old;
	NhlContourLayerPart	*ocnp = &(cold->contour);

	entry_name = init ? "ContourInitialize" : "ContourSetValues";
	*modified = False;

	if (! cnp->min_level_set) cnp->min_level_val = cnp->zmin; 
	if (! cnp->max_level_set) cnp->max_level_val = cnp->zmax; 
		
	if (init || cnp->data_changed ||
	    cnp->levels != ocnp->levels ||
	    cnp->level_selection_mode != ocnp->level_selection_mode ||
	    cnp->max_level_count != ocnp->max_level_count ||
	    cnp->level_spacing != ocnp->level_spacing ||
	    cnp->min_level_set || cnp->max_level_set) {

		cnp->new_draw_req = True;
		if (cnp->min_level_val >= cnp->zmax) {
			if (! cnp->data_changed) {
				ret = MIN(NhlWARNING,ret);
				e_text =
     "%s: Set minimum level exceeds or equals data maximum value: defaulting";
				NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			}
			cnp->min_level_val = cnp->zmin;
		}
		if (cnp->max_level_val <= cnp->zmin) {
			if (! cnp->data_changed) {
				ret = MIN(NhlWARNING,ret);
				e_text =
  "%s: Set maximum level less than or equal to data mimimum value: defaulting";
				NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			}
			cnp->max_level_val = cnp->zmax;
		}

		switch (cnp->level_selection_mode) {

		case Nhl_cnMANUAL:

			subret = SetupLevelsManual(cnew,cold,
						   levels,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				return ret;
			}
			*modified = True;
			break;

		case Nhl_cnEQUALSPACING:

			subret = SetupLevelsEqual(cnew,cold,
						  levels,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				return ret;
			}
			*modified = True;
			break;

		case Nhl_cnAUTOMATIC:

			subret = SetupLevelsAutomatic(cnew,cold,
						      levels,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				return ret;
			}
			*modified = True;
			break;
			
		case Nhl_cnEXPLICIT:

			subret = SetupLevelsExplicit(cnew,cold,
						     levels,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				return ret;
			}
			*modified = True;
			break;

		default:
			ret = NhlFATAL;
			e_text = "%s: Invalid level selection mode";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			return NhlFATAL;
		}
	}

	return ret;

}

/*
 * Function:  SetupLevelsManual
 *
 * Description: Sets up Manual mode levels
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    SetupLevelsManual
#if __STDC__
	(NhlContourLayer	cnew, 
	 NhlContourLayer	cold,
	 float		**levels,
	 char		*entry_name)
#else
(cnew,cold,levels,entry_name)
	NhlContourLayer	cnew;
	NhlContourLayer	cold;
	float		**levels;
	char		*entry_name;

#endif

{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	int			i, count;
	float			zmin,zmax;
	float			*fp;

	zmax = cnp->max_level_val;
	zmin = cnp->min_level_val;
	count = (zmax - zmin) / cnp->level_spacing + 1.001;

	if (count > cnp->max_level_count) {
		ret = MIN(NhlWARNING,ret);
		e_text =
      "%s: Level max count exceeded with specified level spacing: defaulting";
		NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
		subret = ChooseSpacingLin(&zmin,&zmax,&cnp->level_spacing,7,
					  cnp->max_level_count,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error choosing spacing";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			return ret;
		}
		count = (zmax - zmin) / cnp->level_spacing + 1.001;
	}

	if ((*levels = (float *) 
	     NhlMalloc(count * sizeof(float))) == NULL) {
		e_text = "%s: dynamic memory allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	for (i=0, fp = *levels; i<count; i++) {
		*(fp++) = zmin + i * cnp->level_spacing;
	}

	cnp->level_count = count;
	cnp->fill_count = (*levels)[count-1] < zmin ? count + 1 : count;

	return ret;
}

/*
 * Function:  SetupLevelsEqual
 *
 * Description: Sets up Equally spaced levels
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    SetupLevelsEqual
#if __STDC__
	(NhlContourLayer	cnew,
	 NhlContourLayer	cold,
	 float		**levels,
	 char		*entry_name)
#else
(cnew,cold,levels,entry_name)
	NhlContourLayer	cnew;
	NhlContourLayer	cold;
	float		**levels;
	char		*entry_name;

#endif

{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	int			i;
	float			zmin,zmax,size;

	zmin = MAX(cnp->min_level_val, cnp->zmin);
	zmax = MIN(cnp->max_level_val, cnp->zmax);
	size = (zmax - zmin) / cnp->max_level_count;

	cnp->level_count = cnp->max_level_count - 1;
	if ((*levels = (float *) 
	     NhlMalloc(cnp->level_count * sizeof(float))) == NULL) {
		e_text = "%s: dynamic memory allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	for (i=0; i<cnp->level_count; i++) {
		(*levels)[i] = cnp->zmin + (i+1) * size;
	}

	cnp->fill_count = cnp->max_level_count;

	return ret;
}


/*
 * Function:  SetupLevelsAutomatic
 *
 * Description: Sets up Automatic mode levels
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    SetupLevelsAutomatic
#if __STDC__
	(NhlContourLayer	cnew, 
	 NhlContourLayer	cold,
	 float		**levels,
	 char		*entry_name)
#else
(cnew,cold,levels,entry_name)
	NhlContourLayer	cnew;
	NhlContourLayer	cold;
	float		**levels;
	char		*entry_name;

#endif

{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	int			i, count;
	float			zmin,zmax,spacing;

	zmin = MAX(cnp->min_level_val, cnp->zmin);
	zmax = MIN(cnp->max_level_val, cnp->zmax);

	subret = ChooseSpacingLin(&zmin,&zmax,&spacing,7,
				  cnp->max_level_count,entry_name);
	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		e_text = "%s: error choosing spacing";
		NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
		return ret;
	}

	count = (int)((zmax - zmin) / spacing + 1.001);
	if ((*levels = (float *) NhlMalloc(count * sizeof(float))) == NULL) {
		e_text = "%s: dynamic memory allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	for (i=0; i<count; i++) {
		(*levels)[i] = zmin + i * spacing;
	}

	cnp->level_count = count;
	cnp->level_spacing = spacing;
	cnp->fill_count = (*levels)[count-1] < zmax ? count + 1 : count;

	return ret;
}


/*
 * Function:  SetupLevelsExplicit
 *
 * Description: Sets up Explicit mode levels
 *
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static NhlErrorTypes    SetupLevelsExplicit
#if __STDC__
	(NhlContourLayer	cnew, 
	 NhlContourLayer	cold,
	 float		**levels,
	 char		*entry_name)
#else
(cnew,cold,levels,entry_name)
	NhlContourLayer	cnew;
	NhlContourLayer	cold;
	float		**levels;
	char		*entry_name;

#endif

{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourLayerPart	*cnp = &(cnew->contour);
	int			i, count, ixmin, ixmax;
	float			*fp;
	float			zmin, zmax, spacing;
	NhlBoolean		do_auto = False;

	zmin = MAX(cnp->min_level_val, cnp->zmin);
	zmax = MIN(cnp->max_level_val, cnp->zmax);

	if ((count = cnp->levels->num_elements) > cnp->max_level_count) {
		count = cnp->max_level_count;
		ret = MIN(NhlWARNING,ret);
		e_text = 
	  "%s: Explicit level array count exceeds max level count: defaulting";
		NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
	}
	fp = (float *)cnp->levels->data;

	ixmin = 0;
	if (fp[0] >= zmax) {
		do_auto = True;
		ret = MIN(NhlWARNING,ret);
		e_text = "%s: Out of range explicit level array: defaulting";
		NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
	}
	if (fp[0] < zmin) ixmin = 1;
		
	ixmax = count;
	for (i=1; i < count; i++) {
		if (fp[i] < fp[i-1]) {
			do_auto = True;
			ret = MIN(NhlWARNING,ret);
			e_text =
		"%s: Invalid non-monotonic explicit level array: defaulting";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			break;
		}
		if (fp[i] < zmin) ixmin = i + 1;
		if (fp[i] > zmax && ixmax == count) ixmax = i;
	}
	if (! (ixmin < ixmax)) {
		do_auto = True;
		ret = MIN(NhlWARNING,ret);
		e_text = "%s: Out of range explicit level array: defaulting";
		NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
	}
	count = ixmax - ixmin;

	if (do_auto) {
		subret = ChooseSpacingLin(&zmin,&zmax,&spacing,
					  7,cnp->max_level_count,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			e_text = "%s: error choosing spacing";
			NhlPError(ret,NhlEUNKNOWN,e_text,entry_name);
			return ret;
		}
		count = (zmax - zmin) / spacing + 1.001;
	}

	if ((*levels = (float *) NhlMalloc(count * sizeof(float))) == NULL) {
		e_text = "%s: dynamic memory allocation error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}
	
	if (do_auto) {
		for (i=0; i<count; i++) {
			(*levels)[i] = zmin + i * spacing;
		}
		cnp->level_spacing = spacing;
	}
	else {
		int total = 0;
		for (i = ixmin; i < ixmax; i++) {
			total += fp[i];
			(*levels)[i-ixmin] = fp[i];
		}
		cnp->level_spacing = total / (float) count - zmin;
	}

	cnp->level_count = count;
	cnp->fill_count = (*levels)[count-1] < zmax ? count + 1 : count;

	return ret;
}

/*
 * Function:	ChooseSpacingLin
 *
 * Description: Ethan's tick mark spacing code adapted to choosing 'nice'
 *		contour values; adapted by exchanging the ciel and floor
 *		functions - since the max and min contour values must be
 *		within the data space rather than just outside it as is 
 *		appropriate for tick marks. (Eventually should probably
 *		generalize the code with another parameter to handle either
 *		situation.)
 *
 * In Args:	tstart	requested tick starting location
 *		tend	requested tick ending location
 *		convert_precision  precision used to compare with
 *		max_ticks the maximum number of ticks to be choosen
 * Out Args:
 *		tstart  new start
 *		tend	new end
 *		spacing new spacing
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */
static NhlErrorTypes ChooseSpacingLin
#if	__STDC__
(
	float *tstart,
	float *tend,
	float *spacing,
	int convert_precision,
	int max_ticks,
	NhlString entry_name)
#else
(tstart,tend,spacing,convert_precision,max_ticks,entry_name)
	float *tstart;
	float *tend;
	float *spacing;
	int	convert_precision;
	int	max_ticks;
#endif
{
	double table[10],d,u,t,am1,am2=0.0,ax1,ax2=0.0;
	int	npts,i;
	char	*e_text;

	table[0] = 1.0;
	table[1] = 2.0;
	table[2] = 3.0;
	table[3] = 4.0;
	table[4] = 5.0;
	table[5] = 10.0;
	table[6] = 20.0;
	table[7] = 30.0;
	table[8] = 40.0;
	table[9] = 50.0;
	npts = 10;

	if(_NhlCmpFAny(*tend,*tstart,8)<=0.0) {
		e_text = "%s: Scalar field is constant";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(NhlFATAL);
	}

	d = pow(10.0,floor(log10(*tend-*tstart)) - 1.0);
	u = *spacing = 1e30;
	for(i=0;i<npts; i++) {
		t = table[i] * d;
		am1 = ceil(*tstart/t) *t;
		ax1 = floor(*tend/t) * t;
		if(((i>=npts-1)&&(*spacing == u))||
		   ((t <= *spacing)&&
		    (_NhlCmpFAny((ax1-am1)/t,(double)max_ticks,
				 convert_precision) <= 0.0))){
			*spacing = t;
			ax2 = ax1;
			am2 = am1;
		}
	}
	*tstart = am2;
	*tend = ax2;
	return(NhlNOERROR);
	
}

/*
 * Function:  nhfill_
 *
 * Description: C version of APR user routine called from within ARSCAM 
 *		to fill areas based on the area ID.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
int (_NHLCALLF(nhlfll,NHLFLL))
#if __STDC__
(
	float *xcs, 
	float *ycs, 
	int *ncs, 
	int *iai, 
	int *iag, 
	int *nai
)
#else
(xcs,ycs,ncs,iai,iag,nai)
	float *xcs; 
	float *ycs; 
	int *ncs; 
	int *iai; 
	int *iag; 
	int *nai;
#endif
{
	int i;
	int pat_ix, col_ix;
	float fscale;

	for (i = 0; i < *nai; i++) {
		if (iag[i] == 10 && iai[i] == -1) {
			return 0;
		}
	}

	for (i = 0; i < *nai; i++) {
		if (iag[i] == 3 && iai[i] > 99 && iai[i] < 100 + Fill_Count) {
			int ix = iai[i] - 100;
			col_ix = Mono_Fill_Color ? 
				Fill_Colors[0] : Fill_Colors[ix];
			pat_ix = Mono_Fill_Pattern ?
				Fill_Patterns[0] : Fill_Patterns[ix];
			fscale = Mono_Fill_Scale ?
				Fill_Scales[0] : Fill_Scales[ix];

			NhlVASetValues(Wkptr->base.id,
				       NhlNwkFillIndex, pat_ix,
				       NhlNwkFillColor, col_ix,
				       NhlNwkFillScaleFactorF,fscale,
				       NhlNwkDrawEdges,0,
				       NULL);
			
			_NhlSetFillInfo(Wkptr, Clayer);
			_NhlWorkstationFill(Wkptr,xcs,ycs,*ncs);
		}
	}
	return 0;
}

/*
 * Function:  cpchcl_
 *
 * Description: C version of the CPCHCL function that is called from
 *              the Conpack CPCLDR and CPCLDM functions. 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(cpchcl,CPCHCL))
#if __STDC__
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{

	int pai, dpix, slen, jcrt, jsize = 4;
        float p0, p1;
	char buffer[NhlDASHBUFSIZE];
	int lcol, llcol;
	float thickness;

	if (*iflg != 1) return;
	

	c_cpgeti("PAI", &pai);

	if (pai > 0) {
		pai -= 1;

		c_pcseti("CC",-1);
		c_pcseti("OC",-1);
		thickness = Mono_Line_Thickness ?
			Line_Thicknesses[0] : Line_Thicknesses[pai];
		gset_linewidth(thickness);
		
		lcol = Mono_Line_Color ?
			Line_Colors[0] : Line_Colors[pai];
		gset_line_colr_ind(lcol);

		dpix = Mono_Dash_Pattern ? 
			Dash_Patterns[0] : Dash_Patterns[pai]; 
		dpix %= Dash_Table_Len;

		slen = strlen(Dash_Table[dpix]);

                p0 =  (float) c_kfpy(0.0);

                p1 = Dash_Seglen;
                p1 = (float) c_kfpy(p1);
                jcrt = (int) ((p1 - p0) / slen + 0.5);
		jcrt = jcrt > 1 ? jcrt : 1;

		strcpy(buffer,Dash_Table[dpix]);

		if (Constant_Labels) {
			int tstart;

			p1 = LLabel_AttrsP->height;
			p1 = c_kfpy(p1);
			jsize = (int) (p1 - p0);
			jsize = jsize > 3 ? jsize : 4;

			llcol = LLabel_AttrsP->mono_color ?
			       LLabel_AttrsP->colors[0] : 
				       LLabel_AttrsP->colors[pai];
			gset_text_colr_ind(llcol);

			tstart = slen - strlen(LLabel_AttrsP->text[pai]);
			strcpy(&buffer[tstart],LLabel_AttrsP->text[pai]);
		}
			
                c_dashdc(buffer,jcrt,jsize);
		
	}
	return;
}


/*
 * Function:  cpchhl_
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(cpchhl,CPCHHL))
#if __STDC__
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{

	if (*iflg == 2) {
		gset_fill_colr_ind(High_Label_AttrsP->gks_bcolor);
	}
	else if (*iflg == 3) {
		if (! High_Label_AttrsP->on) return;
		c_pcseti("CC",(int) High_Label_AttrsP->colors);
		c_pcseti("OC",(int) High_Label_AttrsP->colors);
		c_pcsetr("PH",High_Label_AttrsP->pheight);
		c_pcsetr("PW",High_Label_AttrsP->pwidth);
		c_pcseti("CS",High_Label_AttrsP->cspacing);
		c_pcseti("FN",High_Label_AttrsP->font);
		c_pcseti("QU",High_Label_AttrsP->quality);
		c_pcsetc("FC",High_Label_AttrsP->fcode);
		gset_linewidth(High_Label_AttrsP->thickness);
	}
	else if (*iflg == 4 && High_Label_AttrsP->perim_on) {
		if (High_Label_AttrsP->perim_lcolor == NhlTRANSPARENT)
			gset_line_colr_ind((int) High_Label_AttrsP->colors);
		else
			gset_line_colr_ind(High_Label_AttrsP->gks_plcolor);
		gset_linewidth(High_Label_AttrsP->perim_lthick);
	}
	else if (*iflg == 6) {
		gset_fill_colr_ind(Low_Label_AttrsP->gks_bcolor);
	}
	else if (*iflg == 7) {
		if (! Low_Label_AttrsP->on) return;
		c_pcseti("CC",(int) Low_Label_AttrsP->colors);
		c_pcseti("OC",(int) Low_Label_AttrsP->colors);
		c_pcsetr("PH",Low_Label_AttrsP->pheight);
		c_pcsetr("PW",Low_Label_AttrsP->pwidth);
		c_pcseti("CS",Low_Label_AttrsP->cspacing);
		c_pcseti("FN",Low_Label_AttrsP->font);
		c_pcseti("QU",Low_Label_AttrsP->quality);
		c_pcsetc("FC",Low_Label_AttrsP->fcode);
		gset_linewidth(Low_Label_AttrsP->thickness);
	}
	else if (*iflg == 8 && Low_Label_AttrsP->perim_on) {
		if (Low_Label_AttrsP->perim_lcolor == NhlTRANSPARENT)
			gset_line_colr_ind((int) Low_Label_AttrsP->colors);
		else
			gset_line_colr_ind(Low_Label_AttrsP->gks_plcolor);
		gset_linewidth(Low_Label_AttrsP->perim_lthick);
	}

	return;
}

/*
 * Function:  cpchll_
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(cpchll,CPCHLL))
#if __STDC__
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{

	int pai;
	static int llcol;

	if (*iflg == 2) {
		gset_fill_colr_ind(LLabel_AttrsP->gks_bcolor);
	}
	else if (*iflg == 3) {
		c_cpgeti("PAI", &pai);
		if (pai > 0) {
			pai -= 1;

			llcol = LLabel_AttrsP->mono_color ?
				LLabel_AttrsP->colors[0] : 
					LLabel_AttrsP->colors[pai];
			c_pcseti("CC",llcol);
			c_pcseti("OC",llcol);
			c_pcsetr("PH",LLabel_AttrsP->pheight);
			c_pcsetr("PW",LLabel_AttrsP->pwidth);
			c_pcseti("CS",LLabel_AttrsP->cspacing);
			c_pcseti("FN",LLabel_AttrsP->font);
			c_pcseti("QU",LLabel_AttrsP->quality);
			c_pcsetc("FC",LLabel_AttrsP->fcode);
			gset_linewidth(LLabel_AttrsP->thickness);
		}
	}
	else if (*iflg == 4 && LLabel_AttrsP->perim_on) {
		if (LLabel_AttrsP->perim_lcolor == NhlTRANSPARENT)
			gset_line_colr_ind(llcol);
		else
			gset_line_colr_ind(LLabel_AttrsP->gks_plcolor);
		gset_linewidth(LLabel_AttrsP->perim_lthick);
	}

	return;
}


/*
 * Function:  cpmpxy_
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(cpmpxy,CPMPXY))
#if __STDC__
(
	int	*imap,
	float	*xinp,
	float	*yinp,
	float	*xotp,
	float	*yotp
)
#else
(imap,xinp,yinp,xotp,yotp)
	int	*imap;
	float	*xinp;
	float	*yinp;
	float	*xotp;
	float	*yotp;
#endif

{

        if (*imap == 0) {
                if ((int) *xinp == NhlcnMAPVAL)
                        *yinp = 3.0;
                else
                        *yinp = 0.0;
        }
        else if (abs(*imap) != NhlcnMAPVAL) {
                *xotp = *xinp;
                *yotp = *yinp;
        }
        else if (*imap > 0)
                _NhlovCpMapXY(xinp,yinp,xotp,yotp);
        else
                _NhlovCpInvMapXY(xinp,yinp,xotp,yotp);
                
	return;
}


