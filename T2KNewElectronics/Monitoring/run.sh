export fileped=R2019_06_13-11_28_28-000
export filedata=R2019_06_12-18_08_08-000
#export filedata=R2019_05_16-08_31_25-000 #clean tracks

export compile=1
export startevent=1
export maxevent=1000

#export data="/local/home/t2kt/projects/bin/pandax/data/"

export t2k="/home/mlehuraux/GitLab/T2K/T2KNewElectronics"
#export data="/local/home/t2kt/projects/bin/pandax/data"
export input_aqs=$t2k/test_data/aqs
export input_txt=$t2k/test_data/txt
export output_root=$t2k/test_data/root

export make_convert=$t2k/pandax_client_16apr19/projects/pandax/mclient/linux
export bin_convert=$t2k/pandax_client_16apr19/projects/bin/pandax/linux
export here=$t2k/Monitoring

cd $input_aqs
#ln -s $data/$fileped.aqs
#ln -s $data/$filedata.aqs

###############################
#   Pedestals visualisation   #
###############################
#if not done already create the executable
#if [ $compile = 1 ]
#then
#	cd $here/pedestals
#  make clean
#	make
#fi
#cd $here/bin
#./pedestals -i $input_aqs/ $fileped.aqs 0 412 370 250 5

if [ $compile = 1 ]
then
	cd $here/monitor
  make clean
	make
fi
cd $here/bin
#./T2KMonitor -i $input_aqs/ $filedata.aqs
#cd $here
