package org.glob3.mobile.generated; 
public class SimpleTextureMapping extends TextureMapping
{
  private TextureIDReference _glTextureId;

  private IFloatBuffer _texCoords;
  private final boolean _ownedTexCoords;

  private float _translationU;
  private float _translationV;
  private float _scaleU;
  private float _scaleV;
  private float _rotationInRadians;
  private float _rotationCenterU;
  private float _rotationCenterV;

  private final boolean _transparent;

  private void releaseGLTextureId()
  {
  
    if (_glTextureId != null)
    {
      _glTextureId.dispose();
      _glTextureId = null;
    }
    else
    {
      ILogger.instance().logError("Releasing invalid simple texture mapping");
    }
  }


  public SimpleTextureMapping(TextureIDReference glTextureId, IFloatBuffer texCoords, boolean ownedTexCoords, boolean transparent)
  {
     _glTextureId = glTextureId;
     _texCoords = texCoords;
     _ownedTexCoords = ownedTexCoords;
     _transparent = transparent;
     _translationU = 0F;
     _translationV = 0F;
     _scaleU = 1F;
     _scaleV = 1F;
     _rotationInRadians = 0F;
     _rotationCenterU = 0F;
     _rotationCenterV = 0F;
  }

  public SimpleTextureMapping(TextureIDReference glTextureId, IFloatBuffer texCoords, boolean ownedTexCoords, boolean transparent, float translationU, float translationV, float scaleU, float scaleV, float rotationAngleInRadians, float rotationCenterU, float rotationCenterV)
  {
     _glTextureId = glTextureId;
     _texCoords = texCoords;
     _ownedTexCoords = ownedTexCoords;
     _transparent = transparent;
     _translationU = translationU;
     _translationV = translationV;
     _scaleU = scaleU;
     _scaleV = scaleV;
     _rotationInRadians = rotationAngleInRadians;
     _rotationCenterU = rotationCenterU;
     _rotationCenterV = rotationCenterV;
  }

  public final void setTranslation(float translationU, float translationV)
  {
    _translationU = translationU;
    _translationV = translationV;
//C++ TO JAVA CONVERTER TODO TASK: There is no preprocessor in Java:
//#warning updateState();
  }

  public final void setScale(float scaleU, float scaleV)
  {
    _scaleU = scaleU;
    _scaleV = scaleV;
//C++ TO JAVA CONVERTER TODO TASK: There is no preprocessor in Java:
//#warning updateState();
  }

  public final void setRotation(float rotationAngleInRadians, float rotationCenterU, float rotationCenterV)
  {
    _rotationInRadians = rotationAngleInRadians;
    _rotationCenterU = rotationCenterU;
    _rotationCenterV = rotationCenterV;
//C++ TO JAVA CONVERTER TODO TASK: There is no preprocessor in Java:
//#warning updateState();
  }

  public void dispose()
  {
    if (_ownedTexCoords)
    {
      if (_texCoords != null)
         _texCoords.dispose();
    }
  
    releaseGLTextureId();
  
    super.dispose();
  }

  public final IGLTextureId getGLTextureId()
  {
    return _glTextureId.getID();
  }

  public final IFloatBuffer getTexCoords()
  {
    return _texCoords;
  }

  public final void modifyGLState(GLState state)
  {
    if (_texCoords == null)
    {
      ILogger.instance().logError("SimpleTextureMapping::bind() with _texCoords == NULL");
    }
    else
    {
      state.clearGLFeatureGroup(GLFeatureGroupName.COLOR_GROUP);
  
      if ((_scaleU != 1) || (_scaleV != 1) || (_translationU != 0) || (_translationV != 0) || (_rotationInRadians != 0))
      {
        state.addGLFeature(new TextureGLFeature(_glTextureId.getID(), _texCoords, 2, 0, false, 0, _transparent, GLBlendFactor.srcAlpha(), GLBlendFactor.oneMinusSrcAlpha(), _translationU, _translationV, _scaleU, _scaleV, _rotationInRadians, _rotationCenterU, _rotationCenterV), false);
      }
      else
      {
        state.addGLFeature(new TextureGLFeature(_glTextureId.getID(), _texCoords, 2, 0, false, 0, _transparent, GLBlendFactor.srcAlpha(), GLBlendFactor.oneMinusSrcAlpha()), false);
      }
    }
  }

}