/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file actionApplyForce.cpp
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */

#include "general.h"
#include "logger.h"
#include "action.h"

#include "customActionDepotStart.h"

ACTIONDEPOTSTART(CustomActionDepotStart)

CustomActionDepotStart::CustomActionDepotStart()
{
    TXLOG(Severity::info)<<"Library setup"<<std::endl;
}     

void CustomActionDepotStart::configure(const std::map<std::string,std::string>&)
{
    //Nothing todo    
}

void CustomActionDepotStart::stop()
{
    //Nothing todo    
}