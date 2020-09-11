#!/usr/bin/env bash

# To run this script:
# source submit-to-grid.sh

source setup_env.sh
source build/x86_64-centos7-gcc8-opt/setup.sh

GRID_NAME=${RUCIO_ACCOUNT-${USER}}
JOB_TAG=$(date +%F-%H-%M)

#INPUT_DATASETS=(
#   mc16_13TeV.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.deriv.DAOD_SUSY2.e3601_e5984_s3126_r10724_r10726_p3895
#   mc16_13TeV.308093.Sherpa_221_NNPDF30NNLO_Zmm2jets_Min_N_TChannel.deriv.DAOD_SUSY2.e5767_e5984_s3126_r10724_r10726_p3875
#)


lsetup panda

DSID=$(sed -r 's/[^\.]*\.([0-9]{6,8})\..*/\1/' <<< ${IN_DS})
OUT_DS=user.${GRID_NAME}.RNN.${JOB_TAG}
prun --exec "./build/x*/bin/SampleMaker %IN %IN2 %IN3"\
    --athenaTag=AnalysisBase,21.2.97\
	--secondaryDSs IN2:2:mc16_13TeV.394998.MGPy8EG_A14N23LO_SM_N2C1p_110_100_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN3:2:mc16_13TeV.395043.MGPy8EG_A14N23LO_SM_N2C1p_310_300_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN4:2:mc16_13TeV.395062.MGPy8EG_A14N23LO_SM_N2C1m_120_100_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN5:2:mc16_13TeV.394989.MGPy8EG_A14N23LO_SM_N2C1p_90_80_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN6:2:mc16_13TeV.395025.MGPy8EG_A14N23LO_SM_N2C1p_210_200_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN7:2:mc16_13TeV.395078.MGPy8EG_A14N23LO_SM_N2C1m_160_150_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN8:2:mc16_13TeV.395060.MGPy8EG_A14N23LO_SM_N2C1m_110_100_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724,IN9:2:mc16_13TeV.395087.MGPy8EG_A14N23LO_SM_N2C1m_210_200_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724\
    --inDS mc16_13TeV.395096.MGPy8EG_A14N23LO_SM_N2C1m_260_250_2L2MET75_MadSpin.recon.AOD.e7035_a875_r10724\
	--outputs output.root\
	--outDS ${OUT_DS}\
    --noEmail > ${OUT_DS}.log 2>&1
