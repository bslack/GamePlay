#include "Base.h"
#include "AnimationController.h"
#include "Game.h"
#include "Curve.h"

namespace gameplay
{

AnimationController::AnimationController()
    : _state(STOPPED), _animations(NULL)
{
}

AnimationController::~AnimationController()
{
    std::vector<Animation*>::iterator itr = _animations.begin();
    for ( ; itr != _animations.end(); itr++)
    {
        Animation* temp = *itr;
        SAFE_RELEASE(temp);
    }

    _animations.clear();
}

Animation* AnimationController::createAnimation(const char* id, AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned long* keyTimes, float* keyValues, Curve::InterpolationType type)
{
    assert(type != Curve::BEZIER && type != Curve::HERMITE);
    assert(keyCount >= 2 && keyTimes && keyValues && target);

    Animation* animation = new Animation(id, target, propertyId, keyCount, keyTimes, keyValues, type);

    addAnimation(animation);
    
    return animation;
}

Animation* AnimationController::createAnimation(const char* id, AnimationTarget* target, int propertyId, unsigned int keyCount, unsigned long* keyTimes, float* keyValues, float* keyInValue, float* keyOutValue, Curve::InterpolationType type)
{
    assert(target && keyCount >= 2 && keyTimes && keyValues && keyInValue && keyOutValue);
    Animation* animation = new Animation(id, target, propertyId, keyCount, keyTimes, keyValues, keyInValue, keyOutValue, type);

    addAnimation(animation);

    return animation;
}

Animation* AnimationController::createAnimation(const char* id, AnimationTarget* target, const char* animationFile)
{
    assert(target && animationFile);
    
    Properties* p = Properties::create(animationFile);
    assert(p);

    Animation* animation = createAnimation(id, target, p->getNextNamespace());

    SAFE_DELETE(p);

    return animation;
}

Animation* AnimationController::createAnimationFromTo(const char* id, AnimationTarget* target, int propertyId, float* from, float* to, Curve::InterpolationType type, unsigned long duration)
{
    const unsigned int propertyComponentCount = target->getAnimationPropertyComponentCount(propertyId);
    float* keyValues = new float[2 * propertyComponentCount];

    memcpy(keyValues, from, sizeof(float) * propertyComponentCount);
    memcpy(keyValues + propertyComponentCount, to, sizeof(float) * propertyComponentCount);

    unsigned long* keyTimes = new unsigned long[2];
    keyTimes[0] = 0;
    keyTimes[1] = duration;

    Animation* animation = createAnimation(id, target, propertyId, 2, keyTimes, keyValues, type);

    SAFE_DELETE_ARRAY(keyValues);
    SAFE_DELETE_ARRAY(keyTimes);
    
    return animation;
}

Animation* AnimationController::createAnimationFromBy(const char* id, AnimationTarget* target, int propertyId, float* from, float* by, Curve::InterpolationType type, unsigned long duration)
{
    const unsigned int propertyComponentCount = target->getAnimationPropertyComponentCount(propertyId);
    float* keyValues = new float[2 * propertyComponentCount];

    memcpy(keyValues, from, sizeof(float) * propertyComponentCount);
    memcpy(keyValues + propertyComponentCount, by, sizeof(float) * propertyComponentCount);

    unsigned long* keyTimes = new unsigned long[2];
    keyTimes[0] = 0;
    keyTimes[1] = duration;

    Animation* animation = createAnimation(id, target, propertyId, 2, keyTimes, keyValues, type);

    SAFE_DELETE_ARRAY(keyValues);
    SAFE_DELETE_ARRAY(keyTimes);

    return animation;
}

Animation* AnimationController::getAnimation(const char* id) const
{
    unsigned int animationCount = _animations.size();
    for (unsigned int i = 0; i < animationCount; i++)
    {
        if (_animations.at(i)->_id.compare(id) == 0)
        {
            return _animations.at(i);
        }
    }
    return NULL;
}

Animation* AnimationController::getAnimation(AnimationTarget* target) const
{
    if (!target)
        return NULL;
    const unsigned int animationCount = _animations.size();
    for (unsigned int i = 0; i < animationCount; ++i)
    {
        Animation* animation = _animations[i];
        if (animation->targets(target))
        {
            return animation;
        }
    }
    return NULL;
}

void AnimationController::stopAllAnimations() 
{
    std::list<AnimationClip*>::iterator clipIter = _runningClips.begin();
    while (clipIter != _runningClips.end())
    {
        AnimationClip* clip = *clipIter;
        clip->stop();
        clipIter++;
    }
}

Animation* AnimationController::createAnimation(const char* id, AnimationTarget* target, Properties* animationProperties)
{
    assert(target && animationProperties);
    assert(std::strcmp(animationProperties->getNamespace(), "animation") == 0);
    
    const char* propertyIdStr = animationProperties->getString("property");
    assert(propertyIdStr);
    
    // Get animation target property id
    int propertyId = AnimationTarget::getPropertyId(target->_targetType, propertyIdStr);
    assert(propertyId != -1);
    
    unsigned int keyCount = animationProperties->getInt("keyCount");
    assert(keyCount > 0);

    const char* keyTimesStr = animationProperties->getString("keyTimes");
    assert(keyTimesStr);
    
    const char* keyValuesStr = animationProperties->getString("keyValues");
    assert(keyValuesStr);
    
    const char* curveStr = animationProperties->getString("curve");
    assert(curveStr);
    
    char delimeter = ' ';
    unsigned int startOffset = 0;
    unsigned int endOffset = (unsigned int)std::string::npos;
    
    unsigned long* keyTimes = new unsigned long[keyCount];
    for (unsigned int i = 0; i < keyCount; i++)
    {
        endOffset = static_cast<std::string>(keyTimesStr).find_first_of(delimeter, startOffset);
        if (endOffset != std::string::npos)
        {
            keyTimes[i] = std::strtoul(static_cast<std::string>(keyTimesStr).substr(startOffset, endOffset - startOffset).c_str(), NULL, 0);
        }
        else
        {
            keyTimes[i] = std::strtoul(static_cast<std::string>(keyTimesStr).substr(startOffset, static_cast<std::string>(keyTimesStr).length()).c_str(), NULL, 0);
        }
        startOffset = endOffset + 1;
    }

    startOffset = 0;
    endOffset = (unsigned int)std::string::npos;
    
    int componentCount = target->getAnimationPropertyComponentCount(propertyId);
    assert(componentCount > 0);
    
    unsigned int components = keyCount * componentCount;
    
    float* keyValues = new float[components];
    for (unsigned int i = 0; i < components; i++)
    {
        endOffset = static_cast<std::string>(keyValuesStr).find_first_of(delimeter, startOffset);
        if (endOffset != std::string::npos)
        {   
            keyValues[i] = std::atof(static_cast<std::string>(keyValuesStr).substr(startOffset, endOffset - startOffset).c_str());
        }
        else
        {
            keyValues[i] = std::atof(static_cast<std::string>(keyValuesStr).substr(startOffset, static_cast<std::string>(keyValuesStr).length()).c_str());
        }
        startOffset = endOffset + 1;
    }

    const char* keyInStr = animationProperties->getString("keyIn");
    float* keyIn = NULL;
    if (keyInStr)
    {
        keyIn = new float[components];
        startOffset = 0;
        endOffset = (unsigned int)std::string::npos;
        for (unsigned int i = 0; i < components; i++)
        {
            endOffset = static_cast<std::string>(keyInStr).find_first_of(delimeter, startOffset);
            if (endOffset != std::string::npos)
            {   
                keyIn[i] = std::atof(static_cast<std::string>(keyInStr).substr(startOffset, endOffset - startOffset).c_str());
            }
            else
            {
                keyIn[i] = std::atof(static_cast<std::string>(keyInStr).substr(startOffset, static_cast<std::string>(keyInStr).length()).c_str());
            }
            startOffset = endOffset + 1;
        }
    }
    
    const char* keyOutStr = animationProperties->getString("keyOut");
    float* keyOut = NULL;
    if (keyOutStr)
    {   
        keyOut = new float[components];
        startOffset = 0;
        endOffset = (unsigned int)std::string::npos;
        for (unsigned int i = 0; i < components; i++)
        {
            endOffset = static_cast<std::string>(keyOutStr).find_first_of(delimeter, startOffset);
            if (endOffset != std::string::npos)
            {   
                keyOut[i] = std::atof(static_cast<std::string>(keyOutStr).substr(startOffset, endOffset - startOffset).c_str());
            }
            else
            {
                keyOut[i] = std::atof(static_cast<std::string>(keyOutStr).substr(startOffset, static_cast<std::string>(keyOutStr).length()).c_str());
            }
            startOffset = endOffset + 1;
        }
    }

    int curve = Curve::getInterpolationType(curveStr);

    Animation* animation = NULL;
    if (keyIn && keyOut)
    {
        animation = createAnimation(id, target, propertyId, keyCount, keyTimes, keyValues, keyIn, keyOut, (Curve::InterpolationType)curve);
    }
    else
    {
        animation = createAnimation(id, target, propertyId, keyCount, keyTimes, keyValues, (Curve::InterpolationType) curve);
    }

    SAFE_DELETE(keyOut);
    SAFE_DELETE(keyIn);
    SAFE_DELETE(keyValues);
    SAFE_DELETE(keyTimes);

    Properties* pClip = animationProperties->getNextNamespace();
    if (pClip && std::strcmp(pClip->getNamespace(), "clip") == 0)
    {
        int frameCount = animationProperties->getInt("frameCount");
        assert(frameCount > 0);
        animation->createClips(animationProperties, (unsigned int) frameCount);
    }

    return animation;
}

AnimationController::State AnimationController::getState() const
{
    return _state;
}

void AnimationController::initialize()
{
    _state = IDLE;
}

void AnimationController::finalize()
{
    std::list<AnimationClip*>::iterator itr = _runningClips.begin();
    for ( ; itr != _runningClips.end(); itr++)
    {
        AnimationClip* clip = *itr;
        SAFE_RELEASE(clip);
    }
    _runningClips.clear();
    _state = STOPPED;
}

void AnimationController::resume()
{
    if (_runningClips.empty())
        _state = IDLE;
    else
        _state = RUNNING;
}

void AnimationController::pause()
{
    _state = PAUSED;
}

void AnimationController::schedule(AnimationClip* clip)
{
    if (_runningClips.empty())
    {
        _state = RUNNING;
    }

    clip->addRef();
    _runningClips.push_back(clip);
}

void AnimationController::unschedule(AnimationClip* clip)
{
    std::list<AnimationClip*>::iterator clipItr = _runningClips.begin();
    while (clipItr != _runningClips.end())
    {
        AnimationClip* rClip = (*clipItr);
        if (rClip == clip)
        {
            _runningClips.erase(clipItr);
            SAFE_RELEASE(clip);
            break;
        }
        clipItr++;
    }

    if (_runningClips.empty())
        _state = IDLE;
}

void AnimationController::update(long elapsedTime)
{
    if (_state != RUNNING)
        return;

    // Loop through running clips and call update() on them.
    std::list<AnimationClip*>::iterator clipIter = _runningClips.begin();
    while (clipIter != _runningClips.end())
    {
        AnimationClip* clip = (*clipIter);
        if (clip->isClipStateBitSet(AnimationClip::CLIP_IS_RESTARTED_BIT))
        {   // If the CLIP_IS_RESTARTED_BIT is set, we should end the clip and 
            // move it from where it is in the running clips list to the back.
            clip->onEnd();
            clip->setClipStateBit(AnimationClip::CLIP_IS_PLAYING_BIT);
            _runningClips.push_back(clip);
            clipIter = _runningClips.erase(clipIter);
        }
        else if (clip->update(elapsedTime, &_activeTargets))
        {
            SAFE_RELEASE(clip);
            clipIter = _runningClips.erase(clipIter);
        }
        else
        {
            clipIter++;
        }
    }

    // Loop through active AnimationTarget's and reset their _animationPropertyBitFlag for the next frame.
    std::list<AnimationTarget*>::iterator targetItr = _activeTargets.begin();
    while (targetItr != _activeTargets.end())
    {
        AnimationTarget* target = (*targetItr);
        target->_animationPropertyBitFlag = 0x00;
        targetItr++;
    }
    _activeTargets.clear();
    
    if (_runningClips.empty())
        _state = IDLE;
}

void AnimationController::addAnimation(Animation* animation)
{
    _animations.push_back(animation);
}

void AnimationController::destroyAnimation(Animation* animation)
{
    assert(animation);

    std::vector<Animation::Channel*>::iterator cItr = animation->_channels.begin();
    for (; cItr != animation->_channels.end(); cItr++)
    {
        Animation::Channel* channel = *cItr;
        channel->_target->deleteChannel(channel);
    }

    std::vector<Animation*>::iterator aItr = _animations.begin();
    while (aItr != _animations.end())
    {
        if (animation == *aItr)
        {
            Animation* temp = *aItr;
            SAFE_RELEASE(temp);
            _animations.erase(aItr);
            return;
        }
        aItr++;
    }
}

void AnimationController::destroyAllAnimations()
{
    std::vector<Animation*>::iterator aItr = _animations.begin();
    
    while (aItr != _animations.end())
    {
        Animation* animation = *aItr;
        std::vector<Animation::Channel*>::iterator cItr = animation->_channels.begin();
        for (; cItr != animation->_channels.end(); cItr++)
        {
            Animation::Channel* channel = *cItr;
            channel->_target->deleteChannel(channel);
        }

        SAFE_RELEASE(animation);
        aItr++;
    }

    _animations.clear();
}

}
