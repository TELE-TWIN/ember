//
// C++ Implementation: TerrainMod
//
// Description:
//
//
// Author: Tamas Bates <rhymre@gmail.com>, (C) 2008
// Author: Erik Hjortsberg <erik.hjortsberg@iteam.se>, (C) 2008
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

#include "TerrainMod.h"

#include "../EmberEntity.h"
#include "TerrainGenerator.h"
#include "../EmberOgre.h"
#include <Mercator/TerrainMod.h>
#include "TerrainMod_impl.h"

namespace EmberOgre {
namespace Terrain {

InnerTerrainMod::InnerTerrainMod(TerrainMod& terrainMod, const std::string& typemod)
: mTerrainMod(terrainMod)
{
}

InnerTerrainMod::~InnerTerrainMod()
{
}

const std::string& InnerTerrainMod::getTypename() const
{
	return mTypeName;
}

InnerTerrainModCrater::InnerTerrainModCrater(TerrainMod& terrainMod)
: InnerTerrainMod(terrainMod, "cratermod")
, mModifier(0)
{
}

InnerTerrainModCrater::~InnerTerrainModCrater()
{
	delete mModifier;
}

Mercator::TerrainMod* InnerTerrainModCrater::getModifier()
{
	return mModifier;
}


bool InnerTerrainModCrater::parseAtlasData(const Atlas::Message::MapType& modElement)
{

	const Atlas::Message::MapType* shapeMap(0);
	const std::string& shapeType = parseShape(modElement, &shapeMap);
	if (shapeMap) {
		if (shapeType == "ball") {
			WFMath::Point<3> pos = mTerrainMod.getEntity()->getPosition();
			///HACK: This height adjustment shouldn't be necessary
			pos.z() = EmberOgre::getSingleton().getTerrainGenerator()->getHeight(TerrainPosition(pos.x(), pos.y()));
			WFMath::Ball<3>* shape(0);
			if (InnerTerrainMod_impl::parseShapeAtlasData<WFMath::Ball<3> >(*shapeMap, pos, &shape)) {
				mModifier = new Mercator::CraterTerrainMod(*shape);
				delete shape;
				return true;
			}
			delete shape;
		}
	}
	S_LOG_FAILURE("Crater terrain mod defined with incorrect shape");
	return false;
}


InnerTerrainModSlope::InnerTerrainModSlope(TerrainMod& terrainMod)
: InnerTerrainMod(terrainMod, "slopemod")
, mModifier_impl(0)
{
}

InnerTerrainModSlope::~InnerTerrainModSlope()
{
	delete mModifier_impl;
}

Mercator::TerrainMod* InnerTerrainModSlope::getModifier()
{
	return mModifier_impl->getModifier();
}


bool InnerTerrainModSlope::parseAtlasData(const Atlas::Message::MapType& modElement)
{
	WFMath::Point<3> pos = mTerrainMod.getEntity()->getPosition();
	float dx, dy, level;
	// Get slopes
	Atlas::Message::MapType::const_iterator mod_I = modElement.find("slopes");
	if (mod_I != modElement.end()) {
		const Atlas::Message::Element& modSlopeElem = mod_I->second;
		if (modSlopeElem.isList()) {
			const Atlas::Message::ListType & slopes = modSlopeElem.asList();
			if (slopes.size() > 1) {
				if (slopes[0].isNum() && slopes[1].isNum()) {
					dx = static_cast<int>(slopes[0].asNum());
					dy = static_cast<int>(slopes[1].asNum());
					// Get level
					mod_I = modElement.find("height");
					if (mod_I != modElement.end()) {
						const Atlas::Message::Element& modHeightElem = mod_I->second;
						if (modHeightElem.isNum()) {
							level = modHeightElem.asNum();
							pos.z() = level;        // Note that the height of the mod is in pos.z()
							const Atlas::Message::MapType* shapeMap(0);
							const std::string& shapeType = parseShape(modElement, &shapeMap);
							if (shapeMap) {
								if (shapeType == "ball") {
									InnerTerrainModSlope_impl<WFMath::Ball<2> >* modifierImpl = new InnerTerrainModSlope_impl<WFMath::Ball<2> >();
									mModifier_impl = modifierImpl;
									return modifierImpl->createInstance(*shapeMap, pos, level, dx, dy);
								}
							}
						}
					}
				}
			}
		}
	}
	S_LOG_FAILURE("SlopeTerrainMod defined with incorrect shape");
	return false;
}


InnerTerrainModLevel::InnerTerrainModLevel(TerrainMod& terrainMod)
: InnerTerrainMod(terrainMod, "levelmod")
, mModifier_impl(0)
{
}

InnerTerrainModLevel::~InnerTerrainModLevel()
{
	delete mModifier_impl;
}

Mercator::TerrainMod* InnerTerrainModLevel::getModifier()
{
	return mModifier_impl->getModifier();
}


bool InnerTerrainModLevel::parseAtlasData(const Atlas::Message::MapType& modElement)
{

	WFMath::Point<3> pos = mTerrainMod.getEntity()->getPosition();
	// Get level
	Atlas::Message::MapType::const_iterator mod_I = modElement.find("height");
	if (mod_I != modElement.end()) {
		const Atlas::Message::Element& modHeightElem = mod_I->second;
		if (modHeightElem.isNum()) {
			float level = modHeightElem.asNum();
			pos.z() = level;        // Note that the height of the mod is in pos.z()
			const Atlas::Message::MapType* shapeMap(0);
			const std::string& shapeType = parseShape(modElement, &shapeMap);
			if (shapeMap) {
				if (shapeType == "ball") {
					InnerTerrainModLevel_impl<WFMath::Ball<2> >* modifierImpl = new InnerTerrainModLevel_impl<WFMath::Ball<2> >();
					mModifier_impl = modifierImpl;
					return modifierImpl->createInstance(*shapeMap, pos, level);
				}
			}
		}
	}
	S_LOG_FAILURE("Level terrain mod defined with incorrect shape");
	return false;
}

const std::string& InnerTerrainMod::parseShape(const Atlas::Message::MapType& modElement, const Atlas::Message::MapType** shapeMap)
{
	Atlas::Message::MapType::const_iterator shape_I = modElement.find("shape");
	if (shape_I != modElement.end()) {
		const Atlas::Message::Element& shapeElement = shape_I->second;
		if (shapeElement.isMap()) {
			const Atlas::Message::MapType& localShapeMap = shapeElement.asMap();
			*shapeMap = &localShapeMap;
			
			// Get shape's type
			Atlas::Message::MapType::const_iterator type_I = localShapeMap.find("type");
			if (type_I != localShapeMap.end()) {
				const Atlas::Message::Element& shapeTypeElem(type_I->second);
				if (shapeTypeElem.isString()) {
					const std::string& shapeType = shapeTypeElem.asString();
					return shapeType;
				}
			}
		}
	}
	static std::string empty("");
	return empty;
}




TerrainMod::TerrainMod(EmberEntity* entity)
: mEntity(entity)
, mInnerMod(0)
{
}


TerrainMod::~TerrainMod()
{
}

bool TerrainMod::init()
{
	bool successfulParsing = parseMod();
	if (successfulParsing) {
		observeEntity();
	}
	return successfulParsing;
}

bool TerrainMod::parseMod()
{
	if (!mEntity->hasAttr("terrainmod")) {
		S_LOG_FAILURE("TerrainMod defined on entity with no terrainmod attribute");
		return false;
	}

	const Atlas::Message::Element& modifier(mEntity->valueOfAttr("terrainmod"));

	if (!modifier.isMap()) {
		S_LOG_FAILURE("Terrain modifier is not a map");
		return false;
	}
	const Atlas::Message::MapType & modMap = modifier.asMap();

	
	// Get modifier type
	Atlas::Message::MapType::const_iterator mod_I = modMap.find("type");
	if (mod_I != modMap.end()) {
		const Atlas::Message::Element& modTypeElem(mod_I->second);
		if (modTypeElem.isString()) {
			const std::string& modType = modTypeElem.asString();
			// Get modifier position
			
			// Build modifier from data
			if (modType == "slopemod") {
				mInnerMod = new InnerTerrainModSlope(*this);
			} else if (modType == "levelmod") {
				mInnerMod = new InnerTerrainModLevel(*this);
/*				float level;
				// Get level
				mod_I = modMap.find("height");
				if (mod_I != modMap.end()) {
					const Atlas::Message::Element& modHeightElem = mod_I->second;
					level = modHeightElem.asNum();
				}
			
				pos.z() = level;        // Note that the level the terrain will be raised to is in pos.z()
				if ((mModifier = newLevelMod(shapeMap, pos)) != NULL) {
					return true;
				} else {
					return false;
				}*/
			
			} else if (modType == "adjustmod") {
/*				float level;
				// Get level
				mod_I = modMap.find("height");
				if (mod_I != modMap.end()) {
					const Atlas::Message::Element& modHeightElem = mod_I->second;
					level = modHeightElem.asNum();
				}
			
				pos.z() = level;        // Note that the level used in the adjustment is in pos.z()
				if ((mModifier = newAdjustMod(shapeMap, pos)) != NULL) {
					return true;
				} else {
					return false;
				}*/
			
			} else  if (modType == "cratermod") {
				mInnerMod = new InnerTerrainModCrater(*this);
			}
		}
	}
	if (mInnerMod) {
		if (mInnerMod->parseAtlasData(modMap)) {
			return true;
		} else {
			return false;
		}
	}


	return false;
}

void TerrainMod::attributeChanged(const Atlas::Message::Element& attributeValue)
{
	delete mInnerMod;
	mInnerMod = 0;
	if (parseMod()) {
		EventModChanged(this);
	}
}

void TerrainMod::entity_Moved()
{
	delete mInnerMod;
	mInnerMod = 0;
	if (parseMod()) {
		EventModChanged(this);
	}
}

void TerrainMod::entity_Deleted()
{
	EventModDeleted(this);
	delete mInnerMod;
}

void TerrainMod::observeEntity()
{
	mAttrChangedSlot.disconnect();
	if (mEntity) {
		mAttrChangedSlot = sigc::mem_fun(*this, &TerrainMod::attributeChanged);
		mEntity->observe("terrainmod", mAttrChangedSlot);
		mEntity->Moved.connect(sigc::mem_fun(*this, &TerrainMod::entity_Moved));
		mEntity->BeingDeleted.connect(sigc::mem_fun(*this, &TerrainMod::entity_Deleted));
	}
}

Mercator::TerrainMod* TerrainMod::newCraterMod(const Atlas::Message::MapType shapeMap, WFMath::Point<3> pos)
{
	std::string shapeType;

	// Get modifier's shape
	Atlas::Message::MapType::const_iterator shape_I;
	// Get shape's type
	shape_I = shapeMap.find("type");
	if (shape_I != shapeMap.end()) {
		const Atlas::Message::Element& shapeTypeElem(shape_I->second);
		if (shapeTypeElem.isString()) {
			shapeType = shapeTypeElem.asString();
		}
	}
	// end shape data

	if (shapeType == "ball") {
		float shapeRadius;
		// Get sphere's radius
		Atlas::Message::MapType::const_iterator shape_I = shapeMap.find("radius");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapeRadiusElem(shape_I->second);
			shapeRadius = shapeRadiusElem.asNum();
		}

		// Make sphere
        ///HACK: This height adjustment shouldn't be necessary
//         pos.z() = EmberOgre::getSingleton().getTerrainGenerator()->getTerrain().get(pos.x(), pos.y());
//         pos.z() += shapeRadius;

		WFMath::Ball<3> modShape = WFMath::Ball<3>(pos, shapeRadius); ///FIXME: assumes 3d ball...

		// Make modifier
		Mercator::CraterTerrainMod *NewMod;
		NewMod = new Mercator::CraterTerrainMod(modShape);

		return NewMod;
	}

