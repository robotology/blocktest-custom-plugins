
/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file ActionRedBall.h
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */

#pragma once

#include "action.h"

#include <yarp/os/Property.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/sig/Matrix.h>
#include <yarp/math/Math.h>

#include <memory>
#include <atomic>

using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;

using namespace BlockTestCore;

namespace CustomActions
{

class DemoRedBallPosition;

class ActionRedBall : public Action
{
public:
    ActionRedBall(const CommandAttributes& commandAttributes,const std::string& testCode);    
    execution execute(const TestRepetitions& testrepetition) override;
    void beforeExecute() override;

private:        

    void testBallPosition(const Vector &pos,const TestRepetitions& testrepetition);
    std::unique_ptr<DemoRedBallPosition> redBallPos_;

    class Params 
    {
        public:
            std::string robot_;
            std::string eye_;
            double reach_tol_;
            bool use_left_;
            bool use_right_;
            yarp::sig::Vector home_arm;
    };     

    class ArmUnderTest
    {
        public:
            yarp::dev::ICartesianControl *iarm_;
            yarp::dev::IEncoders         *ienc_;
    };       

    ArmUnderTest armUnderTest_;
    Params params_;

    yarp::dev::PolyDriver drvJointArmL_;
    yarp::dev::PolyDriver drvJointArmR_;
    yarp::dev::PolyDriver drvJointTorso_;
    yarp::dev::PolyDriver drvJointHead_;
    yarp::dev::PolyDriver drvCartArmL_;
    yarp::dev::PolyDriver drvCartArmR_;
    yarp::dev::PolyDriver drvGaze_;

    double tollerance_{0.1};
    std::string robotName_;
    std::string eye_;
    bool useleft_;
    bool useright_;

    ACTIONREGISTER_DEC_TYPE(ActionRedBall)        
};


class DemoRedBallPosition
{
public:
    bool start();

    DemoRedBallPosition(const std::string &,PolyDriver &driver,const std::string &eye_);
    ~DemoRedBallPosition();

    bool setPos(const Vector &pos);

    void setVisible();
    void setInvisible();

private:
    void run();

    std::string name;
    IGazeControl *igaze_;
    std::string eye_;
    Vector pos_;
    bool visible_;
    BufferedPort<Bottle> port_;
    
    std::shared_ptr<std::thread> positionThread_;   
    std::atomic<bool> threadIsActive_{true}; 
};

}
