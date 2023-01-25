#!/bin/bash
#--
# This script is used instead of command:
#   idf.py build
# To generate new version for app.
# Ex. usage:
#   ./build.sh 0.4
#--

CV=$1
SV="version.db"
GV="version.txt"
IV=1
DR="versions/"
#
echo "using CV: "$CV
if [[ $CV == "" ]]; then
    echo "usage: ./build.sh your_version"
    echo "usage: ./build.sh 0.4"
    exit 1
fi

#
SV=$DR$CV"_"$SV
echo "using SV: "$SV

#
if [[ -f $SV ]]; then
    echo "File "$SV" exists."
    IV=$(cat $SV)
    echo "prev IV: "$IV
    IV=$(($IV + 1))
    echo $IV > $SV
    echo "new IV: "$IV
    echo $CV"."$IV > "main/"$GV
else
    echo "File "$SV" dont exists."
    echo $IV > $SV
    echo $CV"."$IV > "main/"$GV
fi

#
truncate -s -1 main/$GV
#
echo "Generated version: "$(cat main/$GV)
echo ""
#
idf.py build
