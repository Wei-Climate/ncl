#!/bin/sh

sh op_funcs.sh int NhlTInteger NhlTIntegerGenArray -999 > .tmp.$$

if [ ! $? ]
then
	exit $?
fi

sed \
-e 's/PRINTFORMAT/\%d\\n/' \
-e 's/DATATYPE/int/g' \
-e 's/HLUTYPEREP/NhlTInteger/g' \
-e 's/HLUGENTYPEREP/NhlTIntegerGenArray/g' \
-e 's/DEFAULT_MISS/-9999/g' \
-e 's/DEFAULT_FORMAT/%d/g' \
-e "/REPLACE/r .tmp.$$" \
-e '/REPLACE/d' \
-e '/DSPECIFIC/r NclTypeint.c.specific' \
-e '/DSPECIFIC/d' \
-e 's/Ncl_Type_int_mat_type/NULL/' \
-e 's/Ncl_Type_int_mat/NULL/' \
NclType.c.sed > NclTypeint.c

rm .tmp.$$

echo "created NclTypeint.c"
