#!/usr/bin/env bash

c_files=`find -name "*.c"`
h_files=`find -name "*.h"`

echo "${c_files}"
echo "${h_files}"

for f in ${c_files}
do
    astyle \
        --style=otbs \
        --attach-namespaces \
        --attach-classes \
        --indent=spaces=4 \
        --convert-tabs \
        --align-pointer=name \
        --align-reference=name \
        --keep-one-line-statements \
        --pad-header \
        --pad-oper \
        ${f}
done

for f in ${h_files}
do
    astyle \
        --style=otbs \
        --attach-namespaces \
        --attach-classes \
        --indent=spaces=4 \
        --convert-tabs \
        --align-pointer=name \
        --align-reference=name \
        --keep-one-line-statements \
        --pad-header \
        --pad-oper \
        ${f}
done