	S_LOG_FAILURE("CraterTerrainMod defined with incorrect shape");
	return NULL;
}

Mercator::TerrainMod* TerrainMod::newLevelMod(const Atlas::Message::MapType shapeMap, WFMath::Point<3> pos)
{
	std::string shapeType;
	Atlas::Message::MapType::const_iterator shape_I;

	// Get shape's type
	shape_I = shapeMap.find("type");
	if (shape_I != shapeMap.end()) {
		const Atlas::Message::Element & shapeTypeElem(shape_I->second);
		if (shapeTypeElem.isString()) {
			shapeType = shapeTypeElem.asString();
		}
	}

	if (shapeType == "ball") {
		float shapeRadius;
		// Get sphere's radius
		shape_I = shapeMap.find("radius");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapeRadiusElem(shape_I->second);
			shapeRadius = shapeRadiusElem.asNum();
		}

		// Make disc
		WFMath::Point<2> pos_2d(pos.x(), pos.y());
		WFMath::Ball<2> modShape = WFMath::Ball<2>(pos_2d, shapeRadius); ///FIXME: assumes 2d ball...

		// Make Modifier
		Mercator::LevelTerrainMod<WFMath::Ball<2> > *NewMod;
		NewMod = new Mercator::LevelTerrainMod<WFMath::Ball<2> >(pos.z(), modShape);

		return NewMod;
	}

