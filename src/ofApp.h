#pragma once

#include "ofMain.h"
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"

#include "buffer.h"

#pragma mark - AE mode

// Note: For some models, the value of the property cannot be retrieved as kEdsPropID_AEMode. In this case, Bulb is retrieved as the value of the shutter speed (kEdsPropID_Tv) property.
// Note: Bulb is designed so that it cannot be set on cameras from a computer by means of SetPropertyData.

#pragma mark - Drive mode

#define EDS_DRIVE_MODE_SINGLE_FRAME 0x00000000L
#define EDS_DRIVE_MODE_CONTINUOUS_SHOOTING 0x00000001L
#define EDS_DRIVE_MODE_VIDEO 0x00000002L
#define EDS_DRIVE_MODE_HIGH_SPEED_CONTINUOUS_SHOOTING 0x00000004L
#define EDS_DRIVE_MODE_LOW_SPEED_CONTINUOUS_SHOOTING 0x00000005L
#define EDS_DRIVE_MODE_SILENT_SINGLE_SHOOTING 0x00000006L
#define EDS_DRIVE_MODE_10SEC_SELF_TIMER_AND_CONTINUOUS_SHOOTING 0x00000007L
#define EDS_DRIVE_MODE_10SEC_SELF_TIMER 0x00000010L
#define EDS_DRIVE_MODE_2SEC_SELF_TIMER 0x00000011L

#pragma mark - ofApp

class ofApp : public ofBaseApp
{
public:
    EdsCameraRef mCamera;
    EdsPoint mFocusPoint;
    bool bSdkInitialized;
    bool bIsReady;
    bool bSessionOpened;
    bool bLiveviewStarted;
    bool bExecuteBulbShooting;
    
    float mEvfImageWidth;
    float mEvfImageHeight;
    float mEvfScaleRatioX;
    float mEvfScaleRatioY;
    EdsSize mEvfImageCoord;
    EdsRect mEvfZoomRect;
    
    eds::Buffer * mFrontStreamBuffer;
    eds::Buffer * mBackStreamBuffer;
    std::vector<eds::Buffer*> mMiddleStreamBuffers;
    int mReadIndex;
    int mWriteIndex;
    
    std::vector< ofPtr<ofImage> > mImages;
    int mImageIndex;
    float bytesPerFrame;
    
    eds::Buffer mDownloadImageBuffer;
    
    ofRectangle mFocusRect;
    
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
		
    void initialize();
    void finalize();
    EdsError getFirstCamera(EdsCameraRef * camera);
    EdsUInt32 getPropertyData(EdsPropertyID property, EdsUInt32 inParam);
    
    void updateFocusRect();
    
    void setSaveTo(EdsUInt32 value);
    void setAEMode(EdsUInt32 value);
    void setDriveMode(EdsUInt32 value);
    void setIsoSpeed(EdsUInt32 value);
    void setEvfZoom(EdsUInt32 value);
    void setImageQuality(EdsUInt32 value);
    void setZoomPosition(EdsPoint& position);
    
    void extendShutDownTimer();
    void takePhoto();
    void pressShutterButton(bool halfway = false);
    void releaseShutterButton();
    void doEvfAutoFocus(EdsEvfAFMode mode);
    void driveLensEvf(EdsEvfDriveLens value);
    void downloadImage(EdsDirectoryItemRef itemRef);
    
    // Liveview
    EdsError startLiveview();
    EdsError downloadEvfData();
    EdsError endLiveview();
    
    static EdsError EDSCALLBACK onCameraAdded(EdsVoid* context);
    static EdsError EDSCALLBACK onObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid * context);
    static EdsError EDSCALLBACK onPropertyEvent(EdsPropertyEvent event, EdsPropertyID property, EdsUInt32 parameter, EdsVoid * context);
    static EdsError EDSCALLBACK onStateEvent(EdsStateEvent event, EdsUInt32 parameter, EdsVoid * context);
    static EdsError EDSCALLBACK onProgressEvent(EdsUInt32 percent, EdsVoid* context, EdsBool* cancel);
};
