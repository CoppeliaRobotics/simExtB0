// Copyright 2016 Coppelia Robotics GmbH. All rights reserved.
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// -------------------------------------------------------------------
// Authors:
// Federico Ferri <federico.ferri.it at gmail dot com>
// -------------------------------------------------------------------

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "v_repPlusPlus/Plugin.h"
#include "plugin.h"
#include "stubs.h"
#include "config.h"

void createNode(SScriptCallBack *p, const char *cmd, createNode_in *in, createNode_out *out)
{
}

void initNode(SScriptCallBack *p, const char *cmd, initNode_in *in, initNode_out *out)
{
}

void cleanupNode(SScriptCallBack *p, const char *cmd, cleanupNode_in *in, cleanupNode_out *out)
{
}

void destroyNode(SScriptCallBack *p, const char *cmd, destroyNode_in *in, destroyNode_out *out)
{
}

void createPublisher(SScriptCallBack *p, const char *cmd, createPublisher_in *in, createPublisher_out *out)
{
}

void destroyPublisher(SScriptCallBack *p, const char *cmd, destroyPublisher_in *in, destroyPublisher_out *out)
{
}

void createSubscriber(SScriptCallBack *p, const char *cmd, createSubscriber_in *in, createSubscriber_out *out)
{
}

void destroySubscriber(SScriptCallBack *p, const char *cmd, destroySubscriber_in *in, destroySubscriber_out *out)
{
}

void createServiceClient(SScriptCallBack *p, const char *cmd, createServiceClient_in *in, createServiceClient_out *out)
{
}

void destroyServiceClient(SScriptCallBack *p, const char *cmd, destroyServiceClient_in *in, destroyServiceClient_out *out)
{
}

void createServiceServer(SScriptCallBack *p, const char *cmd, createServiceServer_in *in, createServiceServer_out *out)
{
}

void destroyServiceServer(SScriptCallBack *p, const char *cmd, destroyServiceServer_in *in, destroyServiceServer_out *out)
{
}

class Plugin : public vrep::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");
    }
};

VREP_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
