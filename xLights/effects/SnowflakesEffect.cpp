#include "SnowflakesEffect.h"
#include "SnowflakesPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SnowflakesEffect::SnowflakesEffect(int id) : RenderableEffect(id, "Snowflakes")
{
    //ctor
}

SnowflakesEffect::~SnowflakesEffect()
{
    //dtor
}
wxPanel *SnowflakesEffect::CreatePanel(wxWindow *parent) {
    return new SnowflakesPanel(parent);
}

int static possible_downward_moves(RenderBuffer &buffer, int x, int y)
{
    int moves = 0;
    
    // no moves possible from bottom row
    if (y == 0) {
        return 0;
    }
    
    if (buffer.GetTempPixel(x-1 < 0 ? x-1+buffer.BufferWi : x-1, y-1) == xlBLACK) {
        moves += 1;
    }
    if (buffer.GetTempPixel(x, y-1) == xlBLACK) {
        moves += 2;
    }
    if (buffer.GetTempPixel(x+1 >= buffer.BufferWi ? x+1-buffer.BufferWi : x+1, y-1) == xlBLACK) {
        moves += 4;
    }
    
    return moves;
}

static void set_pixel_if_not_color(RenderBuffer &buffer, int x, int y, xlColor toColor, xlColor notColor, bool wrapx, bool wrapy)
{
    int adjx = x;
    int adjy = y;
    
    if (x < 0) {
        if (wrapx) {
            adjx += buffer.BufferWi;
        }
        else {
            return;
        }
    }
    else if (x >= buffer.BufferWi) {
        if (wrapx) {
            adjx -= buffer.BufferWi;
        }
        else {
            return;
        }
    }
    if (y < 0) {
        if (wrapy) {
            adjy += buffer.BufferHt;
        }
        else {
            return;
        }
    }
    else if (y >= buffer.BufferHt) {
        if (wrapy) {
            adjy -= buffer.BufferHt;
        }
        else {
            return;
        }
    }
    
    // strip off alpha when comparing
    if (buffer.GetTempPixel(adjx, adjy).GetRGB() != notColor.GetRGB()) {
        buffer.SetPixel(adjx, adjy, toColor);
    }
}


class SnowflakesRenderCache : public EffectRenderCache {
public:
    SnowflakesRenderCache() {};
    virtual ~SnowflakesRenderCache() {};
    
    int LastSnowflakeCount;
    int LastSnowflakeType;
    bool LastSnowflakeAccumulate;
    int effectState;
};


void SnowflakesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    int Count = SettingsMap.GetInt("SLIDER_Snowflakes_Count", 0);
    int SnowflakeType = SettingsMap.GetInt("SLIDER_Snowflakes_Type", 0);
    int sSpeed = SettingsMap.GetInt("SLIDER_Snowflakes_Speed", 0);
    bool accumulate = SettingsMap.GetBool("CHECKBOX_Snowflakes_Accumulate");
    
    int i,n,x,x0,y0,y,check,delta_y;
    xlColour color1,color2, color3;
    bool wrapx = false; // set to true if you want snowflakes to draw wrapped around when near edges in the accumulate effect.
    
    
    SnowflakesRenderCache *cache = (SnowflakesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new SnowflakesRenderCache();
        buffer.infoCache[id] = cache;
    }
    
    int &LastSnowflakeCount = cache->LastSnowflakeCount;
    int &LastSnowflakeType = cache->LastSnowflakeType;
    bool &LastSnowflakeAccumulate = cache->LastSnowflakeAccumulate;
    int &effectState = cache->effectState;
    
    
    buffer.palette.GetColor(0, color1);
    buffer.palette.GetColor(1, color2);
    
    if (buffer.needToInit ||
        Count != LastSnowflakeCount ||
        SnowflakeType != LastSnowflakeType ||
        accumulate != LastSnowflakeAccumulate) {
        
        // initialize
        buffer.needToInit = false;
        LastSnowflakeCount=Count;
        LastSnowflakeType=SnowflakeType;
        LastSnowflakeAccumulate=accumulate;
        buffer.ClearTempBuf();
        effectState = 0;
        
        // place Count snowflakes
        for (n=0; n < Count; n++) {
            
            delta_y=buffer.BufferHt/4;
            y0=(n % 4)*delta_y;
            
            if (y0+delta_y > buffer.BufferHt) delta_y = buffer.BufferHt-y0;
            if (delta_y<1) delta_y=1;
            
            // find unused space
            for (check=0; check < 20; check++)
            {
                x=rand() % buffer.BufferWi;
                y=y0 + (rand() % delta_y);
                if (buffer.GetTempPixel(x,y) == xlBLACK) {
                    effectState++;
                    break;
                }
            }
            
            // draw flake, SnowflakeType=0 is random type
            switch (SnowflakeType == 0 ? rand() % 5 : SnowflakeType-1)
            {
                case 0:
                    // single node
                    if(accumulate) {
                        buffer.SetTempPixel(x, y, color1, 0);
                    }
                    else {
                        buffer.SetTempPixel(x, y, color1);
                    }
                    break;
                case 1:
                    // 5 nodes
                    if (x < 1) x+=1;
                    if (y < 1) y+=1;
                    if (x > buffer.BufferWi-2) x-=1;
                    if (y > buffer.BufferHt-2) y-=1;
                    if(accumulate) {
                        buffer.SetTempPixel(x,y,color1, 1);
                    }
                    else {
                        buffer.SetTempPixel(x,y,color1);
                        buffer.SetTempPixel(x-1,y,color2);
                        buffer.SetTempPixel(x+1,y,color2);
                        buffer.SetTempPixel(x,y-1,color2);
                        buffer.SetTempPixel(x,y+1,color2);
                    }
                    break;
                case 2:
                    // 3 nodes
                    if (x < 1) x+=1;
                    if (y < 1) y+=1;
                    if (x > buffer.BufferWi-2) x-=1;
                    if (y > buffer.BufferHt-2) y-=1;
                    if(accumulate)
                    {
                        buffer.SetTempPixel(x,y,color1, 2);
                    }
                    else
                    {
                        buffer.SetTempPixel(x,y,color1);
                        if (rand() % 100 > 50)      // % 2 was not so random
                        {
                            buffer.SetTempPixel(x-1,y,color2);
                            buffer.SetTempPixel(x+1,y,color2);
                        }
                        else
                        {
                            buffer.SetTempPixel(x,y-1,color2);
                            buffer.SetTempPixel(x,y+1,color2);
                        }
                    }
                    break;
                case 3:
                    // 9 nodes
                    if (x < 2) x+=2;
                    if (y < 2) y+=2;
                    if (x > buffer.BufferWi-3) x-=2;
                    if (y > buffer.BufferHt-3) y-=2;
                    if(accumulate) {
                        buffer.SetTempPixel(x, y, color1, 3);
                    }
                    else {
                        buffer.SetTempPixel(x, y, color1);
                        for (i=1; i<=2; i++)
                        {
                            buffer.SetTempPixel(x-i,y,color2);
                            buffer.SetTempPixel(x+i,y,color2);
                            buffer.SetTempPixel(x,y-i,color2);
                            buffer.SetTempPixel(x,y+i,color2);
                        }
                    }
                    break;
                case 4:
                    // 13 nodes
                    if (x < 2) x+=2;
                    if (y < 2) y+=2;
                    if (x > buffer.BufferWi-3) x-=2;
                    if (y > buffer.BufferHt-3) y-=2;
                    if(accumulate) {
                        buffer.SetTempPixel(x, y, color1, 4);
                    }
                    else {
                        buffer.SetTempPixel(x, y, color1);
                        buffer.SetTempPixel(x-1,y,color2);
                        buffer.SetTempPixel(x+1,y,color2);
                        buffer.SetTempPixel(x,y-1,color2);
                        buffer.SetTempPixel(x,y+1,color2);
                        
                        buffer.SetTempPixel(x-1,y+2,color2);
                        buffer.SetTempPixel(x+1,y+2,color2);
                        buffer.SetTempPixel(x-1,y-2,color2);
                        buffer.SetTempPixel(x+1,y-2,color2);
                        buffer.SetTempPixel(x+2,y-1,color2);
                        buffer.SetTempPixel(x+2,y+1,color2);
                        buffer.SetTempPixel(x-2,y-1,color2);
                        buffer.SetTempPixel(x-2,y+1,color2);
                    }
                    break;
                case 5:
                    // 45 nodes (not enabled)
                    break;
            }
        }
    }
    
    // move snowflakes
    int movement = (buffer.curPeriod - buffer.curEffStartPer) * sSpeed * buffer.frameTimeInMs / 50;
    int new_x,new_y;
    int starty = accumulate ? 1 : 0;
    
    for (x=0; x<buffer.BufferWi; x++) {
        new_x = (x+movement/20) % buffer.BufferWi; // CW
        
        for (y=starty; y<buffer.BufferHt; y++) {
            if (accumulate) {
                
                // this controls the speed by skipping movement when slow
                if (((buffer.curPeriod-buffer.curEffStartPer) * (sSpeed+1)) / 30 != ((buffer.curPeriod-buffer.curEffStartPer-1) * (sSpeed + 1)) / 30)
                {
                    // if there is a flake to move
                    buffer.GetTempPixel(x, y, color3);
                    if (color3 != xlBLACK) {
                        
                        // check where we can move to?
                        int moves = possible_downward_moves(buffer, x, y);
                        x0 = x;
                        
                        //we have something to move
                        // randomly move the flake left or right
                        if (moves > 0)
                        {
                            switch(rand() % 5)
                            {
                                case 0:
                                    if (moves & 1) {
                                        x0 = x - 1;
                                    }
                                    else {
                                        if (moves & 2) {
                                            x0 = x;
                                        }
                                        else {
                                            x0 = x + 1;
                                        }
                                    }
                                    break;
                                case 1:
                                    if (moves & 4) {
                                        x0 = x + 1;
                                    }
                                    else {
                                        if (moves & 2) {
                                            x0 = x;
                                        }
                                        else {
                                            x0 = x - 1;
                                        }
                                    }
                                    break;
                                default:  //down more often then left/right to look less "jittery"
                                    if (moves & 2) {
                                        x0 = x;
                                    }
                                    else if ((moves & 5) == 4) {
                                        x0 = x + 1;
                                    }
                                    else if ((moves & 5) == 1) {
                                        x0 = x - 1;
                                    }
                                    else {
                                        switch(rand() % 2)
                                        {
                                            case 0:
                                                x0 = x+1;
                                                break;
                                            default:
                                                x0 = x-1;
                                                break;
                                        }
                                    }
                                    break;
                            }
                            
                            // handle wrap around
                            if (x0 < 0) {
                                x0 += buffer.BufferWi;
                            }
                            else if (x0 >= buffer.BufferWi) {
                                x0 -= buffer.BufferWi;
                            }
                            
                            // and move it down
                            y0 = y - 1;
                            
                            // move the flake down
                            buffer.SetTempPixel(x0, y0, color3);
                            buffer.SetTempPixel(x, y, xlBLACK);
                            
                            int nextmoves = possible_downward_moves(buffer, x0, y0);
                            if (nextmoves == 0) {
                                // we cant move any further so we can add one at the top
                                effectState--;
                            }
                        }
                    }
                }
            } else {
                new_y = (y+movement/10) % buffer.BufferHt;
                // strip off the alpha channel
                buffer.SetPixel(x, y, buffer.GetTempPixel(new_x, new_y));
            }
        }
    }
    
    if (accumulate)
    {
        // add new flakes to the top
        check = 0;
        int placedFullCount = 0;
        while (effectState < Count && check < 20) {
            // find unused space
            x=rand() % buffer.BufferWi;
            if (buffer.GetTempPixel(x, buffer.BufferHt-1) == xlBLACK) {
                effectState++;
                buffer.SetTempPixel(x, buffer.BufferHt-1, color1, SnowflakeType == 0 ? rand() % 5 : SnowflakeType-1);
                
                int nextmoves = possible_downward_moves(buffer, x, buffer.BufferHt-1);
                if (nextmoves == 0) {
                    //the placed pixel fills the column, make sure we note that so we can place
                    //another snowflake
                    placedFullCount++;
                }
            }
            check++;
        }
        effectState -= placedFullCount;
        
        // paint my current state
        for (int y=0; y < buffer.BufferHt; y++) {
            for (int x=0; x < buffer.BufferWi; x++) {
                
                buffer.GetTempPixel(x, y, color3);
                if (color3 != xlBLACK)
                {
                    // draw flake, SnowflakeType=0 is random type
                    switch (color3.Alpha())
                    {
                        case 0:
                            // single node
                            buffer.SetPixel(x, y, color1);
                            break;
                        case 1:
                            // 5 nodes
                            buffer.SetPixel(x,y,color1);
                            set_pixel_if_not_color(buffer, x-1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x, y+1, color2, color1, wrapx, false);
                            break;
                        case 2:
                        {
                            // 3 nodes
                            buffer.SetPixel(x,y,color1);
                            bool isAtBottom = true;
                            for (int yt = 0; yt < y - 1; yt++) {
                                if (buffer.GetTempPixel(x, yt) == xlBLACK) {
                                    isAtBottom = false;
                                    break;
                                }
                            }
                            
                            // when flake has settled always paint it horizontally
                            if (isAtBottom)
                            {
                                set_pixel_if_not_color(buffer, x-1, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(buffer, x+1, y, color2, color1, wrapx, false);
                            }
                            else {
                                if (rand() % 100 > 50)      // % 2 was not so random
                                {
                                    set_pixel_if_not_color(buffer, x-1, y, color2, color1, wrapx, false);
                                    set_pixel_if_not_color(buffer, x+1, y, color2, color1, wrapx, false);
                                }
                                else
                                {
                                    set_pixel_if_not_color(buffer, x, y-1, color2, color1, wrapx, false);
                                    set_pixel_if_not_color(buffer, x, y+1, color2, color1, wrapx, false);
                                }
                            }
                        }
                            break;
                        case 3:
                            // 9 nodes
                            buffer.SetPixel(x,y,color1);
                            for (i=1; i<=2; i++)
                            {
                                set_pixel_if_not_color(buffer, x-i, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(buffer, x+i, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(buffer, x, y-i, color2, color1, wrapx, false);
                                set_pixel_if_not_color(buffer, x, y+i, color2, color1, wrapx, false);
                            }
                            break;
                        case 4:
                            // 13 nodes
                            buffer.SetPixel(x,y,color1);
                            set_pixel_if_not_color(buffer, x-1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x, y+1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x, y-1, color2, color1, wrapx, false);
                            
                            set_pixel_if_not_color(buffer, x-1, y+2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+1, y+2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x-1, y-2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+1, y-2, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+2, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x+2, y+1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x-2, y-1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x-2, y+1, color2, color1, wrapx, false);
                            break;
                        case 5:
                            // 45 nodes (not enabled)
                            break;
                    }
                }
            }
        }
    }
}
