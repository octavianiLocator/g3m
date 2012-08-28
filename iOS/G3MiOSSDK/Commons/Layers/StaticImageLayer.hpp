//
//  StaticImageLayer.hpp
//  G3MiOSSDK
//
//  Created by José Miguel S N on 26/07/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef G3MiOSSDK_StaticImageLayer_hpp
#define G3MiOSSDK_StaticImageLayer_hpp

#include "Layer.hpp"
#include "Sector.hpp"
#include "IImage.hpp"

class IStorage;

class StaticImageLayer: public Layer
{
private:
  Sector            _sector;
  const IImage*     _image;
  const std::string _layerID;
  IStorage* const   _storage;
public:
  
  StaticImageLayer(std::string layerID,
                   IImage* image,
                   const Sector& sector,
                   IStorage* storage,
                   LayerCondition* condition):
  Layer(condition),
  _image(image),
  _sector(sector),
  _layerID(layerID),
  _storage(storage) {
    
  }
  
  ~StaticImageLayer(){
    delete _image;
  }
  
  std::vector<Petition*> getMapPetitions(const RenderContext* rc,
                                         const Tile* tile,
                                         int width, int height) const;
  
  bool isTransparent() const {
    return true;
  }
  
  URL getFeatureInfoURL(const Geodetic2D& g,
                        const IFactory* factory,
                        const Sector& sector,
                        int width, int height) const {
    return URL::null();
  }
  
};


#endif
