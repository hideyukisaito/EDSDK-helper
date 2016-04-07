#include "ofApp.h"

const EdsImageQuality imageQualities[] =
{
    EdsImageQuality_LJF,	/* Jpeg Large Fine - 5760x3240 */
    EdsImageQuality_LJN,	/* Jpeg Large Normal - 5760x3240 */
    EdsImageQuality_MJF,	/* Jpeg Middle Fine - 3840x2160 */
    EdsImageQuality_MJN,	/* Jpeg Middle Normal - 3840x2160 */
    EdsImageQuality_S1JF,	/* Jpeg Small1 Fine - 2880x1624 */
    EdsImageQuality_S1JN,	/* Jpeg Small1 Normal - 2880x1624 */
    EdsImageQuality_S2JF,	/* Jpeg Small2 - 1920x1080 */
    EdsImageQuality_S3JF,	/* Jpeg Small3 - 720x400 */ }
;

const EdsEvfAFMode evfAFModes[] =
{
    Evf_AFMode_Quick,
    Evf_AFMode_Live,
    Evf_AFMode_LiveFace,
    Evf_AFMode_LiveMulti
};

const EdsUInt32 ISOSpeeds[] =
{
    0x00000000, // Auto
    0x00000040, // 50
    0x00000048, // 100
    0x0000004b, // 125
    0x0000004d, // 160
    0x00000050, // 200
    0x00000053, // 250
    0x00000055, // 320
    0x00000058, // 400
    0x0000005b, // 500
    0x0000005d, // 640
    0x00000060, // 800
    0x00000063, // 1000
    0x00000065, // 1250
    0x00000068, // 1600
    0x0000006b, // 2000
    0x0000006d, // 2500
    0x00000070, // 3200
    0x00000073, // 4000
    0x00000075, // 5000
    0x00000078, // 6400
    0x0000007b, // 8000
    0x0000007d, // 10000
    0x00000080, // 12800
    0x00000088, // 25600
    0x00000090, // 51200
    0x00000098  // 102400
};

int enumIndex = 0;

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(30);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    EdsError error_ = EDS_ERR_OK;
    mCamera = NULL;
    
    bSdkInitialized = false;
    bIsReady = false;
    bSessionOpened = false;
    bLiveviewStarted = false;
    bExecuteBulbShooting = false;
    mEvfImageWidth = 0.f;
    mEvfImageHeight = 0.f;
    mEvfScaleRatioX = 1.f;
    mEvfScaleRatioY = 1.f;
    bytesPerFrame = 0.f;
    mFocusRect.set(0, 0, 0, 0);

    initialize();
}

