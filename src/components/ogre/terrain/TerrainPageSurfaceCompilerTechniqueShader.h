//
// C++ Interface: TerrainPageSurfaceCompilerTechniqueShader
//
// Description: 
//
//
// Author: Erik Hjortsberg <erik.hjortsberg@iteam.se>, (C) 2007
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.//
//
#ifndef EMBEROGRE_TERRAINTERRAINPAGESURFACECOMPILERTECHNIQUESHADER_H
#define EMBEROGRE_TERRAINTERRAINPAGESURFACECOMPILERTECHNIQUESHADER_H

#include "../EmberOgrePrerequisites.h"
#include "TerrainPageSurfaceCompiler.h"
#include "TerrainPage.h"


namespace EmberOgre {

namespace Terrain {

class TerrainPageSurfaceLayer;
class TerrainPageShadow;
class TerrainPageSurfaceCompilerShaderPass;

typedef std::vector<TerrainPageSurfaceLayer*> LayerStore;

class TerrainPageSurfaceCompilerShaderPassCoverageBatch
{
public:
	TerrainPageSurfaceCompilerShaderPassCoverageBatch(TerrainPageSurfaceCompilerShaderPass& shaderPass, Ogre::TexturePtr combinedCoverageTexture);
	virtual ~TerrainPageSurfaceCompilerShaderPassCoverageBatch();
	
	void addLayer(TerrainPageSurfaceLayer* layer);
	
	std::vector<TerrainPageSurfaceLayer*>& getLayers();
	Ogre::TexturePtr getCombinedCoverageTexture();
	
	virtual void finalize();
	
protected:
	TerrainPageSurfaceCompilerShaderPass& mShaderPass;
	
	Ogre::TexturePtr mCombinedCoverageTexture;
	Ogre::Image* mCombinedCoverageImage;
	Ogre::MemoryDataStream* mCombinedCoverageDataStream;
	Ogre::DataStreamPtr mCombinedCoverageDataStreamPtr;
	LayerStore mLayers;

	void assignCombinedCoverageTexture();
	void addCoverage(Ogre::Image* coverage, unsigned int channel, unsigned short numberOfChannels);

};


class TerrainPageSurfaceCompilerShaderPass
{
public:
friend class TerrainPageSurfaceCompilerShaderPassCoverageBatch;
	TerrainPageSurfaceCompilerShaderPass(Ogre::Pass* pass, TerrainPage& page);
	virtual ~TerrainPageSurfaceCompilerShaderPass();
	
	virtual void addLayer(TerrainPageSurfaceLayer* layer);
	virtual void setBaseLayer(TerrainPageSurfaceLayer* layer);
	void addShadowLayer(TerrainPageShadow* terrainPageShadow);
	
	virtual bool hasRoomForLayer(TerrainPageSurfaceLayer* layer);
	
	
	/**
	 *    Creates the combined final coverage textures and sets the shader params. Be sure to call this before you load the material.
	 */
	virtual bool finalize();
	
	LayerStore& getLayers();
	
	Ogre::Pass* getPass();
	
protected:
	typedef std::vector<TerrainPageSurfaceCompilerShaderPassCoverageBatch*> CoverageBatchStore;
	
	Ogre::Pass* mPass;
// 	unsigned int mCurrentLayerIndex;
	
	void assignCombinedCoverageTexture();
	TerrainPageSurfaceCompilerShaderPassCoverageBatch* getCurrentBatch();
	virtual TerrainPageSurfaceCompilerShaderPassCoverageBatch* createNewBatch();

	unsigned int getCoveragePixelWidth() const;
	float mScales[16];
	Ogre::TexturePtr getCombinedCoverageTexture(size_t passIndex, size_t batchIndex);
	CoverageBatchStore mCoverageBatches;
	LayerStore mLayers;
	TerrainPageSurfaceLayer* mBaseLayer;
	TerrainPage& mPage;
};

/**
	@author Erik Hjortsberg <erik.hjortsberg@iteam.se>
*/
class TerrainPageSurfaceCompilerTechniqueShader : public TerrainPageSurfaceCompilerTechnique
{
public:
    TerrainPageSurfaceCompilerTechniqueShader();
    virtual ~TerrainPageSurfaceCompilerTechniqueShader();
    
	virtual bool compileMaterial(Ogre::MaterialPtr material, std::map<int, TerrainPageSurfaceLayer*>& terrainPageSurfaces, TerrainPageShadow* terrainPageShadow);
    
    virtual void setPage(TerrainPage* page);

protected:
	typedef std::vector<TerrainPageSurfaceCompilerShaderPass*> PassStore;
	
	TerrainPage* mPage;
	virtual TerrainPageSurfaceCompilerShaderPass* addPass(Ogre::Technique* technique);
	PassStore mPasses;
	
	void addBaseLayer(Ogre::Pass* pass, TerrainPageSurfaceLayer* layer);
	void addLayer(Ogre::Pass* pass, TerrainPageSurfaceLayer* layer);
	
	void reset();

};


class TerrainPageSurfaceCompilerShaderNormalMappedPassCoverageBatch : public TerrainPageSurfaceCompilerShaderPassCoverageBatch
{
public:
 	TerrainPageSurfaceCompilerShaderNormalMappedPassCoverageBatch(TerrainPageSurfaceCompilerShaderPass& shaderPass, Ogre::TexturePtr combinedCoverageTexture);
	virtual ~TerrainPageSurfaceCompilerShaderNormalMappedPassCoverageBatch() {}
	
	virtual void finalize();
	
protected:

};

class TerrainPageSurfaceCompilerShaderNormalMappedPass : public TerrainPageSurfaceCompilerShaderPass
{
public:
 	TerrainPageSurfaceCompilerShaderNormalMappedPass(Ogre::Pass* pass, TerrainPage& page);
	virtual ~TerrainPageSurfaceCompilerShaderNormalMappedPass() {}
	
// 	virtual void addLayer(TerrainPageSurfaceLayer* layer);
// 	virtual void setBaseLayer(TerrainPageSurfaceLayer* layer);
	
	virtual bool hasRoomForLayer(TerrainPageSurfaceLayer* layer);
	
	
	/**
	 *    Creates the combined final coverage textures and sets the shader params. Be sure to call this before you load the material.
	 */
	virtual bool finalize();
	
	
protected:
	virtual TerrainPageSurfaceCompilerShaderPassCoverageBatch* createNewBatch();

};

class TerrainPageSurfaceCompilerTechniqueShaderNormalMapped : public TerrainPageSurfaceCompilerTechniqueShader
{
public:
protected:
	virtual TerrainPageSurfaceCompilerShaderPass* addPass(Ogre::Technique* technique);

};

}

}

#endif