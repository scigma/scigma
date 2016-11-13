#ifndef SCIGMA_DAT_MESH_HPP
#define SCIGMA_DAT_MESH_HPP

#include <vector>
#include <map>
#include <tinythread.h>
#include "../common/pythonobject.hpp"
#include "../gui/definitions.hpp"
#include "wave.hpp"

using scigma::common::PythonObject;

namespace scigma
{
  namespace dat
  {
    class Mesh:
      public PythonObject<Mesh>
      {
      public:
	typedef AbstractWave<double> Wave;
	typedef AbstractWave<GLint> IWave;
	typedef AbstractWave<GLbyte> BWave;
	
	static const size_t NVALS_PER_DIM = 4;

	Mesh(size_t nDim, const std::vector<double>& initial);
	Mesh(const Mesh& initial);

	void add_strip(const std::vector<double>& positions);

	const IWave& triangle_indices() const;
	const IWave& iso_indices() const;

	const BWave& iso_end_points() const;
	
	const Wave& triangle_data() const;

	size_t available_iso_layer(size_t indexSize, size_t endPointsSize, size_t dataSize) const;
	size_t available_triangle_layer(size_t indexSize, size_t dataSize) const;

	size_t max_for_iso_layer(size_t layer) const;
	size_t max_for_triangle_layer(size_t layer) const;

      private:
	Mesh& operator=(const Mesh&);

	IWave triangleIndices_, isoIndices_;
	BWave isoEndPoints_;
	Wave triangleData_;

	size_t nDim_;
	size_t lastStripTriangleDataBegin_,currentStripTriangleDataBegin_;
	size_t lastStripTriangleIndicesBegin_,currentStripTriangleIndicesBegin_;

	std::vector<size_t> isoLayerBegin_;
	std::vector<size_t> triangleLayerBegin_;
	
	std::vector<size_t> availableIsoLayer_;
	std::vector<size_t> availableTriangleLayer_;
	
	double distance_squared(GLint index1, GLint index2, const double* tData) const;

	void add_layer(const std::vector<double>& positions);

	void collect_neighbours_before(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours);
	void collect_neighbours_after(GLint beginIndex, GLint endIndex, std::map<GLint, std::vector<GLint> >& neighbours);

	void compute_triangle_for_normal(GLint index, std::vector<GLint>& neighbourIndices);
	
	void compute_normal_information();
	
	mutable tthread::mutex mutex_;
    };

  } /* end namespace dat */
} /* end namespace scigma */

#endif /* SCIGMA_DAT_MESH_HPP */