//--------------------------------------------------------------
void ofApp::update()
{
    if (bLiveviewStarted)
    {
        downloadEvfData();
        
        if (0 < mMiddleStreamBuffers.at(mReadIndex)->size())
        {
            std::swap(mFrontStreamBuffer, mMiddleStreamBuffers.at(mReadIndex));
            ofBuffer* buf_ = new ofBuffer();
            buf_->set(mFrontStreamBuffer->getBinaryBuffer(), mFrontStreamBuffer->size());

            mImages.at(mImageIndex).get()->loadImage(*buf_);
            
            mEvfImageWidth = mImages.at(mImageIndex).get()->getWidth();
            mEvfImageHeight = mImages.at(mImageIndex).get()->getHeight();
            mEvfScaleRatioX = mEvfImageCoord.width / mEvfImageWidth;
            mEvfScaleRatioY = mEvfImageCoord.height / mEvfImageHeight;
            
            ++mImageIndex %= mImages.size();
            ++mReadIndex %= mMiddleStreamBuffers.size();
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    if (!mImages.empty() && mImages.at(mImageIndex).get()->isAllocated())
    {
        mImages.at(mImageIndex).get()->draw(0, 0);
    }
    
    ofPushStyle();
    {
        ofSetColor(ofColor::red);
        ofNoFill();
        ofRect(mFocusRect);
        
        auto stepX_ = mEvfImageWidth / 3;
        auto stepY_ = mEvfImageHeight / 3;
        
        for (auto x = 0; x < mEvfImageWidth - stepX_; x += stepX_)
        {
            ofLine(x + stepX_, 0, x + stepX_, mEvfImageHeight);
        }
        
        for (auto y = 0; y < mEvfImageHeight - stepY_; y += stepY_)
        {
            ofLine(0, y + stepY_, mEvfImageWidth, y + stepY_);
        }
        
        ofFill();
        ofCircle(ofGetMouseX(), ofGetMouseY(), 3);
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::exit()
{
    finalize();
    
    EdsSetCameraAddedHandler(NULL, this);
    
    if (bSdkInitialized)
    {
        EdsTerminateSDK();
    }
}

#pragma mark - oF events

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if ('0' == key) // set drive mode to single frame
    {
        setDriveMode(EDS_DRIVE_MODE_SINGLE_FRAME);
    }
    else if ('1' == key) // set drive mode to high-speed continuous shooting
    {
        setDriveMode(EDS_DRIVE_MODE_HIGH_SPEED_CONTINUOUS_SHOOTING);
    }
    else if ('2' == key)
    {
        
    }
    else if ('3' == key)
    {
    }
    else if ('4' == key)
    {

    }
    else if (' ' == key) // take photo
    {
        takePhoto();
    }
    else if ('b' == key)
    {
        bExecuteBulbShooting = true;
        pressShutterButton();
    }
    else if (OF_KEY_UP == key)
    {
        driveLensEvf(kEdsEvfDriveLens_Far1);
    }
    else if (OF_KEY_DOWN == key)
    {
        driveLensEvf(kEdsEvfDriveLens_Near1);
    }
    else if ('i' == key)
    {
        setIsoSpeed(ISOSpeeds[enumIndex]);
        ++enumIndex %= (sizeof(ISOSpeeds) / sizeof(ISOSpeeds[0]));
    }
    else if ('h' == key)
    {
        pressShutterButton(true);
    }
    else if ('f' == key)
    {
        updateFocusRect();

    }
    else if ('w' == key)
    {
        EdsPoint point_ = { mFocusRect.x * mEvfScaleRatioX, mFocusRect.y * mEvfScaleRatioY };
        point_.y -= 5 * mEvfScaleRatioY;
        setZoomPosition(point_);
    }
    else if ('a' == key)
    {
        EdsPoint point_ = { mFocusRect.x * mEvfScaleRatioX, mFocusRect.y * mEvfScaleRatioY };
        point_.x += 5 * mEvfScaleRatioX;
        setZoomPosition(point_);
    }
    else if ('s' == key)
    {
        EdsPoint point_ = { mFocusRect.x * mEvfScaleRatioX, mFocusRect.y * mEvfScaleRatioY };
        point_.y += 5 * mEvfScaleRatioY;
        setZoomPosition(point_);
    }
    else if ('d' == key)
    {
        EdsPoint point_ = { mFocusRect.x * mEvfScaleRatioX, mFocusRect.y * mEvfScaleRatioY };
        point_.x -= 5 * mEvfScaleRatioX;
        setZoomPosition(point_);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    if ('b' == key)
    {
        bExecuteBulbShooting = false;
        releaseShutterButton();
    }
    else if ('h' == key)
    {
        releaseShutterButton();
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    EdsPoint point_ = { x * mEvfScaleRatioX - mEvfZoomRect.size.width * 0.5, y * mEvfScaleRatioY - mEvfZoomRect.size.height * 0.5 };
    setZoomPosition(point_);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}

//--------------------------------------------------------------
void ofApp::initialize()
{
    std::cout << "initialize" << std::endl;
    
    EdsError error_ = EDS_ERR_OK;
    
    // initialize SDK
    if (!bSdkInitialized)
    {
        error_ = EdsInitializeSDK();
        
        if (EDS_ERR_OK == error_)
        {
            bSdkInitialized = true;
        }
        else
        {
            std::cout << "couldn't initilize SDK, exit" << std::endl;
            std::exit(-1);
        }
    }
    
    // Get first camera
    if (EDS_ERR_OK == error_)
    {
        std::cout << "getFirstCamera" << std::endl;
        error_ = getFirstCamera(&mCamera);
        
        if (EDS_ERR_DEVICE_NOT_FOUND == error_)
        {
            std::cout << std::hex << "device not found" << std::endl;
            EdsSetCameraAddedHandler(onCameraAdded, this);
            return;
        }
    }
    
    // Set object event handler
    if (EDS_ERR_OK == error_)
    {
        std::cout << "set object event handler" << std::endl;
        error_ = EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, onObjectEvent, this);
        std::cout << std::hex << "error: " << error_ << std::endl;
    }
    
    // Set property event handler
    if (EDS_ERR_OK == error_)
    {
        std::cout << "set prop event handler" << std::endl;
        error_ = EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, onPropertyEvent, this);
        std::cout << std::hex << "error: " << error_ << std::endl;
    }
    
    // Set state event handler
    if (EDS_ERR_OK == error_)
    {
        std::cout << "set state event handler" << std::endl;
        error_ = EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, onStateEvent, this);
        std::cout << std::hex << "error: " << error_ << std::endl;
    }
    
    // Open session with found camera
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsOpenSession(mCamera);
        
        if (EDS_ERR_OK == error_)
        {
            std::cout << "session opened" << std::endl;
            bSessionOpened = true;
            setImageQuality(EdsImageQuality_S2JF);
            extendShutDownTimer();
            startLiveview();
            updateFocusRect();
            bIsReady = true;
        }
        
        std::cout << std::hex << "bSessionOpened: " << bSessionOpened << std::endl;
    }
}

//--------------------------------------------------------------
void ofApp::finalize()
{
    EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, NULL, this);
    EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, NULL, this);
    EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, NULL, this);
    
    if (bLiveviewStarted)
    {
        endLiveview();
    }
    
    if (bSessionOpened)
    {
        EdsCloseSession(mCamera);
    }
    
    if (NULL != mCamera)
    {
        EdsRelease(mCamera);
    }
    
    bIsReady = false;
}

