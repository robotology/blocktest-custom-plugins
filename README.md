
<!-- /TOC -->

# 1. CI Status

[![Build Status (Linux) (master branch)](https://img.shields.io/travis/com/robotology/blocktest/master.svg?logo=travis&label=[master]%20build%20(Linux))](https://travis-ci.com/robotology/blocktest)
[![Build Status (Linux) (devel branch)](https://img.shields.io/travis/com/robotology/blocktest/devel.svg?logo=travis&label=[devel]%20build%20(Linux))](https://travis-ci.com/robotology/blocktest)

# 3. TOC

- [1. CI Status](#1-ci-status)
- [3. TOC](#3-toc)
- [2. Introduction](#2-introduction)
- [4. Redball demo](#4-redball-demo)

# 2. Introduction
This repository has been created to contain all the custom plugin created by researchers.

# 4. Redball demo
This test executes and checks the RedBallDemo application. 
In order to make this demo test work some prerequisited are needed.

```
$ git clone https://github.com/robotology/icub-basic-demos.git
$ mkdir icub-basic-demos/build && cd icub-basic-demos/build
$ ccmake .. && make install
```

Then some blocktest prerequisite in test.xml (or ScriptBuilder) must be activated:

```xml
	<prerequisite enabled="true" command="yarpserver" waitafter="5000" param="--silent" prefix="" kill="true" />
	<prerequisite enabled="true" command="iCub_SIM" waitafter="2000" param="" prefix="" kill="true" />
	<prerequisite enabled="true" command="yarprobotinterface" waitafter="2000" param="--context simCartesianControl --config no_legs.xml" prefix="" kill="true" />
	<prerequisite enabled="true" command="iKinGazeCtrl" waitafter="2000" param="--from configSim.ini" prefix="" kill="true" />
	<prerequisite enabled="true" command="iKinCartesianSolver" waitafter="2000" param="--context simCartesianControl --part right_arm" prefix="" kill="true" />
	<prerequisite enabled="true" command="iKinCartesianSolver" waitafter="2000" param="--context simCartesianControl --part left_arm" prefix="" kill="true" />
	<prerequisite enabled="true" command="iCubGui" waitafter="2000" param="--xpos 800 --ypos 80 --width 370" prefix="" kill="true" />
	<prerequisite enabled="true" command="demoRedBall" waitafter="2000" param="--from config-test.ini" prefix="" kill="true" />
```

Some blocktest library must be activated in test.xml (or ScriptBuilder):

```xml
	<library enabled="true" path="blocktestyarpplugins/libblocktestyarpplugins" name="yarpactiondepot" note="Yarp actions library" />
	<library enabled="true" path="genericactiondepot/genericactiondepot" name="genericactiondepot" note="System generic actions library" />
	<library enabled="true" path="blocktestcustomplugins/libblocktestcustomplugins" name="customactiondepot" note="Custom actions library" />

	<librarysettings enabled="true" name="genericactiondepot" />
	<librarysettings enabled="true" name="yarpactiondepot" wrappername="/right_leg /left_leg /torso /head /right_arm /left_arm" robotname="icubSim" netclock="false" />
	<librarysettings enabled="true" name="customactiondepot" />
```

As usual the test can be executed by (or by ScriptBuilder):

```bash
./blocktestrunner
```

