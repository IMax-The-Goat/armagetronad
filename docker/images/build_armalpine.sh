#!/bin/bash

# builds a basic arma build environment based on alpine
# usage: $0 <base image name> <target image name>

set -x

bin=`echo $1 | sed -e "s,/,_,g"`
wd="`dirname $0`"
bd="${wd}/context/armalpine_${bin}_$2"
sd="${wd}/../scripts"
mkdir -p ${wd}/context
rm -rf ${bd}
cp -ax ${wd}/armalpine ${bd} || exit $?

${sd}/download.sh ${bd}/download/ZThread-2.3.2.tar.gz https://sourceforge.net/projects/zthread/files/ZThread/2.3.2/ZThread-2.3.2.tar.gz/download
${sd}/download.sh ${bd}/download/zthread.patch.bz2 https://forums3.armagetronad.net/download/file.php?id=9628

${sd}/build_image.sh "$1" "$2" "${bd}"