//--------------------------------------------------------------
EdsError ofApp::getFirstCamera(EdsCameraRef *camera)
{
    EdsError error_ = EDS_ERR_OK;
    EdsCameraListRef cameraList_ = NULL;
    EdsUInt32 count_ = 0;
    
    // Get camera list
    error_ = EdsGetCameraList(&cameraList_);
    
    // Get number of cameras
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsGetChildCount(cameraList_, &count_);
        if (0 == count_)
        {
            error_ = EDS_ERR_DEVICE_NOT_FOUND;
        }
    }
    
    // Get first camera retrieved
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsGetChildAtIndex(cameraList_, 0, camera);
    }
    
    // Release camera list
    if (NULL != cameraList_)
    {
        EdsRelease(cameraList_);
        cameraList_ = NULL;
    }
    
    return error_;
}

//--------------------------------------------------------------
EdsUInt32 ofApp::getPropertyData(EdsPropertyID property, EdsUInt32 inParam)
{
    EdsUInt32 value_;
    EdsError error_ = EdsGetPropertyData(mCamera, property, inParam, sizeof(value_), &value_);
    
    if (EDS_ERR_OK == error_)
    {
        std::cout << std::hex << "property: " << property << ", value: " << value_ << std::endl;
    }
    
    return value_;
}

//--------------------------------------------------------------
void ofApp::updateFocusRect()
{
    EdsFocusInfo info_;
    EdsError error_ = EdsGetPropertyData(mCamera, kEdsPropID_FocusInfo, 0, sizeof(info_), &info_);
    
    if (EDS_ERR_OK == error_)
    {
        auto ratioX_ = mEvfImageWidth / mEvfImageCoord.width;
        auto ratioY_ = mEvfImageHeight / mEvfImageCoord.height;
        
        auto x_ = info_.focusPoint[0].rect.point.x * ratioX_;
        auto y_ = info_.focusPoint[0].rect.point.y * ratioY_;
        auto w_ = info_.focusPoint[0].rect.size.width * ratioX_;
        auto h_ = info_.focusPoint[0].rect.size.height * ratioY_;
        
        mFocusRect.set(x_, y_, w_, h_);
    }
}

