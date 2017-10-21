#! /bin/bash -e
# $Id:$
#  --------------------------------------------------------------------------
# dismod_at: Estimating Disease Rates as Functions of Age and Time
#           Copyright (C) 2014-17 University of Washington
#              (Bradley M. Bell bradbell@uw.edu)
#
# This program is distributed under the terms of the
#	     GNU Affero General Public License version 3.0 or later
# see http://www.gnu.org/licenses/agpl.txt
# ---------------------------------------------------------------------------
if [ ! -e "bin/check_srcfile.sh" ]
then
	echo "bin/check_srcfile.sh: must be executed from its parent directory"
	exit 1
fi
cat << EOF > junk.sed
/\$srcfile[^a-z]/! b skip
N
s/^#[ \\t]//
s/^[ \\t]//
s/\\n#[ \\t]//
s/\\n[ \\t]//
s/\$srcfile%//
s/%.*//
p
: skip
EOF
special_case='
	devel/table/get_db_input.cpp
	devel/utility/pack_info.cpp
	omh/install/unix.omh
	bin/check_srcfile.sh
	bin/batch_edit.sh
'
# -----------------------------------------------------------------------------
# Make sure that OMhelp srcfile commands referr to same file as command
echo "Checking that OMhelp srcfile commands include from file they appear in."
echo "----------------------------------------------------------------------"
list=`git ls-files`
different="no"
for file in $list
do
	references=`sed -n -f junk.sed $file`
	for name in $special_case
	do
		if [ "$file" == "$name" ]
		then
			references=''
		fi
	done
	for reference in $references
	do
		if [ "$reference" != '' ]
		then
			if [ "$file" != "$reference" ]
			then
				echo "\$srcfile in '$file' references '$reference'"
				different="yes"
			fi
		fi
	done
done
echo "-------------------------------------------------------------------"
if [ $different = "yes" ]
then
	echo "Error: nothing should be between the two dashed lines above"
	exit 1
else
	echo "OK: nothing is between the two dashed lines above"
	exit 0
fi
