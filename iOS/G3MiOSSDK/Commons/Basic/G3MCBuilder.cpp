//
//  G3MCBuilder.cpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 5/25/13.
//
//

#include "G3MCBuilder.hpp"

#include "ILogger.hpp"
#include "CompositeRenderer.hpp"
#include "TileRenderer.hpp"

#include "EllipsoidalTileTessellator.hpp"
#include "MultiLayerTileTexturizer.hpp"
#include "TilesRenderParameters.hpp"
#include "DownloadPriority.hpp"
#include "G3MWidget.hpp"
#include "SimpleCameraConstrainer.hpp"
#include "CameraRenderer.hpp"
#include "CameraSingleDragHandler.hpp"
#include "CameraDoubleDragHandler.hpp"
#include "CameraRotationHandler.hpp"
#include "CameraDoubleTapHandler.hpp"
#include "BusyMeshRenderer.hpp"
#include "GInitializationTask.hpp"
#include "PeriodicalTask.hpp"
#include "IDownloader.hpp"
#include "IBufferDownloadListener.hpp"
#include "IJSONParser.hpp"
#include "JSONObject.hpp"
#include "JSONString.hpp"
#include "JSONArray.hpp"
#include "G3MCSceneDescription.hpp"
#include "IThreadUtils.hpp"
#include "OSMLayer.hpp"
#include "MapQuestLayer.hpp"
#include "BingMapsLayer.hpp"
#include "CartoDBLayer.hpp"
#include "MapBoxLayer.hpp"


G3MCBuilder::G3MCBuilder(const URL& serverURL,
                         const std::string& sceneId) :
_serverURL(serverURL),
_sceneId(sceneId),
_sceneTimestamp(-1),
_gl(NULL),
_glob3Created(false),
_storage(NULL),
_threadUtils(NULL),
_layerSet( new LayerSet() ),
_baseLayer(NULL),
_downloader(NULL)
{

}

IDownloader* G3MCBuilder::getDownloader() {
  if (_downloader == NULL) {
    _downloader = createDownloader();
  }
  return _downloader;
}

IThreadUtils* G3MCBuilder::getThreadUtils() {
  if (_threadUtils == NULL) {
    _threadUtils = createThreadUtils();
  }
  return _threadUtils;
}


void G3MCBuilder::setGL(GL *gl) {
  if (_gl) {
    ILogger::instance()->logError("LOGIC ERROR: _gl already initialized");
    return;
  }
  if (!gl) {
    ILogger::instance()->logError("LOGIC ERROR: _gl cannot be NULL");
    return;
  }
  _gl = gl;
}

GL* G3MCBuilder::getGL() {
  if (!_gl) {
    ILogger::instance()->logError("Logic Error: _gl not initialized");
  }

  return _gl;
}

TileRenderer* G3MCBuilder::createTileRenderer() {
  const TileTessellator* tessellator = new EllipsoidalTileTessellator(true);

  ElevationDataProvider* elevationDataProvider = NULL;
  const float verticalExaggeration = 1;
  TileTexturizer* texturizer = new MultiLayerTileTexturizer();

  const bool renderDebug = false;
  const bool useTilesSplitBudget = true;
  const bool forceFirstLevelTilesRenderOnStart = true;
  const bool incrementalTileQuality = false;
  const TilesRenderParameters* parameters = new TilesRenderParameters(renderDebug,
                                                                      useTilesSplitBudget,
                                                                      forceFirstLevelTilesRenderOnStart,
                                                                      incrementalTileQuality);

  const bool showStatistics = false;
  long long texturePriority = DownloadPriority::HIGHER;

  return new TileRenderer(tessellator,
                          elevationDataProvider,
                          verticalExaggeration,
                          texturizer,
                          _layerSet,
                          parameters,
                          showStatistics,
                          texturePriority);
}

const Planet* G3MCBuilder::createPlanet() {
  return Planet::createEarth();
}

std::vector<ICameraConstrainer*>* G3MCBuilder::createCameraConstraints() {
  std::vector<ICameraConstrainer*>* cameraConstraints = new std::vector<ICameraConstrainer*>;
  SimpleCameraConstrainer* scc = new SimpleCameraConstrainer();
  cameraConstraints->push_back(scc);

  return cameraConstraints;
}