#pragma mark - Property setters

//--------------------------------------------------------------
void ofApp::setSaveTo(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_SaveTo, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setAEMode(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_AEModeSelect, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setDriveMode(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_DriveMode, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setIsoSpeed(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_ISOSpeed, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setEvfZoom(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_Evf_Zoom, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setImageQuality(EdsUInt32 value)
{
    EdsSetPropertyData(mCamera, kEdsPropID_ImageQuality, 0, sizeof(value), &value);
}

//--------------------------------------------------------------
void ofApp::setZoomPosition(EdsPoint &position)
{
    EdsError error_ = EdsSetPropertyData(mCamera, kEdsPropID_Evf_ZoomPosition, 0, sizeof(position), &position);
    
    if (EDS_ERR_OK == error_)
    {
        updateFocusRect();
    }
    else
    {
        std::cout << std::hex << "error occured at setZoomPosition(): " << error_ << std::endl;
    }
}

#pragma mark - Commands

//--------------------------------------------------------------
void ofApp::extendShutDownTimer()
{
    if (!bSessionOpened)
    {
        return;
    }
    
    EdsSendCommand(mCamera, kEdsCameraCommand_ExtendShutDownTimer, 0);
}

//--------------------------------------------------------------
void ofApp::takePhoto()
{
    EdsSendCommand(mCamera, kEdsCameraCommand_TakePicture, 0);
}

//--------------------------------------------------------------
void ofApp::pressShutterButton(bool halfway)
{
    if (halfway)
    {
        EdsSendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Halfway);
    }
    else
    {
        EdsSendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely);
    }
    
    std::cout << std::hex << "press shutter button" << std::endl;
}

//--------------------------------------------------------------
void ofApp::releaseShutterButton()
{
    EdsSendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_OFF);
    std::cout << std::hex << "release shutter button" << std::endl;
}

//--------------------------------------------------------------
void ofApp::doEvfAutoFocus(EdsEvfAFMode mode)
{
    EdsSendCommand(mCamera, kEdsCameraCommand_DoEvfAf, mode);
}

//--------------------------------------------------------------
void ofApp::driveLensEvf(EdsEvfDriveLens value)
{
    EdsSendCommand(mCamera, kEdsCameraCommand_DriveLensEvf, value);
}

//--------------------------------------------------------------
void ofApp::downloadImage(EdsDirectoryItemRef itemRef)
{
    std::cout << "start downloading image" << std::endl;
    
    EdsError error_ = EDS_ERR_OK;
    EdsStreamRef stream_ = NULL;
    
    EdsDirectoryItemInfo itemInfo_;
    error_ = EdsGetDirectoryItemInfo(itemRef, &itemInfo_);
    
    std::cout << std::hex << "EdsGetDirectoryItemInfo: " << error_ << std::endl;
    
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsCreateMemoryStream(0, &stream_);
        std::cout << std::hex << "EdsCreateMemoryStream: " << error_ << std::endl;
    }
    
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsDownload(itemRef, itemInfo_.size, stream_);
        std::cout << std::hex << "EdsDownload: " << error_ << std::endl;
    }
    
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsDownloadComplete(itemRef);
        std::cout << std::hex << "EdsDownloadComplete: " << error_ << std::endl;
    }
    
    if (NULL != stream_)
    {
        EdsUInt32 length_;
        EdsGetLength(stream_, &length_);
        
        char* streamPtr_;
        EdsGetPointer(stream_, (EdsVoid**)&streamPtr_);
        mDownloadImageBuffer.set(streamPtr_, length_);
        ofLog() << "downloaded item: " << (int) (mDownloadImageBuffer.size() / 1024) << " KB";
        ofLog() << itemInfo_.format;
        
        if (14337 == itemInfo_.format)
        {
            ofLog() << "load complete";
            
            ofBuffer buf_;
            buf_.set(mDownloadImageBuffer.getBinaryBuffer(), mDownloadImageBuffer.size());
            
            ofPixels pix_;
            ofLog() << ofLoadImage(pix_, buf_);
//            pix_.rotate90(orientationMode);
            
            ofImage img_(pix_);
            img_.saveImage(ofToDataPath(ofGetTimestampString() + ".jpg"));
        }
        
        EdsDeleteDirectoryItem(itemRef);
        
        EdsRelease(stream_);
        stream_ = NULL;
    }
}


