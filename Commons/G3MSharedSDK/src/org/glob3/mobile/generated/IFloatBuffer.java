package org.glob3.mobile.generated; 
//
//  IFloatBuffer.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 05/09/12.
//
//

//
//  IFloatBuffer.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 05/09/12.
//
//



public abstract class IFloatBuffer implements IBuffer
{

  public IFloatBuffer()
  {
     super();
  }

  public void dispose()
  {
  }

  public abstract float get(int i);

  public abstract void put(int i, float value);

  public abstract void rawPut(int i, float value);

  public abstract void rawAdd(int i, float value);

}