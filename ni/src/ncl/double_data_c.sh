#!/bin/sh

sh op_funcs.sh double NhlTDouble NhlTDoubleGenArray -9999.0 > .tmp.$$ 

if [ ! $? ]
then
	exit $?
fi

sed \
-e 's/DATATYPE/double/g' \
-e 's/HLUTYPEREP/NhlTDouble/g' \
-e 's/HLUGENTYPEREP/NhlTDoubleGenArray/g' \
-e 's/DEFAULT_MISS/-9999.0/g' \
-e 's/DEFAULT_FORMAT/%lf/g' \
-e "/REPLACE/r .tmp.$$" \
-e '/REPLACE/d' \
-e '/DSPECIFIC/r NclTypedouble.c.specific' \
-e '/DSPECIFIC/d' \
-e 's/Ncl_Type_double_mat_type/NULL/' \
-e 's/Ncl_Type_double_mat/NULL/' \
-e 's/Ncl_Type_double_mod_type/NULL/' \
-e 's/Ncl_Type_double_mod/NULL/' \
NclType.c.sed > NclTypedouble.c

rm .tmp.$$

echo "created NclTypedouble.c"

exit 0
