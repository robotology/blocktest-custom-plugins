/******************************************************************************
 *                                                                            *
 * Copyright (C) 2019 Fondazione Istituto Italiano di Tecnologia (IIT)        *
 * All Rights Reserved.                                                       *
 *                                                                            *
 ******************************************************************************/

/**
 * @file customActionDepotStart.h
 * @author Luca Tricerri <luca.tricerri@iit.it>
 */

#pragma once

#include "actionDepotStart.h"

using namespace BlockTestCore;

/**
 * @class CustomActionDepotStart
 * @brief ...
 * @todo missing brief and detailed description
 */
class CustomActionDepotStart :public ActionDepotStart
{
    public:
        CustomActionDepotStart(); 

        void configure(const std::map<std::string,std::string>&) override;
        void stop() override;
};

