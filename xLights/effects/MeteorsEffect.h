#ifndef METEORSEFFECT_H
#define METEORSEFFECT_H

#include "RenderableEffect.h"


class MeteorsEffect : public RenderableEffect
{
    public:
        MeteorsEffect(int id);
        virtual ~MeteorsEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
    
    
    void RenderMeteorsVertical(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsHorizontal(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsImplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed);
    void RenderIcicleDrip(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsExplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed);

};

#endif // METEORSEFFECT_H
