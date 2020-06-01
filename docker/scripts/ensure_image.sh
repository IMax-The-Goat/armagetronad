#!/bin/bash

# usage: ensure_image.sh <base docker image>
# makes sure base docker image exists (if it is managed by us)

#set -x

sd=`dirname "$0"`

image=$1

# get best version of parent of script directory
if echo "${sd}" | grep "scripts$" > /dev/null; then
    wd=`dirname "${sd}"`
else
    wd="${sd}/.."
fi
id=${wd}/images

. ${id}/digest.sh "${image}" || exit $?

lock=${sd}/.lock.`echo ${image} | sed -e s,/,_,`
while test -r ${lock} && ps -eo pid | grep "[\^ ]`cat ${lock}`\$"; do
    sleep 10
done
echo $$ > ${lock}

# start daemon on demand
docker info > /dev/null 2>&1 || { systemctl --user start docker; sleep 5; }

function invalidate_digest(){
    touch ${id}/${image}.digest.local
}

function build(){
    # force build?
    if test "x$2" = "x-f"; then
	echo "Force buiding base image ${image}..."
	set -x
	${id}/build_${image}.sh || exit $?
	invalidate_digest || exit $?
	exit 0
    fi

    # test if image of given name exists, if not, build it with the canonically named script
    if ! docker inspect --format '{{.Config.User}}' ${REGISTRY}${image}${REFERENCE} > /dev/null && test -f ${id}/build_${image}.sh; then
	set -x
	# download build?
	if test "x$2" = "x-d"; then
	    echo "Trying to download base image ${image}..."
	    docker pull ${REGISTRY}${image}${REFERENCE}
	    rm -f ${id}/${image}.digest.local
	    exit 0
	fi

	echo "Buiding base image $1..."
	${id}/build_${image}.sh || exit $?
	invalidate_digest || exit $?
    fi
}

if build $*; then
    rm -f ${lock}
else
    rm -f ${lock}
    exit 1
fi

