#!/bin/sh
VERSION=1.32
COMPILE=`cat .compile`
COMPILE=`expr $COMPILE + 1`
echo $COMPILE > .compile
echo 'char *sklaff_version = "'$VERSION'(#'$COMPILE')";' > version.c
echo "Building SklaffKOM "$VERSION"("$COMPILE")"