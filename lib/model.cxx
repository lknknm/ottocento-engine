#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "model.h"

#include <algorithm>
#include <map>

std::vector<uint32_t> OttModel::extractBoundaryEdges(std::vector<uint32_t>& indices)
{
    /* 
       Count how many times each edge appears
       In a triangular mesh, an edge is either shared between two triangles 
       or belongs to the boundary of the mesh  (appearing only once).
       
       By counting occurrences of each edge:
       Edges with a count of 2 are shared edges between adjacent triangles (middle edge).
       Edges with a count of 1 are boundary edges.
   */
    std::map<std::pair<uint32_t, uint32_t>, int> edgeCount;
    
    // Store boundary edges
    std::vector<uint32_t> edges; 
 
    auto addEdge = [&edgeCount](uint32_t v1, uint32_t v2) {
        /*
            Ensure consistent edge ordering. Why?
            In a triangle mesh, an edge can appear in different orders depending on how triangles are defined. 
            For example: One triangle might define the edge as (v1, v2).
            An adjacent triangle might define the same edge as (v2, v1).
            If we treat (v1, v2) and (v2, v1) as distinct edges, we would incorrectly count them separately. 
            And remember that we needed the counting to detect the shared edge (between adjacent triangles)
        */
        auto edge = std::minmax(v1, v2); 
        edgeCount[edge]++;
    };
 
    // Process triangles and count edges
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        addEdge(indices[i], indices[i + 1]);
        addEdge(indices[i + 1], indices[i + 2]);
        addEdge(indices[i + 2], indices[i]);
    }
 
    // Collect edges that appear only once
    for (const auto& [edge, count] : edgeCount)
    {
        if (count == 1) {
            edges.push_back(edge.first);
            edges.push_back(edge.second);
        }
    }
    return edges;
}
