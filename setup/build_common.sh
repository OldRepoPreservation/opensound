#!/bin/sh

SRCDIR=$1
OSSLIBDIR=$2

if test ! -d $SRCDIR
then
	echo Bad SRCDIR parameter
	exit 1
fi

if test ! -d prototype
then
	echo Bad prototype directory
	exit 1
fi

# Copy common files to the prototype tree
cp -pRf $SRCDIR/* prototype/$OSSLIBDIR/
rm -f prototype/$OSSLIBDIR/.nomake

chmod 700 prototype/usr/sbin/*

exit 0
