start=4098
for ((z=0; z<12; ++z));
do
  run=$(($start+$z))
  echo "$run"
  #ls /media/ILC-7M_DATA_04/data/RUN_0$run*.acq > FileList/acq/run$run.list
  ls ~/suvorov/data_acq/RUN_0$run*.acq > FileList/acq/run$run.list
  source script/acq_to_root_separate_fem.sh $run
done
