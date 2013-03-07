package org.glob3.mobile.generated; 
//
//  EllipsoidalTileTessellator.cpp
//  G3MiOSSDK
//
//  Created by Agustin Trujillo Pino on 12/07/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//

//
//  EllipsoidalTileTessellator.hpp
//  G3MiOSSDK
//
//  Created by Agustin Trujillo Pino on 12/07/12.
//  Copyright (c) 2012 IGO Software SL. All rights reserved.
//


//class Sector;

public class EllipsoidalTileTessellator extends TileTessellator
{
  private final boolean _skirted;

  private Vector2I calculateResolution(Vector2I rawResolution, Sector sector)
  {
  //  return Vector2I(_resolutionX, _resolutionY);
  
  
    /* testing for dynamic latitude-resolution */
    final double cos = sector.getCenter().latitude().cosinus();
  
    int resolutionY = (int)(rawResolution._y * cos);
    if (resolutionY < 6)
    {
      resolutionY = 6;
    }
  
    int resolutionX = (int)(rawResolution._x * cos);
    if (resolutionX < 6)
    {
      resolutionX = 6;
    }
  
    return new Vector2I(resolutionX, resolutionY);
  }


  public EllipsoidalTileTessellator(boolean skirted)
  {
     _skirted = skirted;

  }

  public void dispose()
  {
  }

  public final Vector2I getTileMeshResolution(Planet planet, Vector2I rawResolution, Tile tile, boolean debug)
  {
    return calculateResolution(rawResolution, tile.getSector());
  }


  public final Mesh createTileMesh(Planet planet, Vector2I rawResolution, Tile tile, ElevationData elevationData, float verticalExaggeration, boolean debug)
  {
  
    final Sector sector = tile.getSector();
    final Vector2I tileResolution = calculateResolution(rawResolution, sector);
  
    double minHeight = 0;
    FloatBufferBuilderFromGeodetic vertices = new FloatBufferBuilderFromGeodetic(CenterStrategy.givenCenter(), planet, sector.getCenter());
  
    int unusedType = -1;
    for (int j = 0; j < tileResolution._y; j++)
    {
      final double v = (double) j / (tileResolution._y-1);
      for (int i = 0; i < tileResolution._x; i++)
      {
        final double u = (double) i / (tileResolution._x-1);
  
        final Geodetic2D position = sector.getInnerPoint(u, v);
  
        double height = 0;
        if (elevationData != null)
        {
  //        height = elevationData->getElevationAt(i, j) * verticalExaggeration;
          height = elevationData.getElevationAt(position, unusedType) * verticalExaggeration;
          if (height < minHeight)
          {
            minHeight = height;
          }
        }
  
        vertices.add(position, height);
      }
    }
  
  
    ShortBufferBuilder indices = new ShortBufferBuilder();
    for (short j = 0; j < (tileResolution._y-1); j++)
    {
      final short jTimesResolution = (short)(j *tileResolution._x);
      if (j > 0)
      {
        indices.add(jTimesResolution);
      }
      for (short i = 0; i < tileResolution._x; i++)
      {
        indices.add((short)(jTimesResolution + i));
        indices.add((short)(jTimesResolution + i + tileResolution._x));
      }
      indices.add((short)(jTimesResolution + 2 *tileResolution._x - 1));
    }
  
  
    // create skirts
    if (_skirted)
    {
      // compute skirt height
      final Vector3D sw = planet.toCartesian(sector.getSW());
      final Vector3D nw = planet.toCartesian(sector.getNW());
      final double skirtHeight = (nw.sub(sw).length() * 0.05 * -1) + minHeight;
  
      int posS = tileResolution._x * tileResolution._y;
      indices.add((short)(posS-1));
  
      // east side
      for (int j = tileResolution._y-1; j > 0; j--)
      {
        vertices.add(sector.getInnerPoint(1, (double)j/(tileResolution._y-1)), skirtHeight);
  
        indices.add((short)(j *tileResolution._x + (tileResolution._x-1)));
        indices.add((short) posS++);
      }
  
      // north side
      for (int i = tileResolution._x-1; i > 0; i--)
      {
        vertices.add(sector.getInnerPoint((double)i/(tileResolution._x-1), 0), skirtHeight);
  
        indices.add((short) i);
        indices.add((short) posS++);
      }
  
      // west side
      for (int j = 0; j < tileResolution._y-1; j++)
      {
        vertices.add(sector.getInnerPoint(0, (double)j/(tileResolution._y-1)), skirtHeight);
  
        indices.add((short)(j *tileResolution._x));
        indices.add((short) posS++);
      }
  
      // south side
      for (int i = 0; i < tileResolution._x-1; i++)
      {
        vertices.add(sector.getInnerPoint((double)i/(tileResolution._x-1), 1), skirtHeight);
  
        indices.add((short)((tileResolution._y-1)*tileResolution._x + i));
        indices.add((short) posS++);
      }
  
      // last triangle
      indices.add((short)((tileResolution._x *tileResolution._y)-1));
      indices.add((short)(tileResolution._x *tileResolution._y));
    }
  
    Color color = Color.newFromRGBA((float) 1.0, (float) 1.0, (float) 1.0, (float) 1.0);
  
    return new IndexedMesh(GLPrimitive.triangleStrip(), true, vertices.getCenter(), vertices.create(), indices.create(), 1, 1, color); //debug ? GLPrimitive::lineStrip() : GLPrimitive::triangleStrip(),
                           //GLPrimitive::lineStrip(),
  }

