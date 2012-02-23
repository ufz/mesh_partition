#ifndef Elem_INC
#define Elem_INC

#include<vector>

#include"vec.h"
#include"SymMatrix.h"
#include "Grain.h"

//------------------------------------------------------ 
//   Topology declartion of geometrical element.
//   WW. 06.2005
//   WW. 02.2012
//------------------------------------------------------ 

namespace Mesh_Group
{
class Node;
class Edge;
//3.  Element declaration
class Elem:public Grain
{
   public:
      Elem(const int Index);
      // Faces: Face, local face index
      Elem( const int Index, Elem* onwer, const int Face);

      ~Elem();

      // Operator
      // virtual void operator = (const Elem& elem);

      // Access to members
      int getNodesNumber() const {return nnodes;}
      int getNodesNumberHQ() const {return nnodesHQ;}
      int getEdgesNumber() const{return nedges;}
      int getFacesNumber() const {return nfaces;}
      int getElementType() const {return ele_Type;}
      int getPatchIndex() const {return PatchIndex;}
      int Dim() const {return ele_dim;}
      std::string getName() const;
      void setJacobian(const Math_Group::SymMatrix& Jac) {Jacobian = Jac;}
      void setVolume(const double Vol) {Volume = Vol;}
      double getVolume() const {return Volume;}

   	  // Nodes
      void getNodeIndeces(Math_Group::vec<long>&  node_index) const 
	            {for (int i=0; i< (int) nodes_index.Size();i++)
                              node_index[i]= nodes_index[i]; } 
      long getNodeIndex(const int loc_lndex) const {  return nodes_index[loc_lndex];}
      void setNodes(Math_Group::vec<Node*>&  ele_nodes, const bool ReSize=false);
      void getNodes(Math_Group::vec<Node*>&  ele_nodes) 
         { for (int i=0; i< (int) nodes.Size();i++) ele_nodes[i]= nodes[i]; }
      void MarkingNodes(bool merker);
	  //
      void SetLocalNodeIndex(const int li, const long n_lindex); 
      //
	  long GetLocalNodeIndex(const int li) const; 
         		   
     // Edges
      void getEdges(Math_Group::vec<Edge*>&  ele_edges) 
        {for (int i=0; i<nedges; i++) ele_edges[i]= edges[i];} 
      void setEdges(Math_Group::vec<Edge*>&  ele_edges) 
        {for (int i=0; i<nedges; i++) edges[i]= ele_edges[i];} 
						
	  void setEdges_Orientation(Math_Group::vec<int>&  ori_edg) 
         {for (int i=0; i<nedges; i++) edges_orientation[i]= ori_edg[i];} 
      // Neighbors
      void setNeighbors(Math_Group::vec<Elem*>&  ele_neighbors) 
         { for (int i=0; i< nfaces;i++) neighbors[i] = ele_neighbors[i];}
      void setNeighbor(const int LocalIndex, Elem* ele_neighbor) 
         { neighbors[LocalIndex] = ele_neighbor;}
      void getNeighbors(Math_Group::vec<Elem*>&  ele_neighbors)  
         {for (int i=0; i< nfaces;i++) ele_neighbors[i]= neighbors[i];} 
      //Domain partition
      long GetDomNodeIndex(const int loc_index) { return locnodes_index[loc_index];}
	  void setDomNodeIndex(const int loc_index, const long dom_nindex) { locnodes_index[loc_index]=dom_nindex;}
      void AllocateLocalIndexVector() {locnodes_index.resize(nodes_index.Size());}
      void setDomainIndex(const int dom) {sub_dom = dom;} 
      int GetDomainIndex() const { return sub_dom;} 
      // Local indicis
      void GetLocalIndices_EdgeNodes(const int Edge, int *EdgeNodes);
      int GetElementFaceNodes(const int Face, int *FacesNode);

 
      int GetFaceType();
      
      // Output
      void Read(std::istream& is = std::cin, int fileType=1);
      void WriteIndex(std::ostream& os = std::cout) const;
      void Write_index(std::ostream& os = std::cout) const;
      void WriteAll(std::ostream& os = std::cout) const;
      void WriteNeighbors(std::ostream& os = std::cout) const;
   private:
      int nnodes;
      int nnodesHQ;
      int ele_dim;         // Dimension of element
      int nfaces;
      int nedges;
      int sub_dom;
	  //
      double Volume;
      Math_Group::SymMatrix Jacobian;     
      int PatchIndex;
	  // Element type
      // 1 Line, 2 Quad, 3 Hex, 4 Tri, 5 Tet, 6 Pris 
      int ele_Type;
      Elem* Owner;
      Math_Group::vec<long>   nodes_index;
      Math_Group::vec<long>   locnodes_index;
      Math_Group::vec<Node*>  nodes;
      Math_Group::vec<Edge*>  edges;
      Math_Group::vec<int>  edges_orientation;
      Math_Group::vec<Elem*>  neighbors;
      //vec<Elem*>  sons;

      // Private methods
      int GetElementFaces1D(int *FaceNode);
      int GetElementFacesTri(const int Face, int *FaceNode);
      int GetElementFacesQuad(const int Face, int *FaceNode);
      int GetElementFacesHex(const int Face, int *FaceNode);
      int GetElementFacesTet(const int Face, int *FaceNode);
      int GetElementFacesPri(const int Face, int *FaceNode);

};

} //end namespace

#endif