CameraRenderer* G3MCBuilder::createCameraRenderer() {
  CameraRenderer* cameraRenderer = new CameraRenderer();
  const bool useInertia = true;
  cameraRenderer->addHandler(new CameraSingleDragHandler(useInertia));
  const bool processRotation = true;
  const bool processZoom = true;
  cameraRenderer->addHandler(new CameraDoubleDragHandler(processRotation,
                                                         processZoom));
  cameraRenderer->addHandler(new CameraRotationHandler());
  cameraRenderer->addHandler(new CameraDoubleTapHandler());

  return cameraRenderer;
}

Renderer* G3MCBuilder::createBusyRenderer() {
  return new BusyMeshRenderer(Color::newFromRGBA(0, 0, 0, 1));
}


class G3MCBuilder_SceneDescriptionBufferListener : public IBufferDownloadListener {
private:
  G3MCBuilder* _builder;

  MapQuestLayer* parseMapQuestLayer(const JSONObject* jsonBaseLayer,
                                    const TimeInterval& timeToCache) const {
    const std::string imagery = jsonBaseLayer->getAsString("imagery", "<imagery not present>");
    if (imagery.compare("OpenAerial") == 0) {
      return MapQuestLayer::newOpenAerial(timeToCache);
    }
    else {
      return MapQuestLayer::newOSM(timeToCache);
    }
  }

  BingMapsLayer* parseBingMapsLayer(const JSONObject* jsonBaseLayer,
                                    const TimeInterval& timeToCache) const {
    const std::string key = jsonBaseLayer->getAsString("key", "");
    const std::string imagerySet = jsonBaseLayer->getAsString("imagerySet", "Aerial");

    return new BingMapsLayer(imagerySet, key, timeToCache);
  }

  CartoDBLayer* parseCartoDBLayer(const JSONObject* jsonBaseLayer,
                                    const TimeInterval& timeToCache) const {
    const std::string userName = jsonBaseLayer->getAsString("userName", "");
    const std::string table    = jsonBaseLayer->getAsString("table",    "");

    return new CartoDBLayer(userName, table, timeToCache);
  }

  MapBoxLayer* parseMapBoxLayer(const JSONObject* jsonBaseLayer,
                                const TimeInterval& timeToCache) const {
    const std::string mapKey = jsonBaseLayer->getAsString("mapKey", "");

    return new MapBoxLayer(mapKey, timeToCache);
  }

  Layer* parseLayer(const JSONObject* jsonBaseLayer) const {
    const TimeInterval defaultTimeToCache = TimeInterval::fromDays(30);

    /*
     "OSM"
     "MapQuest"
     "BingMaps"
     "MapBox"
     "CartoDB"
     
     "WMS"
     */

    const std::string layerType = jsonBaseLayer->getAsString("layer", "<layer not present>");
    if (layerType.compare("OSM") == 0) {
      return new OSMLayer(defaultTimeToCache);
    }
    else if (layerType.compare("MapQuest") == 0) {
      return parseMapQuestLayer(jsonBaseLayer, defaultTimeToCache);
    }
    else if (layerType.compare("BingMaps") == 0) {
      return parseBingMapsLayer(jsonBaseLayer, defaultTimeToCache);
    }
    else if (layerType.compare("CartoDB") == 0) {
      return parseCartoDBLayer(jsonBaseLayer, defaultTimeToCache);
    }
    else if (layerType.compare("MapBox") == 0) {
      return parseMapBoxLayer(jsonBaseLayer, defaultTimeToCache);
    }
    else {
      ILogger::instance()->logError("Unsupported layer type \"%s\"", layerType.c_str());
      return NULL;
    }
  }

public:
  G3MCBuilder_SceneDescriptionBufferListener(G3MCBuilder* builder) :
  _builder(builder)
  {
  }


