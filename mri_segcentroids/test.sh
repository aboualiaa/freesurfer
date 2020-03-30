#!/usr/bin/env bash
source "$(dirname $0)/test_main.sh"

test_command mri_segcentroids --ctab-default --i aseg.mgz --o out.log
compare_file out.log ref.log
