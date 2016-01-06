#include "FireEffect.h"
#include "FirePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

FireEffect::FireEffect(int id) : RenderableEffect(id, "Fire")
{
    //ctor
}

FireEffect::~FireEffect()
{
    //dtor
}

wxPanel *FireEffect::CreatePanel(wxWindow *parent) {
    return new FirePanel(parent);
}

class FirePaletteClass {
public:
    FirePaletteClass() {
        wxImage::HSVValue hsv;
        int i;
        // calc 100 reds, black to bright red
        hsv.hue=0.0;
        hsv.saturation=1.0;
        for (i=0; i<100; i++)
        {
            hsv.value=double(i)/100.0;
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(xlColor(255, 0, 0, i * 255 / 100));
        }
        
        // gives 100 hues red to yellow
        hsv.value=1.0;
        for (i=0; i<100; i++)
        {
            firePalette.push_back(hsv);
            firePaletteColors.push_back(hsv);
            firePaletteColorsAlpha.push_back(hsv);
            hsv.hue+=0.00166666;
        }
    }
    int size() const {
        return firePalette.size();
    }
    const wxImage::HSVValue &operator[](int x) const {
        return firePalette[x];
    }
    const xlColor &asColor(int x) const {
        return firePaletteColors[x];
    }
    const xlColor &asAlphaColor(int x) const {
        return firePaletteColorsAlpha[x];
    }
    
private:
    hsvVector firePalette;
    xlColorVector firePaletteColors;
    xlColorVector firePaletteColorsAlpha;
};
static const FirePaletteClass FirePalette;

// 0 <= x < BufferWi
// 0 <= y < BufferHt
void SetFireBuffer(int x, int y, int PaletteIdx, std::vector<int> &FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi)
    {
        FireBuffer[y*maxWi+x] = PaletteIdx;
    }
}

// 0 <= x < BufferWi
// 0 <= y < BufferHt
int GetFireBuffer(int x, int y, std::vector<int> &FireBuffer, int maxWi, int maxHi)
{
    if (x >= 0 && x < maxWi && y >= 0 && y < maxHi)
    {
        return FireBuffer[y*maxWi+x];
    }
    return -1;
}

static int GetLocation(const std::string &location) {
    if (location == "Bottom") {
        return 0;
    } else if (location == "Top") {
        return 1;
    } else if (location == "Left") {
        return 2;
    } else if (location == "Right") {
        return 3;
    }
    return 0;
}



class FireRenderCache : public EffectRenderCache {
public:
    FireRenderCache() {};
    virtual ~FireRenderCache() {};
    
    std::vector<int> FireBuffer;
};


static FireRenderCache* GetCache(RenderBuffer &buffer, int id) {
    FireRenderCache *cache = (FireRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new FireRenderCache();
        cache->FireBuffer.resize(buffer.BufferHt * buffer.BufferWi);
        buffer.infoCache[id] = cache;
    }
    return cache;
}

// 10 <= HeightPct <= 100
void FireEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int HeightPct = SettingsMap.GetFloat("SLIDER_Fire_Height", 0);
    int HueShift = SettingsMap.GetFloat("SLIDER_Fire_HueShift", 0);
    float cycles = SettingsMap.GetFloat("TEXTCTRL_Fire_GrowthCycles", 0.0);
    
    int x,y,i,r,v1,v2,v3,v4,n,new_index;
    wxImage::HSVValue hsv;
    int loc = GetLocation(SettingsMap.Get("CHOICE_Fire_Location", "Bottom"));
    FireRenderCache *cache = GetCache(buffer, id);
    
    //cycles is 0 - 200 representing growth cycle count of 0 - 20
    if (cycles > 0) {
        double adjust = buffer.GetEffectTimeIntervalPosition(cycles);
        adjust = 0.5 - std::abs(adjust - 0.5);
        HeightPct += adjust * 100;
    }
    
    if (HeightPct<1) HeightPct=1;
    
    
    int maxHt = buffer.BufferHt;
    int maxWi = buffer.BufferWi;
    if (loc == 2 || loc == 3) {
        maxHt = buffer.BufferWi;
        maxWi = buffer.BufferHt;
    }
    
    if (maxHt<1) maxHt=1;
    float mod_state = 4.0;
    if (buffer.needToInit) {
        buffer.needToInit = false;
        for (i=0; i < cache->FireBuffer.size(); i++) {
            cache->FireBuffer[i]=0;
        }
    } else {
        mod_state = 4 / (buffer.curPeriod%4+1);
    }
    // build fire
    for (x=0; x<maxWi; x++) {
        r=x%2==0 ? 190+(rand() % 10) : 100+(rand() % 50);
        SetFireBuffer(x,0,r, cache->FireBuffer, maxWi, maxHt);
    }
    int step=255*100/maxHt/HeightPct;
    int sum;
    for (y=1; y<maxHt; y++)
    {
        for (x=0; x<maxWi; x++)
        {
            v1=GetFireBuffer(x-1,y-1, cache->FireBuffer, maxWi, maxHt);
            v2=GetFireBuffer(x+1,y-1, cache->FireBuffer, maxWi, maxHt);
            v3=GetFireBuffer(x,y-1, cache->FireBuffer, maxWi, maxHt);
            v4=GetFireBuffer(x,y-1, cache->FireBuffer, maxWi, maxHt);
            n=0;
            sum=0;
            if(v1>=0)
            {
                sum+=v1;
                n++;
            }
            if(v2>=0)
            {
                sum+=v2;
                n++;
            }
            if(v3>=0)
            {
                sum+=v3;
                n++;
            }
            if(v4>=0)
            {
                sum+=v4;
                n++;
            }
            new_index=n > 0 ? sum / n : 0;
            if (new_index > 0)
            {
                new_index+=(rand() % 100 < 20) ? step : -step;
                if (new_index < 0) new_index=0;
                if (new_index >= FirePalette.size()) new_index = FirePalette.size()-1;
            }
            SetFireBuffer(x,y,new_index, cache->FireBuffer, maxWi, maxHt);
        }
    }
    
    //  Now play fire
    for (y=0; y<maxHt; y++)
    {
        for (x=0; x<maxWi; x++)
        {
            int xp = x;
            int yp = y;
            if (loc == 1 || loc == 3) {
                yp = maxHt - y - 1;
            }
            if (loc == 2 || loc == 3) {
                int t = xp;
                xp = yp;
                yp = t;
            }
            if (HueShift>0) {
                hsv = FirePalette[GetFireBuffer(x,y, cache->FireBuffer, maxWi, maxHt)];
                hsv.hue = hsv.hue +(HueShift/100.0);
                if (hsv.hue>1.0) hsv.hue=1.0;
                if (buffer.allowAlpha) {
                    xlColor c(hsv);
                    c.alpha = FirePalette.asAlphaColor(GetFireBuffer(x,y, cache->FireBuffer, maxWi, maxHt)).Alpha();
                    buffer.SetPixel(xp, yp, c);
                } else {
                    buffer.SetPixel(xp, yp, hsv);
                }
            } else {
                if (buffer.allowAlpha) {
                    buffer.SetPixel(xp, yp, FirePalette.asAlphaColor(GetFireBuffer(x,y, cache->FireBuffer, maxWi, maxHt)));
                } else {
                    buffer.SetPixel(xp, yp, FirePalette.asColor(GetFireBuffer(x,y, cache->FireBuffer, maxWi, maxHt)));
                }
            }
        }
    }
}
