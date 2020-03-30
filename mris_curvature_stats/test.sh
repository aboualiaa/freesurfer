#!/usr/bin/env bash
source "$(dirname $0)/test_main.sh"

test_command mris_curvature_stats -o raw -h 10 -G cvs_avg35 rh rh.curv

compare_file raw ref
for stat in BE C FI H K1 K2 K S SI; do
  compare_file raw.rh.smoothwm.${stat}.hist ref.rh.smoothwm.${stat}.hist
done