	if (shapeType == "rotbox") {
		WFMath::Point<2> shapePoint;
		WFMath::Vector<2> shapeVector;
		// Get rotbox's position
		shape_I = shapeMap.find("point");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapePointElem(shape_I->second);
			if (shapePointElem.isList()) {
				const Atlas::Message::ListType & pointList = shapePointElem.asList();
				shapePoint = WFMath::Point<2>((int)pointList[0].asNum(), (int)pointList[1].asNum());
			}
		}
		// Get rotbox's vector
		shape_I = shapeMap.find("vector");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapeVectorElem(shape_I->second);
			if (shapeVectorElem.isList()) {
				const Atlas::Message::ListType & vectorList = shapeVectorElem.asList();
				shapeVector = WFMath::Vector<2>((int)vectorList[0].asNum(), (int)vectorList[1].asNum());
			}
		}

		// Make rotbox
		///FIXME: needs to use shapeDim instead of 2
		WFMath::RotBox<2> modShape = WFMath::RotBox<2>(shapePoint, shapeVector, WFMath::RotMatrix<2>());

		// Make modifier
		Mercator::LevelTerrainMod<WFMath::RotBox<2> > *NewMod;
		NewMod = new Mercator::LevelTerrainMod<WFMath::RotBox<2> >(pos.z(), modShape);

		return NewMod;
	}

	S_LOG_FAILURE("LevelTerrainMod defined with incorrect shape data");
	return NULL;
}

