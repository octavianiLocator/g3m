//
//  VectorStreamingRenderer.hpp
//  G3MiOSSDK
//
//  Created by Diego Gomez Deck on 7/30/15.
//
//

#ifndef __G3MiOSSDK__VectorStreamingRenderer__
#define __G3MiOSSDK__VectorStreamingRenderer__

#include "DefaultRenderer.hpp"

#include "URL.hpp"
#include "TimeInterval.hpp"
#include "IBufferDownloadListener.hpp"
#include "IThreadUtils.hpp"
#include "RCObject.hpp"

#include <vector>
#include <string>

class IThreadUtils;
class IByteBuffer;
class Sector;
class Geodetic2D;
class JSONArray;
class JSONObject;
class MarksRenderer;
class Mark;
class GEO2DPointGeometry;
class BoundingVolume;
class Camera;
class Frustum;


class VectorStreamingRenderer : public DefaultRenderer {
public:

  class VectorSet;
  class Node;


  class GEOJSONUtils {
  private:
    GEOJSONUtils();

  public:
    static Sector*     parseSector(const JSONArray* json);
    static Geodetic2D* parseGeodetic2D(const JSONArray* json);
    static Node*       parseNode(const JSONObject* json);

  };


  class Node : public RCObject {
  private:
    const std::string              _id;
    const Sector*                  _sector;
    const int                      _featuresCount;
    const Geodetic2D*              _averagePosition;
#ifdef C_CODE
    const std::vector<std::string> _childrenIDs;
#endif
#ifdef JAVA_CODE
    private final java.util.ArrayList<String> _children;
#endif

    std::vector<Node*>* _children;
    size_t _childrenSize;


    BoundingVolume* _boundingVolume;
    BoundingVolume* getBoundingVolume(const G3MRenderContext *rc);

    bool _loadingChildren;

    bool _wasVisible;
    bool isVisible(const G3MRenderContext* rc,
                   const Frustum* cameraFrustumInModelCoordinates);

    bool _loadedFeatures;
    bool _loadingFeatures;

    bool _wasBigEnough;
    bool isBigEnough(const G3MRenderContext *rc);
    long long renderFeatures(const G3MRenderContext *rc,
                             const GLState *glState);

    void loadFeatures();
    void unloadFeatures();
    void cancelLoadFeatures();

    void loadChildren();
    void unloadChildren();
    void cancelLoadChildren();

    void unload();

    void removeMarks();

  protected:
    ~Node();

  public:
    Node(const std::string&              id,
         const Sector*                   sector,
         const int                       featuresCount,
         const Geodetic2D*               averagePosition,
         const std::vector<std::string>& childrenIDs) :
    _id(id),
    _sector(sector),
    _featuresCount(featuresCount),
    _averagePosition(averagePosition),
    _childrenIDs(childrenIDs),
    _wasVisible(false),
    _loadedFeatures(false),
    _loadingFeatures(false),
    _children(NULL),
    _childrenSize(0),
    _loadingChildren(false),
    _wasBigEnough(false),
    _boundingVolume(NULL)
    {

    }

    long long render(const G3MRenderContext* rc,
                     const Frustum* cameraFrustumInModelCoordinates,
                     const long long cameraTS,
                     GLState* glState);

  };


  class MetadataParserAsyncTask : public GAsyncTask {
  private:
    VectorSet*   _vectorSet;
    IByteBuffer* _buffer;

    bool         _parsingError;

    Sector*             _sector;
    long long           _featuresCount;
    Geodetic2D*         _averagePosition;
    int                 _nodesCount;
    int                 _minNodeDepth;
    int                 _maxNodeDepth;
    std::vector<Node*>* _rootNodes;

  public:
    MetadataParserAsyncTask(VectorSet* vectorSet,
                            IByteBuffer* buffer) :
    _vectorSet(vectorSet),
    _buffer(buffer),
    _parsingError(false),
    _sector(NULL),
    _featuresCount(-1),
    _averagePosition(NULL),
    _nodesCount(-1),
    _minNodeDepth(-1),
    _maxNodeDepth(-1),
    _rootNodes(NULL)
    {
    }

    ~MetadataParserAsyncTask();

    void runInBackground(const G3MContext* context);

    void onPostExecute(const G3MContext* context);

  };


  class MetadataDownloadListener : public IBufferDownloadListener {
  private:
    VectorSet*          _vectorSet;
    const IThreadUtils* _threadUtils;
    const bool          _verbose;

