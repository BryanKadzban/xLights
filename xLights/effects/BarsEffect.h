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

#include "RenderableEffect.h"

#define BARCOUNT_MIN 1
//#define BARCOUNT_MAX 50
#define BARCOUNT_MAX 5

#define BARCYCLES_MIN 0
//#define BARCYCLES_MAX 500
#define BARCYCLES_MAX 300

#define BARCENTER_MIN -100
#define BARCENTER_MAX 100

class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return true; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer &buffer, bool gradient, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index);
    private:
};
