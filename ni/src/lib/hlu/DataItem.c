/*
 *      $Id: DataItem.c,v 1.2 1993-10-19 17:50:25 boote Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1993			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		DataItem.c
 *
 *	Author:		Jeff W. Boote
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Thu Jun 24 10:13:13 MDT 1993
 *
 *	Description:	This class is used to communicate with the DataMgr
 *			class.  It is intended as the Superclass of all
 *			future data classes or at least an example of how
 *			to write data classes.
 */
#include <stdio.h>
#include <string.h>
#include <ncarg/hlu/DataItemP.h>
#include <ncarg/hlu/DataMgrF.h>

#define	PFIX	"-mgr"

/************************************************************************
*									*
*	DataItem Class declarations					*
*									*
************************************************************************/

/*
 * Resource list
 */
#define	Oset(field)	NhlOffset(DataItemLayerRec,dataitem.field)
static NhlResource resources[] = {
	{ NhlNnoManager, NhlCnoManager, NhlTBoolean, sizeof(NhlBoolean),
			Oset(no_manager), NhlTImmediate,(NhlPointer)False}
};
#undef Oset

static NhlErrorTypes DataItemClassPartInitialize(
#if	NhlNeedProto
	LayerClass	lc
#endif
);

static NhlErrorTypes DataItemInitialize(
#if	NhlNeedProto
	LayerClass	lc,	/* class	*/
	Layer		req,	/* requested	*/
	Layer		new,	/* new		*/
	_NhlArgList	args,	/* args		*/
	int		nargs	/* nargs	*/
#endif
);

static NhlErrorTypes DataItemSetValuesHook(
#if	NhlNeedProto
	Layer		old,	/* old		*/
	Layer		req,	/* requested	*/
	Layer		new,	/* new		*/
	_NhlArgList	args,	/* args		*/
	int		nargs	/* nargs	*/
#endif
);

static NhlErrorTypes DataItemDestroy(
#if	NhlNeedProto
	Layer	l	/* layer to destroy	*/
#endif
);

DataItemLayerClassRec dataItemLayerClassRec = {
	/* BaseLayerClassPart */
	{
/* class_name			*/	"DataItem",
/* nrm_class			*/	NrmNULLQUARK,
/* layer_size			*/	sizeof(DataItemLayerRec),
/* class_inited			*/	False,
/* superclass			*/	(LayerClass)&baseLayerClassRec,

/* resources			*/	resources,
/* num_resources		*/	NhlNumber(resources),
/* all_resources		*/	NULL,

/* class_part_initialize	*/	DataItemClassPartInitialize,
/* class_initialize		*/	NULL,
/* layer_initialize		*/	DataItemInitialize,
/* layer_set_values		*/	NULL,
/* layer_set_values_hook	*/	DataItemSetValuesHook,
/* layer_get_values		*/	NULL,
/* layer_reparent		*/	NULL,
/* layer_destroy		*/	DataItemDestroy,

/* child_resources		*/	NULL,

/* layer_draw			*/	NULL,

/* layer_pre_draw		*/	NULL,
/* layer_draw_segonly		*/	NULL,
/* layer_post_draw		*/	NULL,
/* layer_clear			*/	NULL
	},
	/* DataItemLayerClassPart */
	{
/* foo				*/	0
	}
};
	
LayerClass dataItemLayerClass = (LayerClass)&dataItemLayerClassRec;

/************************************************************************
*	New type converters - added to converter table by		*
*	ClassInitialize							*
************************************************************************/

/* none yet */


/************************************************************************
*									*
*	Methode definitions						*
*									*
************************************************************************/

/*
 * Function:	DataItemClassPartInitialize
 *
 * Description:	This function is used to initialize the DataItemLayerClassPart
 *		record.
 *
 * In Args:	
 *		LayerClass	lc
 *
 * Out Args:	none
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
static NhlErrorTypes
DataItemClassPartInitialize
#if	__STDC__
(
	LayerClass	lc
)
#else
(lc)
	LayerClass	lc;
#endif
{
	/*
	 * Register child classes - DataMgr.  Should automatically call
	 * setvalues - and this class should not intercept any of the
	 * resources for the manager.
	 */
	return _NhlRegisterChildClass(lc,dataMgrLayerClass,True,False,NULL);
}