  public:
    MetadataDownloadListener(VectorSet* vectorSet,
                             const IThreadUtils* threadUtils,
                             bool verbose) :
    _vectorSet(vectorSet),
    _threadUtils(threadUtils),
    _verbose(verbose)
    {
    }

    void onDownload(const URL& url,
                    IByteBuffer* buffer,
                    bool expired);

    void onError(const URL& url);

    void onCancel(const URL& url);

    void onCanceledDownload(const URL& url,
                            IByteBuffer* buffer,
                            bool expired);

  };


  class VectorSetSymbolizer {
  public:
    virtual ~VectorSetSymbolizer() {
    }

    virtual Mark* createMark(const GEO2DPointGeometry* geometry) const = 0;

  };


  class VectorSet {
  private:
#ifdef C_CODE
    const URL _serverURL;
#endif
#ifdef JAVA_CODE
    private final URL _serverURL;
#endif
    const std::string  _name;
    const VectorSetSymbolizer* _symbolizer;
    const bool                 _deleteSymbolizer;
    const long long    _downloadPriority;
#ifdef C_CODE
    const TimeInterval _timeToCache;
#endif
#ifdef JAVA_CODE
    private final TimeInterval _timeToCache;
#endif
    const bool         _readExpired;
    const bool         _verbose;

    bool _downloadingMetadata;
    bool _errorDownloadingMetadata;
    bool _errorParsingMetadata;

    Sector*             _sector;
    long long           _featuresCount;
    Geodetic2D*         _averagePosition;
    int                 _nodesCount;
    int                 _minNodeDepth;
    int                 _maxNodeDepth;
    std::vector<Node*>* _rootNodes;
    size_t              _rootNodesSize;

    long long _lastRenderedCount;

  public:

    VectorSet(const URL&                 serverURL,
              const std::string&         name,
              const VectorSetSymbolizer* symbolizer,
              const bool                 deleteSymbolizer,
              long long                  downloadPriority,
              const TimeInterval&        timeToCache,
              bool                       readExpired,
              bool                       verbose) :
    _serverURL(serverURL),
    _name(name),
    _symbolizer(symbolizer),
    _deleteSymbolizer(deleteSymbolizer),
    _downloadPriority(downloadPriority),
    _timeToCache(timeToCache),
    _readExpired(readExpired),
    _verbose(verbose),
    _downloadingMetadata(false),
    _errorDownloadingMetadata(false),
    _errorParsingMetadata(false),
    _sector(NULL),
    _averagePosition(NULL),
    _rootNodes(NULL),
    _rootNodesSize(0),
    _lastRenderedCount(0)
    {

    }

    ~VectorSet();

    const std::string getName() const {
      return _name;
    }

    void initialize(const G3MContext* context);

    RenderState getRenderState(const G3MRenderContext* rc);

    void errorDownloadingMetadata();
    void errorParsingMetadata();
    void parsedMetadata(Sector* sector,
                        long long featuresCount,
                        Geodetic2D* averagePosition,
                        int nodesCount,
                        int minNodeDepth,
                        int maxNodeDepth,
                        std::vector<Node*>* rootNodes);

    void render(const G3MRenderContext* rc,
                const Frustum* cameraFrustumInModelCoordinates,
                const long long cameraTS,
                GLState* glState);

  };


private:
  MarksRenderer* _markRenderer;

  size_t                  _vectorSetsSize;
  std::vector<VectorSet*> _vectorSets;

  std::vector<std::string> _errors;

  GLState* _glState;
  void updateGLState(const Camera* camera);

public:

  VectorStreamingRenderer(MarksRenderer* markRenderer);

  ~VectorStreamingRenderer();

  void render(const G3MRenderContext* rc,
              GLState* glState);

  void onResizeViewportEvent(const G3MEventContext* ec,
                             int width, int height) {

  }

  void onChangedContext();

  void addVectorSet(const URL&                 serverURL,
                    const std::string&         name,
                    const VectorSetSymbolizer* symbolizer,
                    const bool                 deleteSymbolizer,
                    long long                  downloadPriority,
                    const TimeInterval&        timeToCache,
                    bool                       readExpired,
                    bool                       verbose);
  
  void removeAllVectorSets();
  
  RenderState getRenderState(const G3MRenderContext* rc);
  
};

#endif
