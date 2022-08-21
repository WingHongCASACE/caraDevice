# caraDevice
## Sensor devices for COVID risk assessment using CARA model and more

The sensor devices run the [modified CARA model](https://github.com/WingHongCASACE/cara) and show the probability of infection in different scenarios (with/ without mask and staying in a room with an infected person for an hour/ 9 hours). The [original model](https://github.com/CERN/cara) is developed by Henriques et al. from CERN. The devices also run the CO2-based Rudnick-Milton's model to explore the use of CO2 concentration as a biomarker for airborne transmission risk assessment. Two iterations were made. The first one (markI) has a laser-cut plywood enclosure and fleshes between the assessments from the two models. It uses the instantaneous CO2 level to assess the infection propability for the whole period. The second one (markII) has a 3D-printed enclosure and a rotary encoder for control. It uses the the time-weighted average CO2 level to assess the infection propability for the whole period. Since the e-Paper has a slow refresh rate, the interupt function may be out of sync. In the future one can try to employ a more powerful micro-controller and a display with faster refresh rate. 

<p align="center">
<img src="https://user-images.githubusercontent.com/91855312/185805320-0c475db9-7883-4422-851c-a52b9b4845c7.jpg" width=45%/>&nbsp<img src="https://user-images.githubusercontent.com/91855312/185805434-fa0171f4-1f8a-4b04-a264-8d3e99180c27.png" width=45%/>
</p>

In addition to the sensor devices which show the risk numerically, other forms of physical visualization were designed. These devices embrace ambiguity and are designed for risk communication to audiences with low numeracy. They also encourage curiosity about scientific phenomena.

<p align="center">
<img src="https://user-images.githubusercontent.com/91855312/185806254-2144f113-6708-496e-96a3-087adb87d09f.jpg" width=28%/>&nbsp<img src="https://user-images.githubusercontent.com/91855312/185806287-ef729bc5-797e-4f33-a3c1-0583e7c09284.jpg" width=45%/>
</p>
