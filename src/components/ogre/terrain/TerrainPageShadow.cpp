//
// C++ Implementation: TerrainPageShadow
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TerrainPageShadow.h"
#include "TerrainPage.h"
#include "TerrainGenerator.h"
#include "../EmberOgrePrerequisites.h"
#include "services/EmberServices.h"
#include "services/config/ConfigService.h"
#include <wfmath/stream.h>

namespace EmberOgre {
namespace Terrain {


void SimpleTerrainPageShadowTechnique::createShadowData(TerrainPage& page, TerrainGenerator* generator, unsigned char* data, const Ogre::Vector3& lightDirection, const Ogre::ColourValue& lightColour)
{

	int pageSizeInVertices = page.getPageSize();
	int pageSizeInMeters = pageSizeInVertices - 1;
	
	///since Ogre uses a different coord system than WF, we have to do some conversions here
	TerrainPosition origPosition(page.getWFPosition());
	///start in one of the corners...
	origPosition[0] = (page.getWFPosition()[0] * (pageSizeInMeters));
	origPosition[1] = (page.getWFPosition()[1] * (pageSizeInMeters));
	
	WFMath::Vector<3> wfLightDirection = Ogre2Atlas_Vector3(lightDirection);
	wfLightDirection = wfLightDirection.normalize(1);

	 
	TerrainPosition position(origPosition);
	
	for (int i = 0; i < pageSizeInMeters; ++i) {
		position = origPosition;
		position[1] = position[1] - i;
		for (int j = 0; j < pageSizeInMeters; ++j) {
			float height;
			WFMath::Vector<3> normal;
			if (generator->getTerrain().getHeightAndNormal(position.x(), position.y(), height, normal)) {
				float dotProduct = WFMath::Dot(normal, wfLightDirection);

//correct
// 				if (dotProduct > 0) {
// 					*data = (1 - dotProduct) * 255 * bias;
// 				} else {
// 					*data = 255;
// 				}
				
				/// if the dotProduct is > 0, the face is looking away from the sun
				*data = (1.0f - ((dotProduct + 1.0f) * 0.5f)) * 255;


			} else {
				*data = 0;
			}
			data++;
			position[0] = position[0] + 1;
		}
	}

}



TerrainPageShadow::TerrainPageShadow(TerrainPage& terrainPage)
: mTerrainPage(terrainPage)
{
	///we need an unique name for our alpha texture
	std::stringstream shadowTextureNameSS;
	shadowTextureNameSS << mTerrainPage.getMaterial()->getName() << "_shadow";
	const Ogre::String shadowTextureName(shadowTextureNameSS.str());
	
	mTexture = Ogre::Root::getSingletonPtr()->getTextureManager()->createManual(shadowTextureName, "General", Ogre::TEX_TYPE_2D, mTerrainPage.getAlphaTextureSize(), mTerrainPage.getAlphaTextureSize(), 1, Ogre::PF_L8);
}


TerrainPageShadow::~TerrainPageShadow()
{
}

void TerrainPageShadow::setLightDirection(const Ogre::Vector3& lightDirection)
{
	mLightDirection = lightDirection;
}

void TerrainPageShadow::createShadowData(unsigned char* data)
{
	if (!mShadowTechnique) {
		S_LOG_WARNING("Trying to create shadow data without have a technique set.");
	} else {
		mShadowTechnique->createShadowData(mTerrainPage, mTerrainPage.mGenerator, data, mLightDirection, Ogre::ColourValue(1,1,1));
	}
}

void TerrainPageShadow::updateShadow()
{
	mShadowChunk = new Ogre::MemoryDataStream(mTerrainPage.getAlphaTextureSize() * mTerrainPage.getAlphaTextureSize() * 1, false);
	///Create a new image. This image is temporary (since it will be blitted into the hardware memory) and destruction of it will be taken care of by the enveloping DataStreamPtr.
	Ogre::DataStreamPtr dataStreamPtr(mShadowChunk);
	Ogre::Image image; ///the image to hold the data

	createShadowData(mShadowChunk->getPtr());

	
	Ogre::HardwarePixelBufferSharedPtr hardwareBuffer = mTexture->getBuffer();
	
	image.loadRawData(dataStreamPtr, mTerrainPage.getAlphaTextureSize(), mTerrainPage.getAlphaTextureSize(), Ogre::PF_L8);
	
	///blit the whole image to the hardware buffer
	Ogre::PixelBox sourceBox = image.getPixelBox();
	hardwareBuffer->blitFromMemory(sourceBox);
}
	

void TerrainPageShadow::createImage()
{
	mShadowChunk = new Ogre::MemoryDataStream(mTerrainPage.getAlphaTextureSize() * mTerrainPage.getAlphaTextureSize() * 1, false);
	
	memset( mShadowChunk->getPtr(), '\0', mShadowChunk->size());
	
	
	createShadowData(mShadowChunk->getPtr());
	
	Ogre::DataStreamPtr dataStreamPtr(mShadowChunk);
		
	mImage = new Ogre::Image();
	mImage->loadRawData(dataStreamPtr, mTerrainPage.getAlphaTextureSize(), mTerrainPage.getAlphaTextureSize(), Ogre::PF_L8);

	mTexture->loadImage(*mImage);
}

Ogre::TexturePtr TerrainPageShadow::getTexture()
{
	return mTexture;
}

void TerrainPageShadow::setShadowTechnique(ITerrainPageShadowTechnique* shadowTechnique)
{
	mShadowTechnique = shadowTechnique;
}


}
}