package org.glob3.mobile.generated; 
//
//  GLErrorRenderer.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 20/06/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//

//
//  GLErrorRenderer.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 20/06/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//



public class GLErrorRenderer extends Renderer
{

  public void initialize(InitializationContext ic)
  {
  
  }

  public int render(RenderContext rc)
  {
	GL gl = rc.getGL();
	final ILogger logger = rc.getLogger();
  
	GLError error = gl.getError();
	while (error != GLError.NoError)
	{
	  logger.logError("GL Error: %d", error.getValue());
	  error = gl.getError();
	}
  
  
	return Renderer.maxTimeToRender;
  }

  public boolean onTouchEvent(EventContext ec, TouchEvent touchEvent)
  {
	return false;
  }

  public void dispose()
  {
  }

  public final void onResizeViewportEvent(EventContext ec, int width, int height)
  {
  }

  public final boolean isReadyToRender(RenderContext rc)
  {
	return true;
  }

}