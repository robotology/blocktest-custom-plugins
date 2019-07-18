/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file ActionCustom.cpp
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */


#include "actionCustom.h"
#include "logger.h"

ACTIONREGISTER_DEF_TYPE(CustomActions::ActionCustom,"custom");

namespace CustomActions
{

ActionCustom::ActionCustom(const CommandAttributes& commandAttributes,const std::string& testCode):Action(commandAttributes,testCode)
{ 
}     

void ActionCustom::beforeExecute()
{
}

execution ActionCustom::execute(const TestRepetitions&)
{
    return execution::continueexecution;
}

}