  public final Mesh createTileDebugMesh(Planet planet, Vector2I rawResolution, Tile tile)
  {
    final Sector sector = tile.getSector();
  
    final int resolutionXMinus1 = rawResolution._x - 1;
    final int resolutionYMinus1 = rawResolution._y - 1;
    short posS = 0;
  
    // compute offset for vertices
    final Vector3D sw = planet.toCartesian(sector.getSW());
    final Vector3D nw = planet.toCartesian(sector.getNW());
    final double offset = nw.sub(sw).length() * 1e-3;
  
    FloatBufferBuilderFromGeodetic vertices = new FloatBufferBuilderFromGeodetic(CenterStrategy.givenCenter(), planet, sector.getCenter());
  
    ShortBufferBuilder indices = new ShortBufferBuilder();
  
    // west side
    for (int j = 0; j < resolutionYMinus1; j++)
    {
      vertices.add(sector.getInnerPoint(0, (double)j/resolutionYMinus1), offset);
      indices.add(posS++);
    }
  
    // south side
    for (int i = 0; i < resolutionXMinus1; i++)
    {
      vertices.add(sector.getInnerPoint((double)i/resolutionXMinus1, 1), offset);
      indices.add(posS++);
    }
  
    // east side
    for (int j = resolutionYMinus1; j > 0; j--)
    {
      vertices.add(sector.getInnerPoint(1, (double)j/resolutionYMinus1), offset);
      indices.add(posS++);
    }
  
    // north side
    for (int i = resolutionXMinus1; i > 0; i--)
    {
      vertices.add(sector.getInnerPoint((double)i/resolutionXMinus1, 0), offset);
      indices.add(posS++);
    }
  
    Color color = Color.newFromRGBA((float) 1.0, (float) 0, (float) 0, (float) 1.0);
  
    return new IndexedMesh(GLPrimitive.lineLoop(), true, vertices.getCenter(), vertices.create(), indices.create(), 1, 1, color);
  }

  public final boolean isReady(G3MRenderContext rc)
  {
    return true;
  }

  public final IFloatBuffer createTextCoords(Vector2I rawResolution, Tile tile, boolean mercator)
  {
  
    final Vector2I tileResolution = calculateResolution(rawResolution, tile.getSector());
  
    float[] u = new float[tileResolution._x * tileResolution._y];
    float[] v = new float[tileResolution._x * tileResolution._y];
  
    final Sector sector = tile.getSector();
  
    //const IMathUtils* mu = IMathUtils::instance();
  
    final double lowerV = MercatorUtils.getMercatorV(sector.lower().latitude());
    final double upperV = MercatorUtils.getMercatorV(sector.upper().latitude());
    final double deltaV = lowerV - upperV;
  
    for (int j = 0; j < tileResolution._y; j++)
    {
      for (int i = 0; i < tileResolution._x; i++)
      {
        final int pos = j *tileResolution._x + i;
  
        final double uu = (double) i / (tileResolution._x-1);
        double vv = (double) j / (tileResolution._y-1);
  
        int __Mercator_at_work;
        if (mercator)
        {
          final Geodetic2D innerPoint = sector.getInnerPoint(uu, vv);
          final double vMercatorGlobal = MercatorUtils.getMercatorV(innerPoint.latitude());
          //vv = mu->clamp( (vMercatorGlobal - upperV) / deltaV, 0, 1);
          vv = (vMercatorGlobal - upperV) / deltaV;
        }
  
        u[pos] = (float) uu;
        v[pos] = (float) vv;
      }
    }
  
    int textCoordsSize = (tileResolution._x * tileResolution._y) * 2;
    if (_skirted)
    {
      textCoordsSize += ((tileResolution._x-1) * (tileResolution._y-1) * 4) * 2;
    }
  
    IFloatBuffer textCoords = IFactory.instance().createFloatBuffer(textCoordsSize);
  
    int textCoordsIndex = 0;
  
    for (int j = 0; j < tileResolution._y; j++)
    {
      for (int i = 0; i < tileResolution._x; i++)
      {
        final int pos = j *tileResolution._x + i;
        textCoords.rawPut(textCoordsIndex++, u[pos]);
        textCoords.rawPut(textCoordsIndex++, v[pos]);
      }
    }
  
    // create skirts
    if (_skirted)
    {
      // east side
      for (int j = tileResolution._y-1; j > 0; j--)
      {
        final int pos = j *tileResolution._x + tileResolution._x-1;
        textCoords.rawPut(textCoordsIndex++, u[pos]);
        textCoords.rawPut(textCoordsIndex++, v[pos]);
      }
  
      // north side
      for (int i = tileResolution._x-1; i > 0; i--)
      {
        final int pos = i;
        textCoords.rawPut(textCoordsIndex++, u[pos]);
        textCoords.rawPut(textCoordsIndex++, v[pos]);
      }
  
      // west side
      for (int j = 0; j < tileResolution._y-1; j++)
      {
        final int pos = j *tileResolution._x;
        textCoords.rawPut(textCoordsIndex++, u[pos]);
        textCoords.rawPut(textCoordsIndex++, v[pos]);
      }
  
      // south side
      for (int i = 0; i < tileResolution._x-1; i++)
      {
        final int pos = (tileResolution._y-1) * tileResolution._x + i;
        textCoords.rawPut(textCoordsIndex++, u[pos]);
        textCoords.rawPut(textCoordsIndex++, v[pos]);
      }
    }
  
    // free temp memory
    u = null;
    v = null;
  
    //  return textCoords.create();
    return textCoords;
  }

}