Mercator::TerrainMod * TerrainMod::newSlopeMod(const Atlas::Message::MapType shapeMap,
		WFMath::Point<3> pos, float dx, float dy)
{
	std::string shapeType;

	// Get modifier's shape
	Atlas::Message::MapType::const_iterator shape_I;
	// Get shape's type
	shape_I = shapeMap.find("type");
	if (shape_I != shapeMap.end()) {
		const Atlas::Message::Element& shapeTypeElem(shape_I->second);
		if (shapeTypeElem.isString()) {
			shapeType = shapeTypeElem.asString();
		}
	}
	// end shape data

	if (shapeType == "ball") {
		float shapeRadius;
		// Get sphere's radius
		Atlas::Message::MapType::const_iterator shape_I = shapeMap.find("radius");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapeRadiusElem(shape_I->second);
			shapeRadius = shapeRadiusElem.asNum();
		}

		// Make disc
		WFMath::Point<2> pos_2d(pos.x(), pos.y());
		WFMath::Ball<2> modShape = WFMath::Ball<2>(pos_2d, shapeRadius);

		// log(INFO, "Successfully parsed a slopemod");

		// Make modifier

	}

	S_LOG_FAILURE("SlopeTerrainMod defined with incorrect shape data");
	return NULL;
}

Mercator::TerrainMod * TerrainMod::newAdjustMod(const Atlas::Message::MapType shapeMap, WFMath::Point<3> pos)
{
	std::string shapeType;

	// Get modifier's shape
	Atlas::Message::MapType::const_iterator shape_I;
	// Get shape's type
	shape_I = shapeMap.find("type");
	if (shape_I != shapeMap.end()) {
		const Atlas::Message::Element& shapeTypeElem(shape_I->second);
		if (shapeTypeElem.isString()) {
			shapeType = shapeTypeElem.asString();
		}
	}
	// end shape data

	if (shapeType == "ball") {
		float shapeRadius;
		// Get sphere's radius
		Atlas::Message::MapType::const_iterator shape_I = shapeMap.find("radius");
		if (shape_I != shapeMap.end()) {
			const Atlas::Message::Element& shapeRadiusElem(shape_I->second);
			shapeRadius = shapeRadiusElem.asNum();
		}

		// Make sphere
		WFMath::Point<2> pos_2d(pos.x(), pos.y());
		WFMath::Ball<2> modShape = WFMath::Ball<2>(pos_2d, shapeRadius);

		// Make modifier

		// Apply Modifier

	}

	S_LOG_FAILURE("AdjustTerrainMod defined with incorrect shape data");
	return NULL;
}

EmberEntity* TerrainMod::getEntity() const
{
	return mEntity;
}

} // close Namespace Terrain
} // close Namespace EmberOgre