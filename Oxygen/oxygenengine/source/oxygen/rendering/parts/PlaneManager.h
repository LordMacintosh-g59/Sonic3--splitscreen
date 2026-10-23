/*
*	Part of the Oxygen Engine / Sonic 3 A.I.R. software distribution.
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <rmxbase.h>

class PatternManager;


class PlaneManager
{
public:
	enum PlaneType
	{
		PLANE_B		= 0,	// Plane B, background
		PLANE_A		= 1,	// Plane A, foreground
		PLANE_W		= 2,	// Plane W, window (optionally split out of plane A)
		PLANE_DEBUG	= 3,	// Additional plane containing all patterns for debug output

		// Co-op extension: extra planes for player 2's independent camera view.
		// These are NOT backed by VRAM; they are filled each frame via fillCoopPlane().
		PLANE_COOP_B = 4,	// Co-op plane B (background layer for P2 camera)
		PLANE_COOP_A = 5,	// Co-op plane A (foreground layer for P2 camera)

		PLANE_COUNT  = 6	// Total number of plane types (including co-op planes)
	};

	struct CustomPlane
	{
		Recti mRect;
		uint8 mSourcePlane = 0;
		uint8 mScrollOffsets = 0;
		uint16 mRenderQueue = 0;
	};

	struct PlaneRect
	{
		PlaneType mPlane = PLANE_B;
		Recti mRect;
	};

public:
	PlaneManager(PatternManager& patternManager);

	void reset();
	void refresh();

	void resetCustomPlanes();
	bool isPlaneUsed(int index) const;

	inline uint16 getNameTableBaseB() const  { return mNameTableBaseB; }
	inline uint16 getNameTableBaseA() const  { return mNameTableBaseA; }
	inline uint16 getNameTableBaseW() const  { return mNameTableBaseW; }
	inline void setNameTableBaseB(uint16 vramAddress)  { mNameTableBaseB = vramAddress; }
	inline void setNameTableBaseA(uint16 vramAddress)  { mNameTableBaseA = vramAddress; }
	inline void setNameTableBaseW(uint16 vramAddress)  { mNameTableBaseW = vramAddress; }

	Vec2i getPlayfieldSizeInPatterns() const;
	Vec2i getPlayfieldSizeInPixels() const;
	Vec4i getPlayfieldSizeForShaders() const;
	void setPlayfieldSizeInPatterns(const Vec2i& size);
	void setPlayfieldSizeInPixels(const Vec2i& size);

	const uint16* getPlanePatternsBuffer(uint8 index) const;

	// Co-op: fill the P2 camera plane buffer from level RAM, bypassing VRAM.
	// Call once per frame (before rendering) with P2's camera world position.
	// planeType must be PLANE_COOP_B (background) or PLANE_COOP_A (foreground).
	void fillCoopPlane(PlaneType planeType, int cameraX, int cameraY);

	uint16 getPlaneBaseVRAMAddress(int planeIndex) const;
	const uint16* getPlaneDataInVRAM(int planeIndex) const;
	size_t getPlaneSizeInVRAM(int planeIndex) const;

	uint16 getPatternVRAMAddress(int planeIndex, uint16 patternIndex) const;
	uint16 getPatternAtIndex(int planeIndex, uint16 patternIndex) const;
	void setPatternAtIndex(int planeIndex, uint16 patternIndex, uint16 value);

	inline bool isPlaneWBelowSplitY() const  { return mIsPlaneWBelowSplitY; }
	inline uint16 getPlaneAWSplitY() const	 { return mPlaneAWSplitY; }
	void setWindowPlaneSplitX(bool rightSideWindow, uint16 splitX);
	void setWindowPlaneSplitY(bool bottomWindow, uint16 splitY);
	void setRenderPlaneABehindW(bool renderPlaneABehindW);
	void getPlaneRects(std::vector<PlaneRect>& output, const Recti& fullscreenRect) const;

	void dumpAsPaletteBitmap(PaletteBitmap& output, int planeIndex, bool highlightPrioPatterns = false) const;

	inline bool isDefaultPlaneEnabled(uint8 index) const  { return !mDisabledDefaultPlane[index]; }
	void setDefaultPlaneEnabled(uint8 index, bool enabled);

	const std::vector<CustomPlane>& getCustomPlanes() const  { return mCustomPlanes; }
	void setupCustomPlane(const Recti& rect, uint8 sourcePlane, uint8 scrollOffsets, uint16 renderQueue);

	void serializeSaveState(VectorBinarySerializer& serializer, uint8 formatVersion);

private:
	const uint16* getPlaneContent(int planeIndex, uint16 patternIndex = 0) const;

private:
	PatternManager& mPatternManager;

	uint16 mNameTableBaseA = 0xc000;
	uint16 mNameTableBaseB = 0xe000;
	uint16 mNameTableBaseW = 0x8000;

	Vec2i mPlayfieldSize;		// In patterns (8x8 pixels)
	uint16 mPlanePatternsBuffer[PLANE_COUNT][0x1000] = { 0 };	// Indices 0-3: filled from VRAM each frame. Indices 4-5: co-op planes, filled by fillCoopPlane().

	bool mIsPlaneWRightOfSplitX = false;	// If true, plane W is right of plane A, otherwise it's left of plane A
	bool mIsPlaneWBelowSplitY = false;		// If true, plane W is below plane A, otherwise it's above plane A
	uint16 mPlaneAWSplitX = 0;
	uint16 mPlaneAWSplitY = 0;
	bool mRenderPlaneABehindW = false;		// If true, render plane A fullscreen, even where the screen intersects with plane W

	bool mDisabledDefaultPlane[4];	// Only for the four standard planes (0-3)
	std::vector<CustomPlane> mCustomPlanes;
};
