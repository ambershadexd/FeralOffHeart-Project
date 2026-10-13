#ifndef _LOGINSTATE_H_
#define _LOGINSTATE_H_

#include <CharSelectState.h>
#include <MapMakerState.h>
#include <ObjectMakerState.h>
#include <SkyMakerState.h>
#include <MovieMakerState.h>
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <shellapi.h>
#endif

class LoginState : public GameState
{
private:
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    OptionsManager *mOptionsMgr;
    GuiTextField *mUserField;
    GuiMaskedTextField *mPassField;
    GuiCheckBox *mUserRememberBox;
    String mUsername;
    String mPassword;
    GuiPanel *mStartPanel;
    GuiPanel *mToolsPanel;
    GuiPanel *mBulletinPanel;
    GuiTextArea *mBulletinArea;
    Overlay *mTitle;
public:
    LoginState()
    {
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mOptionsMgr = OptionsManager::getSingletonPtr();
        mUsername = "";
        mPassword = "";
        mTitle = OverlayManager::getSingleton().getByName("Title");
    }
    ~LoginState()
    {
        exit();
    }
    void enter()
    {
        init();
        mTitle->show();
        mStartPanel = mGui->addPanel("StartScreen",true);
        mToolsPanel = mGui->addPanel("ToolsScreen",true);
        mBulletinPanel = mGui->addPanel("BulletinScreen",true);
        mBulletinArea = mBulletinPanel->getTextArea("BulletinScreen/Message");
        mBulletinArea->mClickable = false;
        loadBulletin();
        //mGui->addPanel("DebugScreen",true);
        mToolsPanel->show(false);

        mUserField = mStartPanel->getTextField("StartScreen/Username");
        mUserField->bindString(&mUsername);
        mUserField->mMaxLength = 32;
        mPassField = mStartPanel->getMaskedTextField("StartScreen/Password");
        mPassField->bindString(&mPassword);
        mPassField->mMaxLength = 32;
        mUserRememberBox = mStartPanel->getCheckBox("StartScreen/UsernameRem");

        mSkyMgr->setDayTime(710);
        mSkyMgr->setWeatherTime(0);
        mSkyMgr->freezeTime(true);
        mMapMgr->init(mSceneMgr,"WaterMill",false);
        mOptionsMgr->init(mSceneMgr);
        const String savedUsername = mOptionsMgr->getSavedUsername();
        if(savedUsername!="")
        {
            mUserField->setCaption(savedUsername);
            mUserRememberBox->setChecked(true);
        }
        else mUserRememberBox->setChecked(false);

        mCameraMgr->createCameraTrack(160);
        mCameraMgr->addCameraTrackKeyFrame(0,Vector3(4855,146,5346),mCameraMgr->pyr(5,90,0));
        mCameraMgr->addCameraTrackKeyFrame(40,Vector3(6490,100,5055),mCameraMgr->pyr(-10,115,0));
        mCameraMgr->addCameraTrackKeyFrame(60,Vector3(6797,145,3993),mCameraMgr->pyr(-10,200,0));
        mCameraMgr->addCameraTrackKeyFrame(80,Vector3(5680,230,3419),mCameraMgr->pyr(0,300,0));
        mCameraMgr->addCameraTrackKeyFrame(120,Vector3(4795,210,4361),mCameraMgr->pyr(-7,335,0));
        mCameraMgr->addCameraTrackKeyFrame(160,Vector3(4855,146,5346),mCameraMgr->pyr(5,450,0));
        mCameraMgr->doCameraTrack(true);

        mNetMgr->addListener(this);
        mSoundMgr->initMusic(MUSIC_TITLE);
        if(mSplashScreenMgr->isVisible())mSplashScreenMgr->show(false);
        mScreenFXMgr->doFadeIn(0.25f);
        mScreenFXMgr->skipOneUpdate();

        //Remind any bad files that caused an earlier crash
        const String szBadFile = ZipManager::getSingleton().getBadFile();
        if(szBadFile!="")
        {
            mGui->showAlertBox(szBadFile + " caused the game to crash earlier. You might want to remove the file.");
        }
    }
    void exit()
    {
        mOptionsMgr->reset();
        mTitle->hide();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mScreenFXMgr->update(timeElapsed);
        mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        mNetMgr->update(timeElapsed);

        mRoot->renderOneFrame();
    }
    const bool keyPressed(const char &index)
    {
        if(index==KeyboardIndex::KB_OPTIONS)changeState = true;
        mOptionsMgr->keyPressed(index);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button==mUserRememberBox && !mUserRememberBox->isChecked())mOptionsMgr->saveUsername("");
        else if(button->nameIs("StartScreen/Quit"))changeState = true;
        else if(button->nameIs("StartScreen/Login"))
        {
            doLogin();
        }
        else if(button->nameIs("StartScreen/Tools"))
        {
            mStartPanel->show(false);
            mBulletinPanel->show(false);
            mToolsPanel->show(true);
            mTitle->hide();
        }
        else if(button->nameIs("ToolsScreen/Back"))
        {
            mStartPanel->show(true);
            mBulletinPanel->show(true);
            mToolsPanel->show(false);
            mTitle->show();
        }
        else if(button->nameIs("ToolsScreen/ObjectMaker"))
        {
            changeState = true;
            mNextState = new ObjectMakerState();
        }
        else if(button->nameIs("ToolsScreen/MapMaker"))
        {
            changeState = true;
            mNextState = new MapMakerState();
        }
        else if(button->nameIs("ToolsScreen/SkyMaker"))
        {
            changeState = true;
            mNextState = new SkyMakerState();
        }
        else if(button->nameIs("ToolsScreen/CharEdit"))
        {
            changeState = true;
            CharCreateState *state = new CharCreateState();
            state->setPresetMakerMode(true);
            state->setUsername(mUsername);
            mNextState = state;
        }
        else if(button->nameIs("ToolsScreen/MovieMaker"))
        {
            //changeState = true;
            //mNextState = new MovieMakerState();
            mGui->showAlertBox("Coming Soon!");
        }
        else if(button->nameIs("StartScreen/Options"))
        {
            mOptionsMgr->toggleShow();
        }
        else if(button->nameIs("StartScreen/Homepage"))
        {
            mGui->showAlertBox("Open www.feral-heart.com? It may take awhile to load.","Login/Homepage",true);
        }
        mOptionsMgr->buttonClicked(button);
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(event==ChangeListener::EVENT_KEYPRESSED)
        {
            if(component==mUserField)GuiTextHandler::getSingletonPtr()->setActiveField(mPassField,1,1);
            else if(component==mPassField)doLogin();
        }
        mOptionsMgr->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="Login/Homepage" && flag)
        {
            #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            ShellExecute(NULL, "open", "http://www.feral-heart.com", NULL, NULL, SW_SHOWNORMAL);
            #endif
        }
        mOptionsMgr->alertBoxEvent(name,flag);
        return true;
    }
    //!
    void doLogin()
    {
        if(mUserRememberBox->isChecked())mOptionsMgr->saveUsername(mUsername);
        mGui->showAlertBox("(not) Connecting to front server...","",false,false);
        mStartPanel->show(false);
        mBulletinPanel->show(false);
        changeState = true;
        CharSelectState *state = new CharSelectState();
        state->setUsername(mUsername);
        mNextState = state;
    }
    void loadBulletin()
    {
        String caption = "";
        SaveFile sf("bulletin.cfg");
        sf.getSetting("",caption);

        size_t found = caption.find_first_of("%");
        while(found!=String::npos)
        {
            caption[found] = '\n';
            found = caption.find_first_of("%",found+1);
        }
        mBulletinArea->setCaption(caption);
    }
};

#endif
