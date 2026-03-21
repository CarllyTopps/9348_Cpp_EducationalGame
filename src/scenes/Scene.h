#pragma once

// Scene IDs
enum SceneID {
    SCENE_MENU           = 0,
    SCENE_QUIZ           = 1,
    SCENE_RESULT         = 2,
    SCENE_CATEGORY       = 3,
    SCENE_QUESTION_COUNT = 4,
    SCENE_EXIT           = -1,
    SCENE_STAY           = 99
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void update() = 0;
    virtual void draw()   = 0;
    virtual int  nextScene() { return SCENE_STAY; }  // -1 = exit, 0/1/2/3 = switch
};