  void onDownload(const URL& url,
                  IByteBuffer* buffer,
                  bool expired) {

    const JSONBaseObject* jsonBaseObject = IJSONParser::instance()->parse(buffer);

    if (jsonBaseObject == NULL) {
      ILogger::instance()->logError("Can't parse SceneJSON from %s",
                                    url.getPath().c_str());
    }
    else {
      const JSONObject* jsonObject = jsonBaseObject->asObject();
      if (jsonObject == NULL) {
        ILogger::instance()->logError("Invalid SceneJSON (1)");
      }
      else {
        const JSONString* error = jsonObject->getAsString("error");
        if (error == NULL) {
          const int timestamp = (int) jsonObject->getAsNumber("ts", 0);

          if (_builder->getSceneTimestamp() != timestamp) {
            const std::string user = jsonObject->getAsString("user", "<user not present>");
            const std::string name = jsonObject->getAsString("name", "<name not present>");

            const JSONObject* jsonBaseLayer = jsonObject->getAsObject("baseLayer");

            if (jsonBaseLayer == NULL) {
              ILogger::instance()->logError("Attribute 'baseLayer' not found in SceneJSON");
            }
            else {
              Layer* baseLayer = parseLayer(jsonBaseLayer);
              if (baseLayer != NULL) {
                _builder->changeBaseLayer(baseLayer);
                _builder->setSceneTimestamp(timestamp);
              }
            }
          }
        }
        else {
          ILogger::instance()->logError("Server Error: %s",
                                        error->value().c_str());
        }
      }

      delete jsonBaseObject;
    }

    delete buffer;

    //    int __TODO_flag_initialization_task_as_initialized;
    //    _initializationTask->setInitialized(true);
  }

  void onError(const URL& url) {
    ILogger::instance()->logError("Can't download SceneJSON from %s",
                                  url.getPath().c_str());
  }

  void onCancel(const URL& url) {
    // do nothing
  }

  void onCanceledDownload(const URL& url,
                          IByteBuffer* buffer,
                          bool expired) {
    // do nothing
  }

};


//class G3MCBuilder_InitializationTask : public GInitializationTask {
//private:
//  G3MCBuilder* _builder;
//  const URL    _sceneDescriptionURL;
//
//  bool _isInitialized;
//
//public:
//  G3MCBuilder_InitializationTask(G3MCBuilder* builder,
//                         const URL& sceneDescriptionURL) :
//  _builder(builder),
//  _sceneDescriptionURL(sceneDescriptionURL),
//  _isInitialized(false)
//  {
//
//  }
//
//  void run(const G3MContext* context) {
//    IDownloader* downloader = context->getDownloader();
//
//    downloader->requestBuffer(_sceneDescriptionURL,
//                              DownloadPriority::HIGHEST,
//                              TimeInterval::zero(),
//                              true,
//                              new G3MCSceneDescriptionBufferListener(_builder),
//                              true);
//  }
//
//  bool isDone(const G3MContext* context) {
//    //return _isInitialized;
//    int __FIX_IT;
//    return true;
//  }
//};


class G3MCBuilder_PullScenePeriodicalTask : public GTask {
private:
  G3MCBuilder* _builder;

  long long _requestId;


  URL getURL() const {
    const int sceneTimestamp = _builder->getSceneTimestamp();

    const URL _sceneDescriptionURL = _builder->createSceneDescriptionURL();

    if (sceneTimestamp < 0) {
      return _sceneDescriptionURL;
    }

    IStringBuilder* ib = IStringBuilder::newStringBuilder();

    ib->addString(_sceneDescriptionURL.getPath());
    ib->addString("?lastTs=");
    ib->addInt(sceneTimestamp);

    const std::string path = ib->getString();

    delete ib;

    return URL(path, false);
  }


public:
  G3MCBuilder_PullScenePeriodicalTask(G3MCBuilder* builder) :
  _builder(builder),
  _requestId(-1)
  {

  }

  void run(const G3MContext* context) {
    //ILogger::instance()->logInfo("G3MCPeriodicalTask executed");

    IDownloader* downloader = context->getDownloader();
    if (_requestId >= 0) {
      downloader->cancelRequest(_requestId);
    }

    _requestId = downloader->requestBuffer(getURL(),
                                           DownloadPriority::HIGHEST,
                                           TimeInterval::zero(),
                                           true,
                                           new G3MCBuilder_SceneDescriptionBufferListener(_builder),
                                           true);
  }
};


