/*
 *      $Id: c_mpname.c,v 1.2 2002-02-23 03:01:45 haley Exp $
 */
/************************************************************************
*                                                                       *
*                Copyright (C)  2000                                    *
*        University Corporation for Atmospheric Research                *
*                All Rights Reserved                                    *
*                                                                       *
* This file is free software; you can redistribute it and/or modify     *
* it under the terms of the GNU General Public License as published     *
* by the Free Software Foundation; either version 2 of the License, or  *
* (at your option) any later version.                                   *
*                                                                       *
* This software is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
* General Public License for more details.                              *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this software; if not, write to the Free Software         *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
* USA.                                                                  *
*                                                                       *
************************************************************************/


#include <ncarg/ncargC.h>

char *c_mpname
#ifdef NeedFuncProto
(
    int iain
)
#else
(iain)
    int iain;
#endif
{
    int len=64;
    static char buff[65];
#if defined(cray)
    extern NGstring NGCALLF(mpname,MPNAME)(_fcd,int*);
 
    _fcd ft_str;
    ft_str=_cptofcd(buff,len);
    NGCALLF(mpname,MPNAME)(ft_str,&iain);
    strcpy(buff,_fcdtocp(ft_str));
#else
#if defined(AbsoftProFortran)
    extern NGstring NGCALLF(mpname,MPNAME)(char*,int*,int);
    NGCALLF(mpname,MPNAME)(buff,&iain,len);
#else
    extern NGstring NGCALLF(mpname,MPNAME)(char*,int,int*);
    NGCALLF(mpname,MPNAME)(buff,len,&iain);
#endif
#endif
    buff[c_icloem(buff)]='\0';
    return(buff);
}