#pragma mark - Liveview

//--------------------------------------------------------------
EdsError ofApp::startLiveview()
{
    if (!bSessionOpened)
    {
        return;
    }
    
    EdsError error_ = EDS_ERR_OK;
    
    EdsUInt32 device_;
    error_ = EdsGetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device_), &device_);
    
    std::cout << "current output device: " << device_ << std::endl;
    
    if (EDS_ERR_OK == error_)
    {
        device_ |= kEdsEvfOutputDevice_PC;
        error_ = EdsSetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device_), &device_);
        
        if (EDS_ERR_OK == error_)
        {
            bLiveviewStarted = true;
            
            mFrontStreamBuffer = new eds::Buffer();
            mBackStreamBuffer = new eds::Buffer();
            
            for (auto i = 0; i < 2; ++i)
            {
                mMiddleStreamBuffers.push_back(new eds::Buffer());
            }
            
            mReadIndex = 0;
            mWriteIndex = 0;
            
            mImages.push_back(ofPtr<ofImage>(new ofImage()));
            mImages.push_back(ofPtr<ofImage>(new ofImage()));
            mImageIndex = 0;
        }
    }
}

//--------------------------------------------------------------
EdsError ofApp::downloadEvfData()
{
    if (!bLiveviewStarted)
    {
        return;
    }
    
    EdsError error_ = EDS_ERR_OK;
    
    EdsStreamRef stream_ = NULL;
    EdsEvfImageRef evfImage_ = NULL;
    
    // Create memory stream
    error_ = EdsCreateMemoryStream(0, &stream_);
    
    // Create EvfImageRef
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsCreateEvfImageRef(stream_, &evfImage_);
    }
    
    // Download live view image data
    if (EDS_ERR_OK == error_)
    {
        error_ = EdsDownloadEvfImage(mCamera, evfImage_);
    }
    
    // Get the incidental data of the image
    if (EDS_ERR_OK == error_)
    {
        EdsGetPropertyData(evfImage_, kEdsPropID_Evf_CoordinateSystem, 0, sizeof(mEvfImageCoord), &mEvfImageCoord);
        EdsGetPropertyData(evfImage_, kEdsPropID_Evf_ZoomRect, 0, sizeof(mEvfZoomRect), &mEvfZoomRect);
        
        updateFocusRect();
    }
    
    // Display image
    EdsUInt32 length_;
    EdsGetLength(stream_, &length_);
    
    char* streamPtr_;
    EdsGetPointer(stream_, (EdsVoid **)&streamPtr_);
    
    mBackStreamBuffer->set(streamPtr_, length_);
    
    bytesPerFrame = ofLerp(bytesPerFrame, mBackStreamBuffer->size(), 0.01);
    
    std::swap(mBackStreamBuffer, mMiddleStreamBuffers.at(mWriteIndex));
    
    ++mWriteIndex %= mMiddleStreamBuffers.size();
    
    mBackStreamBuffer->clear();
    
    // Release stream
    if (NULL != stream_)
    {
        EdsRelease(stream_);
        stream_ = NULL;
    }
    
    // Release iamge ref
    if (NULL != evfImage_)
    {
        EdsRelease(evfImage_);
        evfImage_ = NULL;
    }
}

