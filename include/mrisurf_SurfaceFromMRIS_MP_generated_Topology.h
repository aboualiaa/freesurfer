namespace Topology {
struct Face : public Repr_Elt {
  using Surface = Topology::Surface;
  using Vertex  = Topology::Vertex;
  inline Face();
  inline Face(Face const &src);
  inline Face(Representation *representation, size_t idx);
  inline Face(XYZPositionM::Face const &src);
  inline Face(XYZPosition::Face const &src);
  inline Face(XYZPositionConsequencesM::Face const &src);
  inline Face(XYZPositionConsequences::Face const &src);
  inline Face(DistortM::Face const &src);
  inline Face(Distort::Face const &src);
  inline Face(AnalysisM::Face const &src);
  inline Face(Analysis::Face const &src);
  inline Face(AllM::Face const &src);
  int fno() const { return idx; }

  inline Vertex v(size_t i) const;
  inline char   ripflag() const;

  inline void set_ripflag(char to);
}; // Face

struct Vertex : public Repr_Elt {
  using Surface = Topology::Surface;
  using Face    = Topology::Face;
  inline Vertex();
  inline Vertex(Vertex const &src);
  inline Vertex(Representation *representation, size_t idx);
  inline Vertex(XYZPositionM::Vertex const &src);
  inline Vertex(XYZPosition::Vertex const &src);
  inline Vertex(XYZPositionConsequencesM::Vertex const &src);
  inline Vertex(XYZPositionConsequences::Vertex const &src);
  inline Vertex(DistortM::Vertex const &src);
  inline Vertex(Distort::Vertex const &src);
  inline Vertex(AnalysisM::Vertex const &src);
  inline Vertex(Analysis::Vertex const &src);
  inline Vertex(AllM::Vertex const &src);
  int vno() const { return idx; }

  inline char ripflag() const; //  vertex no longer exists - placed last to load
                               //  the next vertex into cache
  inline void which_coords(int which, float *x, float *y, float *z) const;

  inline void set_ripflag(char to); //  vertex no longer exists - placed last to
                                    //  load the next vertex into cache
};                                  // Vertex

struct MRIS_MP : public Repr_Elt {
  using Surface = Topology::Surface;
  using Face    = Topology::Face;
  using Vertex  = Topology::Vertex;
  inline MRIS_MP();
  inline MRIS_MP(MRIS_MP const &src);
  inline MRIS_MP(Representation *representation, size_t idx);
  inline MRIS_MP(XYZPositionM::MRIS_MP const &src);
  inline MRIS_MP(XYZPosition::MRIS_MP const &src);
  inline MRIS_MP(XYZPositionConsequencesM::MRIS_MP const &src);
  inline MRIS_MP(XYZPositionConsequences::MRIS_MP const &src);
  inline MRIS_MP(DistortM::MRIS_MP const &src);
  inline MRIS_MP(Distort::MRIS_MP const &src);
  inline MRIS_MP(AnalysisM::MRIS_MP const &src);
  inline MRIS_MP(Analysis::MRIS_MP const &src);
  inline MRIS_MP(AllM::MRIS_MP const &src);

}; // MRIS_MP

struct Surface : public Repr_Elt {
  using Face   = Topology::Face;
  using Vertex = Topology::Vertex;
  inline Surface();
  inline Surface(Surface const &src);
  inline Surface(Representation *representation);
  inline Surface(XYZPositionM::Surface const &src);
  inline Surface(XYZPosition::Surface const &src);
  inline Surface(XYZPositionConsequencesM::Surface const &src);
  inline Surface(XYZPositionConsequences::Surface const &src);
  inline Surface(DistortM::Surface const &src);
  inline Surface(Distort::Surface const &src);
  inline Surface(AnalysisM::Surface const &src);
  inline Surface(Analysis::Surface const &src);
  inline Surface(AllM::Surface const &src);

  inline int nvertices() const; //  # of vertices on surface, change by calling
                                //  MRISreallocVerticesAndFaces et al
  inline int nfaces() const;    //  # of faces on surface, change by calling
                                //  MRISreallocVerticesAndFaces et al
  inline Vertex                vertices(size_t i) const;
  inline Face                  faces(size_t i) const;
  inline FaceNormCacheEntry    faceNormCacheEntries(size_t i) const;
  inline FaceNormDeferredEntry faceNormDeferredEntries(size_t i) const;
  inline double      radius() const; //  radius (if status==MRIS_SPHERE)
  inline MRIS_Status status() const; //  type of surface (e.g. sphere, plane)
  inline MRIS_Status
  origxyz_status() const;     //  type of surface (e.g. sphere, plane) that this
                              //  origxyz were obtained from
  inline int patch() const;   //  if a patch of the surface
  inline int noscale() const; //  don't scale by surface area if true
};                            // Surface

} // namespace Topology
