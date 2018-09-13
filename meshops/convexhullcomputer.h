//
// Created by Computer on 01/08/2018.
//

#ifndef MESHOPS_CONVEXHULLCOMPUTER_H
#define MESHOPS_CONVEXHULLCOMPUTER_H

namespace VHACD
{
	class IVHACD;
}

namespace MeshOps {
class ConvexHullComputer
{
public:
	static MeshMod::MeshPtr generate(MeshMod::MeshPtr const& in);
	static void generateInline(MeshMod::MeshPtr& in);
private:
};

}
#endif //MESHOPS_CONVEXHULLCOMPUTER_H
