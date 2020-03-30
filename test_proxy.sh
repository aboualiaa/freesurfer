#!/usr/bin/env bash -x

dots=""

if [ -z ${goup+x} ]; then
  goup=1
else
  goup=$((goup + 1))
fi

for i in $(seq 1 $goup); do
  dots="$dots/.."
done

temp=$(dirname "$(realpath $0)")
parent=$(realpath "$temp$dots")
goup=$goup source "$parent/test.sh"
echo "$goup levels up"