//--------------------------------------------------------------
EdsError ofApp::endLiveview()
{
    EdsError error_ = EDS_ERR_OK;
    
    // Get the output device for the live view image
    EdsUInt32 device_;
    error_ = EdsGetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device_), &device_);
    
    // PC live view ends if the PC is disconnected from the live view image output device
    if (EDS_ERR_OK == error_)
    {
        device_ &= ~kEdsEvfOutputDevice_PC;
        error_ = EdsSetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device_), &device_);
        
        mBackStreamBuffer = NULL;
        delete mBackStreamBuffer;
        
        for (auto i = 0; i < mMiddleStreamBuffers.size(); ++i)
        {
            mMiddleStreamBuffers.at(i)->clear();
            mMiddleStreamBuffers.at(i) = NULL;
            delete mMiddleStreamBuffers.at(i);
        }
    }
}

#pragma mark - Callbacks

//--------------------------------------------------------------
EdsError EDSCALLBACK ofApp::onCameraAdded(EdsVoid *context)
{
    std::cout << "camera added" << std::endl;
    
    if (!((ofApp*)context)->bIsReady)
    {
        ((ofApp*)context)->initialize();
    }
}

//--------------------------------------------------------------
EdsError EDSCALLBACK ofApp::onObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid *context)
{
    switch (event)
    {
        case kEdsObjectEvent_DirItemRequestTransfer:
            break;
            
        case kEdsObjectEvent_DirItemCreated:
            std::cout << "dir item created" << std::endl;
            ((ofApp*)context)->downloadImage(object);
            break;
            
        default:
            EdsRelease(object);
            break;
    }
    
    // オブジェクトをリリースする
    if (object)
    {
        EdsRelease(object);
    }
    
    return EDS_ERR_OK;
}

//--------------------------------------------------------------
EdsError EDSCALLBACK ofApp::onPropertyEvent(EdsPropertyEvent event, EdsPropertyID property, EdsUInt32 param, EdsVoid *context)
{
    switch (property)
    {
        case kEdsPropID_Evf_OutputDevice:
            std::cout << std::hex << "EVF output device changed: " << param << std::endl;
            break;
            
        case kEdsPropID_DriveMode:
            std::cout << std::hex << "drive mode changed: " << param << std::endl;
            break;
            
        case kEdsPropID_AvailableShots:
            std::cout << std::hex << "available shots: " << param << std::endl;
            break;
            
        case kEdsPropID_ImageQuality:
        {
            EdsUInt32 prop_;
            EdsGetPropertyData(((ofApp*)context)->mCamera, kEdsPropID_ImageQuality, 0, sizeof(prop_), &prop_);
            std::cout << std::hex << "image quality changed: " << prop_ << std::endl;
        }
            break;
            
        case kEdsPropID_FocusInfo:
            break;
            
        default:
            std::cout << std::hex << "property changed - property: " << property << ", param: " << param << std::endl;
            break;
    }
    
    return EDS_ERR_OK;
}

//--------------------------------------------------------------
EdsError EDSCALLBACK ofApp::onStateEvent(EdsStateEvent event, EdsUInt32 parameter, EdsVoid *context)
{
    std::cout << std::hex << "onStateEvent: " << event << std::endl;
    
    switch (event)
    {
        case kEdsStateEvent_WillSoonShutDown:
            std::cout << std::hex << "will soon shutdown" << std::endl;
            ((ofApp*)context)->extendShutDownTimer();
            break;
            
        case kEdsStateEvent_ShutDownTimerUpdate:
            std::cout << std::hex << "shutdown timer uppdated" << std::endl;
            break;
            
        case kEdsStateEvent_Shutdown:
            std::cout << std::hex << "shutdown" << std::endl;
            ((ofApp*)context)->finalize();
            break;
            
        default:
            break;
    }
    
    return EDS_ERR_OK;
}

//--------------------------------------------------------------
EdsError EDSCALLBACK ofApp::onProgressEvent(EdsUInt32 percent, EdsVoid *context, EdsBool *cancel)
{
    std::cout << "percent: " << percent << std::endl;
    
    return EDS_ERR_OK;
}