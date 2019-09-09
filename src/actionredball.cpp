/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file actionredball.cpp
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */


#include "actionredball.h"
#include "logger.h"
#include <string>
#include <chrono>

ACTIONREGISTER_DEF_TYPE(CustomActions::ActionRedBall,"actionredball");

using namespace std::chrono_literals;

namespace CustomActions
{


bool DemoRedBallPosition::start()
{
    std::string dest="/demoRedBall/trackTarget:i";
    port_.open(("/xxx/redballpos:o"));
    bool res=Network::connect(port_.getName(),dest,"udp");
    if(!res)
    {
        TXLOG(Severity::error)<<"Port can't be opened "<<port_.getName()<<std::endl;
    }

    positionThread_=std::make_shared<std::thread>(&DemoRedBallPosition::run,this);
    return true;
}

DemoRedBallPosition::DemoRedBallPosition(const std::string &,PolyDriver &driver,const std::string &eye_):eye_(eye_), pos_(4,0.0),visible_(false)
{
    if (!driver.view(igaze_))
        igaze_=NULL;
    pos_[3]=1.0;
}

DemoRedBallPosition::~DemoRedBallPosition()
{
    threadIsActive_=false;
    positionThread_->join();
}

bool DemoRedBallPosition::setPos(const Vector &pos)
{
    if (pos.length()>=3)
    {
        pos_.setSubvector(0,pos.subVector(0,2));
        return true;
    }
    else
        return false;
}

void DemoRedBallPosition::setVisible()   { visible_=true;  }
void DemoRedBallPosition::setInvisible() { visible_=false; }

void DemoRedBallPosition::run()
{
    while(threadIsActive_)
    {
        if (igaze_!=NULL)
        {
            Vector x,o;
            if (eye_=="left")
                igaze_->getLeftEyePose(x,o);
            else
                igaze_->getRightEyePose(x,o);

            Matrix T=axis2dcm(o);
            T.setSubcol(x,0,3);
            Vector pos=SE3inv(T)*pos_;

            Bottle &cmd=port_.prepare();
            cmd.clear();
            cmd.addDouble(pos[0]);
            cmd.addDouble(pos[1]);
            cmd.addDouble(pos[2]);
            cmd.addDouble(0.0);
            cmd.addDouble(0.0);
            cmd.addDouble(0.0);
            cmd.addDouble(visible_?1.0:0.0);
            port_.write();
            
        }
        Time::delay(0.1);
    }
}

ActionRedBall::ActionRedBall(const CommandAttributes& commandAttributes,const std::string& testCode):Action(commandAttributes,testCode)
{ 
 
}     

void ActionRedBall::beforeExecute()
{
   //std::string context=property.check("context",Value("demoRedBall")).asString();
    //std::string from=property.check("from",Value("config-test.ini")).asString();

    getCommandAttribute("tollerance", tollerance_);
    getCommandAttribute("robotname", robotName_);
    getCommandAttribute("eye", eye_);
    getCommandAttribute("useleft", useleft_);
    getCommandAttribute("useright_", useright_);

    std::string context="demoRedBall";
    std::string from="config-test.ini";

    // retrieve demoRedBall parameters
    ResourceFinder rf; rf.setVerbose();
    rf.setDefaultContext(context.c_str());
    rf.setDefaultConfigFile(from.c_str());
    rf.configure(0,NULL);

    // fallback values
    params_.robot_=robotName_;
    params_.eye_=eye_;
    params_.reach_tol_=tollerance_;
    params_.use_left_=useleft_;
    params_.use_right_=useright_;
    params_.home_arm.resize(7,0.0);

    Bottle &general=rf.findGroup("general");
    if (!general.isNull())
    {
        params_.robot_=general.check("robot_",Value(params_.robot_)).asString();
        params_.eye_=general.check("eye_",Value(params_.eye_)).asString();
        params_.reach_tol_=general.check("reach_tol_",Value(params_.reach_tol_)).asDouble();
        params_.use_left_=(general.check("left_arm",Value(params_.use_left_?"on":"off")).asString()=="on");
        params_.use_right_=(general.check("right_arm",Value(params_.use_right_?"on":"off")).asString()=="on");
    }

    Bottle &home_arm=rf.findGroup("home_arm");
    if (!home_arm.isNull())
    {
        if (home_arm.check("poss"))
        {
            Bottle &poss=home_arm.findGroup("poss");
            for (size_t i=0; i<std::min(params_.home_arm.length(),(size_t)poss.size()-1); i++)
                params_.home_arm[i]=poss.get(1+i).asDouble();
        }
    }

    if (params_.use_left_)
    {
        Property optJoint;
        optJoint.put("device","remote_controlboard");
        optJoint.put("remote",("/"+params_.robot_+"/"+"left_arm"));
        optJoint.put("local",("/xxx/joint/left_arm"));

        Property optCart;
        optCart.put("device","cartesiancontrollerclient");
        optCart.put("remote",("/"+params_.robot_+"/"+"cartesianController/left_arm"));
        optCart.put("local",("/xxx/cartesian/left_arm"));
        bool res=drvJointArmL_.open(optJoint)&&drvCartArmL_.open(optCart);
        if(!res)
            addProblem({0,0},Severity::error,"Unable to open clients for left_arm.",true);        
    }

    if (params_.use_right_)
    {
        Property optJoint;
        optJoint.put("device","remote_controlboard");
        optJoint.put("remote",("/"+params_.robot_+"/"+"right_arm"));
        optJoint.put("local",("/xxx/joint/right_arm"));

        Property optCart;
        optCart.put("device","cartesiancontrollerclient");
        optCart.put("remote",("/"+params_.robot_+"/"+"cartesianController/right_arm"));
        optCart.put("local",("/xxx/cartesian/right_arm"));

        bool res=drvJointArmR_.open(optJoint)&&drvCartArmR_.open(optCart);
        if(!res)
            addProblem({0,0},Severity::error,"Unable to open clients for right_arm.",true);        
    }

    {
        Property optJoint;
        optJoint.put("device","remote_controlboard");
        optJoint.put("remote",("/"+params_.robot_+"/"+"head"));
        optJoint.put("local",("/xxx/joint/head"));

        Property optGaze;
        optGaze.put("device","gazecontrollerclient");
        optGaze.put("remote","/iKinGazeCtrl");
        optGaze.put("local",("/xxx/gaze"));

        bool res=drvJointHead_.open(optJoint)&&drvGaze_.open(optGaze);
        if(!res)
            addProblem({0,0},Severity::error,"Unable to open clients for head.",true);       
    }

    {
        Property optJoint;
        optJoint.put("device","remote_controlboard");
        optJoint.put("remote",("/"+params_.robot_+"/"+"torso"));
        optJoint.put("local",("/xxx/joint/torso"));

        bool res=drvJointTorso_.open(optJoint);
        if(!res)
            addProblem({0,0},Severity::error,"Unable to open clients for torso.",true);
    }

    redBallPos_=std::make_unique<DemoRedBallPosition>("xxx",drvGaze_,params_.eye_);
    redBallPos_->start();    
}

execution ActionRedBall::execute(const TestRepetitions& testrepetition)
{
    Vector pos(3,0.0);
    pos[0]=-0.3;

    pos[1]=-0.15;
    drvJointArmL_.view(armUnderTest_.ienc_);
    drvCartArmL_.view(armUnderTest_.iarm_);
    testBallPosition(pos,testrepetition);

    pos[1]=+0.15;
    drvJointArmR_.view(armUnderTest_.ienc_);
    drvCartArmR_.view(armUnderTest_.iarm_);
    testBallPosition(pos,testrepetition);

    return execution::continueexecution;
}

void ActionRedBall::testBallPosition(const Vector &pos,const TestRepetitions& testrepetition)
{
    redBallPos_->setPos(pos);
    redBallPos_->setVisible();

    Vector x,o,encs;
    int nEncs; IEncoders* ienc_;
    bool done=false;
    double t0;

    IGazeControl* igaze;
    drvGaze_.view(igaze);
    t0=Time::now();
    while (Time::now()-t0<10.0)
    {
        igaze->getFixationPoint(x);
        if (norm(pos-x)<2.0*params_.reach_tol_)
        {
            done=true;
            break;
        }
    }
    if(!done)
        addProblem(testrepetition,Severity::error,"Ball gazed at with the eyes.",true);
    done=false;  

    t0=Time::now();
    while (Time::now()-t0<10.0)
    {
        armUnderTest_.iarm_->getPose(x,o);
        if (norm(pos-x)<params_.reach_tol_)
        {
            done=true;
            break;
        }
        Time::delay(0.01);
    }
    if(!done)
        addProblem(testrepetition,Severity::error,"Ball not reached with the hand.",true);
    done=false;

    redBallPos_->setInvisible();

    armUnderTest_.ienc_->getAxes(&nEncs);
    encs.resize(nEncs,0.0);
    t0=Time::now();
    while (Time::now()-t0<10.0)
    {
        armUnderTest_.ienc_->getEncoders(encs.data());
        if (norm(params_.home_arm-encs.subVector(0,params_.home_arm.length()-1))<5.0)
        {
            done=true;
            break;
        }
        Time::delay(1.0);
    }
    if(!done)
        addProblem(testrepetition,Severity::error,"Arm has not reached home.",true);
    done=false;

    drvJointHead_.view(ienc_);
    ienc_->getAxes(&nEncs);
    encs.resize(nEncs,0.0);
    t0=Time::now();
    while (Time::now()-t0<10.0)
    {
        ienc_->getEncoders(encs.data());
        if (norm(encs.subVector(0,3))<5.0)
        {
            done=true;
            break;
        }
        Time::delay(1.0);
    }
    if(!done)
        addProblem(testrepetition,Severity::error,"Head has not reached home.",true);
    done=false;    

    drvJointTorso_.view(ienc_);
    ienc_->getAxes(&nEncs);
    encs.resize(nEncs,0.0);
    t0=Time::now();
    while (Time::now()-t0<10.0)
    {
        ienc_->getEncoders(encs.data());
        if (norm(encs.subVector(0,3))<5.0)
        {
            done=true;
            break;
        }
        Time::delay(1.0);
    }

    if(!done)
        addProblem(testrepetition,Severity::error,"Torso has not reached home.",true);

}

}
