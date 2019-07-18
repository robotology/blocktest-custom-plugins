/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file ActionCustom.h
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */

#pragma once

#include "action.h"

using namespace BlockTestCore;

namespace CustomActions
{

/**
 * @class ActionCustom
 * @brief ...
 * @todo missing brief and detailed description
 */
class ActionCustom : public Action
{
    public:
        ActionCustom(const CommandAttributes& commandAttributes,const std::string& testCode);    
        execution execute(const TestRepetitions& testrepetition) override;
        void beforeExecute() override;

    private:        
        double seconds_{0};

    ACTIONREGISTER_DEC_TYPE(ActionCustom)        
};

}
