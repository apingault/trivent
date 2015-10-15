#!/bin/bash

echo "==================================================="

date 
uname -a

#echo "============= Noise parameters =================="

RUN=$1
GAIN_CORRECTION_MODE=false
electronic_noise_cut=100000
layerGap=2.8
LayerCut=7
noiseCut=7
timeWin=2

echo "================  ILCSoft - ======================="

source /home/steen/ilcsoft/v01-17-05/init_ilcsoft.sh
export MARLIN_DLL=/home/steen/trivent/trunk/lib/libTrivent.so

echo "========  steering file for Marlin ============="

FILELIST=`ls | grep DHCAL_${RUN}_SO.slcio`
if
    [[ $FILELIST == "DHCAL_${RUN}_SO.slcio" ]]
then
    echo $FILELIST
else 
    scp acqilc@lyosdhcal10.cern.ch:/data/NAS/Avril2015/STREAMOUT_tmp/DHCAL_${RUN}_SO.slcio DHCAL_${RUN}_SO.slcio
    FILELIST=DHCAL_${RUN}_SO.slcio
#FILELIST=`ls  /data/NAS/December2014/STREAMOUT_tmp/DHCAL_${RUN}_I0_0s.slcio`
    echo $FILELIST
fi

cat >LCIO.xml<<EOF
<marlin>
  <!--##########################################
      #                                        #
      #     Steering file for Noise_beta     #
      #                                        #
      ##########################################-->
  <execute>
    <processor name="MyNoiseProc"/>
  </execute>
  
  
  <global>
    <parameter name="LCIOInputFiles">
     ${FILELIST}
    </parameter>
    <parameter name="MaxRecordNumber" value="1000"/>
    <!--parameter name="SkipNEvents" value="0"/-->
    <parameter name="Verbosity" type="string"> MESSAGE </parameter>
    <parameter name="SupressCheck" value="false"/>
  </global>
  

  <processor name="MyNoiseProc" type="NoiseProc">
    <parameter name="HitCollectionName" type="StringVec">DHCALRawHits </parameter>
    <parameter name="DIFMapping" type="string">
      ./DifGeom/Slot1_39_Geom.txt
    </parameter>
    <parameter name="setup_geometry" type="string"> /home/steen/trivent/trunk/DifGeom/m3_bonneteau.xml </parameter>
    <parameter name="GAIN_CORRECTION_MODE" type="bool">false </parameter>
    <parameter name="electronic_noise_cut" type="int">500000 </parameter>
    <parameter name="layerGap" type="double"> 2.8 </parameter>
    <parameter name="noiseCut" type="int">10 </parameter>
    <parameter name="timeWin" type="int">1 </parameter>
    <parameter name="LCIOOutputFile" value="./TDHCALNOISE_${RUN}.slcio"/>
  </processor>
  
  
  
</marlin>

EOF
echo "========  RUNNING TRIVENT RECO ================="

Marlin LCIO.xml
rm LCIO.xml
