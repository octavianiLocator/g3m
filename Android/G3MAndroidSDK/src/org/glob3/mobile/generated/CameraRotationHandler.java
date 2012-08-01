package org.glob3.mobile.generated; 
//
//  CameraRotationHandler.cpp
//  G3MiOSSDK
//
//  Created by Agustín Trujillo Pino on 28/07/12.
//  Copyright (c) 2012 Universidad de Las Palmas. All rights reserved.
//


//
//  CameraRotationHandler.h
//  G3MiOSSDK
//
//  Created by Agustín Trujillo Pino on 28/07/12.
//  Copyright (c) 2012 Universidad de Las Palmas. All rights reserved.
//




public class CameraRotationHandler extends CameraHandler
{

  public final boolean onTouchEvent(TouchEvent touchEvent)
  {
	// three finger needed
	if (touchEvent.getTouchCount()!=3)
		return false;
  
	switch (touchEvent.getType())
	{
	  case Down:
		onDown(touchEvent);
		break;
	  case Move:
		onMove(touchEvent);
		break;
	  case Up:
		onUp(touchEvent);
	  default:
		break;
	}
  
	return true;
  }
  public final int render(RenderContext rc)
  {
	// TEMP TO DRAW A POINT WHERE USER PRESS
	if (false)
	{
	  if (_currentGesture == Gesture.Rotate)
	  {
		float[] vertices = { 0,0,0};
		int[] indices = {0};
		gl.enableVerticesPosition();
		gl.disableTexture2D();
		gl.disableTextures();
		gl.vertexPointer(3, 0, vertices);
		gl.color((float) 1, (float) 1, (float) 0, 1);
		gl.pointSize(10);
		gl.pushMatrix();
		MutableMatrix44D T = MutableMatrix44D.createTranslationMatrix(_initialPoint.asVector3D());
		gl.multMatrixf(T);
		gl.drawPoints(1, indices);
		gl.popMatrix();
		//Geodetic2D g = _planet->toGeodetic2D(_initialPoint.asVector3D());
		//printf ("zoom with initial point = (%f, %f)\n", g.latitude().degrees(), g.longitude().degrees());
	  }
	}
  
	return MAX_TIME_TO_RENDER;
  }
  public final void initialize(InitializationContext ic)
  {
  }
  public final void onResizeViewportEvent(int width, int height)
  {
  }

  private void onDown(TouchEvent touchEvent)
  {
	_camera0 = new Camera(_camera);
	_currentGesture = Gesture.Rotate;
  
	// middle pixel in 2D
	Vector2D pixel0 = touchEvent.getTouch(0).getPos();
	Vector2D pixel1 = touchEvent.getTouch(1).getPos();
	Vector2D pixel2 = touchEvent.getTouch(2).getPos();
	Vector2D averagePixel = pixel0.add(pixel1).add(pixel2).div(3);
	_initialPixel = new MutableVector3D(averagePixel.x(), averagePixel.y(), 0);
	lastYValid = _initialPixel.y();
  
	// compute center of view
	_initialPoint = _camera.centerOfViewOnPlanet(_planet).asMutableVector3D();
	if (_initialPoint.isNan())
	{
	  System.out.print("CAMERA ERROR: center point does not intersect globe!!\n");
	  _currentGesture = Gesture.None;
	}
  
	//printf ("down 3 fingers\n");
  }
  private void onMove(TouchEvent touchEvent)
  {
	//_currentGesture = getGesture(touchEvent);
	if (_currentGesture!=Gesture.Rotate)
		return;
  
	// current middle pixel in 2D
	Vector2D c0 = touchEvent.getTouch(0).getPos();
	Vector2D c1 = touchEvent.getTouch(1).getPos();
	Vector2D c2 = touchEvent.getTouch(2).getPos();
	Vector2D cm = c0.add(c1).add(c2).div(3);
  
	// previous middle pixel in 2D
	Vector2D p0 = touchEvent.getTouch(0).getPrevPos();
	Vector2D p1 = touchEvent.getTouch(1).getPrevPos();
	Vector2D p2 = touchEvent.getTouch(2).getPrevPos();
	Vector2D pm = p0.add(p1).add(p2).div(3);
  
	// compute normal to Initial point
	Vector3D normal = _planet.geodeticSurfaceNormal(_initialPoint.asVector3D());
  
	// vertical rotation around normal vector to globe
	_camera.copyFrom(_camera0);
	Angle angle_v = Angle.fromDegrees((_initialPixel.x()-cm.x())*0.25);
	MutableMatrix44D trans1 = MutableMatrix44D.createTranslationMatrix(_initialPoint.asVector3D());
	MutableMatrix44D rot1 = MutableMatrix44D.createRotationMatrix(angle_v, normal);
	MutableMatrix44D trans2 = MutableMatrix44D.createTranslationMatrix(_initialPoint.times(-1.0).asVector3D());
	MutableMatrix44D M1 = trans1.multiply(rot1).multiply(trans2);
	_camera.applyTransform(M1);
  
	// compute angle between normal and view direction
	Vector3D view = _camera.getViewDirection();
	double dot = normal.normalized().dot(view.normalized().times(-1));
	double initialAngle = Math.acos(dot) / Math.PI * 180;
  
	// rotate more than 85 degrees or less than 0 degrees is not allowed
	double delta = (cm.y() - _initialPixel.y()) * 0.25;
	double finalAngle = initialAngle + delta;
	if (finalAngle > 88)
		delta = 88 - initialAngle;
	if (finalAngle < 0)
		delta = -initialAngle;
  
	// horizontal rotation over the original camera horizontal axix
	Vector3D u = _camera.getHorizontalVector();
	MutableMatrix44D trans3 = MutableMatrix44D.createTranslationMatrix(_initialPoint.asVector3D());
	MutableMatrix44D rot2 = MutableMatrix44D.createRotationMatrix(Angle.fromDegrees(delta), u);
	MutableMatrix44D trans4 = MutableMatrix44D.createTranslationMatrix(_initialPoint.times(-1.0).asVector3D());
	MutableMatrix44D M2 = trans3.multiply(rot2).multiply(trans4);
  
	// update camera only if new view intersects globe
	Camera tempCamera = new Camera(_camera);
	tempCamera.applyTransform(M2);
	tempCamera.updateModelMatrix();
	if (!tempCamera.centerOfViewOnPlanet(_planet).isNan())
	{
	  _camera.copyFrom(tempCamera);
	}
  }
  private void onUp(TouchEvent touchEvent)
  {
	_currentGesture = Gesture.None;
	_initialPixel = Vector3D.nan().asMutableVector3D();
  }

  private double lastYValid;

}