#ifndef _INGAMESTATE_H_
#define _INGAMESTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <MapManager.h>
#include <OptionsManager.h>
#include <GamePanelManager.h>

#define NUM_STATUSFLAGS 2

class InGameState : public GameState
{
protected:
    ControlManager *mControlMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    NameTagManager *mNameTagMgr;
    OptionsManager *mOptionsMgr;
    MiniMapManager *mMiniMapMgr;
    GamePanelManager *mGamePanelMgr;
    GuiPanel *mEscPanel;
    PlayerData mPlayerData;
    bool mLoaded;
    unsigned int mCharID;
    String mUsername;
    Real mArrivalYaw;
    bool mFirstEntrance;
    bool mMapTransition;

    unsigned char mLastSentAction;
    bool mLastSentFlags[NUM_STATUSFLAGS];
public:
    InGameState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mNameTagMgr = NameTagManager::getSingletonPtr();
        mOptionsMgr = OptionsManager::getSingletonPtr();
        mMiniMapMgr = MiniMapManager::getSingletonPtr();
        mGamePanelMgr = GamePanelManager::getSingletonPtr();
        mPlayerData = PlayerData();
        mArrivalYaw = 0;
        mCharID = 0;
        mUsername = "";
        mFirstEntrance = false;
        mMapTransition = false;
    }
    ~InGameState()
    {
        exit();
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mNameTagMgr->init();
        mOptionsMgr->init(mSceneMgr);
        mEscPanel = mGui->addPanel("EscScreen",true);
        mEscPanel->show(false);
        //mGui->addPanel("DebugScreen");

		mLoaded = false;
		// mNetMgr->addListener(mMiniMapMgr); // keep commented for now

		if (!mSplashScreenMgr->isVisible())mSplashScreenMgr->show(true);

    }
    void exit()
    {
        if(!mMapTransition)
        {
            mGamePanelMgr->reset();
        }
        mOptionsMgr->reset();
        mMiniMapMgr->reset();
        mControlMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mScreenFXMgr->update(timeElapsed);
        if(mLoaded)
        {
            mNameTagMgr->update(timeElapsed);
            mControlMgr->update(timeElapsed);
            mControlMgr->updateCamera();
            mGridMgr->update(mCameraMgr->getPosition());
            mSkyMgr->update(timeElapsed);
            mWaterMgr->update(mCameraMgr->getPosition());
            mSoundMgr->update(mCameraMgr->getPosition(),mCameraMgr->getDirection());
            mMiniMapMgr->update(timeElapsed);

            //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mMovieMaker->mDebug);
        }

        mRoot->renderOneFrame();
    }
    void setPlayerData(PlayerData &data)
    {
        mPlayerData = data;
    }
    void setUsername(const String &username)
    {
        mUsername = username;
    }
    void setArrivalYaw(const Real &yaw)
    {
        mArrivalYaw = yaw;
    }
    void setFirstEntrance()
    {
        mFirstEntrance = true;
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        mControlMgr->keyPressed(index);
        mGamePanelMgr->keyPressed(index);
        mOptionsMgr->keyPressed(index);
        return true;
    }
    const bool keyReleased(const char &index)
    {
        mControlMgr->keyReleased(index);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("EscScreen/Options"))
        {
            mEscPanel->show(false);
            mOptionsMgr->show(true);
        }
        else if(button->nameIs("EscScreen/Quit"))
        {
            changeState = true;
        }
        else if(StringUtil::startsWith(button->getName(),"action"))mControlMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"ingamebuttons"))mGamePanelMgr->buttonClicked(button);
        mOptionsMgr->buttonClicked(button);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {

        if(arg.state.buttonDown(OIS::MB_Right))
        {
            mCameraMgr->yaw(-0.5f*arg.state.X.rel*mOptionsMgr->getMouseSensitivity());
            mCameraMgr->pitch(0.5f*arg.state.Y.rel*mOptionsMgr->getMouseSensitivity());
        }
        else if(arg.state.buttonDown(OIS::MB_Middle) && !mCameraMgr->isFreeCamera())mCameraMgr->zoom(0.005f*arg.state.Y.rel);

        return true;
    }
    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)mControlMgr->mouseLeftDown();
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(StringUtil::startsWith(component->getName(),"action"))mControlMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"minimap"))mMiniMapMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"options"))mOptionsMgr->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(StringUtil::startsWith(name,"options"))mOptionsMgr->alertBoxEvent(name,flag);
        return true;
    }
    void mapChange(const GateInfo &mapInfo)
    {
        doMapChange(mapInfo);
    }
    void nearGate(const String &name)
    {
        String displayName = "";
        if(mMapMgr->getMapSeed(name,&displayName)!="")mScreenFXMgr->showMapPeek(true,displayName);
    }
    void charInfo(const unsigned int &charID, PlayerData &data, const Vector3 &position, const unsigned int &userID, const String &username)
    {
        //Silently destroy old clones
        Unit *clone = mUnitMgr->getUnitByCharID(charID);
        if(clone)
        {
            mMiniMapMgr->removeUnitMarker(clone);
            mUnitMgr->destroyUnit(clone);
        }

        //No other characters in default map
        if(mMapMgr->isDefaultMap())return;

        Unit *unit = mUnitMgr->createUnit(data,username);
        unit->mCharID = charID;
        unit->mUserID = userID;
        unit->mIsLocal = false;
        unit->mApplySlopeForces = false;
        unit->setPosition(position);
        mUnitMgr->initUnitPosition(unit);
        unit->addListener(mControlMgr);
        unit->addListener(SkillsManager::getSingletonPtr());
    }
    void charExit(const unsigned int &charID, const bool &isProper, const bool &isMapChange)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            mMiniMapMgr->removeUnitMarker(unit);
            mUnitMgr->destroyUnit(unit);
        }
    }
    void charPositionUpdate(const unsigned int &charID, const Vector3 &position)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(unit->mAntiGravity)
            {
                if(unit->getPosition()!=position)unit->setMoveTarget(position);
            }
            else
            {
                if(unit->getPosition().x!=position.x || unit->getPosition().z!=position.z)unit->setMoveTarget(position);
                unit->setRemoteGroundHeight(position.y);
            }
        }
    }
    void charYawUpdate(const unsigned int &charID, const unsigned char &yaw)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setYaw((Real(yaw)/255)*360);
    }
    void charHeadDirUpdate(const unsigned int &charID, const unsigned char &ew, const unsigned char &ns)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            unit->setHeadEWState(unit->convertHeadEWState(ew));
            unit->setHeadNSState(unit->convertHeadNSState(ns));
        }
    }
    void charActionUpdate(const unsigned int &charID, const unsigned char &action)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(action!=Unit::ACT_IDLE || unit->isSpecialAction())unit->setAction(action);
        }
    }
    void charStatusFlagsUpdate(const unsigned int &charID, const bool &antiGravity, const bool &swimming)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            unit->mAntiGravity = antiGravity;
            unit->mIsSwimming = swimming;
        }
    }
    void charMoveModifierEvent(const unsigned int &charID, const unsigned char &moveModifier)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setMovementModifier(moveModifier);
    }
    void charEmoteEvent(const unsigned int &charID, const unsigned char &emote)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setEmote(emote);
    }
    void charJumpEvent(const unsigned int &charID, const bool &flag)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(flag)unit->startJump();
            else unit->stopJump();
        }
    }
    void findCharEvent(const String &name, const String &mapName)
    {
        if(mapName=="")
        {
            return;
        }
        String displayName = "";
        mMapMgr->getMapSeed(mapName,&displayName);
    }
    void doMapChange(const GateInfo &mapInfo, const bool &isGoHome=false)
    {
        if(mapInfo.mMap!="")
        {
            //Check that map exists
            const String mapSeed = mMapMgr->getMapSeed(mapInfo.mMap,0,isGoHome);
            if(mapSeed=="")
            {
                mGui->showAlertBox("Cannot find Map file.");
                return;
            }
            changeState = true;
            replaceState = true;
            InGameState *state = new InGameState();
            state->setPlayerData(mPlayerData);
            state->setUsername(mUsername);
            state->setArrivalYaw(mapInfo.mYaw);
            mNextState = state;
            mMapTransition = true;
        }
        //Map reset
        else
        {
            changeState = true;
            replaceState = true;
            InGameState *state = new InGameState();
            state->setPlayerData(mPlayerData);
            state->setUsername(mUsername);
            mNextState = state;
            mMapTransition = true;
        }
    }
};
#endif
