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
    bool applySticker1 = false;
    bool applySticker2 = false;
    bool applyEffect1 = false;
    bool applyEffect2 = false;
    
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