/*
 * Function:	DataItemInitialize
 *
 * Description:	This function initializes an instance of a DataItem class
 *		object.
 *
 * In Args:	
 *	LayerClass	lc,	class
 *	Layer		req,	requested
 *	Layer		new,	new
 *	_NhlArgList	args,	args
 *	int		nargs	nargs
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
/*ARGSUSED*/
static NhlErrorTypes
DataItemInitialize
#if	__STDC__
(
	LayerClass	lc,	/* class	*/
	Layer		req,	/* requested	*/
	Layer		new,	/* new		*/
	_NhlArgList	args,	/* args		*/
	int		nargs	/* nargs	*/
)
#else
(lc,req,new,args,nargs)
	LayerClass	lc;	/* class	*/
	Layer		req;	/* requested	*/
	Layer		new;	/* new		*/
	_NhlArgList	args;	/* args		*/
	int		nargs;	/* nargs	*/
#endif
{
	DataItemLayer	dinew = (DataItemLayer)new;
	DataItemLayer	direq = (DataItemLayer)req;
	NhlErrorTypes	ret = NOERROR;
	char		tstring[MAXRESNAMLEN];
	int		tint;

	/*
	 * if this object is being created by a converter as an internal
	 * data set, then a Manager should not be created.
	 */
	if(direq->dataitem.no_manager){
		dinew->dataitem.manager = NULL;
		return NOERROR;
	}

	/*
	 * Create the Manager
	 */
	strcpy(tstring,direq->base.name);
	strcat(tstring,PFIX);
	ret = _NhlCreateChild(&tint,tstring,dataMgrLayerClass,new,
				NULL);

	if(ret < WARNING)
		return ret;

	dinew->dataitem.manager = (DataMgrLayer)_NhlGetLayer(tint);
	if(dinew->dataitem.manager == NULL)
		return FATAL;

	return ret;
}

/*
 * Function:	DataItemSetValuesHook
 *
 * Description:
 *
 * In Args:	
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	void
 * Side Effect:	
 */
/*ARGSUSED*/
static NhlErrorTypes
DataItemSetValuesHook
#if	__STDC__
(
	Layer		old,	/* old		*/
	Layer		req,	/* requested	*/
	Layer		new,	/* new		*/
	_NhlArgList	args,	/* args		*/
	int		nargs	/* nargs	*/
)
#else
(old,req,new,args,nargs)
	Layer		old;	/* old		*/
	Layer		req;	/* requested	*/
	Layer		new;	/* new		*/
	_NhlArgList	args;	/* args		*/
	int		nargs;	/* nargs	*/
#endif
{
	DataItemLayer	dil = (DataItemLayer)new;

	return _NhlNotifyDataComm(dil->dataitem.manager);
}

/*
 * Function:	DataItemDestroy
 *
 * Description:	This function free's any memory that has been allocated
 *		on behalf of this instance of the DataItemLayerClass.
 *
 * In Args:	Layer	l
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
static NhlErrorTypes
DataItemDestroy
#if	__STDC__
(
	Layer	l	/* layer to destroy	*/
)
#else
(l)
	Layer	l;	/* layer to destroy	*/
#endif
{
	DataItemLayer	dil = (DataItemLayer)l;
	Layer		tmp = (Layer)dil->dataitem.manager;
	NhlErrorTypes	ret = NOERROR;

	if(tmp != NULL)
		ret = _NhlDestroyChild(tmp->base.id,l);

	return ret;
}

/************************************************************************
*									*
*	Private API for sub-classes					*
*									*
************************************************************************/

/*
 * Function:	_NhlDataChanged
 *
 * Description:	This function is used by sub-classes of the DataItem during
 *		a set-values, to notify the supre-class, and the manager
 *		that the data has changed.
 *
 * In Args:	
 *		Layer	l	data item layer
 *
 * Out Args:	
 *
 * Scope:	Private to DataItem sub-classes
 * Returns:	void
 * Side Effect:	
 */
void
_NhlDataChanged
#if	__STDC__
(
	DataItemLayer	l	/* data item layer	*/
)
#else
(l)
	DataItemLayer	l;	/* data item layer	*/
#endif
{
	if(!_NhlIsDataItem(l)){
		NhlPError(FATAL,E_UNKNOWN,
		"_NhlDataChanged:Can only be called with a DataItem sub-class");
		return;
	}

	if(l->dataitem.manager != NULL){
		_NhlDataItemModified((DataMgrLayer)l->dataitem.manager);
		return;
	}

	NhlPError(FATAL,E_UNKNOWN,
"_NhlDataChanged:Should only be called by DataItem sub-classes with Managers");

	return;
}

/************************************************************************
*									*
*	Public API							*
*									*
************************************************************************/

/* none yet */
