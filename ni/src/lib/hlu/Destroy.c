/*
 *      $Id: Destroy.c,v 1.5 1994-02-18 02:54:13 boote Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1992			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		Destroy.c
 *
 *	Author:		Jeff W. Boote
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Fri Oct 23 10:53:17 MDT 1992
 *
 *	Description:	This file contains all the functions neccessary
 *			to support the Destroy methode of the hlu's.
 *			Design documentation for this module is
 *			NhlDOCREF(/design/hlu/Destroy.html,here).
 */
#include <ncarg/hlu/hluP.h>
#include <ncarg/hlu/BaseP.h>
#include <ncarg/hlu/WorkstationI.h>


/*
 * Function:	CallDestroy
 *
 * Description:	This function is used to call the destroy methode's of the
 *		given function.  It is a sub-to-superclass chained methode.
 *
 * In Args:	
 *		NhlLayer	l,	NhlLayer to destroy
 *		NhlLayerClass	lc	class or superclass of l
 *
 * Out Args:	
 *
 * Scope:	static
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
static NhlErrorTypes
CallDestroy
#if	NhlNeedProto
(
	NhlLayer	l,	/* NhlLayer to destroy		*/
	NhlLayerClass	lc	/* class or superclass of l	*/
)
#else
(l,lc)
	NhlLayer	l;	/* NhlLayer to destroy		*/
	NhlLayerClass	lc;	/* class or superclass of l	*/
#endif
{
	NhlErrorTypes scret = NhlNOERROR;
	NhlErrorTypes lret = NhlNOERROR;
	

	if(lc->base_class.layer_destroy != NULL){
		lret = (*(lc->base_class.layer_destroy))(l);
	}

	if(lc->base_class.superclass != NULL){
		scret = CallDestroy(l,lc->base_class.superclass);
	}

	return(MIN(scret,lret));
}

/*
 * Function:	NhlDestroy
 *
 * Description:	This function is used to free the memory associated with
 *		an hlu object.  It calls the destroy method of the
 *		object so the object can free all of it's ansilary data
 *		and then this function removes that object from the
 *		internal list of objects and then actually free's the
 *		instance record itself.
 *
 * In Args:	
 *		int	pid	id associated with the object to delete
 *
 * Out Args:	
 *
 * Scope:	Global Public
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
NhlDOCTAG(NhlDestroy)
NhlErrorTypes
NhlDestroy
#if	NhlNeedProto
(
	int	pid	/* id associated with the object to delete	*/
)
#else
(pid)
	int	pid;	/* id associated with the object to delete	*/
#endif
{
	NhlErrorTypes ret, lret;
	NhlLayer l = _NhlGetLayer(pid);

	if(l == NULL){
		NhlPError(NhlFATAL,NhlEUNKNOWN,"Unable to Destroy (Bad PID#%d)",
									pid);
		return NhlFATAL;
	}
	if(_NhlIsWorkstation(l) ) {
		_NhlCloseWorkstation(l);
	}

	ret = CallDestroy(l,l->base.layer_class);

	/*
	 * remove this object from it's parents all_children list
	 */
	if(l->base.parent != NULL){
		_NhlAllChildList *tnodeptr = &l->base.parent->base.all_children;
		_NhlAllChildList tnode = NULL;
		NhlBoolean found = False;

		while(*tnodeptr != NULL){
			if((*tnodeptr)->pid == l->base.id){
				found = True;
				tnode = *tnodeptr;
				*tnodeptr = (*tnodeptr)->next;
				(void)NhlFree(tnode);
				break;
			}
			tnodeptr = &(*tnodeptr)->next;
		}

		if(!found){
			NHLPERROR((NhlWARNING,NhlEUNKNOWN,
				"Unable to remove PID#%d from Parent's list",
								l->base.id));
		}
	}
		

	lret = _NhlRemoveLayer(l);
	(void)NhlFree(l);

	return MIN(ret,lret);
}

/*
 * Function:	_NhlDestroyChild
 *
 * Description:	This function is used from within one of a layer's methods
 *		to remove a child layer that currently exists within it.
 *		It destroy's the child and remove's it from the layer's
 *		children list.
 *
 * In Args:	
 *		int	pid,		pid of layer to destroy
 *		NhlLayer	parent		parent of layer to destroy
 *
 * Out Args:	
 *
 * Scope:	Global layer writer
 * Returns:	NhlErrorTypes
 * Side Effect:	
 */
NhlDOCTAG(_NhlDestroyChild)
NhlErrorTypes
_NhlDestroyChild
#if	NhlNeedProto
(
	int		pid,		/* pid of layer to destroy	*/
	NhlLayer	parent		/* parent of layer to destroy	*/
)
#else
(pid,parent)
	int		pid;		/* pid of layer to destroy	*/
	NhlLayer	parent;		/* parent of layer to destroy	*/
#endif
{
	NhlErrorTypes	ret=NhlNOERROR;
	_NhlChildList	*tchldnodeptr=NULL;
	_NhlChildList	tchldnode=NULL;
	NhlBoolean	found=False;

	/*
	 * Not a valid function to call if parent is an ObjNhlLayer
	 */
	if(_NhlIsObj(parent)){
		NhlPError(NhlFATAL,NhlEUNKNOWN,
				"_NhlDestroyChild:parent has no children");
		return NhlFATAL;
	}

	ret = NhlDestroy(pid);

	tchldnodeptr = &parent->base.children;

	while(*tchldnodeptr != NULL){

		if((*tchldnodeptr)->pid == pid){
			found = True;
			tchldnode = *tchldnodeptr;
			*tchldnodeptr = (*tchldnodeptr)->next;
			(void)NhlFree(tchldnode);
			break;
		}

		tchldnodeptr = &(*tchldnodeptr)->next;
	}

	if(!found){
		NHLPERROR((NhlFATAL,NhlEUNKNOWN,
			"Unable to remove pid#%d from internal table",pid));
		return NhlFATAL;
	}

	return ret;
}
