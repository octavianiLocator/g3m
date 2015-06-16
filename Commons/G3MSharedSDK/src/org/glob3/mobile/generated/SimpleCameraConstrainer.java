package org.glob3.mobile.generated; 
//
//  SimpleCameraConstrainer.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 1/30/13.
//
//

//
//  SimpleCameraConstrainer.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 1/30/13.
//
//




public class SimpleCameraConstrainer implements ICameraConstrainer
{

  public SimpleCameraConstrainer()
  {
     _previousCameraTimeStamp = 0;
     _nextCameraTimeStamp = 0;

  }


  public void dispose()
  {
  }

  public boolean onCameraChange(Planet planet, Camera previousCamera, Camera nextCamera)
  {
  
    long previousCameraTimeStamp = previousCamera.getTimeStamp();
    long nextCameraTimeStamp = nextCamera.getTimeStamp();
    if (previousCameraTimeStamp != _previousCameraTimeStamp || nextCameraTimeStamp != _nextCameraTimeStamp)
    {
      _previousCameraTimeStamp = previousCameraTimeStamp;
      _nextCameraTimeStamp = nextCameraTimeStamp;
      ILogger.instance().logInfo("Cameras TimeStamp: PreviousCam=%lld; NextCam=%lld\n", _previousCameraTimeStamp, _nextCameraTimeStamp);
    }
  
    final double radii = planet.getRadii().maxAxis();
    final double maxHeight = radii *9;
    final double minHeight = 10;
  
    final Geodetic3D cameraPosition = nextCamera.getGeodeticPosition();
    final double cameraHeight = cameraPosition._height;
  
    if (cameraHeight > maxHeight)
    {
      nextCamera.copyFrom(previousCamera);
      /*nextCamera->setGeodeticPosition(cameraPosition._latitude,
                                      cameraPosition._longitude,
                                      maxHeight);*/
    }
    else if (cameraHeight < minHeight)
    {
      nextCamera.copyFrom(previousCamera);
      /*nextCamera->setGeodeticPosition(cameraPosition._latitude,
                                      cameraPosition._longitude,
                                      minHeight);*/
    }
  
    return true;
  }

  private long _previousCameraTimeStamp;
  private long _nextCameraTimeStamp;
}