void G3MCBuilder::recreateLayerSet() {
  _layerSet->removeAllLayers(false);
  if (_baseLayer != NULL) {
    _layerSet->addLayer(_baseLayer);
  }
}

void G3MCBuilder::changeBaseLayer(Layer* baseLayer) {
  if (_baseLayer != baseLayer) {
    if (_baseLayer != NULL) {
      delete _baseLayer;
    }
    _baseLayer = baseLayer;
    recreateLayerSet();
  }
}


const URL G3MCBuilder::createSceneDescriptionURL() const {
  std::string serverPath = _serverURL.getPath();

  return URL(serverPath + "/scenes/" + _sceneId, false);
}


std::vector<PeriodicalTask*>* G3MCBuilder::createPeriodicalTasks() {
  std::vector<PeriodicalTask*>* periodicalTasks = new std::vector<PeriodicalTask*>();

  periodicalTasks->push_back(new PeriodicalTask(TimeInterval::fromSeconds(15),
                                                new G3MCBuilder_PullScenePeriodicalTask(this)));

  return periodicalTasks;
}

IStorage* G3MCBuilder::getStorage() {
  if (_storage == NULL) {
    _storage = createStorage();
  }
  return _storage;
}


G3MWidget* G3MCBuilder::create() {
  if (_glob3Created) {
    ILogger::instance()->logError("The G3MWidget was already created, can't create more than one");
    return NULL;
  }
  _glob3Created = true;


  CompositeRenderer* mainRenderer = new CompositeRenderer();


  TileRenderer* tileRenderer = createTileRenderer();
  mainRenderer->addRenderer(tileRenderer);


  std::vector<ICameraConstrainer*>* cameraConstraints = createCameraConstraints();

  Color backgroundColor = Color::fromRGBA(0, 0.1f, 0.2f, 1);

  // GInitializationTask* initializationTask = new G3MCInitializationTask(this, createSceneDescriptionURL());
  GInitializationTask* initializationTask = NULL;

  std::vector<PeriodicalTask*>* periodicalTasks = createPeriodicalTasks();

  G3MWidget * g3mWidget = G3MWidget::create(getGL(),
                                            getStorage(),
                                            getDownloader(),
                                            getThreadUtils(),
                                            createPlanet(),
                                            *cameraConstraints,
                                            createCameraRenderer(),
                                            mainRenderer,
                                            createBusyRenderer(),
                                            backgroundColor,
                                            false,      // logFPS
                                            false,      // logDownloaderStatistics
                                            initializationTask,
                                            true,       // autoDeleteInitializationTask
                                            *periodicalTasks);

  //  g3mWidget->setUserData(getUserData());

  delete cameraConstraints;
  delete periodicalTasks;

  return g3mWidget;
}


class G3MCBuilder_ScenesDescriptionsBufferListener : public IBufferDownloadListener {
private:
  G3MCBuilderScenesDescriptionsListener* _listener;
  const bool _autoDelete;

public:
  G3MCBuilder_ScenesDescriptionsBufferListener(G3MCBuilderScenesDescriptionsListener* listener,
                                               bool autoDelete) :
  _listener(listener),
  _autoDelete(autoDelete)
  {

  }


