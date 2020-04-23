#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ListenerBase.h"
#include <string>
#include <wx/wx.h>

class wxDatagramSocket;

class ListenerARTNet : public ListenerBase
{
    wxDatagramSocket* _socket;

    bool IsValidHeader(uint8_t* buffer);

public:
    ListenerARTNet(ListenerManager* _listenerManager);
    virtual ~ListenerARTNet() {}
    virtual void Start() override;
    virtual void Stop() override;
    virtual std::string GetType() const override { return "ARTNet"; }
    virtual void StartProcess() override;
    virtual void StopProcess() override;
    virtual void Poll() override;
};
