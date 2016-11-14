#include "transform.h"
#include <boost/scoped_ptr.hpp>

class MeshImpl;

class MeshNew {
public:
    MeshNew();
    ~MeshNew();

    bool loadPLY(const char* filename);
    bool loadCube();
    void render();
    void setMVP(
            float angle,
            const ViewParameters& viewParameters,
            const ProjectionParameters& projectionParameters,
            float rotateYAngle);

private:
    boost::scoped_ptr<MeshImpl> m_impl;
};