  void onDownload(const URL& url,
                  IByteBuffer* buffer,
                  bool expired) {

    const JSONBaseObject* jsonBaseObject = IJSONParser::instance()->parse(buffer);

    if (jsonBaseObject == NULL) {
      ILogger::instance()->logError("Can't parse ScenesDescriptionJSON from %s",
                                    url.getPath().c_str());
      onError(url);
    }
    else {
      const JSONArray* jsonScenesDescriptions = jsonBaseObject->asArray();
      if (jsonScenesDescriptions == NULL) {
        ILogger::instance()->logError("ScenesDescriptionJSON: invalid format (1)");
        onError(url);
      }
      else {
        std::vector<G3MCSceneDescription*>* scenesDescriptions = new std::vector<G3MCSceneDescription*>();

        const int size = jsonScenesDescriptions->size();

        for (int i = 0; i < size; i++) {
          const JSONObject* jsonSceneDescription = jsonScenesDescriptions->getAsObject(i);
          if (jsonSceneDescription == NULL) {
            ILogger::instance()->logError("ScenesDescriptionJSON: invalid format (2) at index #%d", i);
          }
          else {
            const std::string id          = jsonSceneDescription->getAsString("id",          "<invalid id>");
            const std::string user        = jsonSceneDescription->getAsString("user",        "<invalid user>");
            const std::string name        = jsonSceneDescription->getAsString("name",        "<invalid name>");
            const std::string description = jsonSceneDescription->getAsString("description", "");
            const std::string iconURL     = jsonSceneDescription->getAsString("iconURL",     "<invalid iconURL>");

            std::vector<std::string> tags;
            const JSONArray* jsonTags = jsonSceneDescription->getAsArray("tags");
            if (jsonTags == NULL) {
              ILogger::instance()->logError("ScenesDescriptionJSON: invalid format (3) at index #%d", i);
            }
            else {
              const int tagsCount = jsonTags->size();
              for (int j = 0; j < tagsCount; j++) {
                const std::string tag = jsonTags->getAsString(j, "");
                if (tag.size() > 0) {
                  tags.push_back(tag);
                }
              }
            }

            scenesDescriptions->push_back( new G3MCSceneDescription(id,
                                                                    user,
                                                                    name,
                                                                    description,
                                                                    iconURL,
                                                                    tags) );

          }
        }

        _listener->onDownload(scenesDescriptions);
        if (_autoDelete) {
          delete _listener;
        }
      }

      delete jsonBaseObject;
    }

    delete buffer;
  }

  void onError(const URL& url) {
    _listener->onError();
    if (_autoDelete) {
      delete _listener;
    }
  }

  void onCancel(const URL& url) {
    // do nothing
  }

  void onCanceledDownload(const URL& url,
                          IByteBuffer* buffer,
                          bool expired) {
    // do nothing
  }

};

const URL G3MCBuilder::createScenesDescriptionsURL() const {
  std::string serverPath = _serverURL.getPath();

  return URL(serverPath + "/scenes/", false);
}


void G3MCBuilder::requestScenesDescriptions(G3MCBuilderScenesDescriptionsListener* listener,
                                            bool autoDelete) {
  getDownloader()->requestBuffer(createScenesDescriptionsURL(),
                                 DownloadPriority::HIGHEST,
                                 TimeInterval::zero(),
                                 true,
                                 new G3MCBuilder_ScenesDescriptionsBufferListener(listener, autoDelete),
                                 true);
}

int G3MCBuilder::getSceneTimestamp() const {
  return _sceneTimestamp;
}

void G3MCBuilder::setSceneTimestamp(const int timestamp) {
  _sceneTimestamp = timestamp;
}


class G3MCBuilder_ChangeSceneIdTask : public GTask {
private:
  G3MCBuilder*      _builder;
  const std::string _sceneId;

public:
  G3MCBuilder_ChangeSceneIdTask(G3MCBuilder* builder,
                                const std::string& sceneId) :
  _builder(builder),
  _sceneId(sceneId)
  {
  }

  void run(const G3MContext* context) {
    _builder->rawChangeScene(_sceneId);
  }
};


void G3MCBuilder::rawChangeScene(const std::string& sceneId) {
  if (sceneId.compare(_sceneId) != 0) {
    _layerSet->removeAllLayers(false);
    if (_baseLayer != NULL) {
      delete _baseLayer;
      _baseLayer = NULL;
    }
    _sceneTimestamp = -1;
    _sceneId = sceneId;
  }
}

void G3MCBuilder::changeScene(const std::string& sceneId) {
  if (sceneId.compare(_sceneId) != 0) {
    getThreadUtils()->invokeInRendererThread(new G3MCBuilder_ChangeSceneIdTask(this, sceneId),
                                             true);
  }
}
