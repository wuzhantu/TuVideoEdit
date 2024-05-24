//
//  VideoRenderConfig.hpp
//  TuVideoEdit
//
//  Created by zhantu wu on 2024/5/21.
//

#ifndef VideoRenderConfig_hpp
#define VideoRenderConfig_hpp

#include <stdio.h>

using namespace std;

class VideoRenderConfig {
private:
    static VideoRenderConfig* instance;
   
public:
    bool applyInversionFilter = false;
    bool applyGrayscaleFilter = false;
    bool applyTigerSticker = false;
    bool applyAirplaneSticker = false;
    bool applyMirrorEffect = false;
    bool applyFourGridEffect = false;
    const char *text = NULL;
    
public:
    static VideoRenderConfig* shareInstance() {
        if (instance == nullptr) {
            instance = new VideoRenderConfig();
        }
        return instance;
    }
    
    VideoRenderConfig() {};
};

#endif /* VideoRenderConfig_hpp */
