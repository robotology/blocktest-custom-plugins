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

class DemoRedBallPosition
{
public:
    bool start()
    {
        std::string dest="/demoRedBall/trackTarget:i";
        port.open(("/xxx/redballpos:o"));
        bool res=Network::connect(port.getName(),dest,"udp");
        if(!res)
        {
            TXLOG(Severity::error)<<"Port can't be opened "<<port.getName()<<std::endl;
        }

        mythread=std::make_shared<std::thread>(&DemoRedBallPosition::run,this);
        return true;
    }

    DemoRedBallPosition(const std::string &,PolyDriver &driver,const std::string &eye_) :
                        eye_(eye_), pos(4,0.0),
                        visible(false)
    {
        if (!driver.view(igaze))
            igaze=NULL;
        pos[3]=1.0;
    }

    bool setPos(const Vector &pos)
    {
        if (pos.length()>=3)
        {
            this->pos.setSubvector(0,pos.subVector(0,2));
            return true;
        }
        else
            return false;
    }

    void setVisible()   { visible=true;  }
    void setInvisible() { visible=false; }

private:
    void run()
    {
        while(1)
        {
            if (igaze!=NULL)
            {
                Vector x,o;
                if (eye_=="left")
                    igaze->getLeftEyePose(x,o);
                else
                    igaze->getRightEyePose(x,o);

                Matrix T=axis2dcm(o);
                T.setSubcol(x,0,3);
                Vector pos_=SE3inv(T)*pos;

                Bottle &cmd=port.prepare();
                cmd.clear();
                cmd.addDouble(pos_[0]);
                cmd.addDouble(pos_[1]);
                cmd.addDouble(pos_[2]);
                cmd.addDouble(0.0);
                cmd.addDouble(0.0);
                cmd.addDouble(0.0);
                cmd.addDouble(visible?1.0:0.0);
                port.write();
                
            }
            std::this_thread::sleep_for (100ms);
        }
    }

    std::string name;
    IGazeControl *igaze;
    std::string eye_;
    Vector pos;
    bool visible;
    BufferedPort<Bottle> port;
    std::shared_ptr<std::thread> mythread;    
};




ActionRedBall::ActionRedBall(const CommandAttributes& commandAttributes,const std::string& testCode):Action(commandAttributes,testCode)
{ 
    //std::string context=property.check("context",Value("demoRedBall")).asString();
    //std::string from=property.check("from",Value("config-test.ini")).asString();

    std::string context="demoRedBall";
    std::string from="config-test.ini";

    // retrieve demoRedBall parameters
    ResourceFinder rf; rf.setVerbose();
    rf.setDefaultContext(context.c_str());
    rf.setDefaultConfigFile(from.c_str());
    rf.configure(0,NULL);

    // fallback values
    params_.robot_="icubSim";
    params_.eye_="left";
    params_.reach_tol_=0.01;
    params_.use_left_=true;
    params_.use_right_=true;
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
            TXLOG(Severity::error)<<"Unable to open clients for left_arm"<<std::endl;
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
            TXLOG(Severity::error)<<"Unable to open clients for right_arm"<<std::endl;

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
            TXLOG(Severity::error)<<"Unable to open clients for head"<<std::endl;

    }

    {
        Property optJoint;
        optJoint.put("device","remote_controlboard");
        optJoint.put("remote",("/"+params_.robot_+"/"+"torso"));
        optJoint.put("local",("/xxx/joint/torso"));

        bool res=drvJointTorso_.open(optJoint);
        if(!res)
            TXLOG(Severity::error)<<"Unable to open clients for torso"<<std::endl;
    }

    TXLOG(Severity::info)<<"ActionRedBall 2"<<std::endl;
    redBallPos_=std::make_unique<DemoRedBallPosition>("xxx",drvGaze_,params_.eye_);
    redBallPos_->start();
}     

void ActionRedBall::beforeExecute()
{
}

execution ActionRedBall::execute(const TestRepetitions&)
{
    TXLOG(Severity::info)<<"ActionRedBall::execute"<<std::endl;
    Vector pos(3,0.0);
    pos[0]=-0.3;

    pos[1]=-0.15;
    drvJointArmL_.view(armUnderTest_.ienc_);
    drvCartArmL_.view(armUnderTest_.iarm_);
    //ROBOTTESTINGFRAMEWORK_TEST_REPORT("Reaching with the left hand");
    testBallPosition(pos);

    pos[1]=+0.15;
    drvJointArmR_.view(armUnderTest_.ienc_);
    drvCartArmR_.view(armUnderTest_.iarm_);
    //ROBOTTESTINGFRAMEWORK_TEST_REPORT("Reaching with the right hand");
    testBallPosition(pos);

    return execution::continueexecution;
}

void ActionRedBall::testBallPosition(const Vector &pos)
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
        Time::delay(0.01);
    }
    ROBOTTESTINGFRAMEWORK_TEST_CHECK(done,"Ball gazed at with the eyes!");

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
    ROBOTTESTINGFRAMEWORK_TEST_CHECK(done,"Ball reached with the hand!");

    ROBOTTESTINGFRAMEWORK_TEST_REPORT("Going home");
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
    ROBOTTESTINGFRAMEWORK_TEST_CHECK(done,"Arm has reached home!");

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
    ROBOTTESTINGFRAMEWORK_TEST_CHECK(done,"Head has reached home!");

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

    ROBOTTESTINGFRAMEWORK_TEST_CHECK(done,"Torso has reached home!");
}


}