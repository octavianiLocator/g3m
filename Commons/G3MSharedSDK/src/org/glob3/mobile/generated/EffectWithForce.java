package org.glob3.mobile.generated; 
public abstract class EffectWithForce extends Effect
{
  private double _force;
  private final double _friction;

  protected EffectWithForce(double force, double friction)
  {
     _force = force;
     _friction = friction;
  }

  protected final double getForce()
  {
    return _force;
  }

  public void doStep(G3MRenderContext rc, TimeInterval when)
  {
    _force *= _friction;
  }

  public boolean isDone(G3MRenderContext rc, TimeInterval when)
  {
//C++ TO JAVA CONVERTER TODO TASK: There is no preprocessor in Java:
//#warning TODO: adjust this threshold to work properly in singledrag from any cameraheight.
    return (IMathUtils.instance().abs(_force) < 1e-6);
  }

}