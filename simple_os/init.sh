#!/bin/bash
pwd=$(pwd)
if [ x$arg0 = xsetit-sh ]; then
	export SIMPLE_OS_DIR=$pwd/simple_os
elif [ x$arg0 = xsetit-csh ]; then
	setenv SIMPLE_OS_DIR $pwd/simple_os
fi
echo $SIMPLE_OS_